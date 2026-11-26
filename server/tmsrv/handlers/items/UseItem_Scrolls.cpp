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
*/

/// @file UseItem_Scrolls.cpp
/// @brief Sub-handler de retornos, portais, montarias, pergaminhos da água, Sephira e consumíveis temporários.

#include "handlers/items/UseItemInternal.h"

bool UseItem_MountsAndScrolls(int conn, MSG_UseItem* m, STRUCT_ITEM* item, int Vol, int amount)
{
#pragma region Retorno
	if (Vol == 11)
	{
		DoRecall(conn);

		if (amount > 1)
			BASE_SetItemAmount(item, amount - 1);

		else
			memset(item, 0, sizeof(STRUCT_ITEM));

		sprintf(temp, "useitem,recall");
		ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);
		return true;
	}
#pragma endregion
#pragma region Gema Estelar
	if (Vol == 12)
	{
		unsigned char map_att = GetAttribute(pMob[conn].TargetX, pMob[conn].TargetY);

		if((pMob[conn].TargetX/128) == 9 && (pMob[conn].TargetY/128) == 1)
			goto CanSave;

		if((pMob[conn].TargetX/128) == 8 && (pMob[conn].TargetY/128) == 2)
			goto CanSave;

		if (map_att & 4 && pMob[conn].MOB.CurrentScore.Level < 1000)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_Cant_Use_That_Here]);
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

							 
		int Arena;
		Arena = BASE_GetArena(pMob[conn].TargetX, pMob[conn].TargetY);
		int Village;
		Village = BASE_GetVillage(pMob[conn].TargetX, pMob[conn].TargetY);

		if (Arena < MAX_GUILDZONE || Village < MAX_GUILDZONE)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_Cant_Use_That_Here]);
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}
	CanSave:
		int sFace = pMob[conn].MOB.Equip[0].sIndex / 10;
							
		if (sFace == 0)
			SendEmotion(conn, 23, 0);

		else if (sFace == 1)
			SendEmotion(conn, 15, 0);

		else if (sFace == 2)
			SendEmotion(conn, 15, 0);

		else if (sFace == 3)
			SendEmotion(conn, 15, 0);
							 

		pMob[conn].MOB.SPX = pMob[conn].TargetX;
		pMob[conn].MOB.SPY = pMob[conn].TargetY;

		sprintf(temp, "useitem,gema salva %d %d", pMob[conn].TargetX, pMob[conn].TargetY);
		ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);

		if (amount > 1)
			BASE_SetItemAmount(item, amount - 1);

		else
			memset(item, 0, sizeof(STRUCT_ITEM));
							 
		SendClientMessage(conn, g_pMessageStringTable[_NN_Set_Warp]);

		return true;
	}
#pragma endregion
#pragma region Portal
	if (Vol == 13)
	{
		if((pMob[conn].MOB.SPX/128) == 9 && (pMob[conn].MOB.SPY/128) == 1 || (pMob[conn].MOB.SPX/128) == 8 && (pMob[conn].MOB.SPY/128) == 2)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_Cant_Use_That_Here]);
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		unsigned char map_att = GetAttribute(pMob[conn].TargetX, pMob[conn].TargetY);

		if (map_att & 4 && pMob[conn].MOB.CurrentScore.Level < 1000)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_Cant_Use_That_Here]);
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		DoTeleport(conn, pMob[conn].MOB.SPX, pMob[conn].MOB.SPY);

		sprintf(temp, "useitem,teleport portal scrool %d %d", pMob[conn].TargetX, pMob[conn].TargetY);
		ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);

		if (amount > 1)
			BASE_SetItemAmount(item, amount - 1);

		else
			memset(item, 0, sizeof(STRUCT_ITEM));

		return true;
	}
