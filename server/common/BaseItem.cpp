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

#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "common/Basedef.h"
#include "common/ItemEffect.h"
#include "common/BasedefInternal.h"

/// @file BaseItem.cpp
/// @brief Calculo de atributos, refinacao, grade de inventario e validacao de itens.

/// Verifica se uma movimentacao envolvendo o item ou ouro deve gerar log de auditoria.
int BASE_NeedLog(struct STRUCT_ITEM *item, int money)
{
	int idx = item->sIndex;

	if(idx < 0 || idx >= MAX_ITEMLIST)
		return FALSE;

	if(money >= 0)
		return TRUE;

	int sanc = BASE_GetItemSanc(item);

	if(sanc >= 3)
		return TRUE;

	int npos = g_pItemList[idx].nPos;

	int magic = BASE_GetBonusItemAbility(item, EF_MAGIC);
	int dam = BASE_GetBonusItemAbility(item, EF_DAMAGE);

	int guild = BASE_GetGuild(item);

	if(npos == 64 || npos == 192)
	{
		 if(magic >= 12 || dam >= 27)
			return TRUE;
	}
	else  if(magic >= 8 || dam >= 12)
		return TRUE;

	if(guild > 0 || idx == 419 || idx == 420 || idx == 412 || idx == 413 || idx == 753 || idx == 657 || (idx >= 470 && idx < 500) || idx == 522)
		return TRUE;

	if((idx >= 551 || idx <= 562) || item->stEffect[0].cEffect != 0 || item->stEffect[0].cEffect != 59|| item->stEffect[1].cEffect != 0 || item->stEffect[1].cEffect != 59)
		return TRUE;

	int price = g_pItemList[idx].nPrice;

	if(price >= 100)
		return TRUE;

	return FALSE;
}

/// Formata a representacao textual de um item (indice e efeitos) para logs e depuracao.
void BASE_GetItemCode(STRUCT_ITEM * item, char * str)
{
	int iidx = item->sIndex;
	unsigned char c0 = item->stEffect[0].cEffect;
	unsigned char v0 = item->stEffect[0].cValue;
	unsigned char c1 = item->stEffect[1].cEffect;
	unsigned char v1 = item->stEffect[1].cValue;
	unsigned char c2 = item->stEffect[2].cEffect;
	unsigned char v2 = item->stEffect[2].cValue;

	if (iidx <= 0 || iidx >= MAX_ITEMLIST)
	{
		str[0] = 0;
		return;
	}
	sprintf(str, " %s : %d.%d.%d.%d.%d.%d.%d", g_pItemList[item->sIndex].Name, item->sIndex, c0, v0, c1, v1, c2, v2);	
}

/// Consulta um atributo de item ignorando o multiplicador de refinacao (sanc).
int BASE_GetItemAbilityNosanc(STRUCT_ITEM *item, unsigned char type)
{
	// Regra do cliente (client/src/common/BaseItem.cpp:1125).
	int itemId = item->sIndex;
	int value = 0;

	if (itemId < 0 || itemId >= MAX_ITEMLIST)
		return value;

	int nUnique = g_pItemList[itemId].nUnique;
	int nPos = g_pItemList[itemId].nPos;

	if (type == EF_DAMAGEADD || type == EF_MAGICADD)
	{
		if (nUnique < 41 || nUnique > 50)
			return value;
	}

	if (type == EF_CRITICAL)
	{
		if (item->stEffect[1].cEffect == EF_CRITICAL2 || item->stEffect[2].cEffect == EF_CRITICAL2)
			type = EF_CRITICAL2;
	}

	if (type == EF_DAMAGE && nPos == 32)
	{
		if (item->stEffect[1].cEffect == EF_DAMAGE2 || item->stEffect[2].cEffect == EF_DAMAGE2)
			type = EF_DAMAGE2;
	}

	if (type == EF_MPADD)
	{
		if (item->stEffect[1].cEffect == EF_MPADD2 || item->stEffect[2].cEffect == EF_MPADD2)
			type = EF_MPADD2;
	}

	if (type == EF_HPADD)
	{
		if (item->stEffect[1].cEffect == EF_HPADD2 || item->stEffect[2].cEffect == EF_HPADD2)
			type = EF_HPADD2;
	}

	if (type == EF_ACADD)
	{
		if (item->stEffect[1].cEffect == EF_ACADD2 || item->stEffect[2].cEffect == EF_ACADD2)
			type = EF_ACADD2;
	}

	if (type == EF_LEVEL)
		value += g_pItemList[itemId].nReqLvl;

	if (type == EF_REQ_STR)
		value += g_pItemList[itemId].nReqStr;

	if (type == EF_REQ_INT)
		value += g_pItemList[itemId].nReqInt;

	if (type == EF_REQ_DEX)
		value += g_pItemList[itemId].nReqDex;

	if (type == EF_REQ_CON)
		value += g_pItemList[itemId].nReqCon;

	if (type == EF_POS)
		value += g_pItemList[itemId].nPos;

	if (type != EF_INCUBATE)
	{
		for (int i = 0; i < 12; i++)
		{
			if (g_pItemList[itemId].stEffect[i].sEffect != type)
				continue;

			if (g_pItemList[itemId].stEffect[i].sEffect == EF_HPADD || g_pItemList[itemId].stEffect[i].sEffect == EF_HPADD2)
				continue;

			int tvalue = g_pItemList[itemId].stEffect[i].sValue;

			if (type == EF_ATTSPEED && tvalue == 1)
				tvalue = 10;

			value += tvalue;
		}
	}

	if (itemId >= 2330 && itemId < 2390)
	{
		if (type == EF_MOUNTHP)
			return item->stEffect[0].sValue;

		else if (type == EF_MOUNTSANC)
			return item->stEffect[1].cEffect;

		else if (type == EF_MOUNTLIFE)
			return item->stEffect[1].cValue;

		else if (type == EF_MOUNTFEED)
			return item->stEffect[2].cEffect;

		else if (type == EF_MOUNTKILL)
			return item->stEffect[2].cValue;

		if (itemId < 2362 || itemId >= 2390 || item->stEffect[0].sValue <= 0)
			return value;

		int lv = item->stEffect[1].cEffect;
		int cd = item->sIndex - 2360;

		if (type == EF_DAMAGE)
			return g_pMountBonus[cd][0] * (lv + 20) / 100;

		else if (type == EF_MAGIC)
			return g_pMountBonus[cd][1] * (lv + 15) / 100;

		else if (type == EF_PARRY)
			return g_pMountBonus[cd][2];

		else if (type == EF_RESISTALL)
			return g_pMountBonus[cd][3];
		else
			return value;
	}

	if (itemId >= 3980 && itemId <= 3994)
	{

		if (type == EF_DAMAGE)
			return g_pMountBonus2[itemId - 3980][0];

		else if (type == EF_MAGIC)
			return g_pMountBonus2[itemId - 3980][1];

		else if (type == EF_PARRY)
			return g_pMountBonus2[itemId - 3980][2];

		else if (type == EF_RESISTALL)
			return g_pMountBonus2[itemId - 3980][3];
		else
			return value;
	}

	for (int i = 0; i < 3; i++)
	{
		if (item->stEffect[i].cEffect == type)
		{
			int total = item->stEffect[i].cValue;
			if (type == EF_ATTSPEED && total == 1)
				total = 10;

			value += total;
		}
	}

	return value;
}

