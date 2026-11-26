/*
*   Copyright (C) {2015}  {VK, Charles TheHouse}
*
*   This program is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program.  If not, see [http://www.gnu.org/licenses/].
*
*   Contact at:
*/

#include <Windows.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <math.h>
#include <io.h>
#include <errno.h>

#include "common/Basedef.h"
#include "common/CPSock.h"
#include "common/ItemEffect.h"
#include "core/Language.h"
#include "entities/CItem.h"
#include "core/Server.h"
#include "net/GetFunc.h"
#include "net/SendFunc.h"
#include "net/ProcessClientMessage.h"
#include "net/ProcessDBMessage.h"
#include "core/CReadFiles.h"
#include "systems/CWarTower.h"

/// @file WorldEventSystem.cpp
/// @brief Eventos globais, guerras de guilda/reino, coliseu, ranking, limpeza de areas e grupo.

/// Registra em log o resumo das zonas de guilda e impostos.
void GuildZoneReport(void)
{
	MSG_GuildZoneReport sm;
	memset(&sm, 0, sizeof(MSG_GuildZoneReport));

	sm.Type = _MSG_GuildZoneReport;
	sm.Size = sizeof(sm);
	sm.ID = ServerIndex;

	for (int i = 0; i < MAX_GUILDZONE; i++)
	{
		sm.Guild[i] = g_pGuildZone[i].ChargeGuild;
	}

	DBServerSocket.SendOneMessage((char*)&sm, sizeof(sm));
}

/// Inicia desafio de aposta em arena entre jogador e NPC/taxa.
int Challange(int conn, int mob, int coin)
{
	if (mob < MAX_USER || mob >= MAX_MOB)
		return 0;

	int ZoneChall = pMob[mob].MOB.BaseScore.Level;
	int Guild = pMob[conn].MOB.Guild;

	if (ZoneChall < 0 || ZoneChall >= ValidGuild)
		return 0;

	if (ZoneChall == 4)
		return 0;


	if (pMob[conn].MOB.GuildLevel != 9)
	{
		SendSay(mob, g_pMessageStringTable[_NN_Only_Guild_Master_can]);

		return 0;
	}

	if (g_pGuildZone[ZoneChall].ChargeGuild == Guild)
	{
		SendSay(mob, g_pMessageStringTable[_NN_Champions_Cant_Challange]);

		return 0;
	}
	if (pMob[conn].extra.Citizen != (ServerIndex + 1))
	{
		SendSay(mob, g_pMessageStringTable[_DN_ANOTHER_TOWNSPEOPLE]);
		return 0;
	}

	int Group = ServerGroup;
	int Server = Guild / MAX_GUILD;
	int usGuild = Guild & MAX_GUILD - 1;

	int GuildName = 1;

	if (Group >= 0 && Group < MAX_SERVERGROUP && Server >= 0 && Server < 16 && usGuild >= 0 && usGuild < MAX_GUILD && g_pGuildName[Group][Server][usGuild][0] == 0)
		GuildName = 0;


	if (GuildName == 0)
	{
		SendSay(mob, g_pMessageStringTable[_NN_Only_Named_Guild]);

		return 0;
	}

	if (GuildInfo[Guild].Fame < 100)
	{
		sprintf(temp, g_pMessageStringTable[_DN_Need_1000000_For_Challange], 100);
		SendSay(mob, temp);

		return 0;
	}

	MSG_GuildInfo sm;
	memset(&sm, 0, sizeof(MSG_GuildInfo));

	sm.Type = _MSG_GuildInfo;
	sm.Size = sizeof(MSG_GuildInfo);
	sm.ID = conn;

	sm.Guild = Guild;

	coin = GuildInfo[Guild].Fame;

	GuildInfo[Guild].Fame -= 100;

	sm.GuildInfo = GuildInfo[Guild];

	DBServerSocket.SendOneMessage((char*)&sm, sizeof(MSG_GuildInfo));

	SendClientMessage(conn, g_pMessageStringTable[_NN_Challanged]);

	sprintf(temp, "etc,challange guild:%d fame:%d zone:%d", Guild, coin, ZoneChall);
	Log(temp, pMob[conn].MOB.MobName, 0);

	if (pChallangerMoney[ZoneChall] < coin)
	{
		g_pGuildZone[ZoneChall].ChallangeGuild = Guild;
		pChallangerMoney[ZoneChall] = coin;

		CReadFiles::WriteChallanger();
		CReadFiles::WriteGuild();
	}

	return 1;
}

/// Conta jogadores presentes na area retangular informada.
int  GetUserInArea(int x1, int y1, int x2, int y2, char *MobName)
{
	int userCount = 0;

	for (int i = 0; i < MAX_USER; i++)
	{
		if (pUser[i].Mode != USER_PLAY)
			continue;

		if (pMob[i].Mode == 0)
			continue;

		if (pMob[i].TargetX >= x1 && pMob[i].TargetX <= x2 && pMob[i].TargetY >= y1 && pMob[i].TargetY <= y2)
		{
			strncpy(MobName, pMob[i].MOB.MobName, NAME_LENGTH);
			userCount++;
		}
	}
	return userCount;
}

/// Finaliza a guerra do Castelo e distribui recompensas/posse.
void FinishCastleWar()
{
	SetCastleDoor(1);

	ClearAreaGuild(1036, 1672, 1144, 1764, g_pGuildZone[4].ChargeGuild);

	CastleState = 0;

	for (int i = 0; i < MAX_USER; i++)
	{
		if (pUser[i].Mode != USER_PLAY)
			continue;

		SendClientSignal(i, _MSG_SendCastleState, CastleState);
	}

	for (int i = 0; i < MAX_USER; i++)
	{
		if (pUser[i].Mode != USER_PLAY)
			continue;

		SendClientSignalParm(i, ESCENE_FIELD, _MSG_SendCastleState, CastleState);
	}

	for (int i = MAX_USER; i < MAX_MOB; i++)
	{
		if (pMob[i].Mode == MOB_EMPTY)
			continue;

		if (pMob[i].MOB.Equip[0].sIndex != 219)
			continue;

		DeleteMob(i, 2);
	}

	ClearArea(1036, 1672, 1144, 1764);
}

/// Aplica dano em area para todos os jogadores na regiao especificada.
void SendDamage(int x1, int y1, int x2, int y2)
{
	for (int i = 0; i < MAX_USER; i++)
	{
		if (pUser[i].Mode != USER_PLAY)
			continue;

		if (pMob[i].MOB.CurrentScore.Hp <= 0)
			continue;

		if (pMob[i].TargetX < x1 || pMob[i].TargetX > x2 || pMob[i].TargetY < y1 || pMob[i].TargetY > y2)
			continue;

		int UNK = 1048576000;

		int dam_base = 2000;

		int hp = pMob[i].MOB.CurrentScore.Hp;

		if(hp < dam_base)
			dam_base = hp - 1;

		int dam = hp - dam_base;


		if (hp < dam_base)
			dam = hp;

		if (hp < 1)
			dam = 1;

		int req_hp = dam - pMob[i].MOB.CurrentScore.Hp;

		pMob[i].MOB.CurrentScore.Hp = dam;

		pUser[i].ReqHp = dam;

		SetReqHp(i);
		SetReqMp(i);

		MSG_SetHpDam sm;
		memset(&sm, 0, sizeof(MSG_SetHpDam));

		sm.Type = _MSG_SetHpDam;
		sm.Size = sizeof(MSG_SetHpDam);
		sm.ID = i;

		sm.Hp = dam;
		sm.Dam = req_hp;

		GridMulticast(pMob[i].TargetX, pMob[i].TargetY, (MSG_STANDARD*)&sm, 0);
	}
}

/// Aplica dano em area apenas a jogadores do reino/cla especificado.
void SendDamageKingdom(int x1, int y1, int x2, int y2, int Clan)
{
	for (int i = 0; i < MAX_USER; i++)
	{
		if (pUser[i].Mode != USER_PLAY)
			continue;

		if (pMob[i].MOB.CurrentScore.Hp <= 0)
			continue;

		if (pMob[i].TargetX < x1 || pMob[i].TargetX > x2 || pMob[i].TargetY < y1 || pMob[i].TargetY > y2)
			continue;

		if(pMob[i].MOB.Clan == Clan)
			continue;

		int UNK = 1048576000;

		int dam_base = pMob[i].MOB.CurrentScore.MaxHp / 10;

		int hp = pMob[i].MOB.CurrentScore.Hp;

		if(hp < dam_base)
			dam_base = hp - 1;

		int dam = hp - dam_base;


		if (hp < dam_base)
			dam = hp;

		if (hp < 1)
			dam = 1;

		int req_hp = dam - pMob[i].MOB.CurrentScore.Hp;

		pMob[i].MOB.CurrentScore.Hp = dam;

		pUser[i].ReqHp = dam;

		SetReqHp(i);
		SetReqMp(i);

		MSG_SetHpDam sm;
		memset(&sm, 0, sizeof(MSG_SetHpDam));

		sm.Type = _MSG_SetHpDam;
		sm.Size = sizeof(MSG_SetHpDam);
		sm.ID = i;

		sm.Hp = dam;
		sm.Dam = req_hp;

		GridMulticast(pMob[i].TargetX, pMob[i].TargetY, (MSG_STANDARD*)&sm, 0);
	}
}

