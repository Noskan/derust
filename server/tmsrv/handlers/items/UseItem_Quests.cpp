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

/// @file UseItem_Quests.cpp
/// @brief Sub-handler de quests, Pesadelos, Carta de Duelo, Pedidos de Caça, Pedra Ideal e itens especiais de evolução.

#include "handlers/items/UseItemInternal.h"

/// Coordenadas pré-definidas dos Pergaminhos de Caça [mapa][índice][x, y].
int HuntingScrolls[6][10][2] =
{
	//Armia
	{ {2370, 2106}, {2508, 2101}, {2526, 2109}, {2529, 1882}, {2126, 1600}, {2005, 1617}, {2241, 1474}, {1858, 1721}, {2250, 1316}, {1989, 1755} },
	
	//Dungeon
	{ {290, 3799}, {724, 3781}, {481, 4062}, {876, 4058}, {855, 3922}, {808, 3876}, {959, 3813}, {926, 3750}, {1096, 3730}, {1132, 3800} },
	
	//SubMundo
	{ {1242 , 4035}, {1264 , 4017}, {1333 , 3994}, {1358 , 4041}, {1462 , 4033}, {1326 , 3788}, {1493 , 3777}, {1437 , 3741}, {1389 , 3740}, {1422 , 3810} },
	
	//Kult
	{ {1376 , 1722}, {1426 , 1686}, {1381 , 1861}, {1326 , 1896}, {1510 , 1723}, {1543 , 1726}, {1580 , 1758}, {1182 , 1714}, {1634 , 1727}, {1237 , 1764} }, 
	
	//Kefra
	{ {2367 , 4024}, {2236 , 4044}, {2236 , 3993}, {2209 , 3989}, {2453 , 4067}, {2485 , 4043}, {2537 , 3897}, {2489 , 3919}, {2269 , 3910}, {2202 , 3866} },
	
	//Nipple
	{ {3664 , 3024}, {3582 , 3007}, {3514 , 3008}, {3819 , 2977}, {3517 , 2889}, {3745 , 2977}, {3639 , 2877}, {3650 , 2727}, {3660 , 2773}, {3746 , 2879} }
};

bool UseItem_QuestsAndDungeons(int conn, MSG_UseItem* m, STRUCT_ITEM* item, int Vol, int amount)
{
#pragma region Itens de quests
	if (Vol == 191)
	{
		if (pMob[conn].extra.ClassMaster != MORTAL && pMob[conn].extra.ClassMaster != ARCH)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_Level_Limit2]);
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}
		int thisQuest = item->sIndex - 4117;

		int minlevel = pMob[conn].extra.ClassMaster == MORTAL ? CReadFiles::QuestLevel[thisQuest][0] : CReadFiles::QuestLevel[thisQuest][2];
		int maxlevel = pMob[conn].extra.ClassMaster == MORTAL ? CReadFiles::QuestLevel[thisQuest][1] : CReadFiles::QuestLevel[thisQuest][3];

		if (pMob[conn].MOB.CurrentScore.Level < minlevel || pMob[conn].MOB.CurrentScore.Level >= maxlevel)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_Level_limit]);
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		struct tm when;
		time_t now;
		time(&now);
		when = *localtime(&now);

		int QuestExp = pMob[conn].extra.ClassMaster == MORTAL ? CReadFiles::QuestExp[thisQuest][0] : CReadFiles::QuestExp[thisQuest][1];
		int QuestExpParty = QuestExp / 10;

		pMob[conn].MOB.Coin += CReadFiles::QuestCoin[thisQuest];
		pMob[conn].MOB.Exp += QuestExp;

#pragma region Log de Experiência diário
		if (when.tm_yday != pMob[conn].extra.DayLog.YearDay)
			pMob[conn].extra.DayLog.Exp = 0;

		pMob[conn].extra.DayLog.YearDay = when.tm_yday;
		pMob[conn].extra.DayLog.Exp += QuestExp;
#pragma endregion

		if(pMob[conn].MOB.Coin > 2000000000)
			pMob[conn].MOB.Coin = 2000000000;

		sprintf(temp, g_pMessageStringTable[_NS_GETEXP], pMob[conn].extra.ClassMaster == MORTAL ? CReadFiles::QuestExp[thisQuest][0] : CReadFiles::QuestExp[thisQuest][1]);
		SendClientMessage(conn, temp);

		SendEmotion(conn, 14, 3);

		if (pMob[conn].CheckGetLevel() == 4)
		{
			SetCircletSubGod(conn);
			SendClientMessage(conn, g_pMessageStringTable[_NN_Level_Up]);
			SendScore(conn);

			if(pMob[conn].extra.ClassMaster == MORTAL)
				DoItemLevel(conn);
		}

		SendEtc(conn);

		int partyleader = pMob[conn].Leader;

		if (partyleader <= 0)
			partyleader = conn;

		if (partyleader >= MAX_USER)
			partyleader = conn;

		if(partyleader != conn && (pMob[partyleader].extra.ClassMaster == MORTAL || pMob[partyleader].extra.ClassMaster == ARCH)
			&& pMob[partyleader].MOB.CurrentScore.Level < 350 && pUser[partyleader].OnlyTrade == 0)
		{
			pMob[partyleader].MOB.Exp += QuestExpParty;

			sprintf(temp, g_pMessageStringTable[_NS_GETEXP], QuestExpParty);
			SendClientMessage(partyleader, temp);

#pragma region Log de Experiência diário
			if (when.tm_yday != pMob[partyleader].extra.DayLog.YearDay)
				pMob[partyleader].extra.DayLog.Exp = 0;

			pMob[partyleader].extra.DayLog.YearDay = when.tm_yday;
			pMob[partyleader].extra.DayLog.Exp += QuestExpParty;
#pragma endregion

			SendEmotion(partyleader, 14, 3);

			if (pMob[partyleader].CheckGetLevel() == 4)
			{
				SetCircletSubGod(partyleader);
				SendClientMessage(partyleader, g_pMessageStringTable[_NN_Level_Up]);
				SendScore(partyleader);

				if(pMob[partyleader].extra.ClassMaster == MORTAL)
				DoItemLevel(partyleader);
			}
			SendEtc(partyleader);
		}

		for (int i = 0; i < MAX_PARTY; i++)
		{
			int partymember = pMob[partyleader].PartyList[i];

			if (partymember <= 0 || partymember >= MAX_USER)
				continue;

			if (pUser[partymember].Mode != USER_PLAY || partymember == conn)
				continue;

			if(partymember != conn && (pMob[partymember].extra.ClassMaster == MORTAL || pMob[partymember].extra.ClassMaster == ARCH)
				&& pMob[partymember].MOB.CurrentScore.Level < 350 && pUser[partymember].OnlyTrade == 0)
			{
				pMob[partymember].MOB.Exp += QuestExpParty;

#pragma region Log de Experiência diário
				if (when.tm_yday != pMob[partymember].extra.DayLog.YearDay)
					pMob[partymember].extra.DayLog.Exp = 0;

				pMob[partymember].extra.DayLog.YearDay = when.tm_yday;
				pMob[partymember].extra.DayLog.Exp += QuestExpParty;
#pragma endregion

				sprintf(temp, g_pMessageStringTable[_NS_GETEXP], QuestExpParty);
				SendClientMessage(partymember, temp);

				SendEmotion(partymember, 14, 3);

				if (pMob[partymember].CheckGetLevel() == 4)
				{
					SetCircletSubGod(partymember);
					SendClientMessage(partymember, g_pMessageStringTable[_NN_Level_Up]);
					SendScore(partymember);

					if(pMob[partymember].extra.ClassMaster == MORTAL)
						DoItemLevel(partymember);
				}

				SendEtc(partymember);
			}
		}
		sprintf(temp, "useitem,%s level:%d classmaster:%d", g_pItemList[item->sIndex].Name, pMob[conn].MOB.BaseScore.Level, pMob[conn].extra.ClassMaster);
		ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);

		if (amount > 1)
			BASE_SetItemAmount(item, amount - 1);

		else
			memset(item, 0, sizeof(STRUCT_ITEM));
		return true;
	}
