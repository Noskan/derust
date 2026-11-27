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

#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>
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

/// @file MobSpawnSystem.cpp
/// @brief Geracao, leitura, combate basico e ciclo de vida de NPCs, monstros e evocacoes.

/// Evoca criatura aliada (summon) vinculada ao jogador.
int GenerateSummon(int conn, int SummonID, STRUCT_ITEM *sItem, int Num)
{
	int Leader = pMob[conn].Leader;

	if (Leader <= 0)
		Leader = conn;

	int MobEmpty = GetEmptyNPCMob();

	if (MobEmpty == 0)
	{
		SendClientMessage(conn, g_pMessageStringTable[_NN_Cant_Create_More_Summons]);

		return 0;
	}

	if (SummonID < 0 || SummonID >= MAX_SUMMONLIST)
		return 0;

	int sFace = mSummon.Mob[SummonID].Equip[0].sIndex;
	int Unk = 0;
	int SummonNum = 0;

	int IsSummonOther = 0;

	for (int i = 0; i < MAX_PARTY; i++)
	{
		int partyconn = pMob[Leader].PartyList[i];

		if (partyconn == 0)
			continue;

		if (partyconn <= 0 || partyconn >= MAX_MOB)
			continue;

		for (int k = 0; k < MAX_SUMMONLIST; k++)
		{
			if (pMob[partyconn].MOB.Equip[0].sIndex == mSummon.Mob[k].Equip[0].sIndex && mSummon.Mob[k].Equip[0].sIndex != sFace)
			{
				return 0;
			}
		}

		if (pMob[partyconn].MOB.Equip[0].sIndex != sFace || SummonID >= MAX_SUMMONLIST)
			continue;

		int posX = pMob[conn].TargetX;
		int posY = pMob[conn].TargetY;

		SummonNum++;


		if (GetEmptyMobGrid(partyconn, &posX, &posY) != 0 && GetInView(conn, partyconn) == FALSE)
		{
			MSG_Action sm;
			memset(&sm, 0, sizeof(MSG_Action));

			GetAction(partyconn, posX, posY, &sm);

			sm.Effect = 8;
			sm.Speed = 6;

			GridMulticast(partyconn, posX, posY, (MSG_STANDARD*)&sm);

			if (SummonNum >= Num)
				return 0;
			else
				continue;
		}
		else if (SummonNum >= Num)
			return 0;
	}
	for (int i = SummonNum; i < Num; i++)
	{
		MobEmpty = GetEmptyNPCMob();

		if (MobEmpty == 0)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_Cant_Create_More_Summons]);

			return 0;
		}
		int j = 0;

		for (j = 0; j < MAX_PARTY; j++)
		{
			if (pMob[Leader].PartyList[j])
				continue;

			else
				break;
		}

		if (j >= MAX_PARTY)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_Party_Full_Cant_Summon]);

			return 0;
		}

		memset(pMob[MobEmpty].PartyList, 0, sizeof(pMob[MobEmpty].PartyList));
		memcpy(&pMob[MobEmpty], &mSummon.Mob[SummonID], sizeof(mSummon.Mob[SummonID]));

		pMob[MobEmpty].MOB.BaseScore.Level = pMob[conn].MOB.BaseScore.Level <= MAX_LEVEL ? pMob[conn].MOB.BaseScore.Level : MAX_LEVEL;

		strcat(pMob[MobEmpty].MOB.MobName, "^");

		for (int k = 0; k < NAME_LENGTH; k++)
		{
			if (pMob[MobEmpty].MOB.MobName[k] == '_')
				pMob[MobEmpty].MOB.MobName[k] = ' ';
		}

		memset(pMob[MobEmpty].Affect, 0, sizeof(pMob[MobEmpty].Affect));

		int Int = pMob[conn].MOB.CurrentScore.Int;
		int Evocacao = pMob[conn].MOB.CurrentScore.Special[2];

		int sBase = Int * pSummonBonus[SummonID].Int_Damage / 100;
		int sMax = Evocacao * pSummonBonus[SummonID].Spe_Damage / 100;

		pMob[MobEmpty].MOB.BaseScore.Damage += sMax + sBase;

		sBase = Int * pSummonBonus[SummonID].Int_Ac / 100;
		sMax = Evocacao * pSummonBonus[SummonID].Spe_Ac / 100;

		pMob[MobEmpty].MOB.BaseScore.Ac += sMax + sBase;

		sBase = Int * pSummonBonus[SummonID].Int_Hp / 100;
		sMax = Evocacao * pSummonBonus[SummonID].Spe_Hp / 100;

		pMob[MobEmpty].MOB.BaseScore.MaxHp += sMax + sBase;

		pMob[MobEmpty].MOB.CurrentScore.MaxHp = pMob[MobEmpty].MOB.BaseScore.MaxHp;

		pMob[MobEmpty].GenerateIndex = -1;

		pMob[MobEmpty].Formation = 5;

		pMob[MobEmpty].RouteType = 5;
		pMob[MobEmpty].Mode = MOB_PEACE;

		//	*(int*)&pMob[MobEmpty].Unk6[64] = 0;
		//	*(int*)&pMob[MobEmpty].Unk6[60] = 0;

		pMob[MobEmpty].SegmentProgress = 0;
		pMob[MobEmpty].SegmentDirection = 0;

		memset(&pMob[MobEmpty].SegmentX, 0, sizeof(pMob[MobEmpty].SegmentX));
		memset(&pMob[MobEmpty].SegmentY, 0, sizeof(pMob[MobEmpty].SegmentY));

		//	memset(pMob[MobEmpty].Unk6, 0, 20);
		//	memset(&pMob[MobEmpty].Unk6[20], 0, 20);

		pMob[MobEmpty].Leader = Leader;
		pMob[MobEmpty].LastTime = CurrentTime;

		if (sItem != 0)
		{
			int mountsanc = BASE_GetItemAbility(sItem, EF_MOUNTSANC);

			if (mountsanc >= 100)
				mountsanc = 100;

			int face = pMob[MobEmpty].MOB.Equip[0].sIndex;
			int con = 0;

			if (face >= 315 && face < 345)
			{
				int mcon = /*0x4C7974[face]*/0; // Voltar e identificar essa var
				int mUnk = mcon / 2 - 1000;
				int mUnk2 = mcon - mUnk;

				con = mountsanc * (mcon - mUnk) / 100 + mcon / 2 - 1000;
			}

			pMob[MobEmpty].MOB.BaseScore.Con = con;
			pMob[MobEmpty].MOB.CurrentScore.Con = con;

			pMob[MobEmpty].MOB.BaseScore.Damage += 6 * BASE_GetItemAbility(sItem, EF_MOUNTSANC);
			pMob[MobEmpty].MOB.CurrentScore.Damage = pMob[MobEmpty].MOB.BaseScore.Damage;
		}

		if (pMob[conn].GuildDisable == 0)
		{
			pMob[MobEmpty].MOB.Guild = pMob[MobEmpty].MOB.Guild;
			pMob[MobEmpty].MOB.GuildLevel = 0;
		}

		GetGuild(MobEmpty);


		pMob[MobEmpty].MOB.CurrentScore.Hp = pMob[MobEmpty].MOB.CurrentScore.MaxHp;

		if (pMob[conn].MOB.Equip[15].sIndex == 543 || pMob[conn].MOB.Equip[15].sIndex == 545)
		{
			memset(&pMob[MobEmpty].MOB.Equip[15], 0, sizeof(STRUCT_ITEM));
			pMob[MobEmpty].MOB.Equip[15].sIndex = 734;
		}

		if (pMob[conn].MOB.Equip[15].sIndex == 544 || pMob[conn].MOB.Equip[15].sIndex == 546)
		{
			memset(&pMob[MobEmpty].MOB.Equip[15], 0, sizeof(STRUCT_ITEM));
			pMob[MobEmpty].MOB.Equip[15].sIndex = 735;
		}

		if (pMob[conn].MOB.Equip[15].sIndex == 548 || pMob[conn].MOB.Equip[15].sIndex == 549)
		{
			memset(&pMob[MobEmpty].MOB.Equip[15], 0, sizeof(STRUCT_ITEM));
			pMob[MobEmpty].MOB.Equip[15].sIndex = 550;
		}

		pMob[MobEmpty].MOB.Clan = 4;

		pMob[MobEmpty].GetCurrentScore(MAX_USER);

		memset(pMob[MobEmpty].EnemyList, 0, sizeof(pMob[MobEmpty].EnemyList));

		int posX = pMob[conn].TargetX;
		int posY = pMob[conn].TargetY;

		int tmob = GetEmptyMobGrid(MobEmpty, &posX, &posY);

		if (tmob == 0)
		{
			pMob[MobEmpty].Mode = MOB_EMPTY;

			return 0;
		}

		pMob[MobEmpty].TargetX = posX;
		pMob[MobEmpty].LastX = posX;
		pMob[MobEmpty].TargetY = posY;
		pMob[MobEmpty].LastY = posY;

		if (sItem == 0)
		{
			pMob[MobEmpty].Affect[0].Type = 24;
			pMob[MobEmpty].Affect[0].Level = 0;
			pMob[MobEmpty].Affect[0].Value = 0;
			pMob[MobEmpty].Affect[0].Time = 20;

			if (SummonID >= 28 && SummonID <= 37)
				pMob[MobEmpty].Affect[0].Time = 2000000000;
		}

		if (sItem != 0)
		{
			if (sItem->stEffect[0].sValue > pMob[MobEmpty].MOB.CurrentScore.MaxHp)
				sItem->stEffect[0].sValue = pMob[MobEmpty].MOB.CurrentScore.MaxHp;

			pMob[MobEmpty].MOB.BaseScore.Hp = sItem->stEffect[0].sValue;
			pMob[MobEmpty].MOB.CurrentScore.Hp = pMob[MobEmpty].MOB.BaseScore.Hp;
		}
		MSG_CreateMob sm;
		memset(&sm, 0, sizeof(MSG_CreateMob));

		GetCreateMob(MobEmpty, &sm);

		sm.CreateType |= 3u;
		pMobGrid[posY][posX] = MobEmpty;

		GridMulticast(posX, posY, (MSG_STANDARD*)&sm, 0);

		pMob[Leader].PartyList[j] = MobEmpty;
		pMob[MobEmpty].Leader = Leader;

		if (SummonNum == 0)
			SendAddParty(Leader, Leader, 0);

		SendAddParty(MobEmpty, Leader, 0);

		if (SummonNum == 0)
			SendAddParty(MobEmpty, MobEmpty, j + 1);

		SendAddParty(Leader, MobEmpty, j + 1);

		for (int l = 0; l < MAX_PARTY; l++)
		{
			if (pMob[Leader].PartyList[l] == 0)
				continue;

			if (pMob[Leader].PartyList[l] != MobEmpty)
				SendAddParty(pMob[Leader].PartyList[l], MobEmpty, l + 1);

			SendAddParty(MobEmpty, pMob[Leader].PartyList[l], l + 1);
		}

		pMob[MobEmpty].Summoner = conn;
	}
	return 1;
}