/// Aplica dano em area a lideres/membros no evento de guerra.
void SendDamageLeader(int x1, int y1, int x2, int y2)
{
	int HaveUser = 0;
	
	for(int x = x1; x < x2; x++)
	{
		for(int y = y1; y < y2; y++)
		{
			if(x < 0 || x >= MAX_GRIDX || y < 0 || y >= MAX_GRIDY)
				continue;

			if(pMobGrid[y][x] == 0)
				continue;

			if(pMobGrid[y][x] >= MAX_USER)
				continue;

			int tmob = pMobGrid[y][x];

			if(tmob == Pista[4].Party[0].LeaderID)
			{
				HaveUser++;
				continue;
			}

			if(tmob == Pista[4].Party[1].LeaderID)
			{
				HaveUser++;
				continue;
			}
			if(tmob == Pista[4].Party[1].LeaderID)
			{
				HaveUser++;
				continue;
			}
		}
	}

	if(HaveUser)
		return;
	
	for (int i = 0; i < MAX_USER; i++)
	{
		if (pUser[i].Mode != USER_PLAY)
			continue;

		if (pMob[i].MOB.CurrentScore.Hp <= 0)
			continue;

		if (pMob[i].TargetX < x1 || pMob[i].TargetX > x2 || pMob[i].TargetY < y1 || pMob[i].TargetY > y2)
			continue;

		int UNK = 1048576000;

		int dam_base = 2000;

		int hp = pMob[i].MOB.CurrentScore.Hp;

		if(hp < dam_base)
			dam_base = hp - 1;

		int dam = hp - dam_base;


		if (hp < dam_base)
			dam = hp;

		if (hp < 1)
			dam = 1;

		int req_hp = dam - pMob[i].MOB.CurrentScore.Hp;

		pMob[i].MOB.CurrentScore.Hp = dam;

		pUser[i].ReqHp = dam;

		SetReqHp(i);
		SetReqMp(i);

		MSG_SetHpDam sm;
		memset(&sm, 0, sizeof(MSG_SetHpDam));

		sm.Type = _MSG_SetHpDam;
		sm.Size = sizeof(MSG_SetHpDam);
		sm.ID = i;

		sm.Hp = dam;
		sm.Dam = req_hp;

		GridMulticast(pMob[i].TargetX, pMob[i].TargetY, (MSG_STANDARD*)&sm, 0);
	}
}

/// Teleporta membros de uma guilda para coordenadas alvo (variante 2).
void SummonGuild2(int Guild, int tx, int ty, int maxsummon, int village)
{
	if (Guild <= 0)
		return;

	int summons = 0;

	for (int i = 0; i < MAX_USER; i++)
	{
		if (pUser[i].Mode != USER_PLAY || pMob[i].Mode == USER_EMPTY)
			continue;

		if (pMob[i].MOB.Guild != Guild)
			continue;

		if (pUser[i].OnlyTrade)
			continue;

		if (village < 1 || village > 3 || BASE_GetVillage(pMob[i].TargetX, pMob[i].TargetY) == village)
		{
			DoTeleport(i, tx, ty);

			summons++;

			if (summons == 30)
			{
				tx++;
				ty++;
			}

			if (summons == 45)
			{
				tx = tx - 2;
				ty = ty - 2;
			}

			if (summons >= maxsummon)
				break;
		}
	}
}

/// Teleporta membros de uma guilda para uma vila/coordenada.
void SummonGuild(int Guild, int tx, int ty, int maxsummon, int village)
{
	if (Guild <= 0)
		return;

	int summons = 0;

	for (int i = 0; i < MAX_USER; i++)
	{
		if (pUser[i].Mode != USER_PLAY || pMob[i].Mode == USER_EMPTY)
			continue;

		if (pMob[i].MOB.Guild != Guild)
			continue;

		if (pUser[i].OnlyTrade)
			continue;

		if (village && village != 1 && village != 3 || BASE_GetVillage(pMob[i].TargetX, pMob[i].TargetY) == village)
		{
			if (village != 2 || pMob[i].extra.ClassMaster == MORTAL)
			{
				DoTeleport(i, tx, ty);

				summons++;

				if (summons == 30)
				{
					tx++;
					ty++;
				}

				if (summons == 45)
				{
					tx = tx - 2;
					ty = ty - 2;
				}

				if (summons >= maxsummon)
					break;
			}
			else
				SendClientMessage(i, g_pMessageStringTable[_NN_3rd_village_limit]);
		}
	}
}

/// Limpa jogadores/entidades de uma area de quest.
void ClearAreaQuest(int x1, int y1, int x2, int y2)
{
	for (int i = 0; i < MAX_USER; i++)
	{
		if (pUser[i].Mode != USER_PLAY)
			continue;

		if (pMob[i].Mode == USER_EMPTY)
			continue;

		if (pMob[i].TargetX < x1 || pMob[i].TargetX > x2 || pMob[i].TargetY < y1 || pMob[i].TargetY > y2)
			continue;

		if (pMob[i].MOB.CurrentScore.Hp <= 0)
		{
			pMob[i].MOB.CurrentScore.Hp = 2;
			SendScore(i);
		}

		pMob[i].QuestFlag = 0;
		DoRecall(i);
	}
}

/// Remove jogadores de uma area retangular retornando-os a cidade.
void ClearArea(int x1, int y1, int x2, int y2)
{
	for (int i = 0; i < MAX_USER; i++)
	{
		if (pUser[i].Mode != USER_PLAY)
			continue;

		if (pMob[i].Mode == USER_EMPTY)
			continue;

		if (pMob[i].TargetX < x1 || pMob[i].TargetX > x2 || pMob[i].TargetY < y1 || pMob[i].TargetY > y2)
			continue;

		if (pMob[i].MOB.CurrentScore.Hp <= 0)
		{
			pMob[i].MOB.CurrentScore.Hp = 2;
			SendScore(i);
		}

		DoRecall(i);
	}
}

/// Remove jogadores fora da faixa de nivel permitida em uma area.
void ClearAreaLevel(int x1, int y1, int x2, int y2, int minLv, int maxLv)
{
	for (int i = 0; i < MAX_USER; i++)
	{
		if (pUser[i].Mode != USER_PLAY)
			continue;

		if (pMob[i].Mode == USER_EMPTY)
			continue;

		if (pMob[i].TargetX < x1 || pMob[i].TargetX > x2 || pMob[i].TargetY < y1 || pMob[i].TargetY > y2)
			continue;

		if (pMob[i].MOB.CurrentScore.Level < minLv)
			continue;

		if (pMob[i].MOB.CurrentScore.Level > maxLv)
			continue;

		if (pMob[i].MOB.CurrentScore.Hp <= 0)
		{
			pMob[i].MOB.CurrentScore.Hp = 2;
			SendScore(i);
		}

		DoRecall(i);
	}
}

/// Remove jogadores de uma area exceto membros da guilda especificada.
void ClearAreaGuild(int x1, int y1, int x2, int y2, int Guild)
{
	for (int i = 0; i < MAX_USER; i++)
	{
		if (pUser[i].Mode != USER_PLAY)
			continue;

		if (pMob[i].Mode == USER_EMPTY)
			continue;

		if (pMob[i].TargetX < x1 || pMob[i].TargetX > x2 || pMob[i].TargetY < y1 || pMob[i].TargetY > y2)
			continue;

		if (pMob[i].MOB.Guild == Guild)
			continue;

		if (pMob[i].MOB.CurrentScore.Hp <= 0)
		{
			pMob[i].MOB.CurrentScore.Hp = 2;
			SendScore(i);
		}

		DoRecall(i);
	}
}

/// Teleporta todos os jogadores dentro de uma area para (tx, ty).
void ClearAreaTeleport(int x1, int y1, int x2, int y2, int tx, int ty)
{
	for (int i = 0; i < MAX_USER; i++)
	{
		if (pUser[i].Mode != USER_PLAY)
			continue;

		if (pMob[i].Mode == USER_EMPTY)
			continue;

		if (pMob[i].TargetX < x1 || pMob[i].TargetX > x2 || pMob[i].TargetY < y1 || pMob[i].TargetY > y2)
			continue;

		if (pMob[i].MOB.CurrentScore.Hp <= 0)
		{
			pMob[i].MOB.CurrentScore.Hp = 1;

			SendScore(i);
		}
		/*
		if(rand()%1 == 0)
			tx -= rand()%3;
		else
			tx += rand()%3;

		if(rand()%1 == 0)
			ty -= rand()%3;
		else
			ty += rand()%3;
*/
		DoTeleport(i, tx, ty);
	}
}