#pragma endregion
#pragma region Molar do Gargula
	if (Vol == 194)
	{
		if (pMob[conn].extra.ClassMaster != MORTAL)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_Level_Limit2]);
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		if (pMob[conn].MOB.CurrentScore.Level < 199 || pMob[conn].MOB.CurrentScore.Level >= 254)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_Level_limit]);
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		if (pMob[conn].extra.QuestInfo.Mortal.MolarGargula)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_Youve_Done_It_Already]);
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		for (int i = 1; i < 6; i++)
		{
			if (pMob[conn].MOB.Equip[i].sIndex == 0)
				continue;

			int sanc = BASE_GetItemSanc(&pMob[conn].MOB.Equip[i]);

			if (sanc < 6)
			{
				BASE_SetItemSanc(&pMob[conn].MOB.Equip[i], 6, 0);
				SendItem(conn, ITEM_PLACE_EQUIP, i, &pMob[conn].MOB.Equip[i]);
			}
		}
		pMob[conn].extra.QuestInfo.Mortal.MolarGargula = 1;
		pMob[conn].GetCurrentScore(conn);

		sprintf(temp, g_pMessageStringTable[_DN_Play_Quest], "Molar do Gargula");
		SendClientMessage(conn, temp);

		sprintf(temp, "useitem,itemquest molar %d", pMob[conn].extra.QuestInfo.Mortal.MolarGargula);
		ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);

		if (amount > 1)
			BASE_SetItemAmount(item, amount - 1);

		else
			memset(item, 0, sizeof(STRUCT_ITEM));

		return true;
	}
#pragma endregion
#pragma region Pesadelo N
	if (Vol == 173)
	{
		int TargetX = pMob[conn].TargetX;
		int TargetY = pMob[conn].TargetY;

		if ((TargetX / 128) != 19 || (TargetY / 128) != 15)
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
		if ((pMob[conn].extra.ClassMaster == CELESTIAL || pMob[conn].extra.ClassMaster == CELESTIALCS || pMob[conn].extra.ClassMaster == SCELESTIAL) && pMob[conn].MOB.CurrentScore.Level >= 180)
		{
			SendItem(conn, m->SourType, m->SourPos, item);

			return true;
		}

		struct tm when;
		time_t now;
		time(&now);
		when = *localtime(&now);

		if (when.tm_min >= 4 && when.tm_min <= 19 || when.tm_min >= 24 && when.tm_min <= 39
			|| when.tm_min >= 44 && when.tm_min <= 59)
		{
			SendItem(conn, m->SourType, m->SourPos, item);

			return true;
		}
		
		int isParty = item->sIndex == 3324 ? 1 : 0;

		if (isParty == 0)
			goto lbl_n_noparty;

		if (PartyPesa[0] >= maxNightmare)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_Night_Limited]);
			SendItem(conn, m->SourType, m->SourPos, item);

			return true;
		}
		PartyPesa[0]++;

lbl_n_noparty:
		int NigthTime = 240;

		if (when.tm_min >= 20 && when.tm_min <= 23)
			NigthTime -= ((when.tm_min-20) * 60) + when.tm_sec;

		else if (when.tm_min >= 40 && when.tm_min <= 43)
			NigthTime -= ((when.tm_min-40) * 60) + when.tm_sec;

		else if (when.tm_min >= 0 && when.tm_min <= 3)
			NigthTime -= (when.tm_min * 60) + when.tm_sec;


		DoTeleport(conn, PesaNPosStandard[0][0], PesaNPosStandard[0][1]);
		SendClientSignalParm(conn, ESCENE_FIELD, _MSG_StartTime, NigthTime);

		for (int i = 0; i < MAX_PARTY && isParty == 1; i++)
		{
			int partyconn = pMob[conn].PartyList[i];

			if((pMob[partyconn].extra.ClassMaster == CELESTIAL || pMob[partyconn].extra.ClassMaster == CELESTIALCS || pMob[partyconn].extra.ClassMaster == SCELESTIAL) && pMob[partyconn].MOB.CurrentScore.Level >= 180)
				continue;

			if (partyconn > 0 && partyconn < MAX_USER && partyconn != conn && pUser[partyconn].Mode == USER_PLAY)
			{
				DoTeleport(partyconn, PesaNPosStandard[i][0], PesaNPosStandard[i][1]);
				SendClientSignalParm(partyconn, ESCENE_FIELD, _MSG_StartTime, NigthTime);
			}
		}

		sprintf(temp, "useitem,nightmare N ticket used party:%d", isParty);
		ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);

		if (amount > 1)
			BASE_SetItemAmount(item, amount - 1);

		else
			memset(item, 0, sizeof(STRUCT_ITEM));

		return true;
	}
#pragma endregion
#pragma region Pesadelo M
	if (Vol == 174)
	{
		int TargetX = pMob[conn].TargetX;
		int TargetY = pMob[conn].TargetY;

		if ((TargetX / 128) != 16 || (TargetY / 128) != 16)
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
		if ((pMob[conn].extra.ClassMaster == CELESTIAL || pMob[conn].extra.ClassMaster == CELESTIALCS || pMob[conn].extra.ClassMaster == SCELESTIAL) && pMob[conn].MOB.CurrentScore.Level >= 180)
		{
			SendItem(conn, m->SourType, m->SourPos, item);

			return true;
		}

		struct tm when;
		time_t now;
		time(&now);
		when = *localtime(&now);

		if (when.tm_min >= 0 && when.tm_min <= 4 || when.tm_min >= 9 && when.tm_min <= 24
			|| when.tm_min >= 29 && when.tm_min <= 44 || when.tm_min >= 49 && when.tm_min <= 59)
		{
			SendItem(conn, m->SourType, m->SourPos, item);

			return true;
		}
		int isParty = item->sIndex == 3325 ? 1 : 0;

		if (isParty == 0)
			goto lbl_m_noparty;

		if (PartyPesa[1] >= maxNightmare)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_Night_Limited]);
			SendItem(conn, m->SourType, m->SourPos, item);

			return true;
		}
		PartyPesa[1]++;

