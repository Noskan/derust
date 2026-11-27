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

/// @file UseItem_Mithril.cpp
/// @brief Sub-handler de feijões mágicos, perdão, gemas elementais, jóias PvP, armazenagem e Barras de Mythril.

#include "handlers/items/UseItemInternal.h"

bool UseItem_GemsAndMithril(int conn, MSG_UseItem* m, STRUCT_ITEM* item, int Vol, int amount)
{
#pragma region Feijões mágicos - Removedor
	if(Vol == 186)
	{
		STRUCT_ITEM *dest = GetItemPointer(&pMob[conn].MOB, pUser[conn].Cargo, m->DestType, m->DestPos);

		if (dest == NULL)
		{
			sprintf(temp, "err,use item fail dest %d %d", m->DestType, m->DestPos);
			ItemLog(temp, "-system", 0);
			return true;
		}

		if (m->DestType || m->DestPos >= 6 && m->DestPos < 16 || m->DestType == ITEM_PLACE_EQUIP && m->DestPos == 0)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_Only_To_Equips]);

			ItemLog("err,use item fail - dest", pUser[conn].AccountName, pUser[conn].IP);

			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		int sanc = BASE_GetItemSanc(dest);

		if(sanc < 1)
		{
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		int color = item->sIndex - 3407;


		//Removedor
		if(color == 10)
		{
			if ((dest->stEffect[0].cEffect && dest->stEffect[0].cEffect < 116 || dest->stEffect[0].cEffect && dest->stEffect[0].cEffect > 125))
			{
				if ((dest->stEffect[1].cEffect && dest->stEffect[1].cEffect < 116 || dest->stEffect[1].cEffect && dest->stEffect[1].cEffect > 125))
				{
					if ((dest->stEffect[2].cEffect && dest->stEffect[2].cEffect < 116 || dest->stEffect[2].cEffect && dest->stEffect[2].cEffect > 125))
					{
						SendClientMessage(conn, g_pMessageStringTable[_NN_Cant_Refine_More]);
						SendItem(conn, m->SourType, m->SourPos, item);
						return true;
					}
					dest->stEffect[2].cEffect = 43;
				}
				else
					dest->stEffect[1].cEffect = 43;
			}
			else
				dest->stEffect[0].cEffect = 43;
		}
		else
		{
			if (dest->stEffect[0].cEffect && dest->stEffect[0].cEffect != 43 && (dest->stEffect[0].cEffect && dest->stEffect[0].cEffect < 116 || dest->stEffect[0].cEffect && dest->stEffect[0].cEffect > 125))
			{
				if (dest->stEffect[1].cEffect && dest->stEffect[1].cEffect != 43 && (dest->stEffect[1].cEffect && dest->stEffect[1].cEffect < 116 || dest->stEffect[1].cEffect && dest->stEffect[1].cEffect > 125))
				{
					if (dest->stEffect[2].cEffect && dest->stEffect[2].cEffect != 43 && (dest->stEffect[2].cEffect && dest->stEffect[2].cEffect < 116 || dest->stEffect[2].cEffect && dest->stEffect[2].cEffect > 125))
					{
						SendClientMessage(conn, g_pMessageStringTable[_NN_Cant_Refine_More]);
						SendItem(conn, m->SourType, m->SourPos, item);
						return true;
					}
					dest->stEffect[2].cEffect = 116 + color;
				}
				else
					dest->stEffect[1].cEffect = 116 + color;
			}
			else
				dest->stEffect[0].cEffect = 116 + color;
		}

		sprintf(temp, "%s", g_pMessageStringTable[_NN_Refine_Success]);
		SendClientMessage(conn, temp);

		pMob[conn].GetCurrentScore(conn);

		SendScore(conn);
		sprintf(temp, "useitem,paint success %d+%d - (%d,%d,%d)", dest->sIndex, sanc, item->stEffect[0].cEffect, item->stEffect[1].cEffect, item->stEffect[2].cEffect);
		ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);

		if (amount > 1)
			BASE_SetItemAmount(item, amount - 1);

		else
			memset(item, 0, sizeof(STRUCT_ITEM));

		SendEquip(conn, 0);
		SendItem(conn, m->DestType, m->DestPos, dest);
		SendEmotion(conn, 14, 3);

		return true;
	}
	#pragma endregion
#pragma region Pergaminho do Perdão
	if (Vol == 203)
	{
		SetPKPoint(conn, 150);

		MSG_CreateMob sm_pp;
		memset(&sm_pp, 0, sizeof(MSG_CreateMob));
		GetCreateMob(conn, &sm_pp);

		GridMulticast(pMob[conn].TargetX, pMob[conn].TargetY, (MSG_STANDARD*)&sm_pp, 0);

		SendEmotion(conn, 14, 3);

		sprintf(temp, "useitem,perdao new cp %d", 75);
		ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);

		if (amount > 1)
			BASE_SetItemAmount(item, amount - 1);

		else
			memset(item, 0, sizeof(STRUCT_ITEM));

		return true;
	}