/// Carrega template de NPC/mob do disco e instancia no mundo.
int CreateMob(char *MobName, int PosX, int PosY, char *folder, int Type)
{
	int tmob = GetEmptyNPCMob();

	if (tmob == 0)
	{
		sprintf(temp, "err,no emptymob create %s", MobName);
		Log(temp, "-system", 0);

		return -1;
	}

	strncpy(pMob[tmob].MOB.MobName, MobName, NAME_LENGTH);

	memset(&pMob[tmob].PartyList, 0, sizeof(pMob[tmob].PartyList));
	
	int read = ReadMob(&pMob[tmob].MOB, folder);

	if(read == 0)
		return 0;

	pMob[tmob].MOB.MobName[NAME_LENGTH - 1] = 0;
	pMob[tmob].MOB.BaseScore.Merchant = 0;


	for (int i = 0; i < NAME_LENGTH; i++)
	{
		if (pMob[tmob].MOB.MobName[i] == 95)
			pMob[tmob].MOB.MobName[i] = 32;

		if (pMob[tmob].MOB.MobName[i] == '@')
			pMob[tmob].MOB.MobName[i] = 0;
	}
	memset(pMob[tmob].Affect, 0, sizeof(pMob[tmob].Affect));


	for (int i = 0; i < MAX_SEGMENT; i++)
	{
		if(pMob[tmob].MOB.Equip[0].sIndex != 220 && pMob[tmob].MOB.Equip[0].sIndex != 219 && pMob[tmob].MOB.Equip[0].sIndex != 358)
		{
			pMob[tmob].SegmentListX[i] = PosX + W2Rand()%5 - 2;
			pMob[tmob].SegmentListY[i] = PosY + W2Rand()%5 - 2;
		}

		else 
		{
			pMob[tmob].SegmentListX[i] = PosX;
			pMob[tmob].SegmentListY[i] = PosY;
		}
	}


	if (pMob[tmob].MOB.Equip[0].sIndex == 220 || pMob[tmob].MOB.Equip[0].sIndex == 219)
	{
		if (pMob[tmob].MOB.Equip[0].sIndex == 219)
		{
			pMob[tmob].MOB.Guild = g_pGuildZone[4].ChargeGuild;
			pMob[tmob].MOB.GuildLevel = 0;
		}
	}

	pMob[tmob].GenerateIndex = -1;

	pMob[tmob].Formation = 0;

	pMob[tmob].RouteType = Type;
	pMob[tmob].Mode = MOB_PEACE;

	pMob[tmob].SegmentDirection = 0;

	pMob[tmob].Leader = 0;
	pMob[tmob].WaitSec = Type == 0 ? 10 : 40;
	pMob[tmob].LastTime = CurrentTime;

	pMob[tmob].GetCurrentScore(MAX_USER);
	pMob[tmob].MOB.CurrentScore.Hp = pMob[tmob].MOB.CurrentScore.MaxHp;

	if (NewbieEventServer && pMob[tmob].MOB.CurrentScore.Level < 120)
		pMob[tmob].MOB.CurrentScore.Hp = 3 * pMob[tmob].MOB.CurrentScore.Hp / 4;

	pMob[tmob].SegmentProgress = Type == 0 ? 0 : 4;

	memset(pMob[tmob].EnemyList, 0, sizeof(pMob[tmob].EnemyList));

	int px = pMob[tmob].SegmentListX[0];
	int py = pMob[tmob].SegmentListY[0];

	int mobslot = GetEmptyMobGrid(tmob, &px, &py);

	if (mobslot == 0)
	{
		sprintf(temp, "err,No empty mobgrid:%s x:%d y:%d", MobName, px, py);
		Log(temp, "-system", 0);

		pMob[tmob].Mode = 0;
		pMob[tmob].MOB.MobName[0] = 0;
		pMob[tmob].GenerateIndex = -1;

		return -2;
	}

	pMob[tmob].SegmentX = px;
	pMob[tmob].TargetX = px;
	pMob[tmob].LastX = px;
	pMob[tmob].SegmentY = py;
	pMob[tmob].TargetY = py;
	pMob[tmob].LastY = py;

	int skillnum = pMob[tmob].MOB.BaseScore.MaxMp;

	if (skillnum)
	{
		SetAffect(tmob, skillnum, 30000, 200);
		SetTick(tmob, skillnum, 30000, 200);
	}

	MSG_CreateMob sm;
	memset(&sm, 0, sizeof(MSG_CreateMob));

	GetCreateMob(tmob, &sm);

	sm.CreateType |= 2;

	pMobGrid[py][px] = tmob;

	GridMulticast(px, py, (MSG_STANDARD*)&sm, 0);

	return TRUE;
}