#pragma endregion
#pragma region DBRecord
	if (Vol == 14)
	{
		MSG_MessageDBRecord sm_dbrec;
		memset(&sm_dbrec, 0, sizeof(MSG_MessageDBRecord));

		sm_dbrec.Type = _MSG_MessageDBRecord;
		sm_dbrec.ID = 0;
		sm_dbrec.Size = sizeof(MSG_MessageDBRecord);

		sprintf(sm_dbrec.Record, "TCK %d %s %d %d %d %d", ServerIndex, pUser[conn].AccountName, item->stEffect[0].cEffect, item->stEffect[1].cEffect, item->stEffect[2].cEffect, item->sIndex);

		DBServerSocket.SendOneMessage((char*)&sm_dbrec, sizeof(MSG_MessageDBRecord));

		if (amount > 1)
			BASE_SetItemAmount(item, amount - 1);

		else
			memset(item, 0, sizeof(STRUCT_ITEM));

		return true;
	}
#pragma endregion
#pragma region Rações
	if (Vol == 15)
	{
		if (m->DestType || m->DestPos != 14)
		{
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		STRUCT_ITEM *dest = &pMob[conn].MOB.Equip[14];

		if (dest->sIndex < 2330 || dest->sIndex >= 2390)
		{
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		int mount = (dest->sIndex - 2330) % 30;

		if (mount >= 6 && mount <= 15 || mount == 27)
			mount = 6;

		if (mount == 19)
			mount = 7;

		if (mount == 20)
			mount = 8;

		if (mount == 21 || mount == 22 || mount == 23 || mount == 28)
			mount = 9;

		if (mount == 24 || mount == 25 || mount == 26)
			mount = 10;

		if (mount == 29)
			mount = 19;

		int racid = item->sIndex >= 3367 ? item->sIndex - 3367 : item->sIndex - 2420;

		int racao = racid % 30;

		if (mount == racao)
		{
			if (dest->stEffect[0].sValue > 0)
			{
				if ((pMob[conn].MOB.Equip[14].stEffect[0].sValue + 5000) > 30000)
					pMob[conn].MOB.Equip[14].stEffect[0].sValue = 30000;
				else
					pMob[conn].MOB.Equip[14].stEffect[0].sValue += 5000;

				int _racao = dest->stEffect[2].cEffect + 2;

				if (_racao > 100)
					_racao = 100;

				dest->stEffect[2].cEffect = _racao;

				if (dest->sIndex >= 2330 && dest->sIndex < 2360)
					MountProcess(conn, 0);

				if (dest->sIndex >= 2360 && dest->sIndex < 2390)
					ProcessAdultMount(conn, 0);

				if (amount > 1)
					BASE_SetItemAmount(item, amount - 1);

				else
					memset(item, 0, sizeof(STRUCT_ITEM));
									 
				SendClientSignalParm(conn, ESCENE_FIELD, _MSG_SoundEffect, 270);
				SendItem(conn, m->DestType, m->DestPos, dest);
			}
			else
				SendItem(conn, m->SourType, m->SourPos, item);
		}
		else
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_Mount_Not_Match]);
			SendItem(conn, m->SourType, m->SourPos, item);
		}

		sprintf(temp, "useitem,ração mount:%d", mount);
		ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);
		return true;
	}