/// Calcula o valor efetivo de um atributo de item (EF_*) aplicando bonus de refinacao.
int BASE_GetItemAbility(STRUCT_ITEM *item, unsigned char Type)
{
	// Regra do cliente. Diferenças para a versão antiga do servidor: HPADD2/MPADD2, refinação +11..+16
	// com a tabela de multiplicadores do cliente, regeneração multiplicada pela refinação, montarias
	// temporárias até 3999, e EF_RESISTALL não somado em cada resistência.
	int value = 0;
	int idx = item->sIndex;

	if (idx <= 0 || idx >= MAX_ITEMLIST)
		return 0;

	int nUnique = g_pItemList[idx].nUnique;
	int nPos = g_pItemList[idx].nPos;

	if ((Type == EF_DAMAGEADD || Type == EF_MAGICADD) && (nUnique < 41 || nUnique > 50))
		return 0;

	if (Type == EF_CRITICAL && (item->stEffect[1].cEffect == EF_CRITICAL2 || item->stEffect[2].cEffect == EF_CRITICAL2))
		Type = EF_CRITICAL2;

	if (Type == EF_DAMAGE && nPos == 32 && (item->stEffect[1].cEffect == EF_DAMAGE2 || item->stEffect[2].cEffect == EF_DAMAGE2))
		Type = EF_DAMAGE2;

	if (Type == EF_MPADD && (item->stEffect[1].cEffect == EF_MPADD2 || item->stEffect[2].cEffect == EF_MPADD2))
		Type = EF_MPADD2;

	if (Type == EF_HPADD && (item->stEffect[1].cEffect == EF_HPADD2 || item->stEffect[2].cEffect == EF_HPADD2))
		Type = EF_HPADD2;

	if (Type == EF_ACADD && (item->stEffect[1].cEffect == EF_ACADD2 || item->stEffect[2].cEffect == EF_ACADD2))
		Type = EF_ACADD2;

	if (Type == EF_LEVEL)
		value = g_pItemList[idx].nReqLvl;

	if (Type == EF_REQ_STR)
		value += g_pItemList[idx].nReqStr;

	if (Type == EF_REQ_INT)
		value += g_pItemList[idx].nReqInt;

	if (Type == EF_REQ_DEX)
		value += g_pItemList[idx].nReqDex;

	if (Type == EF_REQ_CON)
		value += g_pItemList[idx].nReqCon;

	if (Type == EF_POS)
		value += g_pItemList[idx].nPos;

	if (Type != EF_INCUBATE)
	{
		for (int i = 0; i < MAX_STATICEFFECT; ++i)
		{
			if (g_pItemList[idx].stEffect[i].sEffect == Type ||
				g_pItemList[idx].stEffect[i].sEffect == EF_HPADD && Type == EF_HPADD2)
			{
				int tvalue = g_pItemList[idx].stEffect[i].sValue;

				if (Type == EF_ATTSPEED && tvalue == 1)
					tvalue = 10;

				value += tvalue;
			}
		}
	}

	if (item->sIndex >= 2330 && item->sIndex < 2390)
	{
		switch (Type)
		{
		case EF_MOUNTHP:
			return item->stEffect[0].sValue;
		case EF_MOUNTSANC:
			return item->stEffect[1].cEffect;
		case EF_MOUNTLIFE:
			return item->stEffect[1].cValue;
		case EF_MOUNTFEED:
			return item->stEffect[2].cEffect;
		case EF_MOUNTKILL:
			return item->stEffect[2].cValue;
		}

		if (item->sIndex < 2362 || item->sIndex >= 2390 || item->stEffect[0].sValue <= 0)
			return value;

		int lv = item->stEffect[1].cEffect;
		int cd = item->sIndex - 2360;

		switch (Type)
		{
		case EF_DAMAGE:
			value = g_pMountBonus[cd][0] * (lv + 20) / 100;
			break;
		case EF_MAGIC:
			value = g_pMountBonus[cd][1] * (lv + 15) / 100;
			break;
		case EF_PARRY:
			value = g_pMountBonus[cd][2];
			break;
		case EF_RESISTALL:
			value = g_pMountBonus[cd][3];
			break;
		default:
			break;
		}
	}
	else if (item->sIndex >= 3980 && item->sIndex < 4000)
	{
		int cd = item->sIndex - 3980;

		switch (Type)
		{
		case EF_DAMAGE:
			value = g_pMountBonus2[cd][0];
			break;
		case EF_MAGIC:
			value = g_pMountBonus2[cd][1];
			break;
		case EF_PARRY:
			value = g_pMountBonus2[cd][2];
			break;
		case EF_RESISTALL:
			value = g_pMountBonus2[cd][3];
			break;
		default:
			break;
		}
	}
	else
	{
		for (int j = 0; j < 3; ++j)
		{
			if (item->stEffect[j].cEffect == Type)
			{
				int tvalue = item->stEffect[j].cValue;

				if (Type == EF_ATTSPEED && tvalue == 1)
					tvalue = 10;

				value += tvalue;
			}
		}

		int sanc = BASE_GetItemSanc(item);

		if (item->sIndex <= 40)
			sanc = 0;

		if (sanc >= 9 && nPos & 0xF00)
			sanc++;

		if (sanc
			&& Type != EF_GRID
			&& Type != EF_CLASS
			&& Type != EF_POS
			&& Type != EF_WTYPE
			&& Type != EF_RANGE
			&& Type != EF_LEVEL
			&& Type != EF_REQ_STR
			&& Type != EF_REQ_INT
			&& Type != EF_REQ_DEX
			&& Type != EF_REQ_CON
			&& Type != EF_VOLATILE
			&& Type != EF_INCUBATE
			&& Type != EF_INCUDELAY
			&& Type != EF_PREVBONUS
			&& Type != EF_TRANS
			&& Type != EF_REFLEVEL
			&& Type != EF_GAMEROOM
			&& Type != EF_REGENMP
			&& Type != EF_REGENHP
			&& Type != EF_FAME)
		{
			if (sanc > 10)
			{
				// +11..+16: multiplicador em centésimos.
				int UpSanc = sanc - 10;

				switch (UpSanc)
				{
				case 1:
					UpSanc = 220;
					break;
				case 2:
					UpSanc = 250;
					break;
				case 3:
					UpSanc = 280;
					break;
				case 4:
					UpSanc = 320;
					break;
				case 5:
					UpSanc = 370;
					break;
				case 6:
					UpSanc = 400;
					break;
				}

				value = UpSanc * 10 * value / 100 / 10;
			}
			else
				value = value * (sanc + 10) / 10;
		}

		if (Type == EF_RUNSPEED)
		{
			if (value >= 3)
				value = 2;

			if (value > 0 && sanc >= 9)
				value++;
		}

		if (Type == EF_REGENMP || Type == EF_REGENHP)
			value *= sanc;

		if (Type == EF_GRID && (value < 0 || value > 7))
			value = 0;
	}

	return value;
}