lbl_m_noparty:
		int NigthTime = 240;

		if (when.tm_min >= 5 && when.tm_min <= 8)
			NigthTime -= ((when.tm_min - 5) * 60) + when.tm_sec;

		else if (when.tm_min >= 25 && when.tm_min <= 28)
			NigthTime -= ((when.tm_min - 25) * 60) + when.tm_sec;

		else if (when.tm_min >= 45 && when.tm_min <= 48)
			NigthTime -= ((when.tm_min - 45) * 60) + when.tm_sec;

		if((pMob[conn].MOB.SPX/128) == 8 && (pMob[conn].MOB.SPY/128) == 2)
			DoTeleport(conn, pMob[conn].MOB.SPX, pMob[conn].MOB.SPY);
		else
			DoTeleport(conn, PesaMPosStandard[0][0], PesaMPosStandard[0][1]);
							 
		SendClientSignalParm(conn, ESCENE_FIELD, _MSG_StartTime, NigthTime);

		for (int i = 0; i < MAX_PARTY && isParty == 1; i++)
		{
			int partyconn = pMob[conn].PartyList[i];

			if((pMob[partyconn].extra.ClassMaster == CELESTIAL || pMob[partyconn].extra.ClassMaster == CELESTIALCS || pMob[partyconn].extra.ClassMaster == SCELESTIAL) && pMob[partyconn].MOB.CurrentScore.Level >= 180)
				continue;

			if (partyconn > 0 && partyconn < MAX_USER && partyconn != conn && pUser[partyconn].Mode == USER_PLAY)
			{
				if((pMob[partyconn].MOB.SPX/128) == 8 && (pMob[partyconn].MOB.SPY/128) == 2)
					DoTeleport(partyconn, pMob[partyconn].MOB.SPX, pMob[partyconn].MOB.SPY);
				else
					DoTeleport(partyconn, PesaMPosStandard[i][0], PesaMPosStandard[i][1]);

				SendClientSignalParm(partyconn, ESCENE_FIELD, _MSG_StartTime, NigthTime);
			}
		}

		sprintf(temp, "useitem,nightmare M ticket used party:%d", isParty);
		ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);

		if (amount > 1)
			BASE_SetItemAmount(item, amount - 1);

		else
			memset(item, 0, sizeof(STRUCT_ITEM));

		return true;
	}
#pragma endregion
#pragma region Pesadelo A
	if (Vol == 175)
	{
		int TargetX = pMob[conn].TargetX;
		int TargetY = pMob[conn].TargetY;


		if ((TargetX / 128) != 19 || (TargetY / 128) != 13)
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

		// Lido antes do goto: o bilhete solo pula a checagem de horário abaixo, mas usa o relógio
		// para calcular NigthTime (antes ficava sem valor nesse caminho).
		struct tm when;
		time_t now;
		time(&now);
		when = *localtime(&now);

		int isParty = item->sIndex == 3326 ? 1 : 0;

		if (isParty == 0)
			goto lbl_a_noparty;

		if ((pMob[conn].extra.ClassMaster == CELESTIAL || pMob[conn].extra.ClassMaster == CELESTIALCS || pMob[conn].extra.ClassMaster == SCELESTIAL) && pMob[conn].MOB.CurrentScore.Level >= 180)
		{
			SendItem(conn, m->SourType, m->SourPos, item);

			return true;
		}

		if (when.tm_min >= 0 && when.tm_min <= 9 || when.tm_min >= 14 && when.tm_min <= 29
			|| when.tm_min >= 34 && when.tm_min <= 49 || when.tm_min >= 54 && when.tm_min <= 60)
		{
			SendItem(conn, m->SourType, m->SourPos, item);

			return true;
		}

		if ((pMob[conn].extra.ClassMaster == CELESTIAL || pMob[conn].extra.ClassMaster == SCELESTIAL || pMob[conn].extra.ClassMaster == CELESTIALCS) && pMob[conn].extra.NT <= 0)
		{
			SendItem(conn, m->SourType, m->SourPos, item);

			return true;
		}

		if (PartyPesa[2] >= maxNightmare)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_Night_Limited]);
			SendItem(conn, m->SourType, m->SourPos, item);

			return true;
		}
		PartyPesa[2]++;

lbl_a_noparty:
		int NigthTime = 240;

		if (when.tm_min >= 10 && when.tm_min <= 13)
			NigthTime -= ((when.tm_min - 10) * 60) + when.tm_sec;

		else if (when.tm_min >= 30 && when.tm_min <= 33)
			NigthTime -= ((when.tm_min - 30) * 60) + when.tm_sec;

		else if (when.tm_min >= 50 && when.tm_min <= 53)
			NigthTime -= ((when.tm_min - 50) * 60) + when.tm_sec;


		if ((pMob[conn].extra.ClassMaster == CELESTIAL || pMob[conn].extra.ClassMaster == SCELESTIAL || pMob[conn].extra.ClassMaster == CELESTIALCS))
			pMob[conn].extra.NT--;

		if((pMob[conn].MOB.SPX/128) == 9 && (pMob[conn].MOB.SPY/128) == 1)
			DoTeleport(conn, pMob[conn].MOB.SPX+rand()%1, pMob[conn].MOB.SPY+rand()%1);

		else
			DoTeleport(conn, PesaAPosStandard[0][0]+rand()%1, PesaAPosStandard[0][1]+rand()%1);

		SendClientSignalParm(conn, ESCENE_FIELD, _MSG_StartTime, NigthTime);

		for (int i = 0; i < MAX_PARTY && isParty == 1; i++)
		{
			int partyconn = pMob[conn].PartyList[i];

			if((pMob[partyconn].extra.ClassMaster == CELESTIAL || pMob[partyconn].extra.ClassMaster == CELESTIALCS || pMob[partyconn].extra.ClassMaster == SCELESTIAL) && pMob[partyconn].MOB.CurrentScore.Level >= 180)
				continue;

			if (partyconn > 0 && partyconn < MAX_USER && partyconn != conn && pUser[partyconn].Mode == USER_PLAY)
			{
				if ((pMob[partyconn].extra.ClassMaster == CELESTIAL || pMob[partyconn].extra.ClassMaster == SCELESTIAL || pMob[partyconn].extra.ClassMaster == CELESTIALCS))
				{
					if (pMob[partyconn].extra.NT <= 0)
						continue;

					pMob[partyconn].extra.NT--;
				}

				if((pMob[partyconn].MOB.SPX/128) == 9 && (pMob[partyconn].MOB.SPY/128) == 1)
					DoTeleport(partyconn, pMob[partyconn].MOB.SPX+rand()%1, pMob[partyconn].MOB.SPY+rand()%1);
				else
				DoTeleport(partyconn, PesaAPosStandard[i][0]+rand()%1, PesaAPosStandard[i][1]+rand()%1);

				SendClientSignalParm(partyconn, ESCENE_FIELD, _MSG_StartTime, NigthTime);
			}
		}
		sprintf(temp, "useitem,nightmare A ticket used party:%d", isParty);
		ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);

		if (amount > 1)
			BASE_SetItemAmount(item, amount - 1);

		else
			memset(item, 0, sizeof(STRUCT_ITEM));

		return true;
	}