/// Spawna mob pertencente a um grupo do NPCGenerator.
void GenerateMob(int index, int PosX, int PosY)
{
	int GenerateMin = mNPCGen.pList[index].MinuteGenerate;

	int nindex = index;

	for (int i = 0; i < MAX_MOB_MERC; i++)
	{
		if (index != pMobMerc[i].GenerateIndex || index == 0)
			continue;

		memcpy(mNPCGen.pList[index].Leader.Carry, pMobMerc[i].Carry, sizeof(mNPCGen.pList[index].Leader.Carry));
		break;
	}

	if (GenerateMin >= 500)
	{
		int segx = mNPCGen.pList[index].SegmentListX[0];
		int segy = mNPCGen.pList[index].SegmentListY[0];

		int res = segx - segy;

		if ((segx - segy) <= 0)
		{
			Log("err,end index < start index", "-system", 0);
			return;
		}

		int reloc = W2Rand() % segx + segy;
		if (reloc < 0 || reloc >= mNPCGen.NumList)
			Log("err,wrong index relocation", "-system", 0);

		index = reloc;
	}

	int Formation = mNPCGen.pList[nindex].Formation;
	int MinGroup = mNPCGen.pList[nindex].MinGroup;

	int qmob = mNPCGen.pList[nindex].MaxGroup - mNPCGen.pList[nindex].MinGroup + 1;

	if (qmob <= 0)
	{
		sprintf(temp, "err,zero divide : %d", index);
		Log(temp, "-system", 0);
		qmob = 1;
	}

	MinGroup += W2Rand() % qmob;

	if (mNPCGen.pList[nindex].CurrentNumMob >= mNPCGen.pList[nindex].MaxNumMob)
		return;

	if (mNPCGen.pList[nindex].CurrentNumMob + MinGroup > mNPCGen.pList[nindex].MaxNumMob)
		MinGroup = mNPCGen.pList[nindex].MaxNumMob - mNPCGen.pList[nindex].CurrentNumMob;

	int tmob = GetEmptyNPCMob();

	if (tmob == 0)
	{
		sprintf(temp, "err,no emptymob leader: %d", index);
		//Log(temp, "-system", 0);

		return;
	}

	if(mNPCGen.pList[index].Leader.Equip[0].sIndex != 219 && mNPCGen.pList[index].SegmentListX[0] >= 2440 && mNPCGen.pList[index].SegmentListX[0] <= 2545 && mNPCGen.pList[index].SegmentListY[0] >= 1845 && mNPCGen.pList[index].SegmentListY[0] <= 1921)
	{
		mNPCGen.pList[index].MinuteGenerate = -1;
		return;
	}

	int tmob2 = tmob;

	memset(&pMob[tmob].PartyList, 0, sizeof(pMob[tmob].PartyList));
	memcpy(&pMob[tmob].MOB, &mNPCGen.pList[nindex].Leader, sizeof(STRUCT_MOB));

	pMob[tmob].MOB.MobName[NAME_LENGTH - 1] = 0;
	pMob[tmob].MOB.BaseScore.Merchant = 0;

	strncpy(pMob[tmob].MOB.MobName, mNPCGen.pList[nindex].Leader.MobName, NAME_LENGTH);

	if(strcmp(pMob[tmob].MOB.MobName, mNPCGen.pList[nindex].Leader.MobName) != 0)
		memcpy(pMob[tmob].MOB.MobName, mNPCGen.pList[nindex].Leader.MobName, NAME_LENGTH);

	for (int i = 0; i < NAME_LENGTH; i++)
	{
		if (pMob[tmob].MOB.MobName[i] == 95)
			pMob[tmob].MOB.MobName[i] = 32;

		if (pMob[tmob].MOB.MobName[i] == 64)
			pMob[tmob].MOB.MobName[i] = 32;
	}
	memset(pMob[tmob].Affect, 0, sizeof(pMob[tmob].Affect));

	for (int i = 0; i < MAX_SEGMENT; i++)
	{
		if (mNPCGen.pList[index].SegmentListX[i])
		{
			if (mNPCGen.pList[index].SegmentRange[i])
			{
				pMob[tmob].SegmentListX[i] = mNPCGen.pList[index].SegmentListX[i]
					- mNPCGen.pList[index].SegmentRange[i] + (W2Rand() % (mNPCGen.pList[index].SegmentRange[i] + 1));

				pMob[tmob].SegmentListY[i] = mNPCGen.pList[index].SegmentListY[i]
					- mNPCGen.pList[index].SegmentRange[i] + (W2Rand() % (mNPCGen.pList[index].SegmentRange[i] + 1));
			}
			else
			{
				pMob[tmob].SegmentListX[i] = mNPCGen.pList[index].SegmentListX[i];
				pMob[tmob].SegmentListY[i] = mNPCGen.pList[index].SegmentListY[i];
			}
			pMob[tmob].SegmentWait[i] = mNPCGen.pList[index].SegmentWait[i];
		}
		else
		{
			pMob[tmob].SegmentListX[i] = 0;
			pMob[tmob].SegmentListY[i] = 0;
		}
	}

	if (PosX && PosY)
	{
		for (int i = 0; i < MAX_SEGMENT; i++)
		{
			pMob[tmob].SegmentListX[i] = PosX;
			pMob[tmob].SegmentListY[i] = PosY;
		}
	}

	if (pMob[tmob].MOB.Equip[0].sIndex == 220 || pMob[tmob].MOB.Equip[0].sIndex == 219)
	{
		int br = BrState;
		if (BrState > 0 && br < MAX_USER)
		{
			if (pUser[br].Mode == USER_PLAY && RvRState == 0 && GTorreState == 0)
			{
				*(int*)&pMob[tmob].MOB.Equip[15] = *(int*)&pMob[br].MOB.Equip[15];
				*(int*)(&pMob[tmob].MOB.Equip[15] + 4) = *(int*)(&pMob[br].MOB.Equip[15] + 4);

				if (pMob[tmob].MOB.Equip[0].sIndex == 219)
				{
					pMob[tmob].MOB.Guild = pMob[br].MOB.Guild;

					GetGuild(tmob);

					if (pMob[br].MOB.GuildLevel == 9)
						pMob[tmob].MOB.GuildLevel = 0;
				}
			}
		}
	}

	CWarTower::GGenerateMob(index, PosX, PosY, tmob);

	pMob[tmob].GenerateIndex = index;

	pMob[tmob].Formation = mNPCGen.pList[nindex].Formation;

	pMob[tmob].RouteType = mNPCGen.pList[nindex].RouteType;
	pMob[tmob].Mode = MOB_PEACE;

	pMob[tmob].SegmentDirection = 0;

	pMob[tmob].Leader = 0;
	pMob[tmob].WaitSec = pMob[tmob].SegmentWait[0];
	pMob[tmob].LastTime = CurrentTime;

	pMob[tmob].GetCurrentScore(MAX_USER);
	pMob[tmob].MOB.CurrentScore.Hp = pMob[tmob].MOB.CurrentScore.MaxHp;

	if (NewbieEventServer && pMob[tmob].MOB.CurrentScore.Level < 120)
		pMob[tmob].MOB.CurrentScore.Hp = 3 * pMob[tmob].MOB.CurrentScore.Hp / 4;

	pMob[tmob].SegmentProgress = 0;

	memset(pMob[tmob].EnemyList, 0, sizeof(pMob[tmob].EnemyList));

	if (pMob[tmob].MOB.Clan == 1 && W2Rand() % 10 == 1)
		pMob[tmob].MOB.Clan = 2;

	int mobslot = GetEmptyMobGrid(tmob, &pMob[tmob].SegmentListX[0], &pMob[tmob].SegmentListY[0]);

	if (mobslot == 0)
	{
		sprintf(temp, "err,No empty mobgrid(1): %d %d %d", index, pMob[tmob].SegmentListX[0], pMob[tmob].SegmentListY[0]);
		Log(temp, "-system", 0);

		pMob[tmob].Mode = 0;
		pMob[tmob].MOB.MobName[0] = 0;
		pMob[tmob].GenerateIndex = -1;

		return;
	}

	//Imposto
	if(pMob[tmob].MOB.Merchant == 6 && tmob >= MAX_USER)
	{
		int level = pMob[tmob].MOB.CurrentScore.Level;

		GuildImpostoID[level] = tmob;
		pMob[tmob].MOB.Guild = g_pGuildZone[level].ChargeGuild;
	}


	pMob[tmob].SegmentX = pMob[tmob].SegmentListX[0];
	pMob[tmob].TargetX = pMob[tmob].SegmentListX[0];
	pMob[tmob].LastX = pMob[tmob].SegmentListX[0];
	pMob[tmob].SegmentY = pMob[tmob].SegmentListY[0];
	pMob[tmob].TargetY = pMob[tmob].SegmentListY[0];
	pMob[tmob].LastY = pMob[tmob].SegmentListY[0];

	if (mNPCGen.pList[nindex].CurrentNumMob < 0)
		mNPCGen.pList[nindex].CurrentNumMob = 0;

	mNPCGen.pList[nindex].CurrentNumMob++;

	int skillnum = mNPCGen.pList[nindex].Leader.BaseScore.MaxMp;

	if (skillnum)
	{
		SetAffect(tmob, skillnum, 30000, 200);
		SetTick(tmob, skillnum, 30000, 200);
	}

	MSG_CreateMob sm;
	memset(&sm, 0, sizeof(MSG_CreateMob));
	GetCreateMob(tmob, &sm);

	sm.CreateType |= 2;

	pMobGrid[pMob[tmob].SegmentListY[0]][pMob[tmob].SegmentListX[0]] = tmob;

	GridMulticast(pMob[tmob].SegmentListX[0], pMob[tmob].SegmentListY[0], (MSG_STANDARD*)&sm, 0);

	for (int i = 0; i < MinGroup && i < MAX_PARTY; i++)
	{
		int tempmob = GetEmptyNPCMob();

		if (tempmob == 0)
		{
			pMob[tmob2].PartyList[i] = 0;

			sprintf(temp, "err,no empty mob: %d", index);

			Log(temp, "-system", 0);
			continue;
		}

		memset(&pMob[tempmob].PartyList, 0, sizeof(pMob[tempmob].PartyList));

		pMob[tmob2].PartyList[i] = tempmob;

		memcpy(&pMob[tempmob], &mNPCGen.pList[nindex].Follower, sizeof(STRUCT_MOB));

		pMob[tempmob].MOB.BaseScore.Merchant = 0;

		strncpy(pMob[tempmob].MOB.MobName, mNPCGen.pList[nindex].Follower.MobName, NAME_LENGTH);

		for (int j = 0; j < NAME_LENGTH; ++j)
		{
			if (pMob[tempmob].MOB.MobName[j] == 95)
				pMob[tempmob].MOB.MobName[j] = 32;

			if (pMob[tempmob].MOB.MobName[j] == '@')
				pMob[tempmob].MOB.MobName[j] = 0;
		}
		memset(pMob[tempmob].Affect, 0, sizeof(pMob[tempmob].Affect));

		for (int j = 0; j < MAX_SEGMENT; j++)
		{
			if (mNPCGen.pList[index].SegmentRange[j])
			{
				pMob[tempmob].SegmentListX[j] = g_pFormation[i][j][Formation] + pMob[tmob].SegmentListX[j];

				pMob[tempmob].SegmentListY[j] = g_pFormation[i][j][Formation] + pMob[tmob].SegmentListY[j];


				pMob[tempmob].SegmentWait[j] = mNPCGen.pList[index].SegmentWait[j];
			}
			else
			{
				pMob[tempmob].SegmentListX[j] = mNPCGen.pList[index].SegmentListX[j];
				pMob[tempmob].SegmentListY[j] = mNPCGen.pList[index].SegmentListY[j];
			}
		}
		if (PosX && PosY)
		{
			for (int k = 0; k < MAX_SEGMENT; k++)
			{
				pMob[tempmob].SegmentListX[k] = PosX + W2Rand() % 5 - 2;
				pMob[tempmob].SegmentListY[k] = PosY + W2Rand() % 5 - 2;
			}
		}

		pMob[tempmob].GenerateIndex = index;

		pMob[tempmob].Formation = mNPCGen.pList[nindex].Formation;

		pMob[tempmob].RouteType = mNPCGen.pList[nindex].RouteType;
		pMob[tempmob].Mode = MOB_PEACE;

		pMob[tempmob].SegmentProgress = 0;

		pMob[tempmob].Leader = tmob2;
		pMob[tempmob].LastTime = CurrentTime;

		pMob[tempmob].GetCurrentScore(MAX_USER);
		pMob[tempmob].MOB.CurrentScore.Hp = pMob[tempmob].MOB.CurrentScore.MaxHp;

		if (NewbieEventServer && pMob[tempmob].MOB.CurrentScore.Level < 120)
			pMob[tempmob].MOB.CurrentScore.Hp = 3 * pMob[tempmob].MOB.CurrentScore.Hp / 4;

		pMob[tempmob].WaitSec = pMob[tempmob].SegmentWait[0];
		pMob[tempmob].SegmentDirection = 0;

		memset(pMob[tempmob].EnemyList, 0, sizeof(pMob[tempmob].EnemyList));

		if (pMob[tempmob].MOB.Clan == 1 && W2Rand() % 10 == 1)
			pMob[tempmob].MOB.Clan = 2;

		int mslot = GetEmptyMobGrid(tempmob, &pMob[tempmob].SegmentListX[0], &pMob[tempmob].SegmentListY[0]);

		if (mslot == 0)
		{
			sprintf(temp, "err,No empty mobgrid: %d %d %d", index, pMob[tempmob].SegmentListX[0], pMob[tempmob].SegmentListY[0]);
			Log(temp, "-system", 0);

			pMob[tempmob].Mode = 0;
			pMob[tempmob].MOB.MobName[0] = 0;
			pMob[tempmob].GenerateIndex = -1;
			pMob[tmob2].PartyList[i] = 0;

			return;
		}


		pMob[tempmob].SegmentX = pMob[tempmob].SegmentListX[0];
		pMob[tempmob].TargetX = pMob[tempmob].SegmentListX[0];
		pMob[tempmob].LastX = pMob[tempmob].SegmentListX[0];
		pMob[tempmob].SegmentY = pMob[tempmob].SegmentListY[0];
		pMob[tempmob].TargetY = pMob[tempmob].SegmentListY[0];
		pMob[tempmob].LastY = pMob[tempmob].SegmentListY[0];

		skillnum = mNPCGen.pList[nindex].Leader.BaseScore.MaxMp;

		if (skillnum)
		{
			SetAffect(tempmob, skillnum, 30000, 200);
			SetTick(tempmob, skillnum, 30000, 200);
		}

		MSG_CreateMob sm2;
		memset(&sm2, 0, sizeof(MSG_CreateMob));
		GetCreateMob(tempmob, &sm2);

		sm2.CreateType |= 2;

		pMobGrid[pMob[tempmob].SegmentListY[0]][pMob[tempmob].SegmentListX[0]] = tempmob;

		GridMulticast(pMob[tempmob].SegmentListX[0], pMob[tempmob].SegmentListY[0], (MSG_STANDARD*)&sm2, 0);

		if (mNPCGen.pList[nindex].CurrentNumMob < 0)
			mNPCGen.pList[nindex].CurrentNumMob = 0;

		mNPCGen.pList[nindex].CurrentNumMob++;
	}
}