#pragma endregion
#pragma region Gema de diamante
	if(Vol == 180)
	{
		STRUCT_ITEM *dest = GetItemPointer(&pMob[conn].MOB, pUser[conn].Cargo, m->DestType, m->DestPos);

		if (dest == NULL)
		{
			sprintf(temp, "err,use item fail dest %d %d", m->DestType, m->DestPos);
			ItemLog(temp, "-system", 0);
			return true;
		}

		if (m->DestType || m->DestPos >= 8 && m->DestPos < 16 || m->DestType == ITEM_PLACE_EQUIP && m->DestPos == 0)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_Only_To_Equips]);

			ItemLog("err,use item fail - dest", pUser[conn].AccountName, pUser[conn].IP);

			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		int sanc = BASE_GetItemSanc(dest);
		int ItemId = dest->sIndex;

		if(sanc < REF_10 && g_pItemList[ItemId].nGrade < 5)
		{
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		if(g_pItemList[ItemId].nGrade == 6)
			ItemId -= 1;

		else if(g_pItemList[ItemId].nGrade == 7)
			ItemId -= 2;

		else if(g_pItemList[ItemId].nGrade == 8)
			ItemId -= 3;

		dest->sIndex = ItemId;

		if(sanc == REF_10)
			BASE_SetItemSanc(dest, 10, 0);

		else if(sanc == REF_11)
			BASE_SetItemSanc(dest, 11, 0);

		else if(sanc == REF_12)
			BASE_SetItemSanc(dest, 12, 0);

		else if(sanc == REF_13)
			BASE_SetItemSanc(dest, 13, 0);

		else if(sanc == REF_14)
			BASE_SetItemSanc(dest, 14, 0);

		else if(sanc == REF_15)
			BASE_SetItemSanc(dest, 15, 0);

		pMob[conn].GetCurrentScore(conn);

		SendScore(conn);
		sprintf(temp, "useitem,change jewel success %d+%d - (%d,%d,%d)", dest->sIndex, sanc, item->stEffect[0].cEffect, item->stEffect[1].cEffect, item->stEffect[2].cEffect);
		ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);

		if (amount > 1)
			BASE_SetItemAmount(item, amount - 1);

		else
			memset(item, 0, sizeof(STRUCT_ITEM));

		SendItem(conn, m->DestType, m->DestPos, dest);
		SendEmotion(conn, 14, 3);

		return true;
	}
#pragma endregion
#pragma region Gema de esmeralda
	if(Vol == 181)
	{
		STRUCT_ITEM *dest = GetItemPointer(&pMob[conn].MOB, pUser[conn].Cargo, m->DestType, m->DestPos);

		if (dest == NULL)
		{
			sprintf(temp, "err,use item fail dest %d %d", m->DestType, m->DestPos);
			ItemLog(temp, "-system", 0);
			return true;
		}

		if (m->DestType || m->DestPos >= 8 && m->DestPos < 16 || m->DestType == ITEM_PLACE_EQUIP && m->DestPos == 0)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_Only_To_Equips]);

			ItemLog("err,use item fail - dest", pUser[conn].AccountName, pUser[conn].IP);

			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		int sanc = BASE_GetItemSanc(dest);
		int ItemId = dest->sIndex;

		if(sanc < REF_10 && g_pItemList[ItemId].nGrade < 5)
		{
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		if(g_pItemList[ItemId].nGrade == 5)
			ItemId += 1;

		else if(g_pItemList[ItemId].nGrade == 7)
			ItemId -= 1;

		else if(g_pItemList[ItemId].nGrade == 8)
			ItemId -= 2;

		dest->sIndex = ItemId;

		if(sanc == REF_10)
			BASE_SetItemSanc(dest, 10, 1);

		else if(sanc == REF_11)
			BASE_SetItemSanc(dest, 11, 1);

		else if(sanc == REF_12)
			BASE_SetItemSanc(dest, 12, 1);

		else if(sanc == REF_13)
			BASE_SetItemSanc(dest, 13, 1);

		else if(sanc == REF_14)
			BASE_SetItemSanc(dest, 14, 1);

		else if(sanc == REF_15)
			BASE_SetItemSanc(dest, 15, 1);

		pMob[conn].GetCurrentScore(conn);

		SendScore(conn);
		sprintf(temp, "useitem,change jewel success %d+%d - (%d,%d,%d)", dest->sIndex, sanc, item->stEffect[0].cEffect, item->stEffect[1].cEffect, item->stEffect[2].cEffect);
		ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);

		if (amount > 1)
			BASE_SetItemAmount(item, amount - 1);

		else
			memset(item, 0, sizeof(STRUCT_ITEM));

		SendItem(conn, m->DestType, m->DestPos, dest);
		SendEmotion(conn, 14, 3);

		return true;
	}