/// Limpa zonas de guerra/PK de guilda.
void ClearGuildPKZone()
{
	int x1 = 128;
	int y1 = 128;

	int x2 = 256;
	int y2 = 256;

	for (int i = 0; i < MAX_USER; i++)
	{
		if (pUser[i].Mode != USER_PLAY)
			continue;

		if (pMob[i].Mode == 0)
			continue;

		if (pMob[i].TargetX >= x1 && pMob[i].TargetX <= x2 && pMob[i].TargetY >= y1 && pMob[i].TargetY <= y2)
			DoRecall(i);
	}
}

/// Abre ou fecha os portoes principais do Coliseu.
void SetColoseumDoor(int state)
{
	int gate = 0;

	int height = 0;

	for (int i = 0; i < 2; i++)
	{
		gate = i + 12;

		if (pItem[i + 12].ITEM.sIndex <= 0 || pItem[gate].ITEM.sIndex >= MAX_ITEMLIST)
			continue;

		if (pItem[gate].State == state)
			continue;

		UpdateItem(gate, state, &height);

		if (state == STATE_OPEN)
		{
			MSG_UpdateItem sm;
			memset(&sm, 0, sizeof(MSG_UpdateItem));

			sm.ID = ESCENE_FIELD;
			sm.Type = _MSG_UpdateItem;

			sm.ItemID = gate + 10000;

			sm.Size = sizeof(MSG_UpdateItem);

			//		sm.Height = height;
			sm.State = pItem[gate].State;

			GridMulticast(pItem[gate].PosX, pItem[gate].PosY, (MSG_STANDARD*)&sm, 0);
		}
		else
		{
			MSG_CreateItem sm;
			memset(&sm, 0, sizeof(MSG_CreateItem));

			GetCreateItem(gate, &sm);

			GridMulticast(pItem[gate].PosX, pItem[gate].PosY, (MSG_STANDARD*)&sm, 0);
		}
		pItem[gate].Delay = 0;
	}
}

/// Abre ou fecha os portoes secundarios do Coliseu.
void SetColoseumDoor2(int state)
{
	int gate = 0;

	int height = 0;

	for (int i = 0; i < 5; i++)
	{
		gate = i + 14;

		if (pItem[i + 14].ITEM.sIndex <= 0 || pItem[gate].ITEM.sIndex >= MAX_ITEMLIST)
			continue;

		if (pItem[gate].State == state)
			continue;

		UpdateItem(gate, state, &height);

		if (state == STATE_OPEN)
		{
			MSG_UpdateItem sm;
			memset(&sm, 0, sizeof(MSG_UpdateItem));

			sm.ID = ESCENE_FIELD;
			sm.Type = _MSG_UpdateItem;

			sm.ItemID = gate + 10000;

			sm.Size = sizeof(MSG_UpdateItem);

			//		sm.Height = height;
			sm.State = pItem[gate].State;

			GridMulticast(pItem[gate].PosX, pItem[gate].PosY, (MSG_STANDARD*)&sm, 0);
		}
		else
		{
			MSG_CreateItem sm;
			memset(&sm, 0, sizeof(MSG_CreateItem));

			GetCreateItem(gate, &sm);

			GridMulticast(pItem[gate].PosX, pItem[gate].PosY, (MSG_STANDARD*)&sm, 0);
		}
		pItem[gate].Delay = 0;
	}
}

/// Gera os monstros da rodada do Coliseu a partir do InitID.
void GenerateColoseum(int InitID)
{
	int MaxSpawn = 4 + rand() % 4;

	for (int i = 0; i < MaxSpawn; i++)
		GenerateMob(InitID, 0, 0);
}

/// Remove monstros remanescentes do Coliseu.
void DeleteColoseum()
{
	for (int i = MAX_USER; i < MAX_MOB; i++)
	{
		if (!pMob[i].Mode)
			continue;

		if (pMob[i].MOB.CurrentScore.Hp <= 0)
			continue;

		if (!pMob[i].GenerateIndex || pMob[i].GenerateIndex == 1 || pMob[i].GenerateIndex == 2)
			DeleteMob(i, 2);

		if (pMob[i].GenerateIndex == 5 || pMob[i].GenerateIndex == 6 || pMob[i].GenerateIndex == 7)
			DeleteMob(i, 2);
	}
}

/// Controla estado das portas da Arena.
void SetArenaDoor(int state)
{
	for (int i = 0; i < ValidGuild; i++)
	{
		if (i == 4)
			continue;

		for (int j = 0; j < 3; j++)
		{
			int gate = 3 * i + j + 1;
			int height = 0;

			if (pItem[gate].ITEM.sIndex <= 0 || pItem[gate].ITEM.sIndex >= MAX_ITEMLIST)
				continue;

			if (pItem[gate].State == state)
				continue;

			UpdateItem(gate, state, &height);

			if (state == STATE_OPEN)
			{
				MSG_UpdateItem sm;
				memset(&sm, 0, sizeof(MSG_UpdateItem));

				sm.ID = ESCENE_FIELD;
				sm.Type = _MSG_UpdateItem;

				sm.ItemID = gate + 10000;

				sm.Size = sizeof(MSG_UpdateItem);

				//sm.Height = height;
				sm.State = pItem[gate].State;

				GridMulticast(pItem[gate].PosX, pItem[gate].PosY, (MSG_STANDARD*)&sm, 0);
			}
			else
			{
				MSG_CreateItem sm;
				memset(&sm, 0, sizeof(MSG_CreateItem));

				GetCreateItem(gate, &sm);

				GridMulticast(pItem[gate].PosX, pItem[gate].PosY, (MSG_STANDARD*)&sm, 0);
			}
			pItem[gate].Delay = 0;
		}
	}
}

/// Controla estado das portas do Castelo de Noatum.
void SetCastleDoor(int state)
{
	for (int i = 0; i < 4; i++)
	{
		int DoorId = i + 36;

		if (pItem[DoorId].ITEM.sIndex <= 0 || pItem[DoorId].ITEM.sIndex >= MAX_ITEMLIST)
			continue;

		if (pItem[DoorId].State == state)
			continue;

		int height = 0;

		UpdateItem(DoorId, state, &height);

		if (state == STATE_OPEN)
		{
			MSG_UpdateItem sm;
			memset(&sm, 0, sizeof(MSG_UpdateItem));

			sm.ID = ESCENE_FIELD;
			sm.Type = _MSG_UpdateItem;
			sm.ItemID = DoorId + 10000;
			sm.Size = sizeof(MSG_UpdateItem);
			//		sm.Height = height;
			sm.State = pItem[DoorId].State;

			GridMulticast(pItem[DoorId].PosX, pItem[DoorId].PosY, (MSG_STANDARD*)&sm, 0);
		}
		else
		{
			int keyid = BASE_GetItemAbility(&pItem[DoorId].ITEM, EF_KEYID);

			if (keyid == 15)
				CreateMob("GATE", pItem[DoorId].PosX, pItem[DoorId].PosY, "npc", 0);

			MSG_CreateItem sm;
			memset(&sm, 0, sizeof(MSG_CreateItem));

			GetCreateItem(DoorId, &sm);

			GridMulticast(pItem[DoorId].PosX, pItem[DoorId].PosY, (MSG_STANDARD*)&sm, 0);
		}
		pItem[DoorId].Delay = 0;
	}
}