#pragma endregion
#pragma region Âmago
	if (Vol == 16)
	{
		if (m->DestType || m->DestPos != 14)
		{
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		STRUCT_ITEM *dest = &pMob[conn].MOB.Equip[14];

		if (dest->sIndex < 2330 || dest->sIndex >= 2390 || dest->stEffect[0].sValue <= 0)
		{
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		int mountIndex = (dest->sIndex - 2330) % 30;
		int amgIndex = (item->sIndex - 2390) % 30;

		if(mountIndex == 28)//Sleipnir
			mountIndex = 21;

		if(mountIndex == 27)//Svadilfire
			mountIndex = 10;

		if (mountIndex != amgIndex)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_Mount_Not_Match]);
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		dest->stEffect[0].sValue = 20000;
		dest->stEffect[2].cEffect = 100;
		int level = dest->stEffect[1].cEffect;

		if (level >= 120 && dest->sIndex >= 2360 && dest->sIndex < 2390)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_Cant_Upgrade_More]);
			SendItem(conn, m->SourType, m->SourPos, item);
			ProcessAdultMount(conn, 0);
			return true;
		}

		if (dest->sIndex >= 2360 && dest->sIndex < 2390)
		{
			int rate = BASE_GetGrowthRate(dest);
			int _rand = rand() % 101;
								
			if (_rand > rate)
			{
				if (pMob[conn].MOB.Equip[0].sIndex / 10)
					SendEmotion(conn, 15, 0);
				else
					SendEmotion(conn, 20, 0);
									 
				sprintf(temp, "%s", g_pMessageStringTable[_NN_Fail_To_Refine]);
				SendClientMessage(conn, temp);
									 
				sprintf(temp, "useitem,mount refine fail %d+%d (%d,%d,%d)", dest->sIndex, dest->stEffect[1].cEffect,
					item->stEffect[0].cEffect, item->stEffect[1].cEffect, item->stEffect[2].cEffect);
									 
				ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);
									 
				if (amount > 1)
					BASE_SetItemAmount(item, amount-1);
									 
				else
					memset(item, 0, 8);

				if (!(rand() % 5) && dest->stEffect[1].cEffect > 0)
					dest->stEffect[1].cEffect--;

				ProcessAdultMount(conn, 0);
				return true;
			}
			sprintf(temp, "useitem,mount refine success %d+%d (%d,%d,%d)", dest->sIndex, dest->stEffect[1].cValue,
				item->stEffect[0].cEffect, item->stEffect[1].cEffect, item->stEffect[2].cEffect);

			ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);
		}
		int levelUP = 1;

		if (LOCALSERVER)
			levelUP = 10;

		level = levelUP + level;
		dest->stEffect[1].cEffect = level;
		dest->stEffect[2].cValue = 1;

		if (level >= 25 && dest->sIndex == 2330
			|| level >= 50 && dest->sIndex == 2331
			|| level >= 100 && dest->sIndex >= 2332 && dest->sIndex < 2360)
		{
			dest->sIndex += 30;
								
			dest->stEffect[1].cValue = rand() % 20 + dest->stEffect[1].cEffect;
			dest->stEffect[1].cEffect = 0;
			dest->stEffect[2].cValue = 0;
								 
			SendClientMessage(conn, g_pMessageStringTable[_NN_Mount_Growth]);
			MountProcess(conn, 0);
		}

		SendEmotion(conn, 14, 3);

		if (dest->sIndex >= 2330 && dest->sIndex < 2360)
			MountProcess(conn, 0);

		if (dest->sIndex >= 2360 && dest->sIndex < 2390)
			ProcessAdultMount(conn, 0);

		SendItem(conn, m->DestType, m->DestPos, dest);

		if (amount > 1)
			BASE_SetItemAmount(item, amount - 1);

		else
			memset(item, 0, sizeof(STRUCT_ITEM));

		return true;
	}
#pragma endregion
#pragma region GUILDBOARD
	if (Vol == 17)
	{
		int Guild = BASE_GetGuild(item);

		if (Guild > 0)
		{
			MSG_STANDARDPARM2 sm_gbo;
			memset(&sm_gbo, 0, sizeof(MSG_STANDARDPARM2));

			sm_gbo.Type = _MSG_GuildBoard;
			sm_gbo.ID = ESCENE_FIELD;
			sm_gbo.Parm1 = Guild;
			sm_gbo.Parm2 = GUILDBOARD;
			sm_gbo.Size = sizeof(MSG_STANDARDPARM2);

			pUser[conn].cSock.AddMessage((char*)&sm_gbo, sizeof(MSG_STANDARDPARM2));
		}

		return true;
	}