#pragma endregion
#pragma region Gema de Coral
	if(Vol == 182)
	{
		STRUCT_ITEM *dest = GetItemPointer(&pMob[conn].MOB, pUser[conn].Cargo, m->DestType, m->DestPos);

		if (dest == NULL)
		{
			sprintf(temp, "err,use item fail dest %d %d", m->DestType, m->DestPos);
			ItemLog(temp, "-system", 0);
			return true;
		}

		if (m->DestType || m->DestPos >= 8 && m->DestPos < 16 || m->DestType == ITEM_PLACE_EQUIP && m->DestPos == 0)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_Only_To_Equips]);

			ItemLog("err,use item fail - dest", pUser[conn].AccountName, pUser[conn].IP);

			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		int sanc = BASE_GetItemSanc(dest);
		int ItemId = dest->sIndex;

		if(sanc < REF_10 && g_pItemList[ItemId].nGrade < 5)
		{
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		if(g_pItemList[ItemId].nGrade == 5)
			ItemId += 2;

		else if(g_pItemList[ItemId].nGrade == 6)
			ItemId += 1;

		else if(g_pItemList[ItemId].nGrade == 8)
			ItemId -= 1;

		dest->sIndex = ItemId;

		if(sanc == REF_10)
			BASE_SetItemSanc(dest, 10, 2);

		else if(sanc == REF_11)
			BASE_SetItemSanc(dest, 11, 2);

		else if(sanc == REF_12)
			BASE_SetItemSanc(dest, 12, 2);

		else if(sanc == REF_13)
			BASE_SetItemSanc(dest, 13, 2);

		else if(sanc == REF_14)
			BASE_SetItemSanc(dest, 14, 2);

		else if(sanc == REF_15)
			BASE_SetItemSanc(dest, 15, 2);

		pMob[conn].GetCurrentScore(conn);

		SendScore(conn);
		sprintf(temp, "useitem,change jewel success %d+%d - (%d,%d,%d)", dest->sIndex, sanc, item->stEffect[0].cEffect, item->stEffect[1].cEffect, item->stEffect[2].cEffect);
		ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);

		if (amount > 1)
			BASE_SetItemAmount(item, amount - 1);

		else
			memset(item, 0, sizeof(STRUCT_ITEM));

		SendItem(conn, m->DestType, m->DestPos, dest);
		SendEmotion(conn, 14, 3);

		return true;
	}
#pragma endregion
#pragma region Gema de Garnet
	if(Vol == 183)
	{
		STRUCT_ITEM *dest = GetItemPointer(&pMob[conn].MOB, pUser[conn].Cargo, m->DestType, m->DestPos);

		if (dest == NULL)
		{
			sprintf(temp, "err,use item fail dest %d %d", m->DestType, m->DestPos);
			ItemLog(temp, "-system", 0);
			return true;
		}

		if (m->DestType || m->DestPos >= 8 && m->DestPos < 16 || m->DestType == ITEM_PLACE_EQUIP && m->DestPos == 0)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_Only_To_Equips]);

			ItemLog("err,use item fail - dest", pUser[conn].AccountName, pUser[conn].IP);

			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		int sanc = BASE_GetItemSanc(dest);
		int ItemId = dest->sIndex;

		if(sanc < REF_10 && g_pItemList[ItemId].nGrade < 5)
		{
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		if(g_pItemList[ItemId].nGrade == 5)
			ItemId += 3;

		else if(g_pItemList[ItemId].nGrade == 6)
			ItemId += 2;

		else if(g_pItemList[ItemId].nGrade == 7)
			ItemId += 1;

		dest->sIndex = ItemId;

		if(sanc == REF_10)
			BASE_SetItemSanc(dest, 10, 3);

		else if(sanc == REF_11)
			BASE_SetItemSanc(dest, 11, 3);

		else if(sanc == REF_12)
			BASE_SetItemSanc(dest, 12, 3);

		else if(sanc == REF_13)
			BASE_SetItemSanc(dest, 13, 3);

		else if(sanc == REF_14)
			BASE_SetItemSanc(dest, 14, 3);

		else if(sanc == REF_15)
			BASE_SetItemSanc(dest, 15, 3);

		pMob[conn].GetCurrentScore(conn);

		SendScore(conn);
		sprintf(temp, "useitem,change jewel success %d+%d - (%d,%d,%d)", dest->sIndex, sanc, item->stEffect[0].cEffect, item->stEffect[1].cEffect, item->stEffect[2].cEffect);
		ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);

		if (amount > 1)
			BASE_SetItemAmount(item, amount - 1);

		else
			memset(item, 0, sizeof(STRUCT_ITEM));

		SendItem(conn, m->DestType, m->DestPos, dest);
		SendEmotion(conn, 14, 3);

		return true;
	}
