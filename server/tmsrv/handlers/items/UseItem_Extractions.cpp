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

/// @file UseItem_Extractions.cpp
/// @brief Sub-handler de troca de classe, catalisadores, restauradores, extrações e pacotes especiais.

#include "handlers/items/UseItemInternal.h"

bool UseItem_ExtractionsAndSpecials(int conn, MSG_UseItem* m, STRUCT_ITEM* item, int Vol, int amount)
{
#pragma region Classe
	if(Vol == 190)
	{
		STRUCT_ITEM *dest = GetItemPointer(&pMob[conn].MOB, pUser[conn].Cargo, m->DestType, m->DestPos);

		if (dest == NULL)
		{
			sprintf(temp, "err,use item fail dest %d %d", m->DestType, m->DestPos);
			ItemLog(temp, "-system", 0);
			return true;
		}

		if (!m->DestType || m->DestPos >= 60)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_Only_To_Equips]);

			ItemLog("err,use item fail - dest", pUser[conn].AccountName, pUser[conn].IP);

			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		int sanc = BASE_GetItemSanc(dest);
		int ItemId = dest->sIndex;

		if(sanc > 9 || BASE_GetItemAbility(dest, EF_MOBTYPE) != 0 && BASE_GetItemAbility(dest, EF_MOBTYPE) != 2)
		{
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		int replation = item->sIndex >= 4016 && item->sIndex <= 4020 ? item->sIndex - 4015 : item->sIndex - 4020;

		if(BASE_GetItemAbility(dest, EF_ITEMLEVEL) != replation)
		{
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		SetItemBonus2(dest);
		SendItem(conn, m->DestType, m->DestPos, dest);
		SendEmotion(conn, 14, 3);

		sprintf(temp, "useitem,reset effect %d+%d - (%d,%d,%d)", dest->sIndex, sanc, dest->stEffect[0].cEffect, dest->stEffect[1].cEffect, dest->stEffect[2].cEffect);	
		ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);

		if (amount > 1)
			BASE_SetItemAmount(item, amount - 1);

		else
			memset(item, 0, sizeof(STRUCT_ITEM));

		return true;
	}
#pragma endregion
#pragma region Catalizadores
	if(Vol == 94)
	{
		STRUCT_ITEM *dest = GetItemPointer(&pMob[conn].MOB, pUser[conn].Cargo, m->DestType, m->DestPos);

		if (dest == NULL)
		{
			sprintf(temp, "err,use item fail dest %d %d", m->DestType, m->DestPos);
			ItemLog(temp, "-system", 0);
			return true;
		}

		if (m->DestType || m->DestPos != 14)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_Mount_Not_Match]);

			ItemLog("err,use item fail - dest", pUser[conn].AccountName, pUser[conn].IP);

			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		int mount = (dest->sIndex - 2333);
		int cat = item->sIndex - 3344;

		if(mount >= 0 && mount <= 2)
			mount = 0;

		if(mount >= 3 && mount <= 6 || mount >= 8 && mount <= 11)
			mount = 1;

		if(mount == 7 || mount == 12 || mount == 24)
			mount = 2;

		if(mount >= 13 && mount <= 15)
			mount = 3;

		if(mount >= 18 && mount <= 20 || mount == 25)
			mount = 4;

		if(mount >= 21 && mount <= 23)
			mount = 5;

		if(mount >= 16 && mount <= 17)
			mount = 6;

		if(mount != cat)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_Mount_Not_Match]);

			ItemLog("err,use item fail - dest", pUser[conn].AccountName, pUser[conn].IP);

			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		dest->sIndex += 30;
								
		dest->stEffect[1].cValue = W2Rand() % 20 + dest->stEffect[1].cEffect;
		dest->stEffect[1].cEffect = 0;
		dest->stEffect[2].cValue = 0;
								 
		SendClientMessage(conn, g_pMessageStringTable[_NN_Mount_Growth]);
		MountProcess(conn, 0);

		sprintf(temp, "useitem,catalizador %d - (%d,%d,%d)", dest->sIndex, item->stEffect[1].cEffect, item->stEffect[1].cValue, item->stEffect[2].cValue);
									
		int sFace = pMob[conn].MOB.Equip[0].sIndex / 10;

		if (sFace)
			SendEmotion(conn, 15, 0);
		else
			SendEmotion(conn, 20, 0);
								 
		SendItem(conn, m->DestType, m->DestPos, dest);

		ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);

		if (amount > 1)
			BASE_SetItemAmount(item, amount - 1);

		else
			memset(item, 0, sizeof(STRUCT_ITEM));
						
		return true;
	}