/// Consulta um atributo estatico definido apenas na tabela base ItemList.
int BASE_GetStaticItemAbility(STRUCT_ITEM *item, unsigned char Type)
{
	// Regra do cliente (client/src/common/BaseItem.cpp:343).
	int value = 0;
	int idx = item->sIndex;

	if (idx <= 0 || idx >= MAX_ITEMLIST)
		return value;

	if (idx >= 3200 && idx <= 3300)
		return 0;

	int nPos = g_pItemList[idx].nPos;

	if (Type == EF_LEVEL)
		value += g_pItemList[idx].nReqLvl;

	if (Type == EF_REQ_STR)
		value += g_pItemList[idx].nReqStr;

	if (Type == EF_REQ_INT)
		value += g_pItemList[idx].nReqInt;

	if (Type == EF_REQ_DEX)
		value += g_pItemList[idx].nReqDex;

	if (Type == EF_REQ_CON)
		value += g_pItemList[idx].nReqCon;

	if (Type == EF_POS)
		value += g_pItemList[idx].nPos;

	if (Type != EF_INCUBATE)
	{
		for (int i = 0; i < 12; i++)
		{
			if (g_pItemList[idx].stEffect[i].sEffect != Type)
				continue;

			int tvalue = g_pItemList[idx].stEffect[i].sValue;

			if (Type == EF_ATTSPEED && tvalue == 1)
				tvalue = 10;

			value += tvalue;
		}
	}

	if (idx >= 2330 && idx < 2390)
	{
		if (Type == EF_MOUNTHP)
			return item->stEffect[0].sValue;

		else if (Type == EF_MOUNTSANC)
			return item->stEffect[1].cEffect;

		else if (Type == EF_MOUNTLIFE)
			return item->stEffect[1].cValue;

		else if (Type == EF_MOUNTFEED)
			return item->stEffect[2].cEffect;

		else if (Type == EF_MOUNTKILL)
			return item->stEffect[2].cValue;

		if (idx < 2362 || idx >= 2390 || item->stEffect[0].sValue <= 0)
			return value;

		int lv = item->stEffect[1].cEffect;
		int cd = item->sIndex - 2360;

		if (Type == EF_DAMAGE)
			return g_pMountBonus[cd][0] * (lv + 20) / 100;

		else if (Type == EF_MAGIC)
			return g_pMountBonus[cd][1] * (lv + 15) / 100;

		else if (Type == EF_PARRY)
			return g_pMountBonus[cd][2];

		else if (Type == EF_RESISTALL)
			return g_pMountBonus[cd][3];
		else
			return value;
	}

	if (idx >= 3980 && idx <= 3994)
	{

		if (Type == EF_DAMAGE)
			return g_pMountBonus2[idx - 3980][0];

		else if (Type == EF_MAGIC)
			return g_pMountBonus2[idx - 3980][1];

		else if (Type == EF_PARRY)
			return g_pMountBonus2[idx - 3980][2];

		else if (Type == EF_RESISTALL)
			return g_pMountBonus2[idx - 3980][3];
		else
			return value;
	}

	int sanc = BASE_GetItemSanc(item);

	if (sanc >= 9 && nPos & 0xF00)
		sanc++;

	if (sanc
		&& Type != EF_GRID
		&& Type != EF_CLASS
		&& Type != EF_POS
		&& Type != EF_WTYPE
		&& Type != EF_RANGE
		&& Type != EF_LEVEL
		&& Type != EF_REQ_STR
		&& Type != EF_REQ_INT
		&& Type != EF_REQ_DEX
		&& Type != EF_REQ_CON
		&& Type != EF_VOLATILE
		&& Type != EF_INCUBATE
		&& Type != EF_INCUDELAY
		&& Type != EF_PREVBONUS
		&& Type != EF_REGENMP
		&& Type != EF_REGENHP)
	{
		if (sanc > 10)
		{
			int UpSanc = sanc - 10;
			switch (UpSanc)
			{
			case 1:
				UpSanc = 220;
				break;
			case 2:
				UpSanc = 250;
				break;
			case 3:
				UpSanc = 280;
				break;
			case 4:
				UpSanc = 320;
				break;
			case 5:
				UpSanc = 370;
				break;
			case 6:
				UpSanc = 400;
				break;
			}
			value = UpSanc * 10 * value / 100 / 10;
		}
		else
		{
			value = value * (sanc + 10) / 10;
		}
	}

	if (Type == EF_RUNSPEED)
	{
		if (value >= 3)
			value = 2;

		if (value > 0 && sanc >= 9)
			value++;
	}

	if ((Type == EF_REGENMP || Type == EF_REGENHP) && sanc > 0)
		value *= sanc;

	return value;
}

/// Retorna o adicional extra de um atributo considerando a refinacao do item.
int BASE_GetBonusItemAbility(STRUCT_ITEM *item, unsigned char Type)
{
	// Regra do cliente (client/src/common/BaseItem.cpp:1039).
	if (item->sIndex <= 0 || item->sIndex >= MAX_ITEMLIST)
		return 0;

	if (item->sIndex >= 3200 && item->sIndex <= 3300)
		return 0;

	if (item->sIndex >= 2330 && item->sIndex < 2390)
		return 0;

	if (item->sIndex >= 3980 && item->sIndex < 4000)
		return 0;

	int value = 0;

	int nPos = g_pItemList[item->sIndex].nPos;

	for (int i = 0; i < 3; ++i)
	{
		if (item->stEffect[i].cEffect == Type)
		{
			int tvalue = item->stEffect[i].cValue;

			if (Type == EF_ATTSPEED && tvalue == 1)
				tvalue = 10;

			value += tvalue;
		}
	}

	int sanc = BASE_GetItemSanc(item);

	if (sanc >= 9 && nPos & 0xF00)
		sanc++;

	if (sanc
		&& Type != EF_GRID
		&& Type != EF_CLASS
		&& Type != EF_POS
		&& Type != EF_WTYPE
		&& Type != EF_RANGE
		&& Type != EF_LEVEL
		&& Type != EF_REQ_STR
		&& Type != EF_REQ_INT
		&& Type != EF_REQ_DEX
		&& Type != EF_REQ_CON
		&& Type != EF_VOLATILE
		&& Type != EF_INCUBATE
		&& Type != EF_INCUDELAY
		&& Type != EF_PREVBONUS)
	{
		if (sanc > 10)
		{
			int UpSanc = sanc - 10;
			switch (UpSanc)
			{
			case 1:
				UpSanc = 220;
				break;
			case 2:
				UpSanc = 250;
				break;
			case 3:
				UpSanc = 280;
				break;
			case 4:
				UpSanc = 320;
				break;
			case 5:
				UpSanc = 370;
				break;
			case 6:
				UpSanc = 400;
				break;
			}
			value = UpSanc * 10 * value / 100 / 10;
		}
		else
		{
			value = value * (sanc + 10) / 10;
		}
	}
	return value;
}