#pragma endregion
#pragma region Entrada do Território
	if(Vol == 188)
	{
		int territorio = item->sIndex - 4111;

		if(territorio <= 1 && pMob[conn].extra.ClassMaster != ARCH && pMob[conn].extra.ClassMaster != MORTAL)
		{
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}
		else if(territorio >= 2 && pMob[conn].extra.ClassMaster != CELESTIAL && pMob[conn].extra.ClassMaster != SCELESTIAL && pMob[conn].extra.ClassMaster != CELESTIALCS)
		{
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		if(territorio == 0)
			DoTeleport(conn, (3639 + W2Rand()%5 - 3), (3639 + W2Rand()%5 - 3));

		else if(territorio == 1)
			DoTeleport(conn, (3782 + W2Rand()%5 - 3), (3527 + W2Rand()%5 - 3));

		else
			DoTeleport(conn, (3911 + W2Rand()%5 - 3), (3655 + W2Rand()%5 - 3));

		sprintf(temp, "useitem,territorio ticket used %d", territorio);
		ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);

		if (amount > 1)
			BASE_SetItemAmount(item, amount - 1);

		else
			memset(item, 0, sizeof(STRUCT_ITEM));

		return true;
	}
#pragma endregion
#pragma region Barra de Prata
	if(Vol == 185)
	{
		int gold = 0;

		if(item->sIndex == 4010)
			gold = 100000000;

		else if(item->sIndex == 4011)
			gold = 1000000000;

		else if(item->sIndex == 4026)
			gold = 1000000;

		else if(item->sIndex == 4027)
			gold = 5000000;

		else if(item->sIndex == 4028)
			gold = 10000000;

		else if(item->sIndex == 4029)
			gold = 50000000;

		unsigned int fGold = pMob[conn].MOB.Coin + gold;

		if(fGold > 2000000000)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_Cant_get_more_than_2G]);
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}
							
		pMob[conn].MOB.Coin = fGold;
		SendEtc(conn);
		SendEmotion(conn, 14, 3);

		sprintf(temp, "useitem,silver coin used V:%d fV:%d", gold, fGold);
		ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);

		if (amount > 1)
			BASE_SetItemAmount(item, amount - 1);

		else
			memset(item, 0, sizeof(STRUCT_ITEM));

		return true;
	}
#pragma endregion
#pragma region Jóias PvP
	if (Vol == 242)
	{
		int sAffect = GetEmptyAffect(conn, 8);

		int joia = 0;

		if(item->sIndex == 3200)
			joia = 0;

		else if(item->sIndex == 3201)
			joia = 1;

		else if(item->sIndex == 3202)
			joia = 2;

		else if(item->sIndex == 3204)
			joia = 3;

		else if(item->sIndex == 3205)
			joia = 4;

		else if(item->sIndex == 3206)
			joia = 5;

		else if(item->sIndex == 3208)
			joia = 6;

		else if(item->sIndex == 3209)
			joia = 7;

		if (sAffect == -1)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_CantEatMore]);
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		if(pMob[conn].Affect[sAffect].Type != 8)
		{
			pMob[conn].Affect[sAffect].Type = 8;
			pMob[conn].Affect[sAffect].Value = 1 << joia;
			pMob[conn].Affect[sAffect].Level = 0;
		}

		else if((pMob[conn].Affect[sAffect].Value & (1 << joia)) == 0)
			pMob[conn].Affect[sAffect].Value |= 1 << joia;

		pMob[conn].Affect[sAffect].Time = AFFECT_1H;
		pMob[conn].GetCurrentScore(conn);
		SendScore(conn);

		if (amount > 1)
			BASE_SetItemAmount(item, amount - 1);

		else
			memset(item, 0, sizeof(STRUCT_ITEM));

		sprintf(temp, "useitem,joia_pvp joia:%d", item->sIndex);
		ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);
		return true;
	}