#pragma endregion		
#pragma region Restaurador
	if(Vol == 93)
	{
		STRUCT_ITEM *dest = GetItemPointer(&pMob[conn].MOB, pUser[conn].Cargo, m->DestType, m->DestPos);

		if (dest == NULL)
		{
			sprintf(temp, "err,use item fail dest %d %d", m->DestType, m->DestPos);
			ItemLog(temp, "-system", 0);
			return true;
		}

		if (m->DestType || m->DestPos != 14)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_Mount_Not_Match]);

			ItemLog("err,use item fail - dest", pUser[conn].AccountName, pUser[conn].IP);

			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		int mount = (dest->sIndex - 2363);
		int cat = item->sIndex - 3351;

		if(mount >= 0 && mount <= 2)
			mount = 0;

		if(mount >= 3 && mount <= 6 || mount >= 8 && mount <= 11)
			mount = 1;

		if(mount == 7 || mount == 12 || mount == 24)
			mount = 2;

		if(mount >= 13 && mount <= 15)
			mount = 3;

		if(mount >= 18 && mount <= 20 || mount == 25)
			mount = 4;

		if(mount >= 21 && mount <= 23)
			mount = 5;

		if(mount >= 16 && mount <= 17)
			mount = 6;

		if(mount != cat)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_Mount_Not_Match]);

			ItemLog("err,use item fail - dest", pUser[conn].AccountName, pUser[conn].IP);

			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}
		if (dest->stEffect[1].cValue >= 50 || dest->stEffect[1].cValue <= 5 || dest->stEffect[0].sValue <= 0)
		{
			ItemLog("err,use item fail - dest", pUser[conn].AccountName, pUser[conn].IP);

			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}
		dest->stEffect[1].cValue += W2Rand() % 2 + 1;

		sprintf(temp, "useitem,restaurador %d - (%d,%d,%d)", dest->sIndex, item->stEffect[1].cEffect, item->stEffect[1].cValue, item->stEffect[2].cValue);
									
		int sFace = pMob[conn].MOB.Equip[0].sIndex / 10;

		if (sFace)
			SendEmotion(conn, 15, 0);
		else
			SendEmotion(conn, 20, 0);
								 
		SendItem(conn, m->DestType, m->DestPos, dest);

		ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);

		if (amount > 1)
			BASE_SetItemAmount(item, amount - 1);

		else
			memset(item, 0, sizeof(STRUCT_ITEM));
						
		return true;
	}