/// Retorna o adicional extra bruto de um atributo sem aplicar refinacao.
int BASE_GetBonusItemAbilityNosanc(STRUCT_ITEM *item, unsigned char Type)
{
	// Regra do cliente (client/src/common/BaseItem.cpp:1007).
	if (item->sIndex <= 0 || item->sIndex >= MAX_ITEMLIST)
		return 0;

	if (item->sIndex >= 3200 && item->sIndex <= 3300)
		return 0;

	if (item->sIndex >= 2330 && item->sIndex < 2390)
		return 0;

	if (item->sIndex >= 3980 && item->sIndex < 4000)
		return 0;

	int value = 0;

	for (int i = 0; i < 3; i++)
	{
		if (item->stEffect[i].cEffect != Type)
			continue;

		int tvalue = item->stEffect[i].cValue;

		if (Type == EF_ATTSPEED && tvalue == 1)
			tvalue = 10;

		value += tvalue;
	}

	return value;
}

/// Extrai o nivel de refinacao (+0..+15) de um item considerando codificacao de tintura.
int BASE_GetItemSanc(STRUCT_ITEM *item)
{
	// Regra do cliente. +10..+16 voltam como 10..16 (REF_10..REF_15 e 16); antes +11..+15 voltavam
	// 12/15/18/22/27, valores que o multiplicador de BASE_GetItemAbility usava direto.
	if (item->sIndex >= 2330 && item->sIndex < 2390)
		return 0;

	if (item->sIndex >= 3200 && item->sIndex < 3300)
		return 0;

	if (item->sIndex >= 3980 && item->sIndex < 4000)
		return 0;

	int sanc = 0;

	if (item->stEffect[0].cEffect != EF_SANC && item->stEffect[1].cEffect != EF_SANC && item->stEffect[2].cEffect != EF_SANC)
	{
		// Sem EF_SANC, a refinação vem na tintura (115..126).
		if (item->stEffect[0].cEffect >= 115 && item->stEffect[0].cEffect <= 126)
			sanc = item->stEffect[0].cValue;

		else if (item->stEffect[1].cEffect >= 115 && item->stEffect[1].cEffect <= 126)
			sanc = item->stEffect[1].cValue;

		else if (item->stEffect[2].cEffect >= 115 && item->stEffect[2].cEffect <= 126)
			sanc = item->stEffect[2].cValue;
	}
	else if (item->stEffect[0].cEffect == EF_SANC)
		sanc = item->stEffect[0].cValue;

	else if (item->stEffect[1].cEffect == EF_SANC)
		sanc = item->stEffect[1].cValue;

	else
		sanc = item->stEffect[2].cValue;

	if (item->sIndex != 786 && item->sIndex != 1936 && item->sIndex != 1937)
	{
		if (sanc < 230)
			sanc %= 10;
		else
			sanc -= 220;

		if (sanc >= 10 && sanc <= 35)
			sanc = (sanc - 10) / 4 + 10;
	}

	return sanc;
}

/// Retorna o indice da gema/tintura associada a um item refinado.
int BASE_GetItemGem(STRUCT_ITEM *item)
{
	if (item->sIndex >= 2330 && item->sIndex < 2390)
		return FALSE;

	unsigned char sanc = 0;

	if (item->stEffect[0].cEffect >= 116 && item->stEffect[0].cEffect <= 125)
		sanc = item->stEffect[0].cValue;

	else if (item->stEffect[1].cEffect >= 116 && item->stEffect[1].cEffect <= 125)
		sanc = item->stEffect[1].cValue;

	else if (item->stEffect[2].cEffect >= 116 && item->stEffect[2].cEffect <= 125)
		sanc = item->stEffect[2].cValue;

	else if (item->stEffect[0].cEffect == EF_SANC || item->stEffect[1].cEffect == EF_SANC || item->stEffect[2].cEffect == EF_SANC)
	{
		if (item->stEffect[0].cEffect == EF_SANC)
			sanc = item->stEffect[0].cValue;
		else if (item->stEffect[1].cEffect == EF_SANC)
			sanc = item->stEffect[1].cValue;
		else
			sanc = item->stEffect[2].cValue;
	}

	int isanc = sanc;

	if(isanc < 230)
		return -1;

	return (isanc-230) % 4;
}

/// Extrai o contador auxiliar de sucesso/bonus embutido em EF_SANC.
/// @brief Efeito que carrega a refinação: EF_SANC ou uma tintura (115..125).
bool BASE_HasSancAdd(unsigned char cEffect)
{
	return cEffect == EF_SANC || (cEffect >= 115 && cEffect < 126);
}

/// @brief Valor bruto do primeiro efeito que carrega a refinação (0 se nenhum).
int BASE_GetSancEffValue(const STRUCT_ITEM &item)
{
	for (auto &effect : item.stEffect)
	{
		if (BASE_HasSancAdd(effect.cEffect))
			return effect.cValue;
	}

	return 0;
}

int BASE_GetItemSancSuccess(STRUCT_ITEM *item)
{
	// Regra do cliente: dezenas do valor bruto (as tentativas acumuladas), até 210.
	if (item->sIndex >= 2330 && item->sIndex < 2390)
		return 0;

	if (item->sIndex >= 3200 && item->sIndex < 3300)
		return 0;

	if (item->sIndex >= 3980 && item->sIndex < 4000)
		return 0;

	int sanc = BASE_GetSancEffValue(*item);

	if (sanc <= 210)
		return sanc / 10;

	return 0;
}

/// @brief Troca o valor de `effect` no item ou, se não houver, o grava no primeiro efeito livre.
void BASE_ChangeOrAddEffectValue(STRUCT_ITEM *item, int effect, int value)
{
	for (auto &i : item->stEffect)
	{
		if (i.cEffect == effect)
		{
			i.cValue = static_cast<unsigned char>(value);
			return;
		}
	}

	for (auto &i : item->stEffect)
	{
		if (i.cEffect == 0)
		{
			i.cEffect = static_cast<unsigned char>(effect);
			i.cValue = static_cast<unsigned char>(value);
			return;
		}
	}
}

/// @brief Apaga todos os efeitos `effect` do item.
void BASE_RemoveEffect(STRUCT_ITEM *item, int effect)
{
	for (auto &i : item->stEffect)
	{
		if (i.cEffect == effect)
		{
			i.cEffect = 0;
			i.cValue = 0;
		}
	}
}

/// Calcula a chance de sucesso de refinacao com Poeira de Ori/Lac.
int BASE_GetSuccessRate(STRUCT_ITEM *item, int OriLacto)
{
	if(item->sIndex >= 2330 && item->sIndex < 2390)
		return FALSE;

	if(OriLacto != 0 && OriLacto != 1 && OriLacto != 2)
		return FALSE;
	
	int sanc = BASE_GetItemSanc(item);

	if(sanc == REF_10 && OriLacto != 2)
		return 15;

	if(sanc == REF_10 && OriLacto == 2)
		return 100;

	sanc %= 10;
	int succ = BASE_GetItemSancSuccess(item);
	int rate = g_pSancRate[OriLacto][sanc + 1];

	rate += succ * g_pSuccessRate[sanc + 1];

	return OriLacto == 2 ? 100 : rate;
}