/// Reconstroi tabelas de geracao e respawns do NPCGenerator.
void RebuildGenerator()
{
	for (int i = 0; i < mNPCGen.NumList; i++)
		mNPCGen.NumOld[i] = 0;

	for (int i = MAX_USER; i < MAX_MOB; i++)
	{
		if (pMob[i].Mode == 0)
			continue;

		int GenerateIndex = pMob[i].GenerateIndex;

		if (GenerateIndex >= 0 && GenerateIndex < mNPCGen.NumList)
			mNPCGen.NumOld[GenerateIndex]++;
	}

	for (int i = MAX_USER; i < mNPCGen.NumList; i++)
	{
		if (mNPCGen.pList[i].CurrentNumMob == mNPCGen.NumOld[i])
			continue;

		sprintf(temp, "Idx:%d Name:%s Old:%d New:%d", i, mNPCGen.pList[i].Leader.MobName, mNPCGen.pList[i].CurrentNumMob, mNPCGen.NumOld[i]);

		Log(temp, "-system", 0);

		mNPCGen.pList[i].CurrentNumMob = mNPCGen.NumOld[i];
	}
}

/// Le arquivo binario de definicao de mob para STRUCT_MOB.
int  ReadMob(STRUCT_MOB *mob, char *dir)
{
	sprintf(temp, "./%s/%s", dir, mob->MobName);

	int Handle = open(temp, O_RDONLY);

	if (Handle == -1)
	{
		if (errno == EINVAL)
			sprintf(temp, "err,ReadMob EEXIST %s", mob->MobName);

		else if (errno == EMFILE)
			sprintf(temp, "err,ReadMob EMFILE %s", mob->MobName);

		else if (errno == ENOENT)
			sprintf(temp, "err,ReadMob ENOENTN %s", mob->MobName);

		else
			sprintf(temp, "err,ReadMob UNKNOW %s", mob->MobName);

		Log(temp, "-system", 0);
		return FALSE;
	}

	strncpy(temp, mob->MobName, NAME_LENGTH);

	int ret = read(Handle, mob, sizeof(STRUCT_MOB));

	if (ret == -1)
	{
		close(Handle);

		return FALSE;
	}

	close(Handle);

	strncpy(mob->MobName, temp, NAME_LENGTH);

	BASE_GetEnglish(mob->MobName);

	mob->BaseScore = mob->CurrentScore;

	return TRUE;
}