#pragma endregion
#pragma region Fogos de Artificio
	if (Vol == 19)
	{
		SendEmotion(conn, 100, rand()%5);

		if (amount > 1)
			BASE_SetItemAmount(item, amount - 1);

		else
			memset(item, 0, sizeof(STRUCT_ITEM));

		sprintf(temp, "useitem,fogos_de_artificio");
		ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);
		return true;
	}
#pragma endregion
#pragma region Pergaminho da agua M
	if (Vol >= 21 && Vol <= 30)
	{
		int TargetX = pMob[conn].TargetX;
		int TargetY = pMob[conn].TargetY;

		int inside = 0;

		for (int i = 0; i < 10; i++)
		{
			if (TargetX >= WaterScrollPosition[1][i][0] - 12 && TargetY >= WaterScrollPosition[1][i][1] - 12
				&& TargetX <= WaterScrollPosition[1][i][0] + 12 && TargetY <= WaterScrollPosition[1][i][1] + 12 && i >= 9)
			{
				inside = 1;
				break;
			}

			if (TargetX >= WaterScrollPosition[1][i][0] - 8 && TargetY >= WaterScrollPosition[1][i][1] - 8
				&& TargetX <= WaterScrollPosition[1][i][0] + 8 && TargetY <= WaterScrollPosition[1][i][1] + 8)
			{
				inside = 1;
				break;
			}
		}

		if (((TargetX / 4) != 491 || (TargetY / 4) != 443 || Vol == 29) && inside == 0)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_Cant_Use_That_Here]);
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		if (pMob[conn].Leader != -1 && pMob[conn].Leader)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_Party_Leader_Only]);
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		char UserName[128];

		int Sala = Vol - 21;
		int UserArea = Sala <= 7 ? GetUserInArea(WaterScrollPosition[1][Vol - 21][0] - 8, WaterScrollPosition[1][Vol - 21][1] - 8, WaterScrollPosition[1][Vol - 21][0] + 8, WaterScrollPosition[1][Vol - 21][1] + 8, UserName) : GetUserInArea(WaterScrollPosition[1][Vol - 21][0] - 12, WaterScrollPosition[1][Vol - 21][1] - 12, WaterScrollPosition[1][Vol - 21][0] + 12, WaterScrollPosition[1][Vol - 21][1] + 12, UserName);
							 
		if (UserArea >= 1)
		{
			sprintf(temp, g_pMessageStringTable[_NN_Someone_is_on_quest], UserName, UserArea - 1);
								
			SendClientMessage(conn, temp);
			SendItem(conn, m->SourType, m->SourPos, item);
								 
			return true;
		}

		WaterClear1[1][Vol - 21] = Sala <= 7 ? 30 : 15;

		DoTeleport(conn, WaterScrollPosition[1][Vol - 21][0], WaterScrollPosition[1][Vol - 21][1]);
		SendClientSignalParm(conn, ESCENE_FIELD, _MSG_StartTime, WaterClear1[1][Vol - 21]*2);

		for (int i = 0; i < MAX_PARTY; i++)
		{
			int partyconn = pMob[conn].PartyList[i];

			if (partyconn > 0 && partyconn < MAX_USER && partyconn != conn && pUser[partyconn].Mode == USER_PLAY)
			{
				DoTeleport(partyconn, WaterScrollPosition[1][Vol - 21][0], WaterScrollPosition[1][Vol - 21][1]);
				SendClientSignalParm(partyconn, ESCENE_FIELD, _MSG_StartTime, WaterClear1[1][Vol - 21]*2);
			}
		}

		if (Sala <= 7)
		{
			GenerateMob(Sala + WATER_M_INITIAL, 0, 0);
			GenerateMob(Sala + WATER_M_INITIAL, 0, 0);
			RebuildGenerator();
		}
		else if (Sala == 9)
		{
			int _rand = rand() % 10;
								 
			if (_rand < 4)
				GenerateMob(WATER_M_INITIAL + 8, 0, 0);
								 
			else if (_rand < 5)
				GenerateMob(WATER_M_INITIAL + 9, 0, 0);

			else if (_rand < 6)
				GenerateMob(WATER_M_INITIAL + 10, 0, 0);
								 
			else
				GenerateMob(WATER_M_INITIAL + 11, 0, 0);
		}

		sprintf(temp, "useitem,water scroll M %d", Sala);
		ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);

		if (amount > 1)
			BASE_SetItemAmount(item, amount - 1);

		else
			memset(item, 0, sizeof(STRUCT_ITEM));

		return true;
	}