/// Calcula a taxa de crescimento de montaria/item baseada em refinacao.
int BASE_GetGrowthRate(STRUCT_ITEM *item)
{
	if(item->sIndex < 2360 || item->sIndex >= 2390)
		return FALSE;

	int sanc = item->stEffect[1].cEffect;
	sanc /= 10;

	// A tabela tem 12 colunas (níveis 0..119). Nível >= 120 é o máximo: não cresce mais
	// (UseItem_Scrolls já recusa antes de chamar; antes lia além do fim de g_pSancRate).
	if (sanc >= 12)
		return 0;

	int rate = g_pSancRate[2][sanc];

	return rate;
}

/// Atualiza o campo EF_SANC do item preservando gema/sucesso quando aplicavel.
int BASE_SetItemSanc(STRUCT_ITEM *dest, int sanc, int success)
{
	if(sanc < 0)
		sanc = 0;
	if(sanc > 15)
		sanc = 0;
	if(success < 0)
		success = 0;
	if(success > 20)
		success = 20;

	int	sc = sanc + (10 * success);

	if (sanc == 10)
		sc = 230 + success;

	else if (sanc == 11)
		sc = 234 + success;

	else if (sanc == 12)
		sc = 238 + success;

	else if (sanc == 13)
		sc = 242 + success;

	else if (sanc == 14)
		sc = 246 + success;

	else if (sanc == 15)
		sc = 250 + success;

	if (dest->stEffect[0].cEffect == EF_SANC || dest->stEffect[0].cEffect >= 116 && dest->stEffect[0].cEffect <= 125)
		dest->stEffect[0].cValue = sc;

	else if (dest->stEffect[1].cEffect == EF_SANC || dest->stEffect[1].cEffect >= 116 && dest->stEffect[1].cEffect <= 125)
		dest->stEffect[1].cValue = sc;
	
	else
	{
		if (dest->stEffect[2].cEffect != EF_SANC && (dest->stEffect[2].cEffect < 116 || dest->stEffect[2].cEffect > 125))
			return FALSE;

		dest->stEffect[2].cValue = sc;
	}

	return TRUE;
}

/// Retorna a quantidade empilhada em EF_AMOUNT (ou 1 se nao empilhavel).
int BASE_GetItemAmount(STRUCT_ITEM *item)
{
	int amount = 0;

	if(item->stEffect[0].cEffect == EF_AMOUNT || item->stEffect[1].cEffect == EF_AMOUNT || item->stEffect[2].cEffect == EF_AMOUNT)
	{
		if(item->stEffect[0].cEffect == EF_AMOUNT)
			amount = item->stEffect[0].cValue;
		else if(item->stEffect[1].cEffect == EF_AMOUNT)
			amount = item->stEffect[1].cValue;
		else
			amount = item->stEffect[2].cValue;
	}

	return amount;
}

/// Define a quantidade empilhada em EF_AMOUNT respeitando os limites do slot.
void BASE_SetItemAmount(STRUCT_ITEM *item, int amount)
{
	// Regra do cliente: poções e pedras que se empilham perdem o EF_UNIQUE; o EF_AMOUNT é trocado ou
	// gravado no primeiro efeito livre.
	if (item->sIndex == 419 || item->sIndex == 420 || item->sIndex == 412 || item->sIndex == 413 ||
		item->sIndex >= 2390 && item->sIndex <= 2418)
	{
		BASE_RemoveEffect(item, EF_UNIQUE);
	}

	BASE_ChangeOrAddEffectValue(item, EF_AMOUNT, amount);
}

/// Localiza o ponteiro para um STRUCT_ITEM em Equip, Carry ou Cargo.
STRUCT_ITEM *GetItemPointer(STRUCT_MOB *mob, STRUCT_ITEM *cargo, int type, int pos)
{
	STRUCT_ITEM * sour = NULL;

    if(type == ITEM_PLACE_EQUIP)
	{   
		if(pos < 0 || pos >= MAX_EQUIP)
			return NULL;

		sour = &mob->Equip[pos];
	}   
	else if(type == ITEM_PLACE_CARRY)
	{   
		if(pos < 0 || pos >= MAX_CARRY)
			return NULL;

		sour = &mob->Carry[pos];
	}   
	else if(type == ITEM_PLACE_CARGO)
	{   
		if(pos < 0 || pos >= MAX_CARGO) 
			return NULL;

		sour = &cargo[pos];
	}  
	else 
		return NULL; 

	if(sour->sIndex < 0 || sour->sIndex >= MAX_ITEMLIST)		
		return NULL;

   return sour;
}

/// Zera todos os campos de um STRUCT_ITEM.
void BASE_ClearItem(STRUCT_ITEM *item)
{
	item->sIndex = 0;

	item->stEffect[0].cEffect = 0;
	item->stEffect[0].sValue = 0;
	item->stEffect[1].cEffect = 0;
	item->stEffect[1].sValue = 0;
	item->stEffect[2].cEffect = 0;
	item->stEffect[2].sValue = 0;
}