#pragma endregion
#pragma region Armazenagem - Recuperação
	if(Vol == 243)
	{
		if(item->sIndex == 3203)
		{
			int flagSkillOff = FALSE;

			for (int k = 0; k < MAX_AFFECT; k++)
			{
				int skillAffectType = pMob[conn].Affect[k].Type;
				if (skillAffectType == 1 || skillAffectType == 3 || skillAffectType == 5 || skillAffectType == 7 || skillAffectType == 10 || skillAffectType == 12 || skillAffectType == 20 || skillAffectType == 32)
				{
					pMob[conn].Affect[k].Type = 0;
					flagSkillOff = TRUE;
				}
			}

			if (flagSkillOff == TRUE)
			{
				pMob[conn].GetCurrentScore(conn);
				SendScore(conn);
			}
		}
		if (amount > 1)
			BASE_SetItemAmount(item, amount - 1);

		else
			memset(item, 0, sizeof(STRUCT_ITEM));

		sprintf(temp, "useitem,joia_pvp joia:%d", item->sIndex);
		ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);
		return true;
	}
#pragma endregion
#pragma region Barra Mytril (Dano)
	if(Vol == 235)
	{
		STRUCT_ITEM *dest = GetItemPointer(&pMob[conn].MOB, pUser[conn].Cargo, m->DestType, m->DestPos);

		if (dest == NULL)
		{
			sprintf(temp, "err,use item fail dest %d %d", m->DestType, m->DestPos);
			ItemLog(temp, "-system", 0);
			return true;
		}

		if (m->DestType || m->DestPos >= 8 && m->DestPos < 16 || m->DestType == ITEM_PLACE_EQUIP && m->DestPos == 0 || m->DestType == ITEM_PLACE_EQUIP && m->DestPos == 1)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_Only_To_Equips]);

			ItemLog("err,use item fail - dest", pUser[conn].AccountName, pUser[conn].IP);

			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		int sanc = BASE_GetItemSanc(dest);
		int ItemId = dest->sIndex;

		if(sanc < REF_10 || BASE_GetItemAbility(dest, EF_MOBTYPE) != 1)
		{
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		int dam = BASE_GetItemAbilityNosanc(dest, EF_DAMAGE);

		int max_add = m->DestPos != 6 && m->DestPos != 7 ? 42 : 81;
		int min_add = m->DestPos != 6 && m->DestPos != 7 ? 12 : 45;

		if(dam < min_add)
		{
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		if(dam >= max_add)
		{
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		int RateSucess = 18;
		int _rand = W2Rand() % 115;
		if(_rand > 100)
			_rand -= 15;

		if (_rand <= RateSucess && RateSucess)
		{
			for(int i = 0; i < 3; i++)
			{
				if(dest->stEffect[i].cEffect == EF_DAMAGE || dest->stEffect[i].cEffect == EF_DAMAGE2)
				{
					dest->stEffect[i].cEffect = EF_DAMAGE;
					dest->stEffect[i].cValue += m->DestPos != 6 && m->DestPos != 7 ? 6 : 9;

					if(dest->stEffect[i].cValue > (m->DestPos != 6 && m->DestPos != 7 ? 42 : 81))
						dest->stEffect[i].cValue = (m->DestPos != 6 && m->DestPos != 7 ? 42 : 81);

					break;
				}
			}

			pMob[conn].GetCurrentScore(conn);

			SendScore(conn);
			sprintf(temp, "useitem,upgrade success %d+%d - (%d,%d,%d)", dest->sIndex, sanc, item->stEffect[0].cEffect, item->stEffect[1].cEffect, item->stEffect[2].cEffect);
			ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);

			if (amount > 1)
				BASE_SetItemAmount(item, amount - 1);

			else
				memset(item, 0, sizeof(STRUCT_ITEM));

			SendItem(conn, m->DestType, m->DestPos, dest);
			SendEmotion(conn, 14, 3);
		}

		else
		{
			if((W2Rand()%7) == 0)
			{
				for(int i = 0; i < 3; i++)
				{
					if(dest->stEffect[i].cEffect == EF_DAMAGE || dest->stEffect[i].cEffect == EF_DAMAGE2)
					{
						dest->stEffect[i].cEffect = EF_DAMAGE;
											
						if(dest->stEffect[i].cValue >= (m->DestPos != 6 && m->DestPos != 7 ? 6 : 9))
							dest->stEffect[i].cValue -= m->DestPos != 6 && m->DestPos != 7 ? 6 : 9;

						break;
					}
				}
				sprintf(temp, "useitem,upgrade fail-- %d+%d - (%d,%d,%d)", dest->sIndex, sanc, item->stEffect[0].cEffect, item->stEffect[1].cEffect, item->stEffect[2].cEffect);

				SendItem(conn, m->DestType, m->DestPos, dest);
				SendEmotion(conn, 14, 3);
			}
			else
			{
				sprintf(temp, "useitem,upgrade fail %d+%d - (%d,%d,%d)", dest->sIndex, sanc, item->stEffect[0].cEffect, item->stEffect[1].cEffect, item->stEffect[2].cEffect);
									
				int sFace = pMob[conn].MOB.Equip[0].sIndex / 10;

				if (sFace)
					SendEmotion(conn, 15, 0);

				else
					SendEmotion(conn, 20, 0);
			}
			ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);

			if (amount > 1)
				BASE_SetItemAmount(item, amount - 1);

			else
				memset(item, 0, sizeof(STRUCT_ITEM));
		}

		return true;
	}