#pragma endregion
#pragma region Carta de Duelo N.M.A
	if (Vol == 20)
	{
		int TargetX = pMob[conn].TargetX;
		int TargetY = pMob[conn].TargetY;


		if ((TargetX / 4) != 261 || (TargetY / 4) != 422)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_Only_at_tor_alter]);
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

		int UserArea = GetUserInArea(774, 3593, 892, 3702, UserName);

		if (UserArea >= 1)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_Someone_is_on_quest]);
			SendItem(conn, m->SourType, m->SourPos, item);

			return true;
		}

		CartaTime = 60;
		CartaSala = 1;

		DoTeleport(conn, CartaPos[0][0], CartaPos[0][1]);
		SendClientSignalParm(conn, ESCENE_FIELD, _MSG_StartTime, CartaTime * 2);

		for (int i = 0; i < MAX_PARTY; i++)
		{
			int partyconn = pMob[conn].PartyList[i];

			if (partyconn > 0 && partyconn < MAX_USER && partyconn != conn && pUser[partyconn].Mode == USER_PLAY)
			{
				DoTeleport(partyconn, CartaPos[0][0], CartaPos[0][1]);
				SendClientSignalParm(partyconn, ESCENE_FIELD, _MSG_StartTime, CartaTime * 2);
			}
		}

		for (int xx = 767; xx < 897; xx++)
		{
			for (int yy = 3582; yy < 3712; yy++)
			{
				int tmob = pMobGrid[yy][xx];

				if (tmob < MAX_USER)
					continue;

				DeleteMob(tmob, 3);
			}
		}

		int cartaindex = item->sIndex - 3171;

		if (cartaindex == 1)
		{//Carta N
			for (int i = SECRET_ROOM_N_INITIAL; i <= SECRET_ROOM_N_END; i++)
			{
				GenerateMob(i, 0, 0);
				GenerateMob(i, 0, 0);
			}

			GenerateMob(SECRET_ROOM_N_BOSS_INITIAL + rand() % 1, 0, 0);
		}

		else if (cartaindex == 0)
		{//Carta M
			for (int i = SECRET_ROOM_M_INITIAL; i <= SECRET_ROOM_M_END; i++)
			{
				GenerateMob(i, 0, 0);
				GenerateMob(i, 0, 0);
			}

			GenerateMob(SECRET_ROOM_M_BOSS_INITIAL + rand() % 1, 0, 0);
		}
		else
		{//Carta A
			for (int i = SECRET_ROOM_A_INITIAL; i <= SECRET_ROOM_A_END; i++)
			{
				GenerateMob(i, 0, 0);
				GenerateMob(i, 0, 0);
			}

			GenerateMob(SECRET_ROOM_A_BOSS_INITIAL + rand() % 1, 0, 0);
		}
		if (amount > 1)
			BASE_SetItemAmount(item, amount - 1);

		else
			memset(item, 0, sizeof(STRUCT_ITEM));

		sprintf(temp, "useitem,secretroom ticket used itemindex:%d", item->sIndex);
		ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);

		return true;
	}
#pragma endregion
#pragma region Pedidos de caça
	if (Vol == 195)
	{
		if (item->sIndex < 3432 || item->sIndex > 3437)
			return true;

		if (m->WarpID <= 0 || m->WarpID > 10)
			return true;

		DoTeleport(conn, HuntingScrolls[item->sIndex - 3432][m->WarpID-1][0], HuntingScrolls[item->sIndex - 3432][m->WarpID-1][1]);

		if (amount > 1)
			BASE_SetItemAmount(item, amount - 1);

		else
			memset(item, 0, sizeof(STRUCT_ITEM));

		sprintf(temp, "useitem,huntingscrolls itemindex:%d warpid:%d tx:%d ty:%d", item->sIndex, m->WarpID, HuntingScrolls[item->sIndex - 3432][m->WarpID-1][0], HuntingScrolls[item->sIndex - 3432][m->WarpID-1][1]);
		ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);
		return true;
	}