#pragma endregion
#pragma region Livros Sephira
	if (Vol >= 31 && Vol <= 38)
	{
		int ItemID = item->sIndex;

		if (ItemID <= 0 || ItemID > MAX_ITEMLIST)
		{
			ItemLog("err, wrong skill book index", pUser[conn].AccountName, pUser[conn].IP);
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		int SkillLearn = 1 << (Vol-7);

		if ((pMob[conn].MOB.LearnedSkill & SkillLearn) != 0)
		{
			SendItem(conn, m->SourType, m->SourPos, item);
			SendClientMessage(conn, g_pMessageStringTable[_NN_Already_Learned_It]);
			return true;
		}

		pMob[conn].MOB.LearnedSkill = pMob[conn].MOB.LearnedSkill | SkillLearn;
							 
		sprintf(temp, g_pMessageStringTable[_SN_Learn_Sephera], g_pItemList[ItemID].Name);
							 
		SendClientMessage(conn, temp);
		SendEtc(conn);
							 
		SetAffect(conn, 44, 20, 20);

		if (amount > 1)
			BASE_SetItemAmount(item, amount - 1);

		else
			memset(item, 0, sizeof(STRUCT_ITEM));

		sprintf(temp, "useitem,skillbook index:%d", ItemID);
		ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);
		return true;
	}
#pragma endregion
#pragma region Contratos do reino
	if (Vol >= 41 && Vol < 50)
	{
		int Evock = Vol - 41;

		if (pMob[conn].MOB.Clan != 7 && pMob[conn].MOB.Clan != 8)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_Only_same_kingdom]);
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}
							 
		if (Evock < 5 && pMob[conn].MOB.Clan == 8)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_Only_same_kingdom]);
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}
							 
		if (Evock >= 5 && pMob[conn].MOB.Clan == 7)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_Only_same_kingdom]);
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

							 
		if (CastleState == 0)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_Only_when_castle_war]);
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

							 
		GenerateSummon(conn, Evock + 28, 0, 1);
							 
		if (amount > 1)
			BASE_SetItemAmount(item, amount - 1);

		else
			memset(item, 0, sizeof(STRUCT_ITEM));

		sprintf(temp, "useitem,contrato_de_reino evock:%d", Evock + 28);
		ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);
		return true;
	}