#pragma endregion
#pragma region Barra Mytril (Magia)
	if(Vol == 236)
	{
		STRUCT_ITEM *dest = GetItemPointer(&pMob[conn].MOB, pUser[conn].Cargo, m->DestType, m->DestPos);

		if (dest == NULL)
		{
			sprintf(temp, "err,use item fail dest %d %d", m->DestType, m->DestPos);
			ItemLog(temp, "-system", 0);
			return true;
		}

		if (m->DestType || m->DestPos >= 8 && m->DestPos < 16 || m->DestType == ITEM_PLACE_EQUIP && m->DestPos == 0 || m->DestType == ITEM_PLACE_EQUIP && m->DestPos == 1)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_Only_To_Equips]);

			ItemLog("err,use item fail - dest", pUser[conn].AccountName, pUser[conn].IP);

			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		int sanc = BASE_GetItemSanc(dest);
		int ItemId = dest->sIndex;

		if(sanc < REF_10 || BASE_GetItemAbility(dest, EF_MOBTYPE) != 1)
		{
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		int dam = BASE_GetItemAbilityNosanc(dest, EF_MAGIC);

		int max_add = m->DestPos != 6 && m->DestPos != 7 ? 14 : 36;
		int min_add = m->DestPos != 6 && m->DestPos != 7 ? 6 : 24;

		if(dam < min_add)
		{
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		if(dam >= max_add)
		{
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		int RateSucess = 18;
		int _rand = W2Rand() % 115;
		if(_rand > 100)
			_rand -= 15;

		if (_rand <= RateSucess && RateSucess)
		{
			for(int i = 0; i < 3; i++)
			{
				if(dest->stEffect[i].cEffect == EF_MAGIC || dest->stEffect[i].cEffect == EF_MAGICADD)
				{
					dest->stEffect[i].cEffect = EF_MAGIC;
					dest->stEffect[i].cValue += m->DestPos != 6 && m->DestPos != 7 ? 2 : 4;

					if(dest->stEffect[i].cValue > (m->DestPos != 6 && m->DestPos != 7 ? 14 : 36))
						dest->stEffect[i].cValue = (m->DestPos != 6 && m->DestPos != 7 ? 14 : 36);

					break;
				}
			}

			pMob[conn].GetCurrentScore(conn);

			SendScore(conn);
			sprintf(temp, "useitem,upgrade success %d+%d - (%d,%d,%d)", dest->sIndex, sanc, item->stEffect[0].cEffect, item->stEffect[1].cEffect, item->stEffect[2].cEffect);
			ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);

			if (amount > 1)
				BASE_SetItemAmount(item, amount - 1);

			else
				memset(item, 0, sizeof(STRUCT_ITEM));

			SendItem(conn, m->DestType, m->DestPos, dest);
			SendEmotion(conn, 14, 3);
		}

		else
		{
			if((W2Rand()%7) == 0)
			{
				for(int i = 0; i < 3; i++)
				{
					if(dest->stEffect[i].cEffect == EF_MAGIC || dest->stEffect[i].cEffect == EF_MAGICADD)
					{
						dest->stEffect[i].cEffect = EF_MAGIC;

						if(dest->stEffect[i].cValue > (m->DestPos != 6 && m->DestPos != 7 ? 2 : 4))
							dest->stEffect[i].cValue -= m->DestPos != 6 && m->DestPos != 7 ? 2 : 4;

						break;
					}
				}
				sprintf(temp, "useitem,upgrade fail-- %d+%d - (%d,%d,%d)", dest->sIndex, sanc, item->stEffect[0].cEffect, item->stEffect[1].cEffect, item->stEffect[2].cEffect);

				SendItem(conn, m->DestType, m->DestPos, dest);
				SendEmotion(conn, 14, 3);
			}
			else
			{
				sprintf(temp, "useitem,upgrade fail %d+%d - (%d,%d,%d)", dest->sIndex, sanc, item->stEffect[0].cEffect, item->stEffect[1].cEffect, item->stEffect[2].cEffect);
									
				int sFace = pMob[conn].MOB.Equip[0].sIndex / 10;

				if (sFace)
				SendEmotion(conn, 15, 0);

				else
					SendEmotion(conn, 20, 0);
			}
			ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);

			if (amount > 1)
				BASE_SetItemAmount(item, amount - 1);

			else
				memset(item, 0, sizeof(STRUCT_ITEM));
		}

		return true;
	}