#pragma endregion
#pragma region Pedra Ideal
	if (Vol == 211)
	{
		if (pMob[conn].extra.ClassMaster != ARCH && pMob[conn].extra.ClassMaster != CELESTIAL)
		{
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		if (pMob[conn].extra.ClassMaster == ARCH && pMob[conn].MOB.CurrentScore.Level < 355)
		{
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		if (pMob[conn].extra.ClassMaster == ARCH && (pMob[conn].MOB.Equip[1].sIndex != 0 || pMob[conn].extra.QuestInfo.Arch.MortalLevel < 99))
		{
			if(pMob[conn].MOB.Equip[1].sIndex)
				SendClientMessage(conn, g_pMessageStringTable[_NN_Cant_with_armor]);

			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		if (pMob[conn].extra.ClassMaster == CELESTIAL && (pMob[conn].MOB.Equip[11].sIndex < 1760 || pMob[conn].MOB.Equip[11].sIndex > 1763))
		{
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		if (pMob[conn].extra.ClassMaster == CELESTIAL && pMob[conn].MOB.CurrentScore.Level < 120)
		{
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		if (pMob[conn].extra.ClassMaster == CELESTIAL)
		{
			pMob[conn].extra.ClassMaster = CELESTIALCS;

			pMob[conn].extra.SaveCelestial[1].Class = pMob[conn].MOB.Equip[11].sIndex - 1760;

			pMob[conn].extra.SaveCelestial[1].BaseScore = pMob[conn].MOB.BaseScore;

			pMob[conn].extra.SaveCelestial[1].LearnedSkill = pMob[conn].MOB.LearnedSkill;

			pMob[conn].extra.SaveCelestial[1].BaseScore.Level = 0;

			pMob[conn].extra.SaveCelestial[1].BaseScore.Str = BaseSIDCHM[pMob[conn].MOB.Class][0];
			pMob[conn].extra.SaveCelestial[1].BaseScore.Int = BaseSIDCHM[pMob[conn].MOB.Class][1];
			pMob[conn].extra.SaveCelestial[1].BaseScore.Dex = BaseSIDCHM[pMob[conn].MOB.Class][2];
			pMob[conn].extra.SaveCelestial[1].BaseScore.Con = BaseSIDCHM[pMob[conn].MOB.Class][3];

			pMob[conn].extra.SaveCelestial[1].BaseScore.Special[0] = 0;
			pMob[conn].extra.SaveCelestial[1].BaseScore.Special[1] = 0;
			pMob[conn].extra.SaveCelestial[1].BaseScore.Special[2] = 0;
			pMob[conn].extra.SaveCelestial[1].BaseScore.Special[3] = 0;

			pMob[conn].extra.SaveCelestial[1].BaseScore.Ac = 230;
			pMob[conn].extra.SaveCelestial[1].BaseScore.Damage = 0;
			pMob[conn].extra.SaveCelestial[1].BaseScore.Hp = BaseSIDCHM[pMob[conn].MOB.Equip[11].sIndex - 1760][4];
			pMob[conn].extra.SaveCelestial[1].BaseScore.MaxHp = BaseSIDCHM[pMob[conn].MOB.Equip[11].sIndex - 1760][4];
			pMob[conn].extra.SaveCelestial[1].BaseScore.Mp = BaseSIDCHM[pMob[conn].MOB.Equip[11].sIndex - 1760][5];
			pMob[conn].extra.SaveCelestial[1].BaseScore.MaxMp = BaseSIDCHM[pMob[conn].MOB.Equip[11].sIndex - 1760][5];

			pMob[conn].extra.SaveCelestial[1].SpecialBonus = 855;

			pMob[conn].extra.SaveCelestial[1].LearnedSkill &= 0xFF000000;

			pMob[conn].extra.SaveCelestial[1].Exp = 0;

			pMob[conn].extra.SaveCelestial[1].Soul = pMob[conn].extra.Soul;

			memset(&pMob[conn].MOB.Equip[11], 0, sizeof(STRUCT_ITEM));

			SendItem(conn, ITEM_PLACE_EQUIP, 11, &pMob[conn].MOB.Equip[11]);

			if (amount > 1)
				BASE_SetItemAmount(item, amount - 1);

			else
				memset(item, 0, sizeof(STRUCT_ITEM));

			sprintf(temp, "useitem,ideal stone used to make god2");
			ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);

			SendClientMessage(conn, g_pMessageStringTable[_NN_My_King_Bless1]);
			return true;
		}

		if (pMob[conn].extra.ClassMaster == ARCH)
		{
			int ptlevel = 0;

			if (pMob[conn].MOB.CurrentScore.Level < 370)
				ptlevel = 1;
								 
			else if (pMob[conn].MOB.CurrentScore.Level < 380)
				ptlevel = 2;

			else if (pMob[conn].MOB.CurrentScore.Level < 398)
				ptlevel = 3;

			else if (pMob[conn].MOB.CurrentScore.Level == 398)
				ptlevel = 4;
			else
				ptlevel = 5;

			memset(&pMob[conn].MOB.Equip[1], 0, sizeof(STRUCT_ITEM));

			if (pMob[conn].MOB.CurrentScore.Level < 380)
				pMob[conn].MOB.Equip[1].sIndex = 3500;

			else if (pMob[conn].MOB.CurrentScore.Level < 399)
				pMob[conn].MOB.Equip[1].sIndex = 3501;

			else if (pMob[conn].MOB.CurrentScore.Level < 400)
				pMob[conn].MOB.Equip[1].sIndex = 3502;

			pMob[conn].extra.QuestInfo.Celestial.ArchLevel = ptlevel;
			pMob[conn].extra.ClassMaster = CELESTIAL;

			pMob[conn].MOB.BaseScore.Level = 0;

			pMob[conn].MOB.BaseScore.Str = BaseSIDCHM[pMob[conn].MOB.Class][0];
			pMob[conn].MOB.BaseScore.Int = BaseSIDCHM[pMob[conn].MOB.Class][1];
			pMob[conn].MOB.BaseScore.Dex = BaseSIDCHM[pMob[conn].MOB.Class][2];
			pMob[conn].MOB.BaseScore.Con = BaseSIDCHM[pMob[conn].MOB.Class][3];

			pMob[conn].MOB.BaseScore.Special[0] = 0;
			pMob[conn].MOB.BaseScore.Special[1] = 0;
			pMob[conn].MOB.BaseScore.Special[2] = 0;
			pMob[conn].MOB.BaseScore.Special[3] = 0;

			pMob[conn].MOB.BaseScore.Ac = 230;
			pMob[conn].MOB.BaseScore.Damage = 0;
			pMob[conn].MOB.BaseScore.Hp = BaseSIDCHM[pMob[conn].MOB.Class][4];
			pMob[conn].MOB.BaseScore.MaxHp = BaseSIDCHM[pMob[conn].MOB.Class][4];
			pMob[conn].MOB.BaseScore.Mp = BaseSIDCHM[pMob[conn].MOB.Class][5];
			pMob[conn].MOB.BaseScore.MaxMp = BaseSIDCHM[pMob[conn].MOB.Class][5];

			pMob[conn].MOB.SpecialBonus = 855;

			pMob[conn].MOB.LearnedSkill = 1073741824;

			pMob[conn].MOB.Exp = 0;

			memset(&pMob[conn].MOB.Equip[15], 0, sizeof(STRUCT_ITEM));

			if (pMob[conn].MOB.Clan == 7)
				pMob[conn].MOB.Equip[15].sIndex = 3197;

			else if (pMob[conn].MOB.Clan == 8)
				pMob[conn].MOB.Equip[15].sIndex = 3198;

			else
				pMob[conn].MOB.Equip[15].sIndex = 3199;

			pMob[conn].MOB.Equip[0].stEffect[1].cEffect = 98;
			pMob[conn].MOB.Equip[0].stEffect[1].cValue = 3;
			pMob[conn].MOB.Equip[0].stEffect[2].cEffect = 106;
			pMob[conn].MOB.Equip[0].stEffect[2].cValue = (unsigned char)pMob[conn].MOB.Equip[0].sIndex;

			SendClientMessage(conn, g_pMessageStringTable[_NN_My_King_Bless1]);
		}

		if (amount > 1)
			BASE_SetItemAmount(item, amount - 1);

		else
			memset(item, 0, sizeof(STRUCT_ITEM));

		sprintf(temp, "useitem,ideal stone used to make god %d %d", pMob[conn].extra.QuestInfo.Celestial.ArchLevel, pMob[conn].extra.QuestInfo.Arch.Cristal);
		ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);

		CharLogOut(conn);

		SendClientSignalParm(conn, ESCENE_FIELD, _MSG_SendArchEffect, pUser[conn].Slot);

		return true;
	}
#pragma endregion
#pragma region Pedra Misteriosa
	if (item->sIndex == 4148)
	{
		if (pMob[conn].extra.ClassMaster != CELESTIALCS && pMob[conn].extra.ClassMaster != SCELESTIAL)
		{
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		int Village = BASE_GetVillage(pMob[conn].TargetX, pMob[conn].TargetY);

		if (Village < 0 || Village >= 5)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_OnlyVillage]);
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		int cl = pMob[conn].extra.ClassMaster == CELESTIALCS ? 0 : 1;
		int ncl = pMob[conn].extra.ClassMaster == CELESTIALCS ? 1 : 0;

		if (pMob[conn].extra.ClassMaster == CELESTIALCS)
		{
			pMob[conn].extra.ClassMaster = SCELESTIAL;
			pMob[conn].extra.QuestInfo.Celestial.CelestialLevel = pMob[conn].MOB.BaseScore.Level;
		}
		else
		{
			pMob[conn].extra.ClassMaster = CELESTIALCS;
			pMob[conn].extra.QuestInfo.Celestial.SubCelestialLevel = pMob[conn].MOB.BaseScore.Level;
		}
		pMob[conn].extra.SaveCelestial[cl].BaseScore = pMob[conn].MOB.BaseScore;
		pMob[conn].extra.SaveCelestial[cl].Class = pMob[conn].MOB.Class;
		pMob[conn].extra.SaveCelestial[cl].Exp = pMob[conn].MOB.Exp;
		pMob[conn].extra.SaveCelestial[cl].LearnedSkill = pMob[conn].MOB.LearnedSkill;
		pMob[conn].extra.SaveCelestial[cl].ScoreBonus = pMob[conn].MOB.ScoreBonus;
		pMob[conn].extra.SaveCelestial[cl].SkillBonus = pMob[conn].MOB.SkillBonus;
		pMob[conn].extra.SaveCelestial[cl].SpecialBonus = pMob[conn].MOB.SpecialBonus;
		pMob[conn].extra.SaveCelestial[cl].SPX = pMob[conn].MOB.SPX;
		pMob[conn].extra.SaveCelestial[cl].SPY = pMob[conn].MOB.SPY;

		pMob[conn].extra.SaveCelestial[cl].Soul = pMob[conn].extra.Soul;

		memcpy(pMob[conn].extra.SaveCelestial[cl].SkillBar1, pMob[conn].MOB.SkillBar, 4);
		memcpy(pMob[conn].extra.SaveCelestial[cl].SkillBar2, pUser[conn].CharShortSkill, 16);

		pMob[conn].MOB.BaseScore = pMob[conn].extra.SaveCelestial[ncl].BaseScore;
		pMob[conn].MOB.Class = pMob[conn].extra.SaveCelestial[ncl].Class;
		pMob[conn].MOB.Exp = pMob[conn].extra.SaveCelestial[ncl].Exp;
		pMob[conn].MOB.LearnedSkill = pMob[conn].extra.SaveCelestial[ncl].LearnedSkill;
		pMob[conn].MOB.ScoreBonus = pMob[conn].extra.SaveCelestial[ncl].ScoreBonus;
		pMob[conn].MOB.SkillBonus =  pMob[conn].extra.SaveCelestial[ncl].SkillBonus;
		pMob[conn].MOB.SpecialBonus = pMob[conn].extra.SaveCelestial[ncl].SpecialBonus;
		pMob[conn].MOB.SPX = pMob[conn].extra.SaveCelestial[ncl].SPX;
		pMob[conn].MOB.SPY = pMob[conn].extra.SaveCelestial[ncl].SPY;
		pMob[conn].extra.Soul = pMob[conn].extra.SaveCelestial[ncl].Soul;

		memcpy(pMob[conn].MOB.SkillBar, pMob[conn].extra.SaveCelestial[ncl].SkillBar1, 4);
		memcpy(pUser[conn].CharShortSkill, pMob[conn].extra.SaveCelestial[ncl].SkillBar2, 16);

		for (int i = 0; i < MAX_AFFECT; i++)
		{
			if (pMob[conn].Affect[i].Type == 4 || pMob[conn].Affect[i].Type == 8 || pMob[conn].Affect[i].Type == 29 || pMob[conn].Affect[i].Type == 30 || pMob[conn].Affect[i].Type == 34 || pMob[conn].Affect[i].Type == 35 || pMob[conn].Affect[i].Type == 39)
				continue;

			memset(&pMob[conn].Affect[i], 0, sizeof(STRUCT_AFFECT));
		}

		pMob[conn].MOB.Equip[0].sIndex = pMob[conn].extra.MortalFace + 5 + pMob[conn].MOB.Class;
		SendItem(conn, ITEM_PLACE_EQUIP, 0, &pMob[conn].MOB.Equip[0]);

		MSG_SetShortSkill sm_sss;
		memset(&sm_sss, 0, sizeof(MSG_SetShortSkill));

		sm_sss.Size = sizeof(MSG_SetShortSkill);
		sm_sss.ID = ESCENE_FIELD;
		sm_sss.Type = _MSG_SetShortSkill;
							 
		memcpy(sm_sss.Skill, pMob[conn].extra.SaveCelestial[ncl].SkillBar1, 4);
		memcpy(sm_sss.Skill + 4, pMob[conn].extra.SaveCelestial[ncl].SkillBar2, 16);

		pUser[conn].cSock.AddMessage((char*)&sm_sss, sizeof(MSG_SetShortSkill));

		BASE_GetBonusSkillPoint(&pMob[conn].MOB, &pMob[conn].extra);
		BASE_GetBonusScorePoint(&pMob[conn].MOB, &pMob[conn].extra);

		BASE_GetHpMp(&pMob[conn].MOB, &pMob[conn].extra);

		pMob[conn].GetCurrentScore(conn);

		GetGuild(conn);

		SendScore(conn);
		SendEtc(conn);
							 
		sprintf(temp, "useitem,mistery stone change god2god");
		ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);

		if (amount > 1)
			BASE_SetItemAmount(item, amount - 1);

		else
			memset(item, 0, sizeof(STRUCT_ITEM));

		return true;
	}
#pragma endregion
#pragma region Escritura do Pesadelo
	if (Vol == 212)
	{
		time_t mtime;

		time(&mtime);

		if (mtime < (pMob[conn].extra.LastNT + 72000))
		{
			int rtime = (int)((72000 - (mtime - pMob[conn].extra.LastNT)) / 60 / 60);
			int rmin = (int)((72000 - (mtime - pMob[conn].extra.LastNT)) / 60 - (rtime * 60));

			sprintf(temp, g_pMessageStringTable[_DD_NightmareTime], rtime, rmin);
			SendClientMessage(conn, temp);
			
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		pMob[conn].extra.LastNT = mtime;
		pMob[conn].extra.NT += 13;

		sprintf(temp, "useitem,nightmare entrance ticket %d", pMob[conn].extra.NT);
		ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);

		if (amount > 1)
			BASE_SetItemAmount(item, amount - 1);

		else
			memset(item, 0, sizeof(STRUCT_ITEM));

		return true;
	}
#pragma endregion
#pragma region Selo do Guerreiro
	if (item->sIndex == 4146)
	{
		pUser[conn].UseItemTime = GetTickCount();

		if (pMob[conn].extra.Fame >= 2000000000)
			pMob[conn].extra.Fame = 2000000000;

		pMob[conn].extra.Fame += 10;

		SendEmotion(conn, 14, 3);

		if(pMob[conn].extra.ClassMaster == MORTAL && pMob[conn].MOB.CurrentScore.Level >= 354 && pMob[conn].MOB.Equip[15].sIndex != 3191 && pMob[conn].MOB.Equip[15].sIndex != 3192 && pMob[conn].MOB.Equip[15].sIndex != 3193 && pMob[conn].MOB.Equip[15].sIndex != 3194 && pMob[conn].MOB.Equip[15].sIndex != 3195 && pMob[conn].MOB.Equip[15].sIndex != 3196)
		{
			memset(&pMob[conn].MOB.Equip[15], 0, sizeof(STRUCT_ITEM));

			if(pMob[conn].MOB.Clan == 7)
				pMob[conn].MOB.Equip[15].sIndex = 3191;

			else if(pMob[conn].MOB.Clan == 8)
				pMob[conn].MOB.Equip[15].sIndex = 3192;

			else
				pMob[conn].MOB.Equip[15].sIndex = 3193;

			SendItem(conn, ITEM_PLACE_EQUIP, 15, &pMob[conn].MOB.Equip[15]);
		}

		sprintf(temp, "useitem,sealofwarrior %d", pMob[conn].extra.Fame);
		ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);

		if (amount > 1)
			BASE_SetItemAmount(item, amount - 1);

		else
			memset(item, 0, sizeof(STRUCT_ITEM));

		return true;
	}