#pragma endregion
#pragma region Pergaminho da agua N
	if (Vol >= 131 && Vol <= 140)
	{
		int TargetX = pMob[conn].TargetX;
		int TargetY = pMob[conn].TargetY;

		int inside = 0;

		for (int i = 0; i < 10; i++)
		{
			if (TargetX >= WaterScrollPosition[0][i][0] - 12 && TargetY >= WaterScrollPosition[0][i][1] - 12
				&& TargetX <= WaterScrollPosition[0][i][0] + 12 && TargetY <= WaterScrollPosition[0][i][1] + 12 && i >= 9)
			{
				inside = 1;
				break;
			}

			if (TargetX >= WaterScrollPosition[0][i][0] - 8 && TargetY >= WaterScrollPosition[0][i][1] - 8
				&& TargetX <= WaterScrollPosition[0][i][0] + 8 && TargetY <= WaterScrollPosition[0][i][1] + 8)
			{
				inside = 1;
				break;
			}
		}

		if (((TargetX / 4) != 491 || (TargetY / 4) != 443 || Vol == 139) && inside == 0)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_Cant_Use_That_Here]);
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		if (pMob[conn].Leader != -1 && pMob[conn].Leader)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_Party_Leader_Only]);
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		char UserName[128];

		int Sala = Vol - 131;
		int UserArea = Sala <= 7 ? GetUserInArea(WaterScrollPosition[0][Vol - 131][0] - 8, WaterScrollPosition[0][Vol - 131][1] - 8, WaterScrollPosition[0][Vol - 131][0] + 8, WaterScrollPosition[0][Vol - 131][1] + 8, UserName) : GetUserInArea(WaterScrollPosition[0][Vol - 131][0] - 12, WaterScrollPosition[0][Vol - 131][1] - 12, WaterScrollPosition[0][Vol - 131][0] + 12, WaterScrollPosition[0][Vol - 131][1] + 12, UserName);

		if (UserArea >= 1)
		{
			sprintf(temp, g_pMessageStringTable[_NN_Someone_is_on_quest], UserName, UserArea - 1);

			SendClientMessage(conn, temp);
			SendItem(conn, m->SourType, m->SourPos, item);

			return true;
		}

		WaterClear1[0][Vol - 131] = Sala <= 7 ? 30 : 15;

		DoTeleport(conn, WaterScrollPosition[0][Vol - 131][0], WaterScrollPosition[0][Vol - 131][1]);
		SendClientSignalParm(conn, ESCENE_FIELD, _MSG_StartTime, WaterClear1[0][Vol - 131] * 2);

		for (int i = 0; i < MAX_PARTY; i++)
		{
			int partyconn = pMob[conn].PartyList[i];

			if (partyconn > 0 && partyconn < MAX_USER && partyconn != conn && pUser[partyconn].Mode == USER_PLAY)
			{
				DoTeleport(partyconn, WaterScrollPosition[0][Vol - 131][0], WaterScrollPosition[0][Vol - 131][1]);
				SendClientSignalParm(partyconn, ESCENE_FIELD, _MSG_StartTime, WaterClear1[0][Vol - 131] * 2);
			}
		}

		if (Sala <= 7)
		{
			GenerateMob(Sala + WATER_N_INITIAL, 0, 0);
			GenerateMob(Sala + WATER_N_INITIAL, 0, 0);
			//RebuildGenerator();
		}
		else if (Sala == 9)
		{
			int _rand = rand() % 10;

			if (_rand < 4)
				GenerateMob(WATER_N_INITIAL + 8, 0, 0);

			else if (_rand < 5)
				GenerateMob(WATER_N_INITIAL + 9, 0, 0);

			else if (_rand < 6)
				GenerateMob(WATER_N_INITIAL + 10, 0, 0);

			else
				GenerateMob(WATER_N_INITIAL + 11, 0, 0);
		}

		sprintf(temp, "useitem,water scroll N %d", Sala);
		ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);

		if (amount > 1)
			BASE_SetItemAmount(item, amount - 1);

		else
			memset(item, 0, sizeof(STRUCT_ITEM));

		return true;
	}