#pragma endregion
#pragma region Barra Mytril (Defesa)
	if(Vol == 237)
	{
		STRUCT_ITEM *dest = GetItemPointer(&pMob[conn].MOB, pUser[conn].Cargo, m->DestType, m->DestPos);

		if (dest == NULL)
		{
			sprintf(temp, "err,use item fail dest %d %d", m->DestType, m->DestPos);
			ItemLog(temp, "-system", 0);
			return true;
		}

		if (m->DestType || m->DestPos >= 6 && m->DestPos < 16 || m->DestType == ITEM_PLACE_EQUIP && m->DestPos == 0 || m->DestType == ITEM_PLACE_EQUIP && m->DestPos == 1)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_Only_To_Equips]);

			ItemLog("err,use item fail - dest", pUser[conn].AccountName, pUser[conn].IP);

			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		if(dest->sIndex >= 3500 && dest->sIndex <= 3507)
		{
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		int sanc = BASE_GetItemSanc(dest);
		int ItemId = dest->sIndex;

		if(sanc < REF_10 || BASE_GetItemAbility(dest, EF_MOBTYPE) != 1)
		{
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		int dam = 0;
		int effect = 0;

		for(int i = 0; i < 3; i++)
		{
			if(dest->stEffect[i].cEffect == EF_AC || dest->stEffect[i].cEffect == EF_ACADD2)
			{
				dam = dest->stEffect[i].cValue;
				effect = dest->stEffect[i].cEffect;

				break;
			}
		}

		int max_add = (effect == EF_AC ? 40 : 70);
		int min_add = (effect == EF_AC ? 15 : 15);


		if(dam < min_add)
		{
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		if(dam >= max_add)
		{
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}


		int RateSucess = 50;
		int _rand = W2Rand() % 115;
		if(_rand > 100)
			_rand -= 15;

		if (_rand <= RateSucess && RateSucess)
		{
			for(int i = 0; i < 3; i++)
			{
				if(dest->stEffect[i].cEffect == EF_AC || dest->stEffect[i].cEffect == EF_ACADD2)
				{
					dest->stEffect[i].cValue += (dest->stEffect[i].cEffect == EF_AC ? 5 : 10);

					if(dest->stEffect[i].cValue > (dest->stEffect[i].cEffect == EF_AC ? 40 : 70))
						dest->stEffect[i].cValue = (dest->stEffect[i].cEffect == EF_AC ? 40 : 70);

					break;
				}
			}

			pMob[conn].GetCurrentScore(conn);

			SendScore(conn);
			sprintf(temp, "useitem,upgrade success %d+%d - (%d,%d,%d)", dest->sIndex, sanc, item->stEffect[0].cEffect, item->stEffect[1].cEffect, item->stEffect[2].cEffect);
			ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);

			if (amount > 1)
				BASE_SetItemAmount(item, amount - 1);

			else
				memset(item, 0, sizeof(STRUCT_ITEM));

			SendItem(conn, m->DestType, m->DestPos, dest);
			SendEmotion(conn, 14, 3);
		}

		else
		{
			if((W2Rand()%7) == 0)
			{
				for(int i = 0; i < 3; i++)
				{
					if(dest->stEffect[i].cEffect == EF_AC || dest->stEffect[i].cEffect == EF_ACADD2)
					{
						if(dest->stEffect[i].cEffect == EF_AC && dest->stEffect[i].cValue > 5 || dest->stEffect[i].cEffect == EF_ACADD2 && dest->stEffect[i].cValue > 10)
							dest->stEffect[i].cValue -= (dest->stEffect[i].cEffect == EF_AC ? 5 : 10);

						break;
					}
				}
				sprintf(temp, "useitem,upgrade fail-- %d+%d - (%d,%d,%d)", dest->sIndex, sanc, item->stEffect[0].cEffect, item->stEffect[1].cEffect, item->stEffect[2].cEffect);

				SendItem(conn, m->DestType, m->DestPos, dest);
				SendEmotion(conn, 14, 3);
			}
			else
			{
				sprintf(temp, "useitem,upgrade fail %d+%d - (%d,%d,%d)", dest->sIndex, sanc, item->stEffect[0].cEffect, item->stEffect[1].cEffect, item->stEffect[2].cEffect);
									
				int sFace = pMob[conn].MOB.Equip[0].sIndex / 10;

				if (sFace)
				SendEmotion(conn, 15, 0);

				else
					SendEmotion(conn, 20, 0);
			}
			ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);

			if (amount > 1)
				BASE_SetItemAmount(item, amount - 1);

			else
				memset(item, 0, sizeof(STRUCT_ITEM));
		}

		return true;
	}