#pragma endregion
#pragma region Extração Elmo
	if (item->sIndex == 3021)
	{
		int extra = BASE_GetItemAbility(item, EF_ITEMLEVEL);

		if(pMob[conn].MOB.Equip[1].sIndex == 0)
		{
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		if(extra <= 5)
		{//Item abaixo de E
			int elmoextra = BASE_GetItemAbility(&pMob[conn].MOB.Equip[1], EF_ITEMLEVEL);
			int elmomobtype = BASE_GetItemAbility(&pMob[conn].MOB.Equip[1], EF_MOBTYPE);

			if(elmoextra != 0 && elmoextra != 5 || elmoextra != extra || elmomobtype != 2 && elmomobtype != 0)
			{
				SendItem(conn, m->SourType, m->SourPos, item);
				return true;
			}

			int sanc = BASE_GetItemSanc(&pMob[conn].MOB.Equip[1]);

			if(sanc != REF_11)
			{
				sprintf(temp, g_pMessageStringTable[_DN_SANCREQ], 11);
				SendClientMessage(conn, temp);
				SendItem(conn, m->SourType, m->SourPos, item);
				return true;
			}

			pMob[conn].MOB.Equip[1].stEffect[1].cEffect = item->stEffect[1].cEffect;
			pMob[conn].MOB.Equip[1].stEffect[1].cValue = item->stEffect[1].cValue;

			pMob[conn].MOB.Equip[1].stEffect[2].cEffect = item->stEffect[2].cEffect;
			pMob[conn].MOB.Equip[1].stEffect[2].cValue = item->stEffect[2].cValue;

			SendItem(conn, ITEM_PLACE_EQUIP, 1, &pMob[conn].MOB.Equip[1]);

			sprintf(temp, "useitem,extracao %d-%d:%d:%d:%d:%d:%d", item->sIndex, item->stEffect[0].cEffect, item->stEffect[0].cValue, item->stEffect[1].cEffect, item->stEffect[1].cValue, item->stEffect[2].cEffect, item->stEffect[2].cValue);
			ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);

			if (amount > 1)
				BASE_SetItemAmount(item, amount - 1);

			else
				memset(item, 0, sizeof(STRUCT_ITEM));

			SendEmotion(conn, 14, 3);
			return true;
		}
		//Extração de cythera

		else 
		{
			int elmo = pMob[conn].MOB.Equip[1].sIndex;

			if(elmo < 3500 || elmo > 3507)
			{
			//SendClientMessage(conn, g_pMessageStringTable[_NN_ExtraWrongItemLevel]);
				SendItem(conn, m->SourType, m->SourPos, item);
				return true;
			}

			int sanc = BASE_GetItemSanc(&pMob[conn].MOB.Equip[1]);

			if(sanc != REF_11)
			{
				sprintf(temp, g_pMessageStringTable[_DN_SANCREQ], 11);
				SendClientMessage(conn, temp);
				SendItem(conn, m->SourType, m->SourPos, item);
				return true;
			}

			pMob[conn].MOB.Equip[1].stEffect[1].cEffect = item->stEffect[1].cEffect;
			pMob[conn].MOB.Equip[1].stEffect[1].cValue = item->stEffect[1].cValue;

			pMob[conn].MOB.Equip[1].stEffect[2].cEffect = item->stEffect[2].cEffect;
			pMob[conn].MOB.Equip[1].stEffect[2].cValue = item->stEffect[2].cValue;

			SendItem(conn, ITEM_PLACE_EQUIP, 1, &pMob[conn].MOB.Equip[1]);

			sprintf(temp, "useitem,extracao %d-%d:%d:%d:%d:%d:%d", item->sIndex, item->stEffect[0].cEffect, item->stEffect[0].cValue, item->stEffect[1].cEffect, item->stEffect[1].cValue, item->stEffect[2].cEffect, item->stEffect[2].cValue);
			ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);

			if (amount > 1)
				BASE_SetItemAmount(item, amount - 1);

			else
				memset(item, 0, sizeof(STRUCT_ITEM));

			SendEmotion(conn, 14, 3);
			return true;
		}

		return true;
	}
#pragma endregion
#pragma region Extração Peito
	if (item->sIndex == 3022)
	{
		int extra = BASE_GetItemAbility(item, EF_ITEMLEVEL);

		if(pMob[conn].MOB.Equip[2].sIndex == 0)
		{
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		if(extra <= 5)
		{//Item abaixo de E
			int destextra = BASE_GetItemAbility(&pMob[conn].MOB.Equip[2], EF_ITEMLEVEL);
			int desttype = BASE_GetItemAbility(&pMob[conn].MOB.Equip[2], EF_MOBTYPE);

			if(destextra != 0 && destextra != 5 || destextra != extra || desttype != 2 && desttype != 0)
			{
				SendItem(conn, m->SourType, m->SourPos, item);
				return true;
			}

			int sanc = BASE_GetItemSanc(&pMob[conn].MOB.Equip[2]);

			if(sanc != REF_11)
			{
				sprintf(temp, g_pMessageStringTable[_DN_SANCREQ], 11);
				SendClientMessage(conn, temp);
				SendItem(conn, m->SourType, m->SourPos, item);
				return true;
			}

			pMob[conn].MOB.Equip[2].stEffect[1].cEffect = item->stEffect[1].cEffect;
			pMob[conn].MOB.Equip[2].stEffect[1].cValue = item->stEffect[1].cValue;

			pMob[conn].MOB.Equip[2].stEffect[2].cEffect = item->stEffect[2].cEffect;
			pMob[conn].MOB.Equip[2].stEffect[2].cValue = item->stEffect[2].cValue;

			SendItem(conn, ITEM_PLACE_EQUIP, 2, &pMob[conn].MOB.Equip[2]);
								 
			sprintf(temp, "useitem,extracao %d-%d:%d:%d:%d:%d:%d", item->sIndex, item->stEffect[0].cEffect, item->stEffect[0].cValue, item->stEffect[1].cEffect, item->stEffect[1].cValue, item->stEffect[2].cEffect, item->stEffect[2].cValue);
			ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);
								 
			if (amount > 1)
				BASE_SetItemAmount(item, amount - 1);

			else
				memset(item, 0, sizeof(STRUCT_ITEM));

			SendEmotion(conn, 14, 3);
			return true;
		}
		//Extração de item arch

		else 
		{
			int desttype = BASE_GetItemAbility(&pMob[conn].MOB.Equip[2], EF_MOBTYPE);

			if(desttype != 1)
			{
				SendItem(conn, m->SourType, m->SourPos, item);
				return true;
			}

			int sanc = BASE_GetItemSanc(&pMob[conn].MOB.Equip[2]);

			if(sanc != REF_11)
			{
				sprintf(temp, g_pMessageStringTable[_DN_SANCREQ], 11);
				SendClientMessage(conn, temp);
				SendItem(conn, m->SourType, m->SourPos, item);
				return true;
			}

			pMob[conn].MOB.Equip[2].stEffect[1].cEffect = item->stEffect[1].cEffect;
			pMob[conn].MOB.Equip[2].stEffect[1].cValue = item->stEffect[1].cValue;

			pMob[conn].MOB.Equip[2].stEffect[2].cEffect = item->stEffect[2].cEffect;
			pMob[conn].MOB.Equip[2].stEffect[2].cValue = item->stEffect[2].cValue;

			SendItem(conn, ITEM_PLACE_EQUIP, 2, &pMob[conn].MOB.Equip[2]);

			sprintf(temp, "useitem,extracao %d-%d:%d:%d:%d:%d:%d", item->sIndex, item->stEffect[0].cEffect, item->stEffect[0].cValue, item->stEffect[1].cEffect, item->stEffect[1].cValue, item->stEffect[2].cEffect, item->stEffect[2].cValue);
			ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);

			if (amount > 1)
				BASE_SetItemAmount(item, amount - 1);

			else
				memset(item, 0, sizeof(STRUCT_ITEM));

			SendEmotion(conn, 14, 3);
			return true;
		}

		return true;
	}