/// Decide a guilda vencedora da guerra de cidades/castelo.
void DecideWinner()
{
	for (int i = 0; i < ValidGuild; i++)
	{
		if (i == 3 || i == 4)
			continue;

		int ChargeSurvivor = 0;
		int ChallangeSurvivor = 0;

		int ChargeGuild = g_pGuildZone[i].ChargeGuild;
		int ChallangeGuild = g_pGuildZone[i].ChallangeGuild;

		if (ChallangeGuild == 0)
			continue;

		char GuildChargeName[256];
		char GuildChallangeName[256];

		BASE_GetGuildName(ServerGroup, g_pGuildZone[i].ChargeGuild, GuildChargeName);
		BASE_GetGuildName(ServerGroup, ChallangeGuild, GuildChallangeName);

		for (int j = g_pGuildZone[i].WarAreaY1; j <= g_pGuildZone[i].WarAreaY2; j++)
		{
			for (int k = g_pGuildZone[i].WarAreaX1; k <= g_pGuildZone[i].WarAreaX2; k++)
			{
				if(j < 0 || j >= MAX_GRIDY || k < 0 || k >= MAX_GRIDX)
					continue;

				int tmob = pMobGrid[j][k];

				if (tmob <= 0 || tmob >= MAX_USER)
					continue;

				if (pUser[tmob].Mode != USER_PLAY)
					continue;

				if (pMob[tmob].Mode == MOB_EMPTY)
					continue;

				if (pMob[tmob].MOB.CurrentScore.Hp <= 0)
					continue;

				int Guild = pMob[tmob].MOB.Guild;
				int Level = pMob[tmob].extra.ClassMaster != MORTAL && pMob[tmob].extra.ClassMaster != MORTAL ? pMob[tmob].MOB.CurrentScore.Level + MAX_LEVEL : pMob[tmob].MOB.CurrentScore.Level;

				if (Guild == 0)
					continue;

				if (Guild == ChargeGuild || Guild == ChallangeGuild)
				{
					sprintf(temp, "sys,guild war survive %s(%d-%d)", pMob[tmob].MOB.MobName, Guild, Level);
					Log(temp, "-system", 0);

					if (Guild == ChargeGuild)
						ChargeSurvivor += Level;

					else if (Guild == ChallangeGuild)
						ChallangeSurvivor += Level;

					else
						Log("err, more than 3 guild, at same guild zone", "-system", 0);
				}
			}
		}

		sprintf(temp, "sys,guild war %d result %d(%d) %d(%d)", i, ChargeGuild, ChargeSurvivor, ChallangeGuild, ChallangeSurvivor);
		Log(temp, "-system", 0);

		if (ChargeSurvivor < ChallangeSurvivor)
		{
			sprintf(temp, g_pMessageStringTable[_SSNN_GuildWarResult], GuildChallangeName, GuildChargeName, ChallangeSurvivor, ChargeSurvivor);
			SendGuildNotice(ChargeGuild, temp);
			SendGuildNotice(ChallangeGuild, temp);

			g_pGuildZone[i].ChargeGuild = ChallangeGuild;
			g_pGuildZone[i].Victory = 0;
		}
		else
		{
			sprintf(temp, g_pMessageStringTable[_SSNN_GuildWarResult], GuildChargeName, GuildChallangeName, ChargeSurvivor, ChallangeSurvivor);
			SendGuildNotice(ChargeGuild, temp);
			SendGuildNotice(ChallangeGuild, temp);

			if(g_pGuildZone[i].Victory < 4)
				g_pGuildZone[i].Victory++;
		}

		g_pGuildZone[i].ChallangeGuild = 0;
	}

	if(g_pGuildZone[1].ChargeGuild == g_pGuildZone[0].ChargeGuild && g_pGuildZone[2].ChargeGuild == g_pGuildZone[0].ChargeGuild && g_pGuildZone[3].ChargeGuild == g_pGuildZone[0].ChargeGuild)
		g_pGuildZone[4].ChargeGuild = g_pGuildZone[0].ChargeGuild;
}