/// Remove um mob/NPC do mundo e libera seus grids.
void DeleteMob(int conn, int Type)
{
	MSG_RemoveMob sm;
	memset(&sm, 0, sizeof(MSG_RemoveMob));

	sm.Type = _MSG_RemoveMob;
	sm.Size = sizeof(MSG_RemoveMob);
	sm.ID = conn;
	sm.RemoveType = Type;

	GridMulticast(pMob[conn].TargetX, pMob[conn].TargetY, (MSG_STANDARD*)&sm, conn);

	if (Type != 0)
	{
		if (conn >= MAX_USER)
		{
			int geneidx = pMob[conn].GenerateIndex;

			if (geneidx >= 0 && geneidx < MAX_NPCGENERATOR)
			{
				mNPCGen.pList[geneidx].CurrentNumMob--;

				if (mNPCGen.pList[geneidx].CurrentNumMob < 0)
					mNPCGen.pList[geneidx].CurrentNumMob = 0;
			}
		}
	
		pMob[conn].MOB.CurrentScore.Hp = 0;
		pMob[conn].Mode = 0;
		pMobGrid[pMob[conn].TargetY][pMob[conn].TargetX] = 0;

		RemoveParty(conn);
	}

}

/// Define alvo de combate e estado de agro para um mob.
void SetBattle(int mob, int target)
{
	if (mob <= 0 || target <= 0 || mob >= MAX_MOB || target >= MAX_MOB)
		return;

	if (pMob[mob].Mode == MOB_EMPTY)
		return;

	if (pMob[target].Mode == MOB_EMPTY)
		return;

	if(mob == target)
		return;

	if (mob >= MAX_USER && (target >= MAX_USER ||pUser[target].Mode == USER_PLAY) )
	{
		if (pMob[mob].TargetX >= pMob[target].TargetX - 23 && pMob[mob].TargetX <= pMob[target].TargetX + 23 && pMob[mob].TargetY >= pMob[target].TargetY - 23 && pMob[mob].TargetY <= pMob[target].TargetY + 23)
		{
			pMob[mob].Mode = MOB_COMBAT;

			pMob[mob].AddEnemyList(target);

			

			int generateindex = pMob[mob].GenerateIndex;
			int say = W2Rand() % 4;

			if (generateindex < 0 || generateindex >= MAX_NPCGENERATOR)
				return;

			if (mNPCGen.pList[generateindex].FightAction[say][0] && !pMob[mob].Leader)
				SendChat(mob, mNPCGen.pList[generateindex].FightAction[say]);
		}
	}
}