#pragma endregion
#pragma region Pergaminho da agua A
	if (Vol >= 161 && Vol <= 170)
	{
		int TargetX = pMob[conn].TargetX;
		int TargetY = pMob[conn].TargetY;

		int inside = 0;

		for (int i = 0; i < 10; i++)
		{
			if (TargetX >= WaterScrollPosition[2][i][0] - 12 && TargetY >= WaterScrollPosition[2][i][1] - 12
				&& TargetX <= WaterScrollPosition[2][i][0] + 12 && TargetY <= WaterScrollPosition[2][i][1] + 12 && i >= 9)
			{
				inside = 1;
				break;
			}

			if (TargetX >= WaterScrollPosition[2][i][0] - 8 && TargetY >= WaterScrollPosition[2][i][1] - 8
				&& TargetX <= WaterScrollPosition[2][i][0] + 8 && TargetY <= WaterScrollPosition[2][i][1] + 8)
			{
				inside = 1;
				break;
			}
		}

		if (((TargetX / 4) != 491 || (TargetY / 4) != 443 || Vol == 169) && inside == 0)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_Cant_Use_That_Here]);
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		if (pMob[conn].Leader != -1 && pMob[conn].Leader)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_Party_Leader_Only]);
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		char UserName[128];

		int Sala = Vol - 161;
		int UserArea = Sala <= 7 ? GetUserInArea(WaterScrollPosition[2][Vol - 161][0] - 8, WaterScrollPosition[2][Vol - 161][1] - 8, WaterScrollPosition[2][Vol - 161][0] + 8, WaterScrollPosition[2][Vol - 161][1] + 8, UserName) : GetUserInArea(WaterScrollPosition[2][Vol - 161][0] - 12, WaterScrollPosition[2][Vol - 161][1] - 12, WaterScrollPosition[2][Vol - 161][0] + 12, WaterScrollPosition[2][Vol - 161][1] + 12, UserName);

		if (UserArea >= 1)
		{
			sprintf(temp, g_pMessageStringTable[_NN_Someone_is_on_quest], UserName, UserArea - 1);

			SendClientMessage(conn, temp);
			SendItem(conn, m->SourType, m->SourPos, item);

			return true;
		}

		WaterClear1[2][Vol - 161] = Sala <= 7 ? 30 : 15;

		DoTeleport(conn, WaterScrollPosition[2][Vol - 161][0], WaterScrollPosition[2][Vol - 161][1]);
		SendClientSignalParm(conn, ESCENE_FIELD, _MSG_StartTime, WaterClear1[2][Vol - 161] * 2);

		for (int i = 0; i < MAX_PARTY; i++)
		{
			int partyconn = pMob[conn].PartyList[i];

			if (partyconn > 0 && partyconn < MAX_USER && partyconn != conn && pUser[partyconn].Mode == USER_PLAY)
			{
				DoTeleport(partyconn, WaterScrollPosition[2][Vol - 161][0], WaterScrollPosition[2][Vol - 161][1]);
				SendClientSignalParm(partyconn, ESCENE_FIELD, _MSG_StartTime, WaterClear1[2][Vol - 161] * 2);
			}
		}

		if (Sala <= 7)
		{
			GenerateMob(Sala + WATER_A_INITIAL, 0, 0);
			GenerateMob(Sala + WATER_A_INITIAL, 0, 0);
			//RebuildGenerator();
		}
		else if (Sala == 9)
		{
			int _rand = rand() % 10;

			if (_rand < 4)
				GenerateMob(WATER_A_INITIAL + 8, 0, 0);

			else if (_rand < 5)
				GenerateMob(WATER_A_INITIAL + 9, 0, 0);

			else if (_rand < 6)
				GenerateMob(WATER_A_INITIAL + 10, 0, 0);

			else
				GenerateMob(WATER_A_INITIAL + 11, 0, 0);
		}
		sprintf(temp, "useitem,water scroll A %d", Sala);
		ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);

		if (amount > 1)
			BASE_SetItemAmount(item, amount - 1);

		else
			memset(item, 0, sizeof(STRUCT_ITEM));

		return true;
	}