#pragma endregion
#pragma region Extração Calça
	if (item->sIndex == 3023)
	{
		int extra = BASE_GetItemAbility(item, EF_ITEMLEVEL);

		if(pMob[conn].MOB.Equip[3].sIndex == 0)
		{
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		if(extra <= 5)
		{//Item abaixo de E
			int destextra = BASE_GetItemAbility(&pMob[conn].MOB.Equip[3], EF_ITEMLEVEL);
			int desttype = BASE_GetItemAbility(&pMob[conn].MOB.Equip[3], EF_MOBTYPE);

			if(destextra != 0 && destextra != 5 || destextra != extra || desttype != 2 && desttype != 0)
			{
				SendItem(conn, m->SourType, m->SourPos, item);
				return true;
			}

			int sanc = BASE_GetItemSanc(&pMob[conn].MOB.Equip[3]);

			if(sanc != REF_11)
			{
				sprintf(temp, g_pMessageStringTable[_DN_SANCREQ], 11);
				SendClientMessage(conn, temp);
				SendItem(conn, m->SourType, m->SourPos, item);
				return true;
			}

			pMob[conn].MOB.Equip[3].stEffect[1].cEffect = item->stEffect[1].cEffect;
			pMob[conn].MOB.Equip[3].stEffect[1].cValue = item->stEffect[1].cValue;

			pMob[conn].MOB.Equip[3].stEffect[2].cEffect = item->stEffect[2].cEffect;
			pMob[conn].MOB.Equip[3].stEffect[2].cValue = item->stEffect[2].cValue;

			SendItem(conn, ITEM_PLACE_EQUIP, 3, &pMob[conn].MOB.Equip[3]);

			sprintf(temp, "useitem,extracao %d-%d:%d:%d:%d:%d:%d", item->sIndex, item->stEffect[0].cEffect, item->stEffect[0].cValue, item->stEffect[1].cEffect, item->stEffect[1].cValue, item->stEffect[2].cEffect, item->stEffect[2].cValue);
			ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);

			if (amount > 1)
				BASE_SetItemAmount(item, amount - 1);

			else
				memset(item, 0, sizeof(STRUCT_ITEM));

			SendEmotion(conn, 14, 3);
			return true;
		}
		//Extração de item arch

		else 
		{
			int desttype = BASE_GetItemAbility(&pMob[conn].MOB.Equip[3], EF_MOBTYPE);

			if(desttype != 1)
			{
				SendItem(conn, m->SourType, m->SourPos, item);
				return true;
			}

			int sanc = BASE_GetItemSanc(&pMob[conn].MOB.Equip[3]);

			if(sanc != REF_11)
			{
				sprintf(temp, g_pMessageStringTable[_DN_SANCREQ], 11);
				SendClientMessage(conn, temp);
				SendItem(conn, m->SourType, m->SourPos, item);
				return true;
			}

			pMob[conn].MOB.Equip[3].stEffect[1].cEffect = item->stEffect[1].cEffect;
			pMob[conn].MOB.Equip[3].stEffect[1].cValue = item->stEffect[1].cValue;

			pMob[conn].MOB.Equip[3].stEffect[2].cEffect = item->stEffect[2].cEffect;
			pMob[conn].MOB.Equip[3].stEffect[2].cValue = item->stEffect[2].cValue;

			SendItem(conn, ITEM_PLACE_EQUIP, 3, &pMob[conn].MOB.Equip[3]);

			sprintf(temp, "useitem,extracao %d-%d:%d:%d:%d:%d:%d", item->sIndex, item->stEffect[0].cEffect, item->stEffect[0].cValue, item->stEffect[1].cEffect, item->stEffect[1].cValue, item->stEffect[2].cEffect, item->stEffect[2].cValue);
			ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);

			if (amount > 1)
				BASE_SetItemAmount(item, amount - 1);

			else
				memset(item, 0, sizeof(STRUCT_ITEM));

			SendEmotion(conn, 14, 3);
			return true;
		}

		return true;
	}