/// Maquina de estados periodica das guerras de guilda, apostas e posse de cidades.
void GuildProcess()
{
	time_t rawtime;
	tm *timeinfo;

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	if (timeinfo->tm_hour == 3 && GuildNameInitialized == 0)
	{
		BASE_InitializeGuildName();
		
		Log("sys,GuildName initialized", "-system", 0);
		
		GuildNameInitialized = 1;
	}

	if (timeinfo->tm_hour > 3 && GuildNameInitialized == 1)
		GuildNameInitialized = 0;

	if (CastleServer == 1 && (timeinfo->tm_hour == 21))
	{
		if (CastleState || timeinfo->tm_min >= 5)
		{
			if (CastleState != 1 || timeinfo->tm_min <= 5 || timeinfo->tm_min >= 10)
			{
				if (CastleState != 2 || timeinfo->tm_min <= 50)
				{
					if (CastleState == 3 && timeinfo->tm_min > 55)
					{
						sprintf(temp, g_pMessageStringTable[_DN_Castle_closed], timeinfo->tm_hour - 17);
						SendNotice(temp);
						FinishCastleWar();
					}
				}
				else
				{
					sprintf(temp, g_pMessageStringTable[_DN_Castle_will_be_closed], timeinfo->tm_hour - 17);
					SendNotice(temp);
					CastleState = 3;
				}
			}
			else
			{
				ClearAreaGuild(1036, 1672, 1144, 1764, g_pGuildZone[4].ChargeGuild);
				
				ClearAreaTeleport(1129, 1705, 1129, 1709, 1057, 1742);
				ClearAreaTeleport(1116, 1705, 1116, 1709, 1057, 1742);
				ClearAreaTeleport(1094, 1688, 1094, 1692, 1057, 1742);
				ClearAreaTeleport(1087, 1609, 1087, 1713, 1057, 1742);
				ClearAreaTeleport(1050, 1690, 1050, 1690, 1057, 1742);
				ClearAreaTeleport(1046, 1690, 1047, 1691, 1057, 1742);
				ClearAreaTeleport(1124, 1708, 1124, 1708, 1057, 1742);
				
				SetCastleDoor(3);
				
				for (int i = 0; i < 3; ++i)
				{
					GenerateMob(i + TORRE_NOATUM1, 0, 0);
					LiveTower[i] = 1;
				}
				
				sprintf(temp, g_pMessageStringTable[_DN_Castle_opened], timeinfo->tm_hour - 17);
				SendNotice(temp);
				CastleState = 2;
			}
		}
		else
		{
			sprintf(temp, g_pMessageStringTable[_DN_Castle_will_be_open], timeinfo->tm_hour - 17);
			SendNotice(temp);
			CastleState = 1;
			
			for (int j = 0; j < MAX_USER; j++)
			{
				if (pUser[j].Mode == USER_PLAY)
					SendClientSignalParm(j, ESCENE_FIELD, 940, CastleState);
			}
		}
	}

	int NewbieServerID = (timeinfo->tm_mday - 1) % NumServerInGroup;

	if ((TESTSERVER || LOCALSERVER || ServerIndex == NewbieServerID) && BRItem > 0)
	{
		int tmmin2 = timeinfo->tm_min / 20;
		int tmmin = timeinfo->tm_min % 20;
		BrMod = timeinfo->tm_min % 20;
		BrGrid = timeinfo->tm_min / 20;

		if (timeinfo->tm_hour != BRHour || BrState || tmmin)
		{
			if (timeinfo->tm_hour != BRHour || BrState != 1 || tmmin != 5)
			{
				if (timeinfo->tm_hour == BRHour && BrState == 2 && tmmin == 13)
				{
					if (BRItem > 0 && BRItem < MAX_ITEMLIST)
					{
						STRUCT_ITEM item;

						BASE_ClearItem(&item);
						item.sIndex = BRItem;

						int create = CreateItem(2621, 1726, &item, rand()%4, 1);
						sprintf(temp, "etc,britem created %d/%d", BRItem, create);
						Log(temp, "-system", 0);
					}
					BrState = 3;
					SendNoticeArea(g_pMessageStringTable[_NN_BR_Rewarded], 2580, 1688, 2665, 1765);
				}
			}
			else
			{
				SetColoseumDoor(3);
				BrState = 2;

				if (tmmin2)
				{
					if (tmmin2 == 1)
					{
						SendNoticeArea(g_pMessageStringTable[_NN_BR_Start2], 2580, 1688, 2665, 1765);
						ClearAreaLevel(2604, 1708, 2648, 1744, 200, 400);
					}
					else
						SendNoticeArea(g_pMessageStringTable[_NN_BR_Start3], 2580, 1688, 2665, 1765);
				}
				else
				{
					SendNoticeArea(g_pMessageStringTable[_NN_BR_Start1], 2580, 1688, 2665, 1765);
					ClearAreaLevel(2604, 1708, 2648, 1744, 100, 400);
				}
				SetColoseumDoor(3);

				for (int k = 0; k < MAX_USER; k++)
				{
					if (pUser[k].Mode == USER_PLAY && pMob[k].TargetX >= 2604 && pMob[k].TargetX <= 2648 && pMob[k].TargetY >= 1708 && pMob[k].TargetY <= 1744)
					{
						SendGridMob(k);

						if (pMob[k].Leader)
							RemoveParty(k);
					}
				}
			}
		}
		else
		{
			BrState = 1;
			if (tmmin2)
			{
				if (tmmin2 == 1)
				{
					SendNoticeArea(g_pMessageStringTable[_NN_BR_Ready2], 2580, 1708, 2665, 1765);
					ClearAreaLevel(2604, 1708, 2648, 1744, 200, 400);
				}
				else
					SendNoticeArea(g_pMessageStringTable[_NN_BR_Ready3], 2580, 1708, 2665, 1765);
				
			}
			else
			{
				SendNoticeArea(g_pMessageStringTable[_NN_BR_Ready1], 2580, 1708, 2665, 1765);
				ClearAreaLevel(2604, 1708, 2648, 1744, 100, 400);
			}
			for (int k = 0; k < MAX_USER; k++)
			{
				if (pUser[k].Mode == USER_PLAY && pMob[k].TargetX >= 2604 && pMob[k].TargetX <= 2648 && pMob[k].TargetY >= 1708 && pMob[k].TargetY <= 1744)
				{
					SendGridMob(k);

					if (pMob[k].Leader)
						RemoveParty(k);
				}
			}
		}

		if (timeinfo->tm_hour == BRHour && BrState == 3 && tmmin >= 14)
		{
			SetColoseumDoor(1);
			BrState = 0;
		}
	}

	if (timeinfo->tm_hour != GuildHour && timeinfo->tm_hour != NewbieHour || timeinfo->tm_min <  3 || ColoState)
	{
		if (timeinfo->tm_hour != GuildHour && timeinfo->tm_hour != NewbieHour || timeinfo->tm_min <  4 || ColoState != 1)
		{
			if (timeinfo->tm_hour != GuildHour && timeinfo->tm_hour != NewbieHour || timeinfo->tm_min < 5 || ColoState != 2)
			{
				if (timeinfo->tm_hour != GuildHour && timeinfo->tm_hour != NewbieHour || timeinfo->tm_min <  7 || ColoState != 3)
				{
					if (timeinfo->tm_hour != GuildHour && timeinfo->tm_hour != NewbieHour || timeinfo->tm_min <  9 || ColoState != 4)
					{
						if (timeinfo->tm_hour != GuildHour && timeinfo->tm_hour != NewbieHour || timeinfo->tm_min < 11 || ColoState != 5)
						{
							if (timeinfo->tm_hour != GuildHour && timeinfo->tm_hour != NewbieHour || timeinfo->tm_min < 13 || ColoState != 6)
							{
								if (timeinfo->tm_hour != GuildHour && timeinfo->tm_hour != NewbieHour || timeinfo->tm_min < 15 || ColoState != 7)
								{
									if ((timeinfo->tm_hour == GuildHour || timeinfo->tm_hour == NewbieHour) && timeinfo->tm_min < 3 && ColoState == 8)
									{
										ColoState = 0;

										for (int m = 0; m < MAX_GUILDZONE; m++)
											TaxChanged[m] = 0;

										Colo150Limit = 0;
									}
								}
								else
								{
									SetColoseumDoor(1);
									SetColoseumDoor2(3);
									DeleteColoseum();
									ColoState = 8;
									Colo150Limit = 0;
								}
							}
							else
							{
								if (timeinfo->tm_hour == GuildHour)
								{
									GenerateColoseum(1);
									GenerateColoseum(2);
								}
								else
								{
									GenerateColoseum(6);
									GenerateColoseum(7);
								}
								ColoState = 7;
							}
						}
						else
						{
							if (timeinfo->tm_hour == GuildHour)
							{
								GenerateColoseum(1);
								GenerateColoseum(2);
							}
							else
							{
								GenerateColoseum(6);
								GenerateColoseum(7);
							}
							ColoState = 6;
						}
					}
					else
					{
						if (timeinfo->tm_hour == GuildHour)
						{
							GenerateColoseum(0);
							GenerateColoseum(1);
						}
						else
						{
							GenerateColoseum(5);
							GenerateColoseum(6);
						}
						ColoState = 5;
					}
				}
				else
				{
					if (timeinfo->tm_hour == GuildHour)
						GenerateColoseum(1);

					else
						GenerateColoseum(6);

					ColoState = 4;
				}
			}
			else
			{
				SetColoseumDoor2(1);
				ColoState = 3;
			}
		}
		else
		{
			if (timeinfo->tm_hour == GuildHour)
				GenerateColoseum(0);
			else
				GenerateColoseum(5);
			ColoState = 2;
		}
	}
	else
	{
		SetColoseumDoor(3);
		ColoState = 1;

		if (timeinfo->tm_hour == NewbieHour)
		{
			Colo150Limit = 1;
			ClearAreaLevel(2604, 1708, 2648, 1744, 150, 400);
		}
	}
	if (timeinfo->tm_hour == NewbieHour && timeinfo->tm_min == 54)
		PotionReady = 0;

	if (timeinfo->tm_hour == NewbieHour && timeinfo->tm_min == 55 && PotionReady == 0)
	{
		GenerateMob(22, 0, 0);
		PotionReady = 1;
	}
	int day = GuildDay - 1;

	if (day < 0)
		day += 7;
	
	int fweek = timeinfo->tm_wday;

	if (ForceWeekMode != -1)
		fweek = -1;

	if (WeekMode || ((fweek != GuildDay) || timeinfo->tm_hour < GuildHour) && ForceWeekMode != 1)
	{
		if (WeekMode != 1 || (fweek != GuildDay || (timeinfo->tm_hour < GuildHour) || timeinfo->tm_min <= 2) && ForceWeekMode != 2)
		{
			if (WeekMode != 2 || ((fweek != GuildDay) || timeinfo->tm_hour < GuildHour || timeinfo->tm_min <= 5) && ForceWeekMode != 3)
			{
				if (WeekMode != 3
					|| (fweek != GuildDay || (timeinfo->tm_hour < GuildHour) || timeinfo->tm_min <= 20) && ForceWeekMode != 4)
				{
					if (WeekMode != 4 || fweek != day && ForceWeekMode != 5)
					{
						if (WeekMode == 5 && (fweek == GuildDay || !ForceWeekMode))
						{
							SendNotice(g_pMessageStringTable[_NN_Guild_Battle_Notice6]);
							WeekMode = 0;

							for(int i = 0; i < MAX_GUILDZONE; i++)
							{
								if(i == 4)
									continue;

								if(g_pGuildZone[i].ChallangeGuild == 0 && g_pGuildZone[i].Victory < 4)
									g_pGuildZone[i].Victory++; 
							}

						}
					}
					else
					{
						SendNotice(g_pMessageStringTable[_NN_Guild_Battle_Notice5]);
						WeekMode = 5;
					}
				}
				else
				{
					SendNotice(g_pMessageStringTable[_NN_Guild_Battle_Notice4]);
					DecideWinner();
					ClearGuildPKZone();
					ClearChallanger();
					CReadFiles::WriteChallanger();
					SetArenaDoor(STATE_LOCKED);
					WeekMode = 4;

					GuildZoneReport();
				}
			}
			else
			{
				SendNotice(g_pMessageStringTable[_NN_Guild_Battle_Notice3]);
				WeekMode = 3;
				SetArenaDoor(STATE_OPEN);
			}
		}
		else
		{
			SendNotice(g_pMessageStringTable[_NN_Guild_Battle_Notice2]);
			ClearGuildPKZone();
			WeekMode = 2;
			SetArenaDoor(STATE_LOCKED);

			for (int n = 0; n < ValidGuild; n++)
			{
				if (n != 4)
				{
					if (g_pGuildZone[n].ChallangeGuild)
					{
						if (g_pGuildZone[n].ChargeGuild)
							SummonGuild(g_pGuildZone[n].ChargeGuild, g_pGuildZone[n].ChargeWarSpawnX, g_pGuildZone[n].ChargeWarSpawnY, NumGuildBattle, n);
					}
				}
			}

			for (int n = 0; n < ValidGuild; n++)
			{
				if (n != 4)
				{
					if (g_pGuildZone[n].ChallangeGuild)
						SummonGuild(g_pGuildZone[n].ChallangeGuild,	g_pGuildZone[n].ChallangeWarSpawnX, g_pGuildZone[n].ChallangeWarSpawnY, NumGuildBattle, n);
				}
			}
		}
	}
	else
	{
		SendNotice(g_pMessageStringTable[_NN_Guild_Battle_Notice1]);
		WeekMode = 1;
	}

#ifdef RVR_ON
#pragma region Guerra de Reino
	if (NewbieEventServer == 1 && timeinfo->tm_wday != 0 && timeinfo->tm_wday != 6 && timeinfo->tm_hour == RvRHour)
	{
		if (RvRState == 0 && timeinfo->tm_min < 5)
		{
			sprintf(temp, g_pMessageStringTable[_DD_KINGDOMWAR_BEGIN], 5, ServerIndex+1);
			SendNotice(temp);
			RvRState = 1;
		}
		if (RvRState == 1 && timeinfo->tm_min >= 5)
		{
			GenerateMob(RVRTORRE_1, 0, 0);
			GenerateMob(RVRTORRE_2, 0, 0);

			SendNotice(g_pMessageStringTable[_DD_KINGDOMWAR_START]);
			RvRState = 2;
			RvRRedPoint = 0;
			RvRBluePoint = 0;
		}

		if (RvRState == 2 && timeinfo->tm_min >= 29)
			RvRState = 3;

		if (RvRState == 3 && timeinfo->tm_min == 34)
		{
			ClearArea(1020, 1916, 1286, 2178);

			for(int i = MAX_USER; i < MAX_MOB; i++)
			{
				if(pMob[i].GenerateIndex == RVRTORRE_1 || pMob[i].GenerateIndex == RVRTORRE_2)
					DeleteMob(i, 1);
			}

			SendNotice(g_pMessageStringTable[_DD_KINGDOMWAR_END]);
			RvRState = 0;
			RvRRedPoint = 0;
			RvRBluePoint = 0;
		}
	}
#pragma endregion
#endif
	CWarTower::GuildProcess(timeinfo);
}