#pragma endregion
#pragma region Cristal - Elime - Sylphed - Thelion - Noas
	if (Vol == 187)
	{
		if (pMob[conn].extra.ClassMaster != ARCH)
		{
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		if(pMob[conn].MOB.CurrentScore.Level < 355)
		{
			sprintf(temp, g_pMessageStringTable[_DN_Level_Limit], 356);
			SendClientMessage(conn, temp);
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		int quest = item->sIndex - 4106 + 1;

		if(pMob[conn].extra.QuestInfo.Arch.Cristal == quest)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_Youve_Done_It_Already]);
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		if(pMob[conn].extra.QuestInfo.Arch.Cristal >= quest)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_Youve_Done_It_Already]);
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		if(pMob[conn].extra.QuestInfo.Arch.Cristal != quest-1)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_Need_BeforeQuest]);
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		pMob[conn].extra.QuestInfo.Arch.Cristal = quest;
		
		if (quest == 1)
			pMob[conn].MOB.BaseScore.MaxMp += 80;
		
		else if (quest == 2)
			pMob[conn].MOB.BaseScore.Ac += 30;

		else if (quest == 3)
			pMob[conn].MOB.BaseScore.MaxHp += 80;

		else
		{
			pMob[conn].MOB.BaseScore.MaxHp += 60;
			pMob[conn].MOB.BaseScore.MaxMp += 60;
			pMob[conn].MOB.BaseScore.Ac += 20;
		}

		pMob[conn].MOB.Exp -= 100000000;

		int newlevel = 0;

		for(int i = 0; i < MAX_LEVEL; i++)
		{
			if(pMob[conn].MOB.Exp >= g_pNextLevel[i] && pMob[conn].MOB.Exp <= g_pNextLevel[i+1])
				newlevel = i;
		}

		pMob[conn].MOB.BaseScore.Level = newlevel;
		int check = pMob[conn].CheckGetLevel();

		if(check == 4)
			SetCircletSubGod(conn);

		BASE_GetBonusScorePoint(&pMob[conn].MOB, &pMob[conn].extra);

		pMob[conn].GetCurrentScore(conn);
		SendEtc(conn);
		SendScore(conn);

		SendEmotion(conn, 14, 3);

		sprintf(temp, g_pMessageStringTable[_DN_Play_Quest], g_pItemList[item->sIndex].Name);
		SendClientMessage(conn, temp);

		sprintf(temp, "useitem,archcristal complete etapa:%d", quest);
		ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);

		if (amount > 1)
			BASE_SetItemAmount(item, amount - 1);
		else
			memset(item, 0, sizeof(STRUCT_ITEM));

		return true;
	}