#pragma endregion
#pragma region Extração Luva
	if (item->sIndex == 3024)
	{
		int extra = BASE_GetItemAbility(item, EF_ITEMLEVEL);

		if(pMob[conn].MOB.Equip[4].sIndex == 0)
		{
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		if(extra <= 5)
		{//Item abaixo de E
			int destextra = BASE_GetItemAbility(&pMob[conn].MOB.Equip[4], EF_ITEMLEVEL);
			int desttype = BASE_GetItemAbility(&pMob[conn].MOB.Equip[4], EF_MOBTYPE);

			if(destextra != 0 && destextra != 5 || destextra != extra || desttype != 2 && desttype != 0)
			{
				SendItem(conn, m->SourType, m->SourPos, item);
				return true;
			}

			int sanc = BASE_GetItemSanc(&pMob[conn].MOB.Equip[4]);

			if(sanc != REF_11)
			{
				sprintf(temp, g_pMessageStringTable[_DN_SANCREQ], 11);
				SendClientMessage(conn, temp);
				SendItem(conn, m->SourType, m->SourPos, item);
				return true;
			}

			pMob[conn].MOB.Equip[4].stEffect[1].cEffect = item->stEffect[1].cEffect;
			pMob[conn].MOB.Equip[4].stEffect[1].cValue = item->stEffect[1].cValue;

			pMob[conn].MOB.Equip[4].stEffect[2].cEffect = item->stEffect[2].cEffect;
			pMob[conn].MOB.Equip[4].stEffect[2].cValue = item->stEffect[2].cValue;

			SendItem(conn, ITEM_PLACE_EQUIP, 4, &pMob[conn].MOB.Equip[4]);

			sprintf(temp, "useitem,extracao %d-%d:%d:%d:%d:%d:%d", item->sIndex, item->stEffect[0].cEffect, item->stEffect[0].cValue, item->stEffect[1].cEffect, item->stEffect[1].cValue, item->stEffect[2].cEffect, item->stEffect[2].cValue);
			ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);

			if (amount > 1)
				BASE_SetItemAmount(item, amount - 1);

			else
				memset(item, 0, sizeof(STRUCT_ITEM));

			SendEmotion(conn, 14, 3);
			return true;
		}
		//Extração de item arch

		else 
		{
			int desttype = BASE_GetItemAbility(&pMob[conn].MOB.Equip[4], EF_MOBTYPE);

			if(desttype != 1)
			{
				SendItem(conn, m->SourType, m->SourPos, item);
				return true;
			}

			int sanc = BASE_GetItemSanc(&pMob[conn].MOB.Equip[4]);

			if(sanc != REF_11)
			{
				sprintf(temp, g_pMessageStringTable[_DN_SANCREQ], 11);
				SendClientMessage(conn, temp);
				SendItem(conn, m->SourType, m->SourPos, item);
				return true;
			}

			pMob[conn].MOB.Equip[4].stEffect[1].cEffect = item->stEffect[1].cEffect;
			pMob[conn].MOB.Equip[4].stEffect[1].cValue = item->stEffect[1].cValue;

			pMob[conn].MOB.Equip[4].stEffect[2].cEffect = item->stEffect[2].cEffect;
			pMob[conn].MOB.Equip[4].stEffect[2].cValue = item->stEffect[2].cValue;

			SendItem(conn, ITEM_PLACE_EQUIP, 4, &pMob[conn].MOB.Equip[4]);

			sprintf(temp, "useitem,extracao %d-%d:%d:%d:%d:%d:%d", item->sIndex, item->stEffect[0].cEffect, item->stEffect[0].cValue, item->stEffect[1].cEffect, item->stEffect[1].cValue, item->stEffect[2].cEffect, item->stEffect[2].cValue);
			ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);

			if (amount > 1)
				BASE_SetItemAmount(item, amount - 1);

			else
				memset(item, 0, sizeof(STRUCT_ITEM));

			SendEmotion(conn, 14, 3);
			return true;
		}

		return true;
	}