#pragma endregion
#pragma region Barra Mytril (Crítico)
	if(Vol == 238)
	{
		STRUCT_ITEM *dest = GetItemPointer(&pMob[conn].MOB, pUser[conn].Cargo, m->DestType, m->DestPos);

		if (dest == NULL)
		{
			sprintf(temp, "err,use item fail dest %d %d", m->DestType, m->DestPos);
			ItemLog(temp, "-system", 0);
			return true;
		}

		if (m->DestType || m->DestPos >= 8 && m->DestPos < 16 || m->DestType == ITEM_PLACE_EQUIP && m->DestPos == 0 || m->DestType == ITEM_PLACE_EQUIP && m->DestPos == 1)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_Only_To_Equips]);

			ItemLog("err,use item fail - dest", pUser[conn].AccountName, pUser[conn].IP);

			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		int sanc = BASE_GetItemSanc(dest);
		int ItemId = dest->sIndex;

		if(sanc < REF_10 || BASE_GetItemAbility(dest, EF_MOBTYPE) != 1)
		{
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		int dam = BASE_GetItemAbilityNosanc(dest, EF_CRITICAL2);

		int max_add = m->DestPos != 6 && m->DestPos != 7 ? 90 : 0;
		int min_add = m->DestPos != 6 && m->DestPos != 7 ? 50 : 0;

		if(dam < min_add)
		{
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		if(dam >= max_add)
		{
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}


		int RateSucess = 18;
		
		int _rand = W2Rand() % 115;
		
		if(_rand > 100)
			_rand -= 15;

		if (_rand <= RateSucess && RateSucess)
		{
			for(int i = 0; i < 3; i++)
			{
				if(dest->stEffect[i].cEffect == EF_CRITICAL || dest->stEffect[i].cEffect == EF_CRITICAL2)
				{
					dest->stEffect[i].cEffect = EF_CRITICAL2;
					dest->stEffect[i].cValue += m->DestPos != 6 && m->DestPos != 7 ? 10 : 1;

					if(dest->stEffect[i].cValue > (m->DestPos != 6 && m->DestPos != 7 && m->DestPos != 4 ? 90 : 0))
						dest->stEffect[i].cValue = (m->DestPos != 6 && m->DestPos != 7 && m->DestPos != 4 ? 90 : 0);

					break;
				}
			}

			pMob[conn].GetCurrentScore(conn);

			SendScore(conn);
			sprintf(temp, "useitem,upgrade success %d+%d - (%d,%d,%d)", dest->sIndex, sanc, item->stEffect[0].cEffect, item->stEffect[1].cEffect, item->stEffect[2].cEffect);
			ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);

			if (amount > 1)
				BASE_SetItemAmount(item, amount - 1);

			else
				memset(item, 0, sizeof(STRUCT_ITEM));

			SendItem(conn, m->DestType, m->DestPos, dest);
			SendEmotion(conn, 14, 3);
		}

		else
		{
			if((W2Rand()%7) == 0)
			{
				for(int i = 0; i < 3; i++)
				{
					if(dest->stEffect[i].cEffect == EF_CRITICAL || dest->stEffect[i].cEffect == EF_CRITICAL2)
					{
						dest->stEffect[i].cEffect = EF_CRITICAL2;
						if(dest->stEffect[i].cValue > (m->DestPos != 6 && m->DestPos != 7 ? 10 : 10))
							dest->stEffect[i].cValue -= m->DestPos != 6 && m->DestPos != 7 ? 10 : 10;
						break;
					}
				}
				sprintf(temp, "useitem,upgrade fail-- %d+%d - (%d,%d,%d)", dest->sIndex, sanc, item->stEffect[0].cEffect, item->stEffect[1].cEffect, item->stEffect[2].cEffect);

				SendItem(conn, m->DestType, m->DestPos, dest);
				SendEmotion(conn, 14, 3);
			}
			else
			{
				sprintf(temp, "useitem,upgrade fail %d+%d - (%d,%d,%d)", dest->sIndex, sanc, item->stEffect[0].cEffect, item->stEffect[1].cEffect, item->stEffect[2].cEffect);
									
				int sFace = pMob[conn].MOB.Equip[0].sIndex / 10;

				if (sFace)
					SendEmotion(conn, 15, 0);

				else
					SendEmotion(conn, 20, 0);
			}
			ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);

			if (amount > 1)
				BASE_SetItemAmount(item, amount - 1);

			else
				memset(item, 0, sizeof(STRUCT_ITEM));
		}

		return true;
	}
#pragma endregion

	return false;
}