/// Valida requisitos (nivel, atributos, classe, slot) para equipar um item.
int BASE_CanEquip(STRUCT_ITEM *item, STRUCT_SCORE *score, int Pos, int Class, STRUCT_ITEM *pBaseEquip, int OriginalFace, bool hasSoulLimitSkill)
{
	// Regra do cliente. Class é o rosto atual (MOB.Equip[0].sIndex: dezena = classe, unidade acima
	// de 5 = evoluído) e OriginalFace o rosto antes de uma transformação (22..25, 32);
	// hasSoulLimitSkill é o bit 0x40000000 de LearnedSkill. Antes o servidor decidia pela ClassMaster.
	int idx = item->sIndex;
	if (idx <= 0 || idx >= MAX_ITEMLIST)
		return FALSE;

	int nUnique = g_pItemList[idx].nUnique;
	if (Pos == 15)
		return FALSE;

	if (Pos != -1)
	{
		int tpos = BASE_GetItemAbility(item, EF_POS);
		int pos = (tpos >> Pos) & 1;

		if (pos == 0)
			return FALSE;

		if (Pos == 6 || Pos == 7)
		{
			int OtherPos = (Pos == 6) ? 7 : 6;
			int	OtherIdx = pBaseEquip[OtherPos].sIndex;

			if (OtherIdx > 0 && OtherIdx < MAX_ITEMLIST)
			{
				int nUnique2 = g_pItemList[OtherIdx].nUnique;
				int otherpos = BASE_GetItemAbility(&pBaseEquip[OtherPos], EF_POS);

				if (tpos == 64 || otherpos == 64)
				{
					if (nUnique == 46)
					{
						if (otherpos != 128)
							return FALSE;
					}
					else if (nUnique2 == 46)
					{
						if (tpos != 128)
							return FALSE;
					}
					else
						return FALSE;
				}
			}
		}
	}

	if (Class >= 22 && Class <= 25 || Class == 32)
		Class = OriginalFace;

	int trans = Class % 10;

	if (Pos == 1 && trans > 5
		&& item->sIndex != 747
		&& item->sIndex != 3500
		&& item->sIndex != 3507
		&& item->sIndex != 3501
		&& item->sIndex != 3502
		&& item->sIndex != 3303
		&& item->sIndex != 3507)
	{
		return FALSE;
	}

	int transitem = BASE_GetItemAbility(item, EF_TRANS);
	switch (transitem)
	{
	case 1:
		if (trans < 6)
			return FALSE;
		break;
	case 2:
		if (trans >= 6)
			return FALSE;
		break;
	case 3:
		if (trans < 6)
			return FALSE;
		if (!hasSoulLimitSkill)
			return FALSE;
		break;
	}

	// O x86 usa só os 5 bits baixos da contagem de deslocamento; a máscara explícita dá o mesmo
	// resultado sem comportamento indefinido quando Class / 10 passa de 31.
	if (!((BASE_GetItemAbility(item, EF_CLASS) >> ((Class / 10) & 31)) & 1))
	{
		if (trans <= 5)
			return FALSE;

		int pos = BASE_GetItemAbility(item, EF_POS);
		if (pos != 64 && pos != 128 && pos != 192)
			return FALSE;
	}
	if (nUnique % 10 >= 9 && nUnique < 40 && trans < 6)
		return FALSE;

	int lvl = BASE_GetItemAbility(item, EF_LEVEL);
	int str = BASE_GetItemAbility(item, EF_REQ_STR);
	int spt = BASE_GetItemAbility(item, EF_REQ_INT);
	int agi = BASE_GetItemAbility(item, EF_REQ_DEX);
	int con = BASE_GetItemAbility(item, EF_REQ_CON);

	int wtype = BASE_GetItemAbility(item, EF_WTYPE);

	int weapontype = wtype % 10;
	int modweapon = wtype % 10;

	int divweapon = wtype % 10 / 10;
	if (Pos == 7 && weapontype)
	{
		int rate = 100;
		if (divweapon || modweapon <= 1)
		{
			if (divweapon == 6 && modweapon > 1)
				rate = 150;
		}
		else
		{
			rate = 130;
		}
		lvl = rate * lvl / 100;
		str = rate * str / 100;
		spt = rate * spt / 100;
		agi = rate * agi / 100;
		con = rate * con / 100;
	}

	if (trans < 5)
	{
		if (lvl > score->Level)
			return FALSE;
		if (str > score->Str)
			return FALSE;
		if (spt > score->Int)
			return FALSE;
		if (agi > score->Dex)
			return FALSE;
		if (con > score->Con)
			return FALSE;
	}

	return TRUE;
}

/// Índice do formato do item em g_pItemGrid (EF_GRID). Valor fora de 0..7 (dado corrompido)
/// vale como item de uma célula, em vez de ler além do fim de g_pItemGrid.
static int ItemGridIndex(STRUCT_ITEM *item)
{
	int grid = BASE_GetItemAbility(item, EF_GRID);

	if (grid < 0 || grid >= 8)
		return 0;

	return grid;
}

/// Verifica se ha espaco livre na grade do inventario (Carry) para o item.
int BASE_CanCarry(STRUCT_ITEM *item, STRUCT_ITEM *carry, int DestX, int DestY, int *error)
{
	int grid = ItemGridIndex(item);

	char SourGrid[8];

	memcpy(SourGrid, g_pItemGrid[grid], sizeof(SourGrid));

	char CarryGrid[MAX_CARRY];

	memset(CarryGrid, 0, MAX_CARRY-1);

	for(int i = 0; i < MAX_CARRY; i++)
	{
		if(carry[i].sIndex == 0)
			continue;

		grid = ItemGridIndex(&carry[i]);

		char TempGrid[8];

		memcpy(TempGrid, g_pItemGrid[grid], sizeof(TempGrid));

		int tx = i % 9;
		int ty = i / 9;

		for(int yy = 0; yy < 4; yy++)
		{
			for(int xx = 0; xx < 2; xx++)
			{
				if(TempGrid[yy * 2 + xx] == 0)
					continue;

				if(yy + ty < 0 || xx + tx < 0 || yy + ty >= CARRYGRIDY || xx + tx >= CARRYGRIDX)
					continue;

				CarryGrid[(yy + ty) * CARRYGRIDX + xx + tx] = i + 1;
			}
		}
	}

	for(int yy = 0; yy < 4; yy++)
	{
		for(int xx = 0; xx < 2; xx++)
		{
			if(SourGrid[(yy * 2) + xx] == 0)
				continue;

			if(yy + DestY < 0 || xx + DestX < 0 || yy + DestY >= CARRYGRIDY || xx + DestX >= CARRYGRIDX)
			{
				*error = -1;

				return FALSE;
			}

			if(CarryGrid[(yy + DestY) * CARRYGRIDX + xx + DestX] != 0)
			{
				*error = CarryGrid[(yy + DestY) * CARRYGRIDX + xx + DestX];

				return FALSE;
			}
		}
	}

	return TRUE;
}

/// Verifica se ha espaco livre na grade do bau/armazem (Cargo) para o item.
int BASE_CanCargo(STRUCT_ITEM *item, STRUCT_ITEM *cargo, int DestX, int DestY)
{
	// Regra do cliente: a grade de cada item vem de BASE_GetItemAbility(EF_GRID).
	int grid = BASE_GetItemAbility(item, EF_GRID);

	char SourGrid[4][2]{};
	char CargoGrid[CARGOGRIDY][CARGOGRIDX]{};

	memcpy((char*)SourGrid, (char*)g_pItemGrid[grid], 8u);

	for (int i = 0; i < MAX_CARGO; ++i)
	{
		if (!cargo[i].sIndex)
			continue;

		char TempGrid[4][2]{};
		int cgrid = BASE_GetItemAbility(&cargo[i], EF_GRID);

		if (cgrid < 0 || cgrid >= 8)
			cgrid = 0;

		memcpy((char*)TempGrid, (char*)g_pItemGrid[cgrid], 8u);

		int tx = i % CARGOGRIDX;
		int ty = i / CARGOGRIDX;

		for (int yy = 0; yy < 4; ++yy)
		{
			for (int xx = 0; xx < 2; ++xx)
			{
				if (TempGrid[yy][xx] == 1 && ty + yy >= 0 && tx + xx >= 0 && ty + yy < CARGOGRIDY && tx + xx < CARGOGRIDX)
					CargoGrid[ty + yy][tx + xx] = 1;
			}
		}
	}

	for (int j = 0; j < 4; ++j)
	{
		for (int k = 0; k < 2; ++k)
		{
			if (SourGrid[j][k] != 1)
				continue;

			if (DestY + j < 0 || DestX + k < 0 || DestY + j >= CARGOGRIDY || DestX + k >= CARGOGRIDX)
				return FALSE;

			if (CargoGrid[DestY + j][DestX + k] == 1)
				return FALSE;
		}
	}

	return TRUE;
}