/// Reseta estado de desafiantes pendentes.
void ClearChallanger()
{
	for (int i = 0; i < ValidGuild; i++)
	{
		g_pGuildZone[i].ChallangeGuild = 0;
		pChallangerMoney[i] = 0;
	}
}

/// Remove o jogador do grupo atual e notifica lider e membros.
void RemoveParty(int conn)
{
	int leader = pMob[conn].Leader;

	if (leader < 0 || leader >= MAX_MOB)
	{
		Log("err,RemoveParty but worng leader", pUser[conn].AccountName, pUser[conn].IP);
		return;
	}

	if (leader && leader < MAX_USER && pUser[leader].Mode != USER_PLAY)
	{
		Log("err,removeparty - conn mode", pUser[conn].AccountName, pUser[conn].IP);
		Log("err,removeparty - leader mode", pUser[conn].AccountName, pUser[conn].IP);

		pMob[conn].Leader = 0;
	}
	else
	{
		if (conn > 0 && conn < MAX_USER)
			SendRemoveParty(conn, 0);

		if (leader)
		{
			SendRemoveParty(leader, conn);
			pMob[conn].Leader = 0;

			for (int i = 0; i < MAX_PARTY; i++)
			{
				int partyconn = pMob[leader].PartyList[i];

				if (partyconn)
				{
					if (partyconn == conn)
						pMob[leader].PartyList[i] = 0;

					if(pMob[partyconn].Summoner && pMob[partyconn].Summoner == conn)
						DeleteMob(partyconn, 3);

					if(!pMob[partyconn].Summoner && partyconn >= MAX_USER && leader < MAX_USER)
						DeleteMob(partyconn, 3);

					if (partyconn > 0 && partyconn <= MAX_USER && pUser[partyconn].Mode == USER_PLAY)
						SendRemoveParty(partyconn, conn);
				}
			}
		}

		else
		{
			int novolider = 0;

			for (int i = 0; i < MAX_PARTY; i++)
			{
				int partyconn = pMob[conn].PartyList[i];

				if(partyconn != 0 && partyconn < MAX_USER)
				{
					novolider = partyconn;

					for(int s = 0; s < 7; s++)
					{
						for(int t = 0; t < 3; t++)
						{
							if(Pista[s].Party[t].LeaderID == conn && strcmp(Pista[s].Party[t].LeaderName, pMob[conn].MOB.MobName) == 0)
							{
								strncpy(Pista[s].Party[t].LeaderName, pMob[novolider].MOB.MobName, NAME_LENGTH);
								Pista[s].Party[t].LeaderID = novolider;
							}
						}
					}

					break;
				}
			}

			for (int i = 0; i < MAX_PARTY; i++)
			{
				int partyconn = pMob[conn].PartyList[i];

				if (partyconn > 0 && partyconn <= MAX_MOB)
				{
					pMob[conn].PartyList[i] = 0;
					pMob[partyconn].Leader = 0;

					if (partyconn > 0 && partyconn < MAX_USER && pUser[partyconn].Mode == USER_PLAY)
						SendRemoveParty(partyconn, 0);

					if (partyconn > MAX_USER)
					{
						if (conn < MAX_USER && pMob[partyconn].Summoner && pMob[partyconn].Summoner == conn)
							DeleteMob(partyconn, 3);

						pMob[partyconn].Summoner = 0;
					}
					if(novolider != 0 && novolider != partyconn && partyconn < MAX_USER && novolider < MAX_USER)
					{
						MSG_AcceptParty sm;
						memset(&sm, 0, sizeof(MSG_AcceptParty));

						sm.Size = sizeof(MSG_AcceptParty);
						sm.Type = _MSG_AcceptParty;
						sm.ID = 0;

						sm.LeaderID = novolider;

						strncpy(sm.MobName, pMob[novolider].MOB.MobName, NAME_LENGTH);

						pMob[partyconn].LastReqParty = novolider;

						ProcessClientMessage(partyconn, (char*)&sm, TRUE);
					}
				}
			}
		}
	}
}

/// Executa retorno do jogador para sua cidade natal ou ponto salvo.
void DoRecall(int conn)
{
	int x = pMob[conn].MOB.SPX;
	int y = pMob[conn].MOB.SPY;
	int CityID = (pMob[conn].MOB.Merchant & 0xC0) >> 6;

	x = rand() % 15 + g_pGuildZone[CityID].CitySpawnX;
	y = rand() % 15 + g_pGuildZone[CityID].CitySpawnY;

	int Guild = pMob[conn].MOB.Guild;

	if (Guild > 0)
	{
		for (int i = 0; i < MAX_GUILDZONE; i++)
		{
			if (Guild == g_pGuildZone[i].ChargeGuild)
			{
				x = g_pGuildZone[i].GuildSpawnX;
				y = g_pGuildZone[i].GuildSpawnY;

				break;
			}
		}
	}

	if(pMob[conn].extra.ClassMaster == MORTAL && pMob[conn].MOB.CurrentScore.Level < FREEEXP)
	{
		x = 2112 + rand()%5 - 3;
		y = 2042 + rand()%5 - 3;
	}

	int tret = GetEmptyMobGrid(conn, &x, &y);

	MSG_Action sm;
	memset(&sm, 0, sizeof(MSG_Action));

	GetAction(conn, x, y, &sm);

	sm.Effect = 1;

	if (conn < MAX_USER)
		pUser[conn].cSock.SendOneMessage((char*)&sm, sizeof(sm));

	GridMulticast(conn, x, y, (MSG_STANDARD*)&sm);
}

/// Declara guerra entre duas guildas.
void DoWar(int myguild, int target)
{
	int max_guild = 65536;

	if (myguild <= 0 || target < 0 || myguild >= max_guild || target >= max_guild)
	{
		sprintf(temp, "err,dowar my: %d ta:%d", myguild, target);
		Log(temp, "-system", 0);
		return;
	}

	char myGuildName[16];
	char targetGuildName[16];

	BASE_GetGuildName(ServerGroup, myguild, myGuildName);
	BASE_GetGuildName(ServerGroup, target, targetGuildName);

	int myGuildWar = g_pGuildWar[myguild];

	if (myGuildWar < 0 || myGuildWar >= max_guild)
		myGuildWar = 0;

	int targetGuildWar = g_pGuildWar[target];

	if (target)
	{
		if (myGuildWar || !target || targetGuildWar == myguild)
		{
			if (myGuildWar || !target || targetGuildWar != myguild)
			{
				sprintf(temp, "err,dowar my:%d ta:%d ct:%d et:%d", myguild, target, myGuildWar, targetGuildWar);
				Log(temp, "-system", 0);
			}
			else
			{
				sprintf(temp, g_pMessageStringTable[_SS_War_Started], myGuildName, targetGuildName);
				SendNotice(temp);

				g_pGuildWar[myguild] = target;

				for (int i = 0; i < MAX_USER; i++)
				{
					if (pUser[i].Mode == USER_PLAY && (pMob[i].MOB.Guild == myguild || pMob[i].MOB.Guild == target))
						SendWarInfo(i, g_pGuildZone[4].Clan);

				}
			}
		}

		else
		{
			sprintf(temp, g_pMessageStringTable[_SS_War_Declared], myGuildName, targetGuildName);

			SendGuildNotice(myguild, temp);
			SendGuildNotice(target, temp);

			g_pGuildWar[myguild] = target;
		}
	}
	else
	{
		if (myGuildWar <= 0 || myGuildWar >= max_guild)
			return;

		targetGuildWar = g_pGuildWar[myGuildWar];

		BASE_GetGuildName(ServerGroup, myGuildWar, targetGuildName);

		if (targetGuildWar == myguild)
		{
			sprintf(temp, g_pMessageStringTable[_SS_War_Canceled], myGuildName, targetGuildName);

			g_pGuildWar[myGuildWar] = 0;
			g_pGuildWar[myguild] = 0;

			for (int j = 0; j < MAX_USER; j++)
			{
				if (pUser[j].Mode == USER_PLAY && (pMob[j].MOB.Guild == myguild || pMob[j].MOB.Guild == myGuildWar))
					SendWarInfo(j, g_pGuildZone[4].Clan);
			}
		}

		else
		{
			sprintf(temp, g_pMessageStringTable[_SS_War_declare_canceled], myGuildName, targetGuildName);
			g_pGuildWar[myguild] = 0;
		}

		SendGuildNotice(myguild, temp);
		SendGuildNotice(myGuildWar, temp);
	}
}

