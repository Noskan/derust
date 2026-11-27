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

/// @file Quest_Special.cpp
/// @brief Handlers de NPCs especiais, habilidades, eventos e trocas avançadas.

#include "handlers/world/QuestInternal.h"

bool HandleQuest_Special(int conn, int npcIndex, int confirm, int npcMode, int npcMerc, int npcGrade, MSG_STANDARDPARM2* m)
{
	switch (npcMode)
	{
#pragma region MESTREHAB
	case MESTREHAB:
	{
		if(confirm == 0)
		{
			sprintf(temp, g_pMessageStringTable[_DN_Want_Stat_Init], StatSapphire);
			SendSay(npcIndex, temp);
			break;
		}

		for(int i = 1; i < 8; i++)
		{
			if(pMob[conn].MOB.Equip[i].sIndex)
			{
				SendSay(npcIndex, g_pMessageStringTable[_NN_Cant_with_armor]);
				return true;
			}
		}

		int NumSappInv = 0;

		int RetornoHab = 0;

		for (int i = 0; i < MAX_CARRY - 4; i = BASE_NextCarry(pMob[conn].MOB.Carry, i))
		{
			if(pMob[conn].MOB.Carry[i].sIndex == 697)
				NumSappInv++;

			else if(pMob[conn].MOB.Carry[i].sIndex == 4131)
				NumSappInv += 10;

			else if(pMob[conn].MOB.Carry[i].sIndex == 3336)
				RetornoHab = 1;
		}

		if(NumSappInv < StatSapphire && RetornoHab == 0)
		{
			sprintf(temp, g_pMessageStringTable[_DN_Need_D_Sapphire2], StatSapphire);
			SendSay(npcIndex, temp);
			break;
		}

		if(RetornoHab == 1)
		{
			for (int i = 0; i < MAX_CARRY - 4; i = BASE_NextCarry(pMob[conn].MOB.Carry, i))
			{
				if(pMob[conn].MOB.Carry[i].sIndex == 3336)
				{
					memset(&pMob[conn].MOB.Carry[i], 0, sizeof(STRUCT_ITEM));
					SendItem(conn, ITEM_PLACE_CARRY, i, &pMob[conn].MOB.Carry[i]);

					break;
				}
			}

			goto DiretoReset;
		}
		if(NumSappInv > StatSapphire)
			NumSappInv = StatSapphire;

		for (int i = 0; i < MAX_CARRY - 4 && NumSappInv > 0; i = BASE_NextCarry(pMob[conn].MOB.Carry, i))
		{
			if(pMob[conn].MOB.Carry[i].sIndex == 697)
			{
				memset(&pMob[conn].MOB.Carry[i], 0, sizeof(STRUCT_ITEM));
				SendItem(conn, ITEM_PLACE_CARRY, i, &pMob[conn].MOB.Carry[i]);

				NumSappInv--;
			}

			else if(pMob[conn].MOB.Carry[i].sIndex == 4131 && NumSappInv >= 10)
			{
				memset(&pMob[conn].MOB.Carry[i], 0, sizeof(STRUCT_ITEM));
				SendItem(conn, ITEM_PLACE_CARRY, i, &pMob[conn].MOB.Carry[i]);

				NumSappInv -= 10;
			}
		}
DiretoReset:

		int cls = pMob[conn].MOB.Class;

		if(cls < 0 || cls > 3)
			break;

		int resetp = 100;
        int score = pMob[conn].MOB.ScoreBonus;
        int str = pMob[conn].MOB.BaseScore.Str - BaseSIDCHM[cls][0];
        int _int = pMob[conn].MOB.BaseScore.Int - BaseSIDCHM[cls][1];
        int dex = pMob[conn].MOB.BaseScore.Dex - BaseSIDCHM[cls][2];
        int con = pMob[conn].MOB.BaseScore.Con - BaseSIDCHM[cls][3];
                                
		if (str <= resetp)
        {
            pMob[conn].MOB.BaseScore.Str -= str;
            score += str;
        }
        else
        {
            pMob[conn].MOB.BaseScore.Str -= resetp;
            score += resetp;
        }
                                
		if (_int <= resetp)
        {
            pMob[conn].MOB.BaseScore.Int -= _int;
            score += _int;
        }
        else
        {
            pMob[conn].MOB.BaseScore.Int -= resetp;
            score += resetp;
        }
                                
		if (dex <= resetp)
        {
            pMob[conn].MOB.BaseScore.Dex -= dex;
            score += dex;
        }
                                
		else
        {
            pMob[conn].MOB.BaseScore.Dex -= resetp;
            score += resetp;
        }

        if (con <= resetp )
        {
            pMob[conn].MOB.BaseScore.Con -= con;
            score += con;
        }
        else
        {
            pMob[conn].MOB.BaseScore.Con -= resetp;
            score += resetp;
        }

		sprintf(temp, "etc,mestre_hab resetscore 100 name:%s conn:%d", pMob[conn].MOB.MobName, conn);
		Log(temp, "-system", 0);

		BASE_GetBonusScorePoint(&pMob[conn].MOB, &pMob[conn].extra);
        BASE_GetHpMp(&pMob[conn].MOB, &pMob[conn].extra);

        SetAffect(conn, 44, 20, 20);

        SendScore(conn);
        SendEtc(conn);
        SendClientMessage(conn, g_pMessageStringTable[_NN_Stat_Initialized]);
	} break;
#pragma endregion
#pragma region COMP_SEPHI
	case COMP_SEPHI:
	{
		if(confirm == 0)
		{
			SendSay(npcIndex, g_pMessageStringTable[_NN_Soul_n_8_Sephera]);
			break;
		}
		int Pedras = 0;

		for(int j = 0; j < 8; j++)
		{
			for (int i = 0; i < MAX_CARRY - 4; i = BASE_NextCarry(pMob[conn].MOB.Carry, i))
			{
				if (pMob[conn].MOB.Carry[i].sIndex != (1744 + j))
					continue;

				Pedras++;
				break;
			}
		}

		if(Pedras < 8)
		{
			SendSay(npcIndex, g_pMessageStringTable[_NN_Need_8_Sephera]);
			break;
		}
		if(pMob[conn].MOB.Coin < 30000000)
		{
			SendSay(npcIndex, g_pMessageStringTable[_NN_Need_8_Sephera]);
			break;
		}

		pMob[conn].MOB.Coin -= 30000000;
		SendEtc(conn);

		for(int j = 0; j < 8; j++)
		{
			for (int i = 0; i < MAX_CARRY - 4; i = BASE_NextCarry(pMob[conn].MOB.Carry, i))
			{
				if (pMob[conn].MOB.Carry[i].sIndex != (1744 + j))
					continue;

				BASE_ClearItem(&pMob[conn].MOB.Carry[i]);
				SendItem(conn, ITEM_PLACE_CARRY, i, &pMob[conn].MOB.Carry[i]);

				Pedras--;
				break;
			}
		}

		STRUCT_ITEM Item;

		memset(&Item, 0, sizeof(STRUCT_ITEM));

		Item.sIndex = 1760 + pMob[npcIndex].MOB.Class;

		PutItem(conn, &Item);

		SendSay(npcIndex, g_pMessageStringTable[_NN_Sepherot_created]);

		sprintf(temp, "etc,sephirot create class:%d name:%s conn:%d", pMob[npcIndex].MOB.Class, pMob[conn].MOB.MobName, conn);
		Log(temp, "-system", 0);
	} break;
#pragma endregion
#pragma region BLACKORACLE
	case BLACKORACLE:
	{
		int i = 0;

		for (i = 0; i < MAX_CARRY - 4; i = BASE_NextCarry(pMob[conn].MOB.Carry, i))
		{
			if(pMob[conn].MOB.Carry[i].sIndex == 1740)
				break;
		}

		if (i == MAX_CARRY - 4)
		{
			SendSay(npcIndex, g_pMessageStringTable[_NN_Set_Soul]);
			break;
		}

		if(pMob[conn].MOB.Carry[i+1].sIndex != 1741)
		{
			SendSay(npcIndex, g_pMessageStringTable[_NN_Set_Soul]);
			break;
		}


		int NumSepInv = 0;
		int j = 0;

		for (j = 0; j < MAX_CARRY - 4; j = BASE_NextCarry(pMob[conn].MOB.Carry, j))
		{
			if (pMob[conn].MOB.Carry[j].sIndex == 697)
				NumSepInv++;

			else if (pMob[conn].MOB.Carry[j].sIndex == 4131)
				NumSepInv += 10;
		}

		if (NumSepInv < 10)
		{
			sprintf(temp, g_pMessageStringTable[_DN_Need_D_Sapphire], 10);
			SendSay(npcIndex, temp);
			break;
		}
		int Saphire = 10;

		for (j = 0; j < MAX_CARRY - 4 && Saphire > 0; j = BASE_NextCarry(pMob[conn].MOB.Carry, j))
		{
			if (pMob[conn].MOB.Carry[j].sIndex == 697)
			{
				memset(&pMob[conn].MOB.Carry[j], 0, sizeof(STRUCT_ITEM));
				SendItem(conn, ITEM_PLACE_CARRY, j, &pMob[conn].MOB.Carry[j]);
				Saphire--;
			}

			else if (pMob[conn].MOB.Carry[j].sIndex == 4131)
			{
				memset(&pMob[conn].MOB.Carry[j], 0, sizeof(STRUCT_ITEM));
				SendItem(conn, ITEM_PLACE_CARRY, j, &pMob[conn].MOB.Carry[j]);

				if((Saphire - 10) < 0)
					Saphire = 0;
												
				else
					Saphire -= 10;
			}
		}

		memset(&pMob[conn].MOB.Carry[i], 0, sizeof(STRUCT_ITEM));
		SendItem(conn, ITEM_PLACE_CARRY, i, &pMob[conn].MOB.Carry[i]);
							
		memset(&pMob[conn].MOB.Carry[i+1], 0, sizeof(STRUCT_ITEM));
		SendItem(conn, ITEM_PLACE_CARRY, i+1, &pMob[conn].MOB.Carry[i+1]);

		STRUCT_ITEM Item;
		memset(&Item, 0, sizeof(STRUCT_ITEM));

		Item.sIndex = 1742;

		PutItem(conn, &Item);

		SendSay(npcIndex, g_pMessageStringTable[_NN_Soul_Merged]);

		sprintf(temp, "etc,stone_immortality create name:%s conn:%d", pMob[conn].MOB.MobName, conn);
		Log(temp, "-system", 0);
	} break;
#pragma endregion
#pragma region CARBUNCLE_WIND
	case CARBUNCLE_WIND:
	{
		if (pMob[conn].extra.ClassMaster != MORTAL && pMob[conn].extra.ClassMaster != ARCH)
		{
			SendSay(npcIndex, g_pMessageStringTable[_NN_Level_Limit2]);
			break;
		}

		if (pMob[conn].MOB.CurrentScore.Level >= 100)
		{
			SendSay(npcIndex, g_pMessageStringTable[_NN_Level_Limit2]);
			break;
		}

		if(pMob[npcIndex].MOB.ShortSkill[0] != 255)
			SetAffect(conn, pMob[npcIndex].MOB.ShortSkill[0], 400, 200);

		if(pMob[npcIndex].MOB.ShortSkill[1] != 255)
			SetAffect(conn, pMob[npcIndex].MOB.ShortSkill[1], 400, 200);

		if(pMob[npcIndex].MOB.ShortSkill[2] != 255)
			SetAffect(conn, pMob[npcIndex].MOB.ShortSkill[2], 400, 200);

		if(pMob[npcIndex].MOB.ShortSkill[3] != 255)
			SetAffect(conn, pMob[npcIndex].MOB.ShortSkill[3], 400, 200);

		pMob[conn].GetCurrentScore(conn);
		SendScore(conn);

		sprintf(temp, g_pMessageStringTable[_SN_CARBUNCLEMSG], pMob[conn].MOB.MobName);
		SendSay(npcIndex, temp);

		sprintf(temp, "etc,carbuncle buff name:%s conn:%d", pMob[conn].MOB.MobName, conn);
		Log(temp, "-system", 0);
	} break;
#pragma endregion
#pragma region PERZEN
	case PERZEN:
	{
		int i = 0;

		for (i = 0; i < MAX_CARRY - 4; i = BASE_NextCarry(pMob[conn].MOB.Carry, i))
		{
			if (pMob[conn].MOB.Carry[i].sIndex != pMob[npcIndex].MOB.Carry[0].sIndex)
				continue;
								
			break;
		}

		if (i == MAX_CARRY - 4)
		{
			sprintf(temp, g_pMessageStringTable[_SN_BRINGITEM], g_pItemList[pMob[npcIndex].MOB.Carry[0].sIndex].Name);
			SendSay(npcIndex, temp);
			break;
		}

		BASE_ClearItem(&pMob[conn].MOB.Carry[i]);

		pMob[conn].MOB.Carry[i].sIndex = pMob[npcIndex].MOB.Carry[1].sIndex;
							
		BASE_SetItemDate(&pMob[conn].MOB.Carry[i], 30);

		SendItem(conn, ITEM_PLACE_CARRY, i, &pMob[conn].MOB.Carry[i]);

		sprintf(temp, "etc,premium mount started type:%d name:%s conn:%d", pMob[npcIndex].MOB.Carry[1].sIndex, pMob[conn].MOB.MobName, conn);
		Log(temp, "-system", 0);
	} break;
#pragma endregion
#pragma region KIBITA
	case KIBITA:
	{
		if(pMob[conn].extra.Citizen == 0 && pMob[conn].MOB.Coin >= 4000000)
		{
			pMob[conn].MOB.Coin -= 4000000;
			pMob[conn].extra.Citizen = ServerIndex + 1;
			SendClientMessage(conn, g_pMessageStringTable[_DD_JOINTOWNPEP]);

			int Guild = pMob[conn].MOB.Guild;

			if (Guild && pMob[conn].MOB.GuildLevel == 9)
			{
				MSG_GuildInfo sm_gi;
				memset(&sm_gi, 0, sizeof(MSG_GuildInfo));

				sm_gi.Type = _MSG_GuildInfo;
				sm_gi.Size = sizeof(MSG_GuildInfo);
				sm_gi.ID = conn;

				sm_gi.Guild = Guild;

				GuildInfo[Guild].Citizen = ServerIndex + 1;

				sm_gi.GuildInfo = GuildInfo[Guild];

				DBServerSocket.SendOneMessage((char*)&sm_gi, sizeof(MSG_GuildInfo));
			}

			sprintf(temp, "etc,set citizen server:%d name:%s conn:%d", ServerIndex + 1, pMob[conn].MOB.MobName, conn);
			Log(temp, "-system", 0);

			break;
		}

		struct tm when;
		time_t now;
		time(&now);
		when = *localtime(&now);

#ifdef KIBITA_SOUL
		if (when.tm_wday != 0 && when.tm_wday != 6 && when.tm_hour == 21 && pMob[conn].extra.ClassMaster == MORTAL && pMob[conn].MOB.CurrentScore.Level < 369)
		{
			int i = 0;

			for (i = 0; i < MAX_CARRY - 4; i = BASE_NextCarry(pMob[conn].MOB.Carry, i))
			{
				if (pMob[conn].MOB.Carry[i].sIndex != 420)
					continue;

				break;
			}

			if (i != MAX_CARRY - 4)
			{
				BASE_ClearItem(&pMob[conn].MOB.Carry[i]);
				SendItem(conn, ITEM_PLACE_CARRY, i, &pMob[conn].MOB.Carry[i]);

				SendClientMessage(conn, g_pMessageStringTable[_DN_TOWN_SOUL_BUFF]);

				int sAffect = GetEmptyAffect(conn, 29);

				if (sAffect == -1)
					return true;
				

				pMob[conn].Affect[sAffect].Type = 29;
				pMob[conn].Affect[sAffect].Level = 0;
				pMob[conn].Affect[sAffect].Value = 0;
				pMob[conn].Affect[sAffect].Time = AFFECT_1H;

				pMob[conn].GetCurrentScore(conn);
				SendScore(conn);

				DoTeleport(conn, 2454 + rand() % 3, 1843 + rand() % 3);

				sprintf(temp, "etc,kibita limit of soul mortal name:%s conn:%d", pMob[conn].MOB.MobName, conn);
				Log(temp, "-system", 0);
				break;
			}

			else
			{
				SendClientMessage(conn, g_pMessageStringTable[_DN_TOWN_SOUL_NOBUFF]);
				break;
			}
		}
#endif
		if(pMob[conn].extra.ClassMaster == MORTAL && (pMob[conn].MOB.LearnedSkill[0] & (1 << 30)) == 0 && pMob[conn].MOB.CurrentScore.Level >= 369)
		{
			int i = 0;

			int stones[4] = {5334, 5336, 5335, 5337};

			for (i = 0; i < MAX_CARRY - 4; i = BASE_NextCarry(pMob[conn].MOB.Carry, i))
			{
				if (pMob[conn].MOB.Carry[i].sIndex != stones[pMob[conn].MOB.Class])
					continue;
								
				break;
			}

			if (i != MAX_CARRY - 4)
			{
				BASE_ClearItem(&pMob[conn].MOB.Carry[i]);
				SendItem(conn, ITEM_PLACE_CARRY, i, &pMob[conn].MOB.Carry[i]);

				pMob[conn].MOB.LearnedSkill[0] |= 1 << 30;

				memset(&pMob[conn].MOB.Equip[15], 0, sizeof(STRUCT_ITEM));

				if(pMob[conn].MOB.Clan == 7)
					pMob[conn].MOB.Equip[15].sIndex = 3194;

				else if(pMob[conn].MOB.Clan == 8)
					pMob[conn].MOB.Equip[15].sIndex = 3195;

				else
					pMob[conn].MOB.Equip[15].sIndex = 3196;

				SendItem(conn, ITEM_PLACE_EQUIP, 15, &pMob[conn].MOB.Equip[15]);

				CharLogOut(conn);

				sprintf(temp, "etc,learned limit of soul mortal name:%s conn:%d", pMob[conn].MOB.MobName, conn);
				Log(temp, "-system", 0);

				SendClientSignalParm(conn, ESCENE_FIELD, _MSG_SendArchEffect, pUser[conn].Slot);
				break;
			}
		}

		SendClientMessage(conn, g_pMessageStringTable[_DN_TOWN_SORRY]);
	} break;
#pragma endregion
#pragma region SOBREVIVENTE
	case SOBREVIVENTE:
	{
		int i = 0;
		for (i = 0; i < MAX_CARRY - 4; i = BASE_NextCarry(pMob[conn].MOB.Carry, i))
		{
			if (pMob[conn].MOB.Carry[i].sIndex != 4127)
				continue;
								
			break;
		}

		if (i != MAX_CARRY - 4)
		{
			BASE_ClearItem(&pMob[conn].MOB.Carry[i]);
			SendItem(conn, ITEM_PLACE_CARRY, i, &pMob[conn].MOB.Carry[i]);

			pMob[conn].extra.KefraTicket += 100;

			sprintf(temp, "etc,kefraticket ticket:%d name:%s conn:%d", pMob[conn].extra.KefraTicket, pMob[conn].MOB.MobName, conn);
			Log(temp, "-system", 0);

			sprintf(temp, g_pMessageStringTable[_DN_CHANGE_COUNT], pMob[conn].extra.KefraTicket);
			SendClientMessage(conn, temp);
		}

	} break;
#pragma endregion
#pragma region GUARDA_REAL_EVT1
	case GUARDA_REAL_EVT1:
		{
			int ItemReq = 4055;

			int i = 0;
			for (i = 0; i < MAX_CARRY - 4; i = BASE_NextCarry(pMob[conn].MOB.Carry, i))
			{
				if (pMob[conn].MOB.Carry[i].sIndex != ItemReq)
					continue;
								
				break;
			}

			if (i != MAX_CARRY - 4)
			{
				BASE_ClearItem(&pMob[conn].MOB.Carry[i]);
				SendItem(conn, ITEM_PLACE_CARRY, i, &pMob[conn].MOB.Carry[i]);

				STRUCT_ITEM Item;
				memset(&Item, 0, sizeof(STRUCT_ITEM));

				int _rd = rand()%100;

				if(_rd < 25)
					Item.sIndex = 413;
				
				else if(_rd < 50)
					Item.sIndex = 412;
				
				else if(_rd < 75)
					Item.sIndex = 413;
				
				else
					Item.sIndex = 412;

				sprintf(temp, g_pMessageStringTable[_SN_Item_Arrived], g_pItemList[Item.sIndex].Name);
				SendClientMessage(conn, temp);
				break;
			}

			sprintf(temp, g_pMessageStringTable[_SN_BRINGITEM], g_pItemList[ItemReq].Name);
			SendSay(npcIndex, temp);
		} break;
#pragma endregion
	default:
		return false;
	}

	return true;
}