/// Compacta/ordena itens no inventario temporario de troca.
void BASE_SortTradeItem(STRUCT_ITEM *Item, int Type)
{
	int Buffer[MAX_TRADE];

	memset(Buffer, -1, sizeof(Buffer));

	for(int i = 0; i < MAX_TRADE; i++)
	{
		if(!Item[i].sIndex)
		{
			Buffer[i] = -1;

			continue;
		}

		Buffer[i] = BASE_GetItemAbility(&Item[i], Type);
	}

	STRUCT_ITEM ItemTemp[MAX_TRADE];

	memset(ItemTemp, 0, sizeof(ItemTemp));

	for(int i = 0; i < MAX_TRADE; i++)
	{
		int MaxBufferIndex = 0;
		int MaxBuffer = -1;		

		for(int j = 0; j < MAX_TRADE; j++)
		{
			if(Buffer[j] <= MaxBuffer)
				continue;

			MaxBufferIndex = j;

			MaxBuffer = Buffer[j];
		}

		if(MaxBuffer == -1)
			break;

		Buffer[MaxBufferIndex] = -1;

		*(int*)&ItemTemp[i] = *(int*)&Item[MaxBufferIndex];
		memcpy((char*)&ItemTemp[i] + 4, (char*)&Item[MaxBufferIndex] + 4, 4);	// bytes 4..7: stEffect[1] e stEffect[2]
	}

	memcpy(Item, ItemTemp, sizeof(ItemTemp));
}

/// Valida se a troca entre dois inventarios possui espaco suficiente para ambos.
/// @brief Regra do cliente: a posição `pos` do inventário pode receber item? 0..29 sempre; 30..44 só
/// com a bolsa (3467) em Carry[60]; 45..59 só com a bolsa em Carry[61].
int BASE_CanCarry(STRUCT_ITEM *Carry, int pos)
{
	if (pos < 30)
		return 1;

	if (pos / 15 == 2 && Carry[60].sIndex != 3467)
		return 0;

	if (pos / 15 != 3 || Carry[61].sIndex == 3467)
		return 1;

	return 0;
}

/// @brief A posição `pos` do inventário (0..59) existe e está liberada? As posições 60..63 guardam as
/// bolsas e não são inventário; BASE_CanCarry, como no cliente, não as recusa.
int BASE_CarryUsable(STRUCT_ITEM *Carry, int pos)
{
	return pos >= 0 && pos < MAX_CARRY - 4 && BASE_CanCarry(Carry, pos);
}

/// @brief Próxima posição liberada depois de `pos`, ou MAX_CARRY - 4 se não há mais. Para percorrer o
/// inventário pulando a área de uma bolsa ausente: com só a bolsa de Carry[61], vai de 29 para 45.
int BASE_NextCarry(STRUCT_ITEM *Carry, int pos)
{
	do
		++pos;
	while (pos < MAX_CARRY - 4 && !BASE_CanCarry(Carry, pos));

	return pos;
}

int BASE_CanTrade(STRUCT_ITEM *Dest, STRUCT_ITEM *Carry, unsigned char *MyTrade, STRUCT_ITEM *OpponentTrade)
{
	// Regra do cliente: os itens recebidos vão para as posições livres que BASE_CanCarry libera (as
	// 60 visíveis), em vez das primeiras 30 + 15 por bolsa.
	STRUCT_ITEM OpponentTemp[MAX_TRADE];

	memcpy(OpponentTemp, OpponentTrade, sizeof(OpponentTemp));

	memcpy(Dest, Carry, sizeof(STRUCT_ITEM) * MAX_CARRY);

	for (int i = 0; i < MAX_TRADE; ++i)
	{
		int pos = MyTrade[i]; // 255 (-1) marca posição vazia

		if (pos < MAX_CARRY)
			BASE_ClearItem(&Dest[pos]);
	}

	BASE_SortTradeItem(OpponentTemp, EF_GRID);

	for (int i = 0; i < MAX_TRADE; i++)
	{
		if (!OpponentTemp[i].sIndex)
			continue;

		int j = 0;

		for (j = 0; j < MAX_CARRY - 4; j++)
		{
			if (!Dest[j].sIndex && BASE_CanCarry(Dest, j))
			{
				Dest[j] = OpponentTemp[i];
				break;
			}
		}

		if (j == MAX_CARRY - 4)
			return FALSE;
	}

	return TRUE;
}

/// Atualiza mascara de colisao no mapa de altura para objetos/portoes (versao com ponteiro de altura).
int BASE_UpdateItem(int maskidx, int CurrentState, int NextState, int xx, int yy, char *pHeight, int rotate, int *height)
{
	*height = 0;
	int delta = 0;

	if(maskidx >= MAX_GROUNDMASK || maskidx < 0)
		return FALSE;

	if(CurrentState == 1 && (NextState == 3 || NextState == 2))
		delta = 1;
	if(NextState == 1 && (CurrentState == 3 || CurrentState == 2))
		delta = -1;

	if(delta == 0)
		return FALSE;

	// Cada máscara tem 6x6 células (antes o laço usava MAX_GROUNDMASK, que era 6).
	for(int y = 0; y <= 5; y++)
	{
		for(int x = 0; x <= 5; x++)
		{
			int val = g_pGroundMask[maskidx][rotate][y][x];
			int xp = xx + x - 2;
            int yp = yy + y - 2;

			if(val == 0)
				continue;

			val *= delta;

			if((xp - g_HeightPosX < 1) || (yp - g_HeightPosY < 1) || ((xp - g_HeightPosX) > (g_HeightWidth - 2)) || ((yp - g_HeightPosY) > (g_HeightHeight - 2)))
				break;

			int Last = val + pHeight[(((yp - g_HeightPosY) * g_HeightWidth) + xp) - g_HeightPosX];

			if(Last > 255)
				Last = 255;
			if(Last < 0)
				Last = 0;

			if(g_pGroundMask[maskidx][rotate][y][x] != 0)
				*height = Last;

			pHeight[(((yp - g_HeightPosY) * g_HeightWidth) + xp) - g_HeightPosX] = Last;
		}
	}

	return TRUE;
}

/// Atualiza mascara de colisao no mapa de altura para objetos/portoes.
int BASE_UpdateItem2(int maskidx, int CurrentState, int NextState, int xx, int yy, char *pHeight, int rotate, int height)
{
	// Regra do cliente (client/src/common/BaseWorld.cpp:735).
	if (maskidx >= MAX_GROUNDMASK || maskidx < 0)
		return 0;

	for (int y = 0; y <= 5; ++y)
	{
		for (int x = 0; x <= 5; ++x)
		{
			int xp = xx + x - 2;
			int yp = yy + y - 2;
			if (xp - g_HeightPosX < 1 || yp - g_HeightPosY < 1 || xp - g_HeightPosX > g_HeightWidth - 2 || yp - g_HeightPosY > g_HeightHeight - 2)
				break;

			if (g_pGroundMask[maskidx][rotate][y][x])
				pHeight[xp + g_HeightWidth * (yp - g_HeightPosY) - g_HeightPosX] = height;
		}
	}

	return 1;
}