#pragma endregion
#pragma region Extração Bota
	if (item->sIndex == 3025)
	{
		int extra = BASE_GetItemAbility(item, EF_ITEMLEVEL);

		if(pMob[conn].MOB.Equip[5].sIndex == 0)
		{
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		if(extra <= 5)
		{//Item abaixo de E
			int destextra = BASE_GetItemAbility(&pMob[conn].MOB.Equip[5], EF_ITEMLEVEL);
			int desttype = BASE_GetItemAbility(&pMob[conn].MOB.Equip[5], EF_MOBTYPE);

			if(destextra != 0 && destextra != 5 || destextra != extra || desttype != 2 && desttype != 0)
			{
				SendItem(conn, m->SourType, m->SourPos, item);
				return true;
			}

			int sanc = BASE_GetItemSanc(&pMob[conn].MOB.Equip[5]);

			if(sanc != REF_11)
			{
				sprintf(temp, g_pMessageStringTable[_DN_SANCREQ], 11);
				SendClientMessage(conn, temp);
				SendItem(conn, m->SourType, m->SourPos, item);
				return true;
			}

			pMob[conn].MOB.Equip[5].stEffect[1].cEffect = item->stEffect[1].cEffect;
			pMob[conn].MOB.Equip[5].stEffect[1].cValue = item->stEffect[1].cValue;

			pMob[conn].MOB.Equip[5].stEffect[2].cEffect = item->stEffect[2].cEffect;
			pMob[conn].MOB.Equip[5].stEffect[2].cValue = item->stEffect[2].cValue;

			if(pMob[conn].MOB.Equip[5].stEffect[1].cEffect == EF_DAMAGE && BASE_GetStaticItemAbility(&pMob[conn].MOB.Equip[5], EF_DAMAGE) != 0)
				pMob[conn].MOB.Equip[5].stEffect[1].cValue -= BASE_GetStaticItemAbility(&pMob[conn].MOB.Equip[5], EF_DAMAGE);

			if(pMob[conn].MOB.Equip[5].stEffect[2].cEffect == EF_DAMAGE && BASE_GetStaticItemAbility(&pMob[conn].MOB.Equip[5], EF_DAMAGE) != 0)
				pMob[conn].MOB.Equip[5].stEffect[2].cValue -= BASE_GetStaticItemAbility(&pMob[conn].MOB.Equip[5], EF_DAMAGE);

			SendItem(conn, ITEM_PLACE_EQUIP, 5, &pMob[conn].MOB.Equip[5]);

			sprintf(temp, "useitem,extracao %d-%d:%d:%d:%d:%d:%d", item->sIndex, item->stEffect[0].cEffect, item->stEffect[0].cValue, item->stEffect[1].cEffect, item->stEffect[1].cValue, item->stEffect[2].cEffect, item->stEffect[2].cValue);
			ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);

			if (amount > 1)
				BASE_SetItemAmount(item, amount - 1);

			else
				memset(item, 0, sizeof(STRUCT_ITEM));

			SendEmotion(conn, 14, 3);
			return true;
		}
		//Extração de item arch

		else 
		{
			int desttype = BASE_GetItemAbility(&pMob[conn].MOB.Equip[5], EF_MOBTYPE);

			if(desttype != 1)
			{
				SendItem(conn, m->SourType, m->SourPos, item);
				return true;
			}

			int sanc = BASE_GetItemSanc(&pMob[conn].MOB.Equip[5]);

			if(sanc != REF_11)
			{
				sprintf(temp, g_pMessageStringTable[_DN_SANCREQ], 11);
				SendClientMessage(conn, temp);
				SendItem(conn, m->SourType, m->SourPos, item);
				return true;
			}

			pMob[conn].MOB.Equip[5].stEffect[1].cEffect = item->stEffect[1].cEffect;
			pMob[conn].MOB.Equip[5].stEffect[1].cValue = item->stEffect[1].cValue;

			pMob[conn].MOB.Equip[5].stEffect[2].cEffect = item->stEffect[2].cEffect;
			pMob[conn].MOB.Equip[5].stEffect[2].cValue = item->stEffect[2].cValue;

			if(pMob[conn].MOB.Equip[5].stEffect[1].cEffect == EF_DAMAGE && BASE_GetStaticItemAbility(&pMob[conn].MOB.Equip[5], EF_DAMAGE) != 0)
				pMob[conn].MOB.Equip[5].stEffect[1].cValue -= BASE_GetStaticItemAbility(&pMob[conn].MOB.Equip[5], EF_DAMAGE);

			if(pMob[conn].MOB.Equip[5].stEffect[2].cEffect == EF_DAMAGE && BASE_GetStaticItemAbility(&pMob[conn].MOB.Equip[5], EF_DAMAGE) != 0)
				pMob[conn].MOB.Equip[5].stEffect[2].cValue -= BASE_GetStaticItemAbility(&pMob[conn].MOB.Equip[5], EF_DAMAGE);

			SendItem(conn, ITEM_PLACE_EQUIP, 5, &pMob[conn].MOB.Equip[5]);

			sprintf(temp, "useitem,extracao %d-%d:%d:%d:%d:%d:%d", item->sIndex, item->stEffect[0].cEffect, item->stEffect[0].cValue, item->stEffect[1].cEffect, item->stEffect[1].cValue, item->stEffect[2].cEffect, item->stEffect[2].cValue);
			ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);

			if (amount > 1)
				BASE_SetItemAmount(item, amount - 1);

			else
				memset(item, 0, sizeof(STRUCT_ITEM));

			SendEmotion(conn, 14, 3);
			return true;
		}

		return true;
	}