#pragma endregion
#pragma region Pedra da Fúria
	if (item->sIndex == 3020)
	{
		if ((pMob[conn].extra.ClassMaster == CELESTIAL || pMob[conn].extra.ClassMaster == CELESTIALCS || pMob[conn].extra.ClassMaster == SCELESTIAL) && pMob[conn].MOB.CurrentScore.Level >= 199)
		{
			if(pMob[conn].extra.Fame < 500)
			{
				SendItem(conn, m->SourType, m->SourPos, item);
				return true;
			}

			if(pMob[conn].extra.QuestInfo.Circle == 0)
			{
				int sAgua = 0;
				int sTerra = 0;
				int sSol = 0;
				int sVento = 0;

				for (int i = 0; i < pMob[conn].MaxCarry; i++)
				{
					if (pMob[conn].MOB.Carry[i].sIndex == 5334)
						sAgua = 1;

					if (pMob[conn].MOB.Carry[i].sIndex == 5335)
						sTerra = 1;

					if (pMob[conn].MOB.Carry[i].sIndex == 5336)
						sSol = 1;

					if (pMob[conn].MOB.Carry[i].sIndex == 5337)
						sVento = 1;
				}

				if (sAgua == 1 && sTerra == 1 && sSol == 1 && sVento == 1)
				{
					for (int i = 0; i < pMob[conn].MaxCarry; i++)
					{
						if (pMob[conn].MOB.Carry[i].sIndex == 5334 && sAgua == 1)
						{
							memset(&pMob[conn].MOB.Carry[i], 0, sizeof(STRUCT_ITEM));
							SendItem(conn, ITEM_PLACE_CARRY, i, &pMob[conn].MOB.Carry[i]);
							sAgua = 0;
						}

						if (pMob[conn].MOB.Carry[i].sIndex == 5335 && sTerra == 1)
						{
							memset(&pMob[conn].MOB.Carry[i], 0, sizeof(STRUCT_ITEM));
							SendItem(conn, ITEM_PLACE_CARRY, i, &pMob[conn].MOB.Carry[i]);
							sTerra = 0;
						}

						if (pMob[conn].MOB.Carry[i].sIndex == 5336 && sSol == 1)
						{
							memset(&pMob[conn].MOB.Carry[i], 0, sizeof(STRUCT_ITEM));
							SendItem(conn, ITEM_PLACE_CARRY, i, &pMob[conn].MOB.Carry[i]);
							sSol = 0;
						}

						if (pMob[conn].MOB.Carry[i].sIndex == 5337 && sVento == 1)
						{
							memset(&pMob[conn].MOB.Carry[i], 0, sizeof(STRUCT_ITEM));
							SendItem(conn, ITEM_PLACE_CARRY, i, &pMob[conn].MOB.Carry[i]);
							sVento = 0;
						}
					}

					pMob[conn].extra.Fame -= 500;

					int _rand = rand()%115;

					if(_rand > 100)
						_rand -= 15;

					if(_rand < 30)
					{
						if(!pMob[conn].MOB.Equip[1].sIndex)
							memset(&pMob[conn].MOB.Equip[1], 0, sizeof(STRUCT_ITEM));

						pMob[conn].MOB.Equip[1].sIndex = 3507;

						SendItem(conn, ITEM_PLACE_EQUIP, 1, &pMob[conn].MOB.Equip[1]);

						sprintf(temp, g_pMessageStringTable[_DN_Play_Quest], "Cythera Arcana");
						SendClientMessage(conn, temp);
												 
						SendEmotion(conn, 14, 3);
						pMob[conn].extra.QuestInfo.Circle = 1;

						sprintf(temp, "useitem,furystone quest arcan complete.");
						ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);
					}

					else 
					{
						SendClientMessage(conn, g_pMessageStringTable[_NN_FAILURE]);
						sprintf(temp, "useitem,furystone quest arcan fail.");
						ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);
					}
					if (amount > 1)
						BASE_SetItemAmount(item, amount - 1);

					else
						memset(item, 0, sizeof(STRUCT_ITEM));

					return true;
				}
				SendItem(conn, m->SourType, m->SourPos, item);
				return true;
			}

			else if(pMob[conn].extra.QuestInfo.Circle && pMob[conn].extra.QuestInfo.Celestial.Reset < 3)
			{
				pMob[conn].extra.QuestInfo.Celestial.Reset++;
				pMob[conn].extra.Fame -= 500;

				pMob[conn].MOB.Exp -= 20000000;

				int newlevel = pMob[conn].MOB.CurrentScore.Level;

				for(int i = 0; i < MAX_CLEVEL; i++)
				{
					if(pMob[conn].MOB.Exp >= g_pNextLevel_2[i] && pMob[conn].MOB.Exp <= g_pNextLevel_2[i+1])
						newlevel = i;
										 
				}

				pMob[conn].MOB.BaseScore.Level = newlevel;
				int check = pMob[conn].CheckGetLevel();

				if(check == 4)
					SetCircletSubGod(conn);

				BASE_GetBonusScorePoint(&pMob[conn].MOB, &pMob[conn].extra);

				pMob[conn].GetCurrentScore(conn);
				SendEtc(conn);
				SendScore(conn);

				SendEmotion(conn, 14, 3);

				sprintf(temp, g_pMessageStringTable[_DN_Play_Quest], "Reset Celestial");
				SendClientMessage(conn, temp);

				sprintf(temp, "useitem,furystone quest reset %d", pMob[conn].extra.QuestInfo.Celestial.Reset);
				ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);

				if (amount > 1)
				BASE_SetItemAmount(item, amount - 1);

				else
				memset(item, 0, sizeof(STRUCT_ITEM));

				return true;
			}
		}
		if (pMob[conn].extra.ClassMaster != CELESTIAL)
		{
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		if(pMob[conn].MOB.CurrentScore.Level < 89)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_Level_Limit2]);
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		if(pMob[conn].extra.QuestInfo.Celestial.Lv90 == 1)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_Youve_Done_It_Already]);
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		if(pMob[conn].extra.Fame < 500)
		{
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		int _rand = rand()%115;

		if(_rand > 100)
			_rand -= 15;

		pMob[conn].extra.Fame -= 500;

		if(_rand < 60)
		{
			pMob[conn].extra.QuestInfo.Celestial.Lv90 = 1;

			SendEmotion(conn, 14, 3);
								 
			sprintf(temp, g_pMessageStringTable[_DN_Play_Quest], "Lv90");
			SendClientMessage(conn, temp);

			if (amount > 1)
				BASE_SetItemAmount(item, amount - 1);

			else
				memset(item, 0, sizeof(STRUCT_ITEM));

			STRUCT_ITEM Prize;
			memset(&Prize, 0, sizeof(STRUCT_ITEM));

			Prize.sIndex = 3502;

			PutItem(conn, &Prize);

			sprintf(temp, "useitem,furystone quest90 sucess");
			ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);
		}
		else
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_FAILURE]);
						
			if (amount > 1)
				BASE_SetItemAmount(item, amount - 1);

			else
				memset(item, 0, sizeof(STRUCT_ITEM));

			sprintf(temp, "useitem,furystone quest90 fail");
			ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);
		}

		return true;
	}