#pragma endregion
#pragma region Divina 7 15 30 Dias
	if (Vol >= 64 && Vol <= 66)
	{
		int sAffect = GetEmptyAffect(conn, 34);

		if (sAffect == -1 || pMob[conn].Affect[sAffect].Type == 34)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_CantEatMore]);
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		time(&pMob[conn].extra.DivineEnd);

		if (Vol == 64)//7DD
			pMob[conn].extra.DivineEnd += 60 * 60 * 24 * 8;

		else  if (Vol == 65)//15D
			pMob[conn].extra.DivineEnd += 60 * 60 * 24 * 16;

		else if (Vol == 66)//30D
			pMob[conn].extra.DivineEnd += 60 * 60 * 24 * 31;


		pMob[conn].Affect[sAffect].Type = 34;
		pMob[conn].Affect[sAffect].Level = 1;
		pMob[conn].Affect[sAffect].Value = 0;
		pMob[conn].Affect[sAffect].Time = 2000000000;

		BASE_GetHpMp(&pMob[conn].MOB, &pMob[conn].extra);
		pMob[conn].GetCurrentScore(conn);

		SendScore(conn);

		sprintf(temp, "useitem,item divine type: %d", Vol-64);
		ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);

		if (amount > 1)
			BASE_SetItemAmount(item, amount - 1);

		else
			memset(item, 0, sizeof(STRUCT_ITEM));

		return true;
	}
#pragma endregion
#pragma region Vigor 1 Hora
	if (Vol == 58)
	{
		int sAffect = GetEmptyAffect(conn, 35);

		if (sAffect == -1)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_CantEatMore]);
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		pMob[conn].Affect[sAffect].Type = 35;
		pMob[conn].Affect[sAffect].Level = 1;
		pMob[conn].Affect[sAffect].Value = 0;
		pMob[conn].Affect[sAffect].Time = AFFECT_1H;

		BASE_GetHpMp(&pMob[conn].MOB, &pMob[conn].extra);
		pMob[conn].GetCurrentScore(conn);

		SendScore(conn);

		sprintf(temp, "useitem,item vigor");
		ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);

		if (amount > 1)
			BASE_SetItemAmount(item, amount - 1);

		else
			memset(item, 0, sizeof(STRUCT_ITEM));

		return true;
	}
#pragma endregion
#pragma region Baú de XP
	if (Vol == 198)
	{
		int sAffect = GetEmptyAffect(conn, 39);

		if (sAffect == -1)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_CantEatMore]);
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		pMob[conn].Affect[sAffect].Type = 39;
		pMob[conn].Affect[sAffect].Level = 0;
		pMob[conn].Affect[sAffect].Value = 0;
		pMob[conn].Affect[sAffect].Time += AFFECT_1H * 2;

		if (pMob[conn].Affect[sAffect].Time >= 324000)
			pMob[conn].Affect[sAffect].Time = 324000;

		pMob[conn].GetCurrentScore(conn);
		SendScore(conn);

		sprintf(temp, "useitem,item bauexp time %d", pMob[conn].Affect[sAffect].Time);
		ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);

		if (amount > 1)
			BASE_SetItemAmount(item, amount - 1);

		else
			memset(item, 0, sizeof(STRUCT_ITEM));

		return true;
	}
#pragma endregion
#pragma region Frango Assado
	if (Vol == 63)
	{
		int sAffect = GetEmptyAffect(conn, 30);

		if (sAffect == -1 || (pMob[conn].Affect[sAffect].Value != 2000 && pMob[conn].Affect[sAffect].Time == 30))
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_CantEatMore]);
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		pMob[conn].Affect[sAffect].Type = 30;
		pMob[conn].Affect[sAffect].Level = 1000;
		pMob[conn].Affect[sAffect].Value = 0;
		pMob[conn].Affect[sAffect].Time = AFFECT_1H * 4;

		if (pMob[conn].Affect[sAffect].Time >= 324000)
			pMob[conn].Affect[sAffect].Time = 324000;

		pMob[conn].GetCurrentScore(conn);
		SendScore(conn);

		sprintf(temp, "useitem,item frango time %d", pMob[conn].Affect[sAffect].Time);
		ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);

		if (amount > 1)
			BASE_SetItemAmount(item, amount - 1);

		else
			memset(item, 0, sizeof(STRUCT_ITEM));

		return true;
	}
#pragma endregion

	return false;
}