#pragma endregion
#pragma region Extração Arma
	if (item->sIndex == 3026)
	{
		int extra = BASE_GetItemAbility(item, EF_ITEMLEVEL);

		if(pMob[conn].MOB.Equip[6].sIndex == 0)
		{
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		if(extra <= 5)
		{//Item abaixo de E
			int destextra = BASE_GetItemAbility(&pMob[conn].MOB.Equip[6], EF_ITEMLEVEL);
			int desttype = BASE_GetItemAbility(&pMob[conn].MOB.Equip[6], EF_MOBTYPE);

			if(destextra != 0 && destextra != 5 || destextra != extra || desttype != 2 && desttype != 0)
			{
				SendItem(conn, m->SourType, m->SourPos, item);
				return true;
			}

			int sanc = BASE_GetItemSanc(&pMob[conn].MOB.Equip[6]);

			if(sanc != REF_11)
			{
				sprintf(temp, g_pMessageStringTable[_DN_SANCREQ], 11);
				SendClientMessage(conn, temp);
				SendItem(conn, m->SourType, m->SourPos, item);
				return true;
			}

			pMob[conn].MOB.Equip[6].stEffect[1].cEffect = item->stEffect[1].cEffect;
			pMob[conn].MOB.Equip[6].stEffect[1].cValue = item->stEffect[1].cValue;

			pMob[conn].MOB.Equip[6].stEffect[2].cEffect = item->stEffect[2].cEffect;
			pMob[conn].MOB.Equip[6].stEffect[2].cValue = item->stEffect[2].cValue;

			SendItem(conn, ITEM_PLACE_EQUIP, 6, &pMob[conn].MOB.Equip[6]);

			sprintf(temp, "useitem,extracao %d-%d:%d:%d:%d:%d:%d", item->sIndex, item->stEffect[0].cEffect, item->stEffect[0].cValue, item->stEffect[1].cEffect, item->stEffect[1].cValue, item->stEffect[2].cEffect, item->stEffect[2].cValue);
			ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);

			if (amount > 1)
				BASE_SetItemAmount(item, amount - 1);

			else
				memset(item, 0, sizeof(STRUCT_ITEM));

			SendEmotion(conn, 14, 3);
			return true;
		}
		//Extração de item arch

		else 
		{
			int desttype = BASE_GetItemAbility(&pMob[conn].MOB.Equip[6], EF_MOBTYPE);

			if(desttype != 1)
			{
				SendItem(conn, m->SourType, m->SourPos, item);
				return true;
			}

			int sanc = BASE_GetItemSanc(&pMob[conn].MOB.Equip[6]);

			if(sanc != REF_11)
			{
				sprintf(temp, g_pMessageStringTable[_DN_SANCREQ], 11);
				SendClientMessage(conn, temp);
				SendItem(conn, m->SourType, m->SourPos, item);
				return true;
			}

			pMob[conn].MOB.Equip[6].stEffect[1].cEffect = item->stEffect[1].cEffect;
			pMob[conn].MOB.Equip[6].stEffect[1].cValue = item->stEffect[1].cValue;

			pMob[conn].MOB.Equip[6].stEffect[2].cEffect = item->stEffect[2].cEffect;
			pMob[conn].MOB.Equip[6].stEffect[2].cValue = item->stEffect[2].cValue;

			SendItem(conn, ITEM_PLACE_EQUIP, 6, &pMob[conn].MOB.Equip[6]);

			sprintf(temp, "useitem,extracao %d-%d:%d:%d:%d:%d:%d", item->sIndex, item->stEffect[0].cEffect, item->stEffect[0].cValue, item->stEffect[1].cEffect, item->stEffect[1].cValue, item->stEffect[2].cEffect, item->stEffect[2].cValue);
			ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);

			if (amount > 1)
				BASE_SetItemAmount(item, amount - 1);

			else
				memset(item, 0, sizeof(STRUCT_ITEM));

			SendEmotion(conn, 14, 3);
			return true;
		}

		return true;
	}