/// Executa logica de ataque de mob/NPC (fisico, skill ou area).
void MobAttack(int attacker, MSG_Attack sm)
{
	int _rd = W2Rand()%100;

	if(sm.SkillIndex == 109)
	{
		if(_rd < 40)
			sm.SkillIndex = 110;

		else if(_rd < 80)
			sm.SkillIndex = 111;
	}

	for(int i = 0; i < MAX_TARGET; i++)
	{
		int skillbase = sm.SkillIndex;
		int Target = sm.Dam[i].TargetID;

		if (Target <= 0 || Target >= MAX_MOB)
			continue;

		if (pMob[Target].MOB.CurrentScore.Hp <= 0)
			continue;

		sm.FlagLocal = 0;

		Target = sm.Dam[i].TargetID;

		if(skillbase != 109)
			sm.Dam[i].Damage = 500 + W2Rand()%200;

		sm.SkillIndex = skillbase;

		int skill = sm.SkillIndex;

		if (skill >= 0 && skill < MAX_SKILLINDEX && sm.SkillParm == 0)
		{
			if (skill == 33)
				sm.Motion = 253;

			int kind = ((skill % MAX_SKILL / 8) + 1);

			if (kind >= 1 && kind <= 3)
			{
				int special = pMob[attacker].MOB.CurrentScore.Special[kind];
						
				int NeedUpdate = 0;
						
				if (SetAffect(Target, skill, 100, special) != FALSE)
					NeedUpdate = 1;

				if (SetTick(Target, skill, 100, special) != FALSE)
					NeedUpdate = 1;

				if (NeedUpdate)
					SendScore(Target);
			}
		}
		if (Target < MAX_USER && sm.Dam[i].Damage > 0)
		{
			int attackdex = pMob[attacker].MOB.CurrentScore.Dex / 5;

			if (pMob[attacker].MOB.LearnedSkill[0] & 0x1000000)
				attackdex += 100;

			if (pMob[attacker].MOB.Rsv & 0x40)
				attackdex += 500;

			int parryretn = GetParryRate(&pMob[Target].MOB, pMob[Target].Parry, attackdex, pMob[attacker].MOB.Rsv);

			if (sm.SkillIndex == 79 || sm.SkillIndex == 22)
				parryretn = 30 * parryretn / 100;

			int rd = W2Rand() % 1000 + 1;

			if (rd < parryretn)
			{
				sm.Dam[i].Damage = -3;

				if ((pMob[Target].MOB.Rsv & 0x200) != 0 && rd < 100)
					sm.Dam[i].Damage = -4;
			}

			else if (pMob[attacker].MOB.Clan == 4)
				sm.Dam[i].Damage = (2 * sm.Dam[i].Damage) / 5;
		}

		int TargetLeader = pMob[Target].CurrentTarget;
		
		if (sm.Dam[i].Damage > 0)
		{
			if (TargetLeader <= MOB_EMPTY)
				TargetLeader = Target;

			SetBattle(TargetLeader, attacker);

			if (pMob[attacker].MOB.Clan != 4)
				SetBattle(attacker, TargetLeader);

			for (int z = 0; z < MAX_PARTY; z++)
			{
				int partyconn = pMob[TargetLeader].PartyList[z];
				if (partyconn <= MAX_USER)
					continue;

				if (pMob[partyconn].Mode == MOB_EMPTY || pMob[partyconn].MOB.CurrentScore.Hp <= 0)
				{
					if (pMob[partyconn].Mode != MOB_EMPTY)
					{
						pMob[partyconn].MOB.CurrentScore.Hp = 0;
						DeleteMob(partyconn, 1);
					}

					pMob[TargetLeader].PartyList[attacker] = MOB_EMPTY;
					continue;
				}

				SetBattle(partyconn, attacker);
				SetBattle(attacker, partyconn);
			}

			int Summoner = pMob[attacker].Summoner;

			if (pMob[attacker].MOB.Clan == 4 && Target >= MAX_USER && Summoner > 0 && Summoner < MAX_USER && pUser[Summoner].Mode == USER_PLAY)
			{
				int posX = pMob[Summoner].TargetX;
				int posY = pMob[Summoner].TargetY;
				int num = 46;
				int dam = 0;
				int exp = 0;
				int setDam = 0;

				if (pMob[attacker].TargetX > posX - num && pMob[attacker].TargetX < posX + num && pMob[attacker].TargetY > posY - num && pMob[attacker].TargetY < posY + num)
				{
					if (pMob[Target].MOB.CurrentScore.Hp < sm.Dam[i].Damage)
						setDam = pMob[Target].MOB.CurrentScore.Hp;
					else
						setDam = sm.Dam[i].Damage;

					dam = setDam;
					exp = (int)(pMob[Target].MOB.Exp * dam / pMob[Target].MOB.CurrentScore.MaxHp);

					if (pMob[Summoner].MOB.CurrentScore.Level == pMob[attacker].MOB.CurrentScore.Level)
					{
						exp = GetExpApply(pMob[attacker].extra, exp, pMob[attacker].MOB.CurrentScore.Level, pMob[Target].MOB.CurrentScore.Level);

						if (exp > 12000)
							exp = 12000;

						if (pMob[Target].MOB.Clan == 4)
							exp = 0;

						pMob[Summoner].MOB.Exp = pMob[Summoner].MOB.Exp + exp;
					}
				}
			}
		}

		if (sm.Dam[i].Damage > 0 || sm.Dam[i].Damage <= -5)
		{
			int DamageNow = sm.Dam[i].Damage;
			int damage = 0;
			int mountindex = pMob[Target].MOB.Equip[14].sIndex;

			if (Target < MAX_USER && mountindex >= 2360 && mountindex < 2390 && pMob[Target].MOB.Equip[14].stEffect[0].sValue > 0)
			{
				DamageNow = (3 * sm.Dam[i].Damage) / 4;
				damage = sm.Dam[i].Damage - DamageNow;

				if (DamageNow <= 0)
					DamageNow = 1;

				sm.Dam[i].Damage = DamageNow;
			}

			for (int c = 0; c < MAX_AFFECT; c++)
			{
				if (pMob[Target].Affect[c].Type == 18)
				{
					if (pMob[Target].MOB.CurrentScore.Mp >((pMob[Target].MOB.CurrentScore.MaxMp / 100) * 10))
					{
						int mana = pMob[Target].MOB.CurrentScore.Mp - (DamageNow >> 1);

						if (mana < 0)
						{
							DamageNow -= mana;
							mana = 0;
						}

						pMob[Target].MOB.CurrentScore.Mp = mana;

						pUser[Target].ReqMp = pUser[Target].ReqMp - (DamageNow >> 1);
						SetReqMp(Target);

						DamageNow = ((DamageNow >> 1) + (DamageNow << 4)) / 80;
						sm.Dam[i].Damage = DamageNow;
					}
				}
			}

			if (pMob[Target].MOB.Equip[13].sIndex == 786)
			{
				int sanc = pMob[Target].MOB.Equip[13].stEffect[0].cValue;
				
				if (sanc < 2)
					sanc = 2;

				pMob[Target].MOB.CurrentScore.Hp = pMob[Target].MOB.CurrentScore.Hp - ((sm.Dam[i].Damage) / sanc);
			}
			else if (pMob[Target].MOB.Equip[13].sIndex == 1936)
			{
				int sanc = pMob[Target].MOB.Equip[13].stEffect[0].cValue;
				
				if (sanc < 2)
					sanc = 2;

				sanc *= 10;

				pMob[Target].MOB.CurrentScore.Hp = pMob[Target].MOB.CurrentScore.Hp - ((sm.Dam[i].Damage) / sanc);
			}
			else if (pMob[Target].MOB.Equip[13].sIndex == 1937)
			{
				int sanc = pMob[Target].MOB.Equip[13].stEffect[0].cValue;
				
				if (sanc < 2)
					sanc = 2;

				sanc *= 1000;

				pMob[Target].MOB.CurrentScore.Hp = pMob[Target].MOB.CurrentScore.Hp - ((sm.Dam[i].Damage) / sanc);
			}
			else
			{

				if (pMob[Target].MOB.CurrentScore.Hp < sm.Dam[i].Damage)
					pMob[Target].MOB.CurrentScore.Hp = 0;

				else
					pMob[Target].MOB.CurrentScore.Hp = pMob[Target].MOB.CurrentScore.Hp - sm.Dam[i].Damage;

			}

			if(Target > MAX_USER)
				SendScore(Target);

			if (Target > MAX_USER && pMob[Target].MOB.Clan == 4)
				LinkMountHp(Target);

			if (damage > 0)
				ProcessAdultMount(Target, damage);
		}

		GridMulticast(pMob[Target].TargetX, pMob[Target].TargetY, (MSG_STANDARD*)&sm, 0);

		if (Target > 0 && Target < MAX_USER)
		{
			if (pUser[Target].ReqHp < sm.Dam[i].Damage)
				pUser[Target].ReqHp = 0;

			else
				pUser[Target].ReqHp = pUser[Target].ReqHp - sm.Dam[i].Damage;

			if(sm.SkillIndex == 109)
			{
				pMob[Target].MOB.CurrentScore.Hp = 0;
				pUser[Target].ReqHp = 0;
			}

			SetReqHp(Target);
			SendSetHpMp(Target);
		}

		if (pMob[Target].MOB.CurrentScore.Hp <= 0)
		{
			pMob[Target].MOB.CurrentScore.Hp = 0;
					
			MobKilled(Target, attacker, 0, 0);
		}
	}
}