/// Estabelece alianca entre duas guildas.
void DoAlly(int myguild, int target)
{
	int max_guild = 65536;

	if (myguild <= 0 || target < 0 || myguild >= max_guild || target >= max_guild)
	{
		sprintf(temp, "err,doally my: %d ta:%d", myguild, target);
		Log(temp, "-system", 0);
		return;
	}

	char myGuildName[16];
	char targetGuildName[16];

	BASE_GetGuildName(ServerGroup, myguild, myGuildName);
	BASE_GetGuildName(ServerGroup, target, targetGuildName);

	int myGuildAlly = g_pGuildAlly[myguild];

	if (myGuildAlly < 0 || myGuildAlly >= max_guild)
		myGuildAlly = 0;

	int targetGuildAlly = g_pGuildAlly[target];

	if (!myGuildAlly)
	{
		if (!myGuildAlly && target)
		{
			sprintf(temp, g_pMessageStringTable[_SS_Ally_Declared], myGuildName, targetGuildName);

			SendNotice(temp);

			g_pGuildAlly[myguild] = target;
		}
	}
	else
	{
		myGuildAlly = g_pGuildAlly[myguild];

		if (myGuildAlly > 0 && myGuildAlly < max_guild)
		{
			BASE_GetGuildName(ServerGroup, myGuildAlly, targetGuildName);

			sprintf(temp, g_pMessageStringTable[_SS_Ally_Canceled], myGuildName, targetGuildName);

			SendNotice(temp);

			g_pGuildAlly[myguild] = 0;
		}
	}

	for (int i = 0; i < MAX_USER; i++)
	{
		if (pUser[i].Mode == USER_PLAY && pMob[i].MOB.Guild == myguild)
			SendWarInfo(i, g_pGuildZone[4].Clan);
	}
}

/// Remove membro ou dissolve privacoes/lideranca de guilda.
void DoDeprivate(int conn, int target)
{
	if (conn <= 0 || conn > MAX_USER)
	{
		Log("err,dodeprivate - conn", "-system", 0);

		return;
	}

	int Guild = pMob[conn].MOB.Guild;
	int GuildLevel = pMob[conn].MOB.GuildLevel;


	if (target <= 0 || target > MAX_USER || pUser[target].Mode != USER_PLAY)
	{
		SendClientMessage(conn, g_pMessageStringTable[_NN_Not_Connected]);

		return;
	}

	if (GuildLevel <= pMob[target].MOB.GuildLevel)
	{
		SendClientMessage(conn, g_pMessageStringTable[_NN_Only_Guild_Master_Can]);

		return;
	}

	int sub = pMob[conn].MOB.GuildLevel;
	int Disqualified = FALSE;


	int gGuild = pMob[target].MOB.Guild;

	if (Guild != gGuild)
		return;

	if (sub >= 6 && sub <= 8 && pMob[target].MOB.GuildLevel < 6 || sub == 9)
	{
		Disqualified = TRUE;

		if(pMob[target].MOB.GuildLevel >= 6 && pMob[target].MOB.GuildLevel <= 8)
		{
			if(GuildInfo[gGuild].Sub1 != 0)
				GuildInfo[gGuild].Sub1 = 0;
			else if(GuildInfo[gGuild].Sub2 != 0)
				GuildInfo[gGuild].Sub2 = 0;
		
			else if(GuildInfo[gGuild].Sub3 != 0)
				GuildInfo[gGuild].Sub3 = 0;
		

			sprintf(temp, "etc,subdelete subname:%s guild:%d ", pMob[target].MOB.MobName, Guild);
			Log(temp, pUser[conn].AccountName, pUser[conn].IP);

			MSG_GuildInfo sm_gi;
			memset(&sm_gi, 0, sizeof(MSG_GuildInfo));

			sm_gi.Type = _MSG_GuildInfo;
			sm_gi.Size = sizeof(MSG_GuildInfo);
			sm_gi.ID = conn;

			sm_gi.Guild = Guild;
			sm_gi.GuildInfo = GuildInfo[Guild];

			DBServerSocket.SendOneMessage((char*)&sm_gi, sizeof(MSG_GuildInfo));
		}

		pMob[target].MOB.Guild = 0;
		pMob[target].MOB.GuildLevel = 0;

		MSG_CreateMob sm;
		memset(&sm, 0, sizeof(MSG_CreateMob));

		GetCreateMob(target, &sm);

		GridMulticast(pMob[target].TargetX, pMob[target].TargetY, (MSG_STANDARD*)&sm, 0);
	}


	if (Disqualified == FALSE)
	{
		SendClientMessage(conn, g_pMessageStringTable[_NN_Deprivate]);

		return;
	}

	SendScore(target);

	SendClientMessage(target, g_pMessageStringTable[_NN_Disqualified_By_Guild_Master]);
	SendClientMessage(conn, g_pMessageStringTable[_NN_Disqualify]);
}

/// Gerencia o ciclo do evento de duelo/ranking em arena.
void ProcessRanking()
{
	if (RankingTime <= 0)
		return;

	RankingTime--;

	if (RankingTime % 5 == 0)
	{
		int Rank1 = 0;
		int Rank2 = 0;

		for (int i = 1; i < MAX_USER; i++)
		{
			if (pUser[i].Mode != USER_PLAY)
				continue;

			if (pMob[i].MOB.CurrentScore.Hp <= 0)
				continue;

			int posX = pMob[i].TargetX;
			int posY = pMob[i].TargetY;

			if (posX >= kRanking1X && posX <= kRanking2X && posY >= kRanking1Y && posY <= kRanking2Y)
			{
				if (RankingProgress == 1)
				{
					if (i == Ranking1)
						Rank1++;

					if (i == Ranking2)
						Rank2++;
				}

				else
				{
					if (pMob[i].MOB.Guild == Ranking1)
						Rank1++;

					if (pMob[i].MOB.Guild == Ranking2)
						Rank2++;
				}
			}
		}

		if (Rank1 == 0 || Rank2 == 0)
			RankingTime = 0;
	}

	if (RankingTime >= 60)
	{
		if (RankingTime >= 120)
		{
			if (RankingTime < 180)
			{
				SendDamage(142, 4007, 195, 4019);
				SendDamage(142, 4070, 195, 4082);

				SendEnvEffect(142, 4007, 168, 4018, 32, 0);
				SendEnvEffect(142, 4071, 168, 4082, 32, 0);
				SendEnvEffect(169, 4007, 195, 4018, 32, 0);
				SendEnvEffect(169, 4071, 195, 4082, 32, 0);
			}
		}
		else
		{
			SendDamage(142, 4007, 195, 4034);
			SendDamage(142, 4055, 195, 4082);

			SendEnvEffect(142, 4007, 168, 4018, 32, 0);
			SendEnvEffect(142, 4071, 168, 4082, 32, 0);
			SendEnvEffect(142, 4019, 168, 4030, 32, 0);
			SendEnvEffect(142, 4059, 168, 4070, 32, 0);
			SendEnvEffect(168, 4007, 195, 4018, 32, 0);
			SendEnvEffect(168, 4071, 195, 4082, 32, 0);
			SendEnvEffect(168, 4019, 195, 4030, 32, 0);
			SendEnvEffect(168, 4059, 195, 4070, 32, 0);
		}
	}
	else
	{
		SendDamage(142, 4007, 195, 4042);
		SendDamage(142, 4046, 195, 4082);
		SendEnvEffect(142, 4007, 168, 4018, 32, 0);
		SendEnvEffect(142, 4071, 168, 4082, 32, 0);
		SendEnvEffect(142, 4019, 168, 4030, 32, 0);
		SendEnvEffect(142, 4059, 168, 4070, 32, 0);
		SendEnvEffect(142, 4031, 168, 4042, 32, 0);
		SendEnvEffect(142, 4047, 168, 4058, 32, 0);
		SendEnvEffect(168, 4007, 195, 4018, 32, 0);
		SendEnvEffect(168, 4071, 195, 4082, 32, 0);
		SendEnvEffect(168, 4019, 195, 4030, 32, 0);
		SendEnvEffect(168, 4059, 195, 4070, 32, 0);
		SendEnvEffect(168, 4031, 195, 4042, 32, 0);
		SendEnvEffect(168, 4047, 195, 4058, 32, 0);
	}

	if (RankingTime <= 0)
	{
		int Rank1 = 0;
		int Rank2 = 0;

		for (int i = 1; i < MAX_USER; i++)
		{
			if (pUser[i].Mode != USER_PLAY)
				continue;

			if (pMob[i].MOB.CurrentScore.Hp <= 0)
				continue;

			int posX = pMob[i].TargetX;
			int posY = pMob[i].TargetY;

			if (posX >= kRanking1X && posX <= kRanking2X && posY >= kRanking1Y && posY <= kRanking2Y)
			{
				if (RankingProgress == 1)
				{
					if (i == Ranking1)
						Rank1++;

					if (i == Ranking2)
						Rank2++;
				}

				else
				{
					if (pMob[i].MOB.Guild == Ranking1)
						Rank1++;

					if (pMob[i].MOB.Guild == Ranking2)
						Rank2++;
				}
			}
		}

		nRanking1[15] = 0;
		nRanking1[16] = 0;
		nRanking2[15] = 0;
		nRanking2[16] = 0;

		nTitle1[15] = 0;
		nTitle1[16] = 0;
		nTitle2[15] = 0;
		nTitle2[16] = 0;


		if (Rank1 == Rank2)
		{
			sprintf(temp, g_pMessageStringTable[_SS_S_S_Draw], nRanking1, nRanking2);

			SendNoticeArea(temp, cRanking1X, cRanking1Y, cRanking2X, cRanking2Y);
			SendNoticeArea(temp, kRanking1X, kRanking1Y, kRanking2X, kRanking2Y);
		}

		else
		{
			if (Rank1 <= Rank2)
				sprintf(temp, g_pMessageStringTable[_SS_S_WinBy_S], nRanking2, nRanking1);
			else
				sprintf(temp, g_pMessageStringTable[_SS_S_WinBy_S], nRanking1, nRanking2);

			SendNoticeArea(temp, cRanking1X, cRanking1Y, cRanking2X, cRanking2Y);
			SendNoticeArea(temp, kRanking1X, kRanking1Y, kRanking2X, kRanking2Y);

			MSG_MessageDBRecord sm;
			memset(&sm, 0, sizeof(MSG_MessageDBRecord));

			sm.Type = _MSG_MessageDBRecord;
			sm.ID = 0;
			sm.Size = sizeof(MSG_MessageDBRecord);

			if (RankingProgress != 1 || Ranking1 <= 0 || Ranking1 >= MAX_USER || Ranking2 <= 0 || Ranking2 >= MAX_USER)
			{
				if (Rank1 <= Rank2)
					sprintf(temp, "RNK %d %d %s %d %d %s %d %d", ServerIndex, RankingProgress, nRanking2, lTitle2, Ranking2, nRanking1, lTitle1, Ranking1);
				else
					sprintf(temp, "RNK %d %d %s %d %d %s %d %d", ServerIndex, RankingProgress, nRanking1, lTitle1, Ranking1, nRanking2, lTitle2, Ranking2);
			}

			else
			{
				int GuildR1 = pMob[Ranking1].MOB.Guild;
				int GuildR2 = pMob[Ranking2].MOB.Guild;

				if (Rank1 <= Rank2)
					sprintf(temp, "RNK %d %d %s %d %d %s %d %d", ServerIndex, RankingProgress, nRanking2, lTitle2, GuildR2, nRanking1, lTitle1, GuildR1);
				else
					sprintf(temp, "RNK %d %d %s %d %d %s %d %d", ServerIndex, RankingProgress, nRanking1, lTitle1, GuildR1, nRanking2, lTitle2, GuildR2);
			}

			DBServerSocket.SendOneMessage((char*)&sm, sm.Size);
		}

		RankingProgress = 0;
		RankingTime = 0;
		Ranking1 = 0;
		Ranking2 = 0;

		nRanking1[0] = 0;
		nRanking2[0] = 0;

		ClearAreaTeleport(kRanking1X, kRanking1Y, kRanking2X, kRanking2Y, 2572, 1752);
	}
}