#pragma endregion
#pragma region Selo das Almas
	if(Vol == 206)
	{
		if(item->stEffect[0].cEffect == 0)
		{//Selo Vazio
			if(pMob[conn].extra.ClassMaster != SCELESTIAL)
			{
				SendClientMessage(conn, g_pMessageStringTable[_NN_GOD2CHARONLY_]);
				SendItem(conn, m->SourType, m->SourPos, item);
				return true;
			}

			if (amount > 1)
				BASE_SetItemAmount(item, amount - 1);

			else
				memset(item, 0, sizeof(STRUCT_ITEM));

			CharLogOut(conn);

			MSG_STANDARDPARM sm_sa;
			memset(&sm_sa, 0, sizeof(MSG_STANDARDPARM));

			sm_sa.Size = sizeof(MSG_STANDARDPARM);
			sm_sa.Type = _MSG_DBPutInCapsule;
			sm_sa.ID = conn;

			sm_sa.Parm = pUser[conn].Slot;

			DBServerSocket.SendOneMessage((char*)&sm_sa, sizeof(MSG_STANDARDPARM));
		}

		return true;
	}
#pragma endregion
#pragma region Bolsa do Andarilho
	if (item->sIndex == 3467)
	{
		if (pMob[conn].MOB.Carry[60].sIndex == 3467 && pMob[conn].MOB.Carry[61].sIndex == 3467)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_MAX_BAG]);
			return true;
		}

		if (pMob[conn].MOB.Carry[60].sIndex != 3467)
		{
			pMob[conn].MOB.Carry[60].sIndex = 3467;
			BASE_SetItemDate(&pMob[conn].MOB.Carry[60], 30);
			SendCarry(conn);
		}
		else
		{
			pMob[conn].MOB.Carry[61].sIndex = 3467;
			BASE_SetItemDate(&pMob[conn].MOB.Carry[61], 30);
			SendCarry(conn);
		}

		SendScore(conn);
		SendEtc(conn);


		if (amount > 1)
			BASE_SetItemAmount(item, amount - 1);

		else
			memset(item, 0, sizeof(STRUCT_ITEM));

		SendCarry(conn);

		return true;
	}
#pragma endregion
#pragma region War Pack
	if (Vol == 184)
	{
		int donate = BASE_GetItemAbility(item, EF_DONATE);

		pUser[conn].Donate += donate;

		SendClientMessage(conn, g_pMessageStringTable[_NN_Cash_ChargeOk]);

		if (amount > 1)
			BASE_SetItemAmount(item, amount - 1);

		else
			memset(item, 0, sizeof(STRUCT_ITEM));

		SendEmotion(conn, 14, 3);
		return true;
	}
#pragma endregion

	return false;
}