/// @brief Valor de refinação como o cliente o lê (BASE_GetItemSanc): o de EF_SANC; sem ele, o da
/// tintura (efeitos 115..126). Em `dye`, o efeito da tintura (0 se não há).
static int VisualSancValue(STRUCT_ITEM *Item, int *dye)
{
	*dye = 0;
	for (int i = 0; i < 3; i++)
	{
		if (Item->stEffect[i].cEffect >= 115 && Item->stEffect[i].cEffect <= 126)
		{
			*dye = Item->stEffect[i].cEffect;
			break;
		}
	}
	for (int i = 0; i < 3; i++)
		if (Item->stEffect[i].cEffect == EF_SANC)
			return Item->stEffect[i].cValue;
	for (int i = 0; i < 3; i++)
		if (Item->stEffect[i].cEffect == *dye && *dye)
			return Item->stEffect[i].cValue;
	return 0;
}

/// @brief Nível de refinação (0..16) de um valor de EF_SANC, como no cliente.
static int VisualSancLevel(int value)
{
	if (value < 230)
		return value % 10;
	if (value < 254)
		return 10 + (value - 230) / 4;
	return 16;
}

/// @brief Codifica índice visual e refinação de equipamento/montaria para envio aos clientes: o
/// nível de refinação vai nos 4 bits altos, que o cliente lê com `Equip[i] >> 12`.
int  BASE_VisualItemCode(STRUCT_ITEM *Item, int mnt)
{
	if (mnt == 14)
	{// montaria
		if (Item->sIndex >= 3980 && Item->sIndex < 3995)
			return Item->sIndex;

		// O cliente, para a própria montaria, usa EF_MOUNTSANC / 10 (stEffect[1].cEffect / 10) e,
		// para a dos outros, estes 4 bits. Antes a condição era `>= 2360 && < 2360` e nunca valia.
		int value = 0;
		if (Item->sIndex >= 2360 && Item->sIndex < 2390)
		{
			if (Item->stEffect[0].sValue <= 0)
				return 0;
			value = Item->stEffect[1].cEffect / 10;
		}

		return Item->sIndex | (value * 0x1000);
	}

	int dye = 0;
	int value = VisualSancValue(Item, &dye);
	int found = dye != 0 || Item->stEffect[0].cEffect == EF_SANC || Item->stEffect[1].cEffect == EF_SANC
		|| Item->stEffect[2].cEffect == EF_SANC;

	if (!found)
		return Item->sIndex;

	return Item->sIndex | (VisualSancLevel(value) * 0x1000);
}

/// @brief Código de cor (`Equip2`) de um item para os outros clientes, como TMHuman::SetColorItem lê:
/// com refinação até +9, o byte inteiro é o código de cor (43, ou o efeito da tintura); acima de +9,
/// o nibble baixo é a tintura (efeito - 115) e o alto o tipo da refinação celestial (1..4).
/// Montarias 2360..2390 levam o traje.
int BASE_VisualAnctCode(STRUCT_ITEM *Item)
{
	if (Item->sIndex >= 2360 && Item->sIndex <= 2390 && Item->stEffect[0].sValue > 0)
	{
		switch (Item->stEffect[2].cValue)
		{
		case 11:
			return 0x10B;
		case 12:
			return 0x10C;
		case 13:
			return 0x10D;
		case 14:
			return 0x10E;
		case 15:
			return 0x10F;
		case 16:
			return 0x110;
		case 17:
			return 0x111;
		case 18:
			return 0x112;
		case 19:
			return 0x113;
		case 20:
			return 0x114;

		case 35:
			if (Item->sIndex == 2363 || Item->sIndex == 2377)
				return 0x115;
			break;

		default:
			return 0x00;
		}

		return 0;
	}

	int dye = 0;
	int value = VisualSancValue(Item, &dye);

	if (dye == 0 && value == 0)
		return 0;

	int level = VisualSancLevel(value);

	// Tipo da refinação celestial (valor % 4): 0 -> 3, 1 -> 4, 2 -> 1, 3 -> 2.
	static const int kType[4] = { 3, 4, 1, 2 };

	if (dye)
	{
		if (level <= 9)
			return dye;
		return (kType[value % 4] << 4) | (dye - 115);
	}

	if (value < 230)
		return 43;

	return kType[value % 4] << 4;
}

/// Grava data de expiracao (em dias a partir de hoje) nos efeitos do item.
void BASE_SetItemDate(STRUCT_ITEM *Item, int day)
{
	struct tm when;
	time_t now;
	time(&now);
	when = *localtime(&now);

	int daynext = when.tm_mday + day;
	int month = day >= 30 ? when.tm_mon + 1 : when.tm_mon;
	int year = month >= 12 ? when.tm_year+1 : when.tm_year;


	if(month >= 12)
		month = 0;

	if(daynext >= 30)
		daynext -= 29;

	if(month == 1 && daynext >= 27)
		daynext -= 26;

	Item->stEffect[0].cEffect = EF_WDAY;
	Item->stEffect[0].cValue = daynext;

	Item->stEffect[1].cEffect = EF_WMONTH;
	Item->stEffect[1].cValue = month + 1;

	Item->stEffect[2].cEffect = EF_YEAR;
	Item->stEffect[2].cValue = year - 100;
}

/// Verifica se um item temporario ja expirou.
int  BASE_CheckItemDate(STRUCT_ITEM *Item)
{
	struct tm when;
	time_t now;
	time(&now);
	when = *localtime(&now);

	if(when.tm_mday >= Item->stEffect[0].cValue && when.tm_mon >= (Item->stEffect[1].cValue-1) && (when.tm_year-100) >= Item->stEffect[2].cValue)
		return 1;

	else if(when.tm_mon > Item->stEffect[1].cValue && (when.tm_year-100) >= Item->stEffect[2].cValue)
		return 1;

	else if((when.tm_year-100) > Item->stEffect[2].cValue)
		return 1;

	return 0;
}

/// Valida e limpa validade expirada de fadas.
void  BASE_CheckFairyDate(STRUCT_ITEM *Item)
{
	if(Item->sIndex < 3900 || Item->sIndex > 3913)
		return;

	int day = Item->stEffect[0].cValue;
	int hour = Item->stEffect[1].cValue;
	int min = Item->stEffect[2].cValue;

	if(day == 0 && hour == 0 && min <= 1)
	{
		BASE_ClearItem(Item);
		return;
	}
	else if(day >= 1 && hour == 0 && min == 1)
	{
		day--;
		hour = 23;
		min = 59;
	}

	else if(hour >= 1 && min == 1)
	{
		hour--;
		min = 59;
	}
	else if(min >= 2)
		min--;

	Item->stEffect[0].cValue = day;
	Item->stEffect[1].cValue = hour;
	Item->stEffect[2].cValue = min;
}