/// Faz parse de string de configuracao de mercenarios/mobs especiais.
int  ParseMobMercString(int Num, char *str)
{
	char str1[128];
	str1[0] = 0;

	char str2[128];
	str2[0] = 0;

	char str3[128];
	str3[0] = 0;

	int  value = 0;
	int secondvalue = 0;

	sscanf(str, "%s %s %s", str1, str2, str3);

	if (str[0] == 47)
		return FALSE;

	if (str[0] == 0)
		return FALSE;

	STRUCT_ITEM Item;
	memset(&Item, 0, sizeof(STRUCT_ITEM));

	char item_str1[256];
	char item_str2[256];
	char item_str3[256];
	char item_str4[256];
	char item_str5[256];
	char item_str6[256];
	char item_str7[256];
	char item_str8[256];

	int item_value[8] = { 0, 0, 0, 0, 0, 0, 0 , 0};

	sscanf(str2, "%s %s %s %s %s %s %s - %s", item_str1, item_str2, item_str3, item_str4, item_str5, item_str6, item_str7, item_str8);
	item_value[0] = atoi(item_str1);
	item_value[1] = atoi(item_str2);
	item_value[2] = atoi(item_str3);
	item_value[3] = atoi(item_str4);
	item_value[4] = atoi(item_str5);
	item_value[5] = atoi(item_str6);
	item_value[6] = atoi(item_str7);
	item_value[7] = atoi(item_str8);

	value = atoi(str2);
	secondvalue = atoi(str3);

	if (strlen(str2) > 80)
		return FALSE;

	W2StrUpper(str1);

	if (!strcmp(str1, "MOBNAME:"))
		strncpy(pMobMerc[Num].MobName, str2, NAME_LENGTH);

	else if (!strcmp(str1, "GENERATEINDEX:"))
		pMobMerc[Num].GenerateIndex = value;

	else if (!strcmp(str1, "RENEWTIME:"))
		pMobMerc[Num].RenewTime = value * 2;

	else if (!strcmp(str1, "REBORNTIME:"))
		pMobMerc[Num].RebornTime = value * 2;

	else
	{
		for (int i = 0; i < MAX_CARRY; i++)
		{
			sprintf(temp, "ITEM_%d:", i);

			if (!strcmp(str1, temp))
			{
				pMobMerc[Num].Carry[i].sIndex = item_value[0];
				pMobMerc[Num].Carry[i].stEffect[0].cEffect = item_value[1];
				pMobMerc[Num].Carry[i].stEffect[0].cValue = item_value[2];
				pMobMerc[Num].Carry[i].stEffect[1].cEffect = item_value[3];
				pMobMerc[Num].Carry[i].stEffect[1].cValue = item_value[4];
				pMobMerc[Num].Carry[i].stEffect[2].cEffect = item_value[5];
				pMobMerc[Num].Carry[i].stEffect[2].cValue = item_value[6];

				pMobMerc[Num].MaxStock[i] = item_value[7];
				pMobMerc[Num].Stock[i] = item_value[7];

				return TRUE;
			}
		}
	}
	return TRUE;
}

/// Remove mobs pertencentes a uma entrada do NPCGenerator.
void DeleteGenerateMob(int generate)
{
	for (int i = MAX_USER; i < MAX_MOB; i++)
	{
		if (pMob[i].Mode == MOB_EMPTY)
			continue;

		if (pMob[i].GenerateIndex != generate)
			continue;

		DeleteMob(i, 3);
	}
}