#pragma endregion
#pragma region Remédio da Coragem
	if (item->sIndex == 4046 || item->sIndex == 646)
	{
		int sAffect = GetEmptyAffect(conn, 30);

		if (sAffect == -1 || pMob[conn].Affect[sAffect].Type == 30 && pMob[conn].Affect[sAffect].Level != 500)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_CantEatMore]);
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		pMob[conn].Affect[sAffect].Type = 30;
		pMob[conn].Affect[sAffect].Level = 1;
		pMob[conn].Affect[sAffect].Value = 500;
		pMob[conn].Affect[sAffect].Time = 100;

		if (pMob[conn].Affect[sAffect].Time >= 324000)
			pMob[conn].Affect[sAffect].Time = 324000;

		pMob[conn].GetCurrentScore(conn);
		SendScore(conn);

		if (amount > 1)
			BASE_SetItemAmount(item, amount - 1);

		else
			memset(item, 0, sizeof(STRUCT_ITEM));

		sprintf(temp, "useitem,remedio_da_coragem dam:500");
		ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);
		return true;
	}
#pragma endregion
#pragma region Elixir da Coragem
	if (Vol == 230 && (item->sIndex == 647 || item->sIndex == 3378))
	{
		int sAffect = GetEmptyAffect(conn, 30);

		if (sAffect == -1 || pMob[conn].Affect[sAffect].Type == 30 && pMob[conn].Affect[sAffect].Level != 2000)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_CantEatMore]);
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		pMob[conn].Affect[sAffect].Type = 30;
		pMob[conn].Affect[sAffect].Level = 1;
		pMob[conn].Affect[sAffect].Value = 2000;
		pMob[conn].Affect[sAffect].Time = item->sIndex != 3378 ? AFFECT_1H : AFFECT_1D*30;

		if (pMob[conn].Affect[sAffect].Time >= 324000)
			pMob[conn].Affect[sAffect].Time = 324000;

		pMob[conn].GetCurrentScore(conn);
		SendScore(conn);

		if (amount > 1)
			BASE_SetItemAmount(item, amount - 1);

		else
			memset(item, 0, sizeof(STRUCT_ITEM));

		sprintf(temp, "useitem,elixir_da_coragem dam:2000");
		ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);
		return true;
	}
#pragma endregion

	return false;
}