/// Inicia ou registra confronto de ranking entre dois jogadores.
void DoRanking(int tDuel, int conn, int enemy)
{
	if (tDuel == 0)
	{
		int duelTime = 121;

		if (conn < 0 || conn >= MAX_USER || enemy < 0 || enemy >= MAX_USER)
			return;

		strncpy(nRanking1, pMob[conn].MOB.MobName, NAME_LENGTH);
		strncpy(nRanking2, pMob[enemy].MOB.MobName, NAME_LENGTH);

		lTitle1 = pMob[conn].MOB.CurrentScore.Level;
		lTitle2 = pMob[enemy].MOB.CurrentScore.Level;

		DoTeleport(conn, Ranking2X, Ranking2Y);
		DoTeleport(enemy, Ranking1X, Ranking1Y);

		sprintf(temp, g_pMessageStringTable[_NN_Battle_Started], pMob[conn].MOB.MobName, pMob[enemy].MOB.MobName);

		SendClientMessage(conn, temp);
		SendClientMessage(enemy, temp);

		SendClientSignalParm(conn, ESCENE_FIELD, _MSG_StartTime, 2 * duelTime - 2);
		SendClientSignalParm(enemy, ESCENE_FIELD, _MSG_StartTime, 2 * duelTime - 2);

		RankingProgress = 1;
		Ranking1 = conn;
		Ranking2 = enemy;
		RankingTime = duelTime;

		return;
	}

	int duelTime = 301;

	int GuildR1 = pMob[conn].MOB.Guild;
	int GuildR2 = pMob[enemy].MOB.Guild;

	if (GuildR1 <= 0 || GuildR2 <= 0)
		return;

	if(GuildR1 == GuildR2)
		return;

	RankingProgress = tDuel + 1;
	Ranking1 = GuildR1;
	Ranking2 = GuildR2;
	RankingTime = duelTime;

	BASE_GetGuildName(ServerGroup, Ranking1, nRanking1);
	BASE_GetGuildName(ServerGroup, Ranking2, nRanking2);

	strncpy(nTitle1, pMob[conn].MOB.MobName, NAME_LENGTH);

	lTitle1 = pMob[conn].MOB.CurrentScore.Level;

	strncpy(nTitle2, pMob[enemy].MOB.MobName, NAME_LENGTH);

	lTitle2 = pMob[enemy].MOB.CurrentScore.Level;

	int jRank1 = 5;
	int jRank2 = 5;

	if (tDuel == 2)
	{
		jRank1 = 10;
		jRank2 = 10;
	}

	else if (tDuel == 3)
	{
		jRank1 = 100;
		jRank2 = 100;

		SummonGuild(Ranking1, Ranking1X, Ranking1Y, 100, 3);
		SummonGuild(Ranking2, Ranking1X, Ranking1Y, 100, 3);
	}

	else
	{
		for (int i = cRanking1Y; i <= cRanking2Y; i++)
		{
			for (int j = cRanking1X; j <= cRanking2X; j++)
			{
				int tmob = pMobGrid[i][j];

				if (tmob <= 0 || tmob >= MAX_USER)
					continue;

				if (jRank1 > 0 && pMob[tmob].MOB.Guild == Ranking1)
				{
					jRank1--;

					DoTeleport(tmob, Ranking1X, Ranking1Y);

					SendClientMessage(tmob, g_pMessageStringTable[194]);
					SendClientSignalParm(tmob, ESCENE_FIELD, _MSG_StartTime, 2 * duelTime - 2);
				}

				if (jRank2 > 0 && pMob[tmob].MOB.Guild == Ranking2)
				{
					jRank2--;

					DoTeleport(tmob, Ranking2X, Ranking2Y);

					SendClientMessage(tmob, g_pMessageStringTable[194]);
					SendClientSignalParm(tmob, ESCENE_FIELD, _MSG_StartTime, 2 * duelTime - 2);
				}
				if (jRank1 <= 0 || jRank2 <= 0)
					break;
			}
		}
	}
}

void ClearMapa(int x, int y)
{
	for (int i = 0; i < MAX_USER; i++)
	{
		if (pUser[i].Mode != USER_PLAY)
			continue;

		if (pMob[i].Mode == MOB_EMPTY)
			continue;

		if ((pMob[i].TargetX /128) != x || (pMob[i].TargetY / 128) != y)
			continue;

		if(pMob[i].MOB.CurrentScore.Hp == 0)
		{
			pMob[i].MOB.CurrentScore.Hp = 2;

			pUser[i].NumError = 0;

			SendScore(i);
			SendSetHpMp(i);
		}

		DoRecall(i);
	}
}

void DeleteMobMapa(int x, int y)
{
	for (int i = MAX_USER; i < MAX_MOB; i++)
	{
		if (pMob[i].Mode == USER_EMPTY)
			continue;

		if ((pMob[i].TargetX / 128) != x || (pMob[i].TargetY / 128) != y)
			continue;

		DeleteMob(i, 1);
	}
}

/// Teleporta todos os jogadores conectados do servidor para uma coordenada.
void SummonServer(int tx, int ty)
{
	int summons = 0;

	for (int i = 0; i < MAX_USER; i++)
	{
		if (pUser[i].Mode != USER_PLAY || pMob[i].Mode == USER_EMPTY)
			continue;

		DoGreatTeleport(i, tx, ty);
	}
}
