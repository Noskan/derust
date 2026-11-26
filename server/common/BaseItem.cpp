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

	int price = g_pItemList[idx].Price;

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
int BASE_GetItemAbilityNosanc(STRUCT_ITEM *item, unsigned char Type)
{
	int value = 0;
	int idx = item->sIndex;

	if(idx <= 0 || idx >= MAX_ITEMLIST)
		return value;

	int nUnique = g_pItemList[idx].nUnique;
	int nPos = g_pItemList[idx].nPos;

	if((Type == EF_DAMAGEADD || Type == EF_MAGICADD) && (nUnique < 41 || nUnique > 50))
		return FALSE;
	if(Type == EF_CRITICAL && (item->stEffect[1].cEffect == EF_CRITICAL2 || item->stEffect[2].cEffect == EF_CRITICAL2))
		Type = EF_CRITICAL2;
	
	if(Type == EF_DAMAGE && nPos == 32 && (item->stEffect[1].cEffect == EF_DAMAGE2 || item->stEffect[2].cEffect == EF_DAMAGE2))
		Type = EF_DAMAGE2;

	if (Type == EF_MPADD && (item->stEffect[1].cEffect == EF_MPADD2 || item->stEffect[2].cEffect == EF_MPADD2))
		Type = EF_MPADD2;
	
	if (Type == EF_HPADD && (item->stEffect[1].cEffect == EF_HPADD2 || item->stEffect[2].cEffect == EF_HPADD2))
		Type = EF_HPADD2;
	
	if(Type == EF_LEVEL && idx >= 2330 && idx < 2360)
		value = item->stEffect[1].cEffect - 1;

	else if(Type == EF_LEVEL)
		value += g_pItemList[idx].ReqLvl;

	if(Type == EF_REQ_STR)
		value += g_pItemList[idx].ReqStr;

	if(Type == EF_REQ_INT)
		value += g_pItemList[idx].ReqInt;

	if(Type == EF_REQ_DEX)
		value += g_pItemList[idx].ReqDex;

	if(Type == EF_REQ_CON)
		value += g_pItemList[idx].ReqCon;

	if(Type == EF_POS)
		value += g_pItemList[idx].nPos;

	if(Type != EF_INCUBATE)
	{
		for(int i = 0; i < MAX_STATICEFFECT; i++)
		{
			if(g_pItemList[idx].stEffect[i].sEffect != Type)
                continue;

			int tvalue = g_pItemList[idx].stEffect[i].sValue;

            if(Type == EF_ATTSPEED && tvalue == 1)
                tvalue = 10;

            value += tvalue;
		}
	}
	
	if(idx >= 2330 && idx < 2390)
	{
		if(Type == EF_MOUNTHP)
			return item->stEffect[0].sValue;

		else if(Type == EF_MOUNTSANC)
			return item->stEffect[1].cEffect;

		else if(Type == EF_MOUNTLIFE)
			return item->stEffect[1].cValue;

		else if(Type == EF_MOUNTFEED)
			return item->stEffect[2].cEffect;

		else if(Type == EF_MOUNTKILL)
			return item->stEffect[2].cValue;

		if(idx < 2362 || idx >= 2390 || item->stEffect[0].sValue <= 0)
			return value;

		int lv = item->stEffect[1].cEffect;
		int cd = item->sIndex - 2360;

		if(Type == EF_DAMAGE)
			return (lv + 20) * g_pMountBonus[cd][0] / 100; //Retorna o DN da montaria no level atual

		else if(Type == EF_MAGIC)
			return (lv + 15) * g_pMountBonus[cd][1] / 100;

		else if(Type == EF_PARRY)
			return g_pMountBonus[cd][2];

		else if(Type == EF_RESIST1 || Type == EF_RESIST2 || Type == EF_RESIST3 || Type == EF_RESIST4)
			return g_pMountBonus[cd][3];
		else
			return value;
	}

	if(idx >= 3980 && idx <= 3994)
	{
		int cd = idx - 3980;

		if (Type == EF_DAMAGE)
			return g_pMountTempBonus[cd][0]; //Retorna o DN da montaria

		else if (Type == EF_MAGIC)
			return g_pMountTempBonus[cd][1];

		else if (Type == EF_PARRY)
			return g_pMountTempBonus[cd][2];

		else if (Type == EF_RESIST1 || Type == EF_RESIST2 || Type == EF_RESIST3 || Type == EF_RESIST4)
			return g_pMountTempBonus[cd][3];
		else
			return value;
	}

	for(int i = 0; i < 3; i++)
	{
		if(item->stEffect[i].cEffect != Type)
            continue;

		int tvalue = item->stEffect[i].cValue;

		if(Type == EF_ATTSPEED && tvalue == 1)
            tvalue = 10;

		value += tvalue;
	}

	if(Type == EF_RESIST1 || Type == EF_RESIST2 || Type == EF_RESIST3 || Type == EF_RESIST4)
	{
		for(int i = 0; i < MAX_STATICEFFECT; i++)
		{
			if(g_pItemList[idx].stEffect[i].sEffect == EF_RESISTALL)
				value += g_pItemList[idx].stEffect[i].sValue;
		}

		for(int i = 0; i < 3; i++)
		{
			if(item->stEffect[i].cEffect == EF_RESISTALL)
				value += item->stEffect[i].cValue;
		}
	}

	return value;
}

/// Calcula o valor efetivo de um atributo de item (EF_*) aplicando bonus de refinacao.
int BASE_GetItemAbility(STRUCT_ITEM *item, unsigned char Type)
{
	int value = 0;

	int idx = item->sIndex;

	if(idx < 0 || idx >= MAX_ITEMLIST)
		return value;

	int nUnique = g_pItemList[idx].nUnique;
	int nPos	= g_pItemList[idx].nPos;

	if(Type == EF_DAMAGEADD || Type == EF_MAGICADD)
	{
		if(nUnique < 41 || nUnique > 50)
			return value;
	}

	if(Type == EF_CRITICAL)
	{
		if(item->stEffect[1].cEffect == EF_CRITICAL2 || item->stEffect[2].cEffect == EF_CRITICAL2)
			Type = EF_CRITICAL2;
	}

	if(Type == EF_DAMAGE && nPos == 32)
	{
		 if(item->stEffect[1].cEffect == EF_DAMAGE2 || item->stEffect[2].cEffect == EF_DAMAGE2)
            Type = EF_DAMAGE2;
	}

	if(Type == EF_ACADD)
	{
        if(item->stEffect[1].cEffect == EF_ACADD2 || item->stEffect[2].cEffect == EF_ACADD2)
            Type = EF_ACADD2;
	}

	if(Type == EF_LEVEL && idx >= 2330 && idx < 2360)
		value = (item->stEffect[1].cEffect - 1);

    else if(Type == EF_LEVEL)
        value += g_pItemList[idx].ReqLvl;

	if(Type == EF_REQ_STR)
		value += g_pItemList[idx].ReqStr;

    if(Type == EF_REQ_INT)
        value += g_pItemList[idx].ReqInt;

    if(Type == EF_REQ_DEX)
        value += g_pItemList[idx].ReqDex;

    if(Type == EF_REQ_CON)
        value += g_pItemList[idx].ReqCon;

	if(Type == EF_POS)
		value += g_pItemList[idx].nPos;

	if(Type != EF_INCUBATE)
	{
		for(int i = 0; i < MAX_STATICEFFECT; i++)
		{
			if(g_pItemList[idx].stEffect[i].sEffect != Type)
                continue;

			int tvalue = g_pItemList[idx].stEffect[i].sValue;

            if(Type == EF_ATTSPEED && tvalue == 1)
                tvalue = 10;

            value += tvalue;
		}
	}

	if(idx >= 2330 && idx < 2390)
	{
		if(Type == EF_MOUNTHP)
			return item->stEffect[0].sValue;

		else if(Type == EF_MOUNTSANC)
            return item->stEffect[1].cEffect;

		else if(Type == EF_MOUNTLIFE)
			return item->stEffect[1].cValue;

        else if(Type == EF_MOUNTFEED)
            return item->stEffect[2].cEffect;

		else if(Type == EF_MOUNTKILL)
            return item->stEffect[2].cValue;

		if(idx < 2362 || idx >= 2390 || item->stEffect[0].sValue <= 0)
			return value;

		int lv = item->stEffect[1].cEffect;
		int cd = item->sIndex - 2360;

		if(Type == EF_DAMAGE)
			return (lv + 20) * g_pMountBonus[cd][0] / 100; //Retorna o DN da montaria no level atual

		else if(Type == EF_MAGIC)
			return (lv + 15) * g_pMountBonus[cd][1] / 100;

		else if(Type == EF_PARRY)
			return g_pMountBonus[cd][2];

		else if(Type == EF_RESIST1 || Type == EF_RESIST2 || Type == EF_RESIST3 || Type == EF_RESIST4)
				return g_pMountBonus[cd][3];
		else
			return value;
	}

	if (idx >= 3980 && idx <= 3994)
	{
		int cd = idx - 3980;

		if (Type == EF_DAMAGE)
			return g_pMountTempBonus[cd][0]; //Retorna o DN da montaria

		else if (Type == EF_MAGIC)
			return g_pMountTempBonus[cd][1];

		else if (Type == EF_PARRY)
			return g_pMountTempBonus[cd][2];

		else if (Type == EF_RESIST1 || Type == EF_RESIST2 || Type == EF_RESIST3 || Type == EF_RESIST4)
			return g_pMountTempBonus[cd][3];
		else
			return value;
	}

	for(int i = 0; i < 3; i++)
	{
		if(item->stEffect[i].cEffect != Type)
            continue;

		int tvalue = item->stEffect[i].cValue;

		if(Type == EF_ATTSPEED && tvalue == 1)
            tvalue = 10;

		value += tvalue;
	}

	if(Type == EF_RESIST1 || Type == EF_RESIST2 || Type == EF_RESIST3 || Type == EF_RESIST4)
    {
        for(int i = 0; i < MAX_STATICEFFECT; i++)
        {
			if(g_pItemList[idx].stEffect[i].sEffect != EF_RESISTALL)
                continue;

            value += g_pItemList[idx].stEffect[i].sValue;
        }

        for(int i = 0; i < 3; i++)
        {
			if(item->stEffect[i].cEffect != EF_RESISTALL)
                continue;

			value += item->stEffect[i].cValue;
        }
    }

	int sanc = BASE_GetItemSanc(item);
	if(sanc >= 9 && (nPos & 0xF00) != 0)
		sanc++;

	if (Type != EF_GRID && Type != EF_CLASS && Type != EF_POS && Type != EF_WTYPE && Type != EF_RANGE && Type != EF_LEVEL && Type != EF_REQ_STR && Type != EF_REQ_INT && Type != EF_REQ_DEX && Type != EF_REQ_CON && Type != EF_VOLATILE && Type != EF_INCUBATE && Type != EF_INCUDELAY && Type != EF_MOBTYPE && Type != EF_ITEMTYPE && Type != EF_ITEMLEVEL && Type != EF_NOTRADE && Type != EF_NOSANC && Type != EF_DONATE)
	{
		value *= sanc + 10;
		value /= 10;
	}

	if(Type == EF_RUNSPEED)
	{
		if(value >= 3)
			value = 2;

		if(value > 0 && sanc == 9)
			value++;
	}
	/* 
	if(Type == EF_HWORDGUILD || Type == EF_LWORDGUILD)
	{
		unsigned char v = value;

		value = v;
	}
	*/
	if(Type == EF_GRID)
	{
		if(value < 0 || value > 7)
			value = 0;
	}

	return value;
}

/// Consulta um atributo estatico definido apenas na tabela base ItemList.
int BASE_GetStaticItemAbility(STRUCT_ITEM *item, unsigned char Type)
{
	int value = 0;
	int idx = item->sIndex;

	if(idx <= 0 || idx >= MAX_ITEMLIST)
		return value;

	int nPos = g_pItemList[idx].nPos;

	if(Type == EF_LEVEL && idx >= 2330 && idx < 2360)
		value = item->stEffect[1].cEffect - 1;
	else if(Type == EF_LEVEL)
		value += g_pItemList[idx].ReqLvl;

	if(Type == EF_REQ_STR)
		value += g_pItemList[idx].ReqStr;

	if(Type == EF_REQ_INT)
		value += g_pItemList[idx].ReqInt;

	if(Type == EF_REQ_DEX)
		value += g_pItemList[idx].ReqDex;

	if(Type == EF_REQ_CON)
		value += g_pItemList[idx].ReqCon;

	if(Type == EF_POS)
		value += g_pItemList[idx].nPos;

	if(Type != EF_INCUBATE)
	{
		for(int i = 0; i < MAX_STATICEFFECT; i++)
		{
			if(g_pItemList[idx].stEffect[i].sEffect != Type)
                continue;

			int tvalue = g_pItemList[idx].stEffect[i].sValue;

            if(Type == EF_ATTSPEED && tvalue == 1)
                tvalue = 10;

            value += tvalue;
		}
	}

	if(Type == EF_RESIST1 || Type == EF_RESIST2 || Type == EF_RESIST3 || Type == EF_RESIST4)
	{
		for(int i = 0; i < MAX_STATICEFFECT; i++)
		{
			if(g_pItemList[idx].stEffect[i].sEffect == EF_RESISTALL)
				value += g_pItemList[idx].stEffect[i].sValue;
		}

		for(int i = 0; i < 3; i++)
		{
			if(item->stEffect[i].cEffect == EF_RESISTALL)
				value += item->stEffect[i].cValue;
		}
	}

	if(idx >= 2330 && idx < 2390)
	{
		if(Type == EF_MOUNTHP)
			return item->stEffect[0].sValue;

		else if(Type == EF_MOUNTSANC)
			return item->stEffect[1].cEffect;

		else if(Type == EF_MOUNTLIFE)
			return item->stEffect[1].cValue;

		else if(Type == EF_MOUNTFEED)
			return item->stEffect[2].cEffect;

		else if(Type == EF_MOUNTKILL)
			return item->stEffect[2].cValue;

		if(idx < 2362 || idx >= 2390 || item->stEffect[0].sValue <= 0)
			return value;

		int lv = item->stEffect[1].cEffect;
		int cd = item->sIndex - 2360;

		if(Type == EF_DAMAGE)
			return (lv + 20) * g_pMountBonus[cd][0] / 100; //Retorna o DN da montaria no level atual

		else if(Type == EF_MAGIC)
			return (lv + 15) * g_pMountBonus[cd][1] / 100;

		else if(Type == EF_PARRY)
			return g_pMountBonus[cd][2];

		else if(Type == EF_RESIST1 || Type == EF_RESIST2 || Type == EF_RESIST3 || Type == EF_RESIST4)
			return g_pMountBonus[cd][3];
		else
			return value;
	}

	if (idx >= 3980 && idx <= 3994)
	{
		int cd = idx - 3980;

		if (Type == EF_DAMAGE)
			return g_pMountTempBonus[cd][0]; //Retorna o DN da montaria

		else if (Type == EF_MAGIC)
			return g_pMountTempBonus[cd][1];

		else if (Type == EF_PARRY)
			return g_pMountTempBonus[cd][2];

		else if (Type == EF_RESIST1 || Type == EF_RESIST2 || Type == EF_RESIST3 || Type == EF_RESIST4)
			return g_pMountTempBonus[cd][3];
		else
			return value;
	}

	int sanc = BASE_GetItemSanc(item);

	if(sanc >= 9 && (nPos & 0xF00) != 0)
		sanc ++;

	if(Type != EF_GRID && Type != EF_CLASS && Type != EF_POS && Type != EF_WTYPE && Type != EF_RANGE && Type != EF_LEVEL && 
		Type != EF_REQ_STR && Type != EF_REQ_INT && Type != EF_REQ_DEX && Type != EF_REQ_CON && Type != EF_INCUBATE && Type != EF_INCUDELAY)
	{
		value *= sanc + 10;
		value /= 10;
	}

	if(Type == EF_RUNSPEED)
	{
		if(value >= 3)
			value = 2;

		if(value > 0 && sanc == 9)
			value++;
	}

	if(Type == EF_HWORDGUILD || Type == EF_LWORDGUILD)
	{
		unsigned char v = value;

		value = v;
	}

	return value;
}

/// Retorna o adicional extra de um atributo considerando a refinacao do item.
int BASE_GetBonusItemAbility(STRUCT_ITEM *item, unsigned char Type)
{
	if(item->sIndex >= 2330 && item->sIndex < 2390)
		return FALSE;

	int value = 0;

	int idx = item->sIndex;

	if(idx <= 0 || idx >= MAX_ITEMLIST)
		return value;

	int nPos = g_pItemList[idx].nPos;

	for(int i = 0; i < 3; i++)
	{
		if(item->stEffect[i].cEffect != Type)
			continue;

		int tvalue = item->stEffect[i].cValue;

		if(Type == EF_ATTSPEED && tvalue == 1)
			tvalue = 10;

		value += tvalue;
	}

	if(Type == EF_RESIST1 || Type == EF_RESIST2 || Type == EF_RESIST3 || Type == EF_RESIST4)
	{
		for(int i = 0; i < MAX_STATICEFFECT; i++)
		{
			if(g_pItemList[idx].stEffect[i].sEffect == EF_RESISTALL)
				value += g_pItemList[idx].stEffect[i].sValue;
		}

		for(int i = 0; i < 3; i++)
		{
			if(item->stEffect[i].cEffect == EF_RESISTALL)
				value += item->stEffect[i].cValue;
		}
	}

	int sanc = BASE_GetItemSanc(item);

	if(sanc >= 9 && (nPos & 0xF00) != 0)
		sanc ++;

	if(sanc == 0)
		return value;

	if(Type != EF_GRID && Type != EF_CLASS && Type != EF_POS && Type != EF_WTYPE && Type != EF_RANGE && Type != EF_LEVEL && Type != EF_REQ_STR && Type != EF_REQ_INT && Type != EF_REQ_DEX && Type != EF_REQ_CON && Type != EF_VOLATILE && Type != EF_INCUBATE && Type != EF_INCUDELAY)
	{
		value *= sanc + 10;
		value /= 10;
	}

	return value;
}

/// Retorna o adicional extra bruto de um atributo sem aplicar refinacao.
int BASE_GetBonusItemAbilityNosanc(STRUCT_ITEM *item, unsigned char Type)
{
	if(item->sIndex >= 2330 && item->sIndex < 2390)
		return FALSE;

	int value = 0;

	int idx = item->sIndex;

	if(idx <= 0 || idx >= MAX_ITEMLIST)
		return value;

	for(int i = 0; i < 3; i++)
	{
		if(item->stEffect[i].cEffect != Type)
			continue;

		int tvalue = item->stEffect[i].cValue;

		if(Type == EF_ATTSPEED && tvalue == 1)
			tvalue = 10;

		value += tvalue;
	}

	if(Type == EF_RESIST1 || Type == EF_RESIST2 || Type == EF_RESIST3 || Type == EF_RESIST4)
	{
		for(int i = 0; i < MAX_STATICEFFECT; i++)
		{
			if(g_pItemList[idx].stEffect[i].sEffect == EF_RESISTALL)
				value += g_pItemList[idx].stEffect[i].sValue;
		}

		for(int i = 0; i < 3; i++)
		{
			if(item->stEffect[i].cEffect == EF_RESISTALL)
				value += item->stEffect[i].cValue;
		}
	}

	return value;
}

/// Extrai o nivel de refinacao (+0..+15) de um item considerando codificacao de tintura.
int BASE_GetItemSanc(STRUCT_ITEM *item)
{
	if(item->sIndex >= 2330 && item->sIndex < 2390)
		return FALSE;

	unsigned char sanc = 0;

	if (item->stEffect[0].cEffect >= 116 && item->stEffect[0].cEffect <= 125)
		sanc = item->stEffect[0].cValue;

	else if (item->stEffect[1].cEffect >= 116 && item->stEffect[1].cEffect <= 125)
		sanc = item->stEffect[1].cValue;
	
	else if (item->stEffect[2].cEffect >= 116 && item->stEffect[2].cEffect <= 125)
		sanc = item->stEffect[2].cValue;

	else if(item->stEffect[0].cEffect == EF_SANC || item->stEffect[1].cEffect == EF_SANC || item->stEffect[2].cEffect == EF_SANC)
	{
		if(item->stEffect[0].cEffect == EF_SANC)
			sanc = item->stEffect[0].cValue;
		else if(item->stEffect[1].cEffect == EF_SANC)
			sanc = item->stEffect[1].cValue;
		else
			sanc = item->stEffect[2].cValue;
	}

	int isanc = sanc;

	if (isanc == 9)
		isanc = 9;
	else if (isanc >= 230 && isanc <= 233)
		isanc = REF_10;
	else if (isanc >= 234 && isanc <= 237)
		isanc = REF_11;
	else if (isanc >= 238 && isanc <= 241)
		isanc = REF_12;
	else if (isanc >= 242 && isanc <= 245)
		isanc = REF_13;
	else if (isanc >= 246 && isanc <= 249)
		isanc = REF_14;
	else if (isanc >= 250 && isanc <= 253)
		isanc = REF_15;
	else
		isanc = isanc % 10;

	return isanc;
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
int BASE_GetItemSancSuccess(STRUCT_ITEM *item)
{
	 if(item->sIndex >= 2330 && item->sIndex < 2390)
		 return FALSE;

	 unsigned char sanc = 0;

	 if (item->stEffect[0].cEffect == EF_SANC || item->stEffect[1].cEffect == EF_SANC || item->stEffect[2].cEffect == EF_SANC || item->stEffect[0].cEffect >= 116 && item->stEffect[0].cEffect <= 125 || item->stEffect[1].cEffect >= 116 && item->stEffect[1].cEffect <= 125 || item->stEffect[2].cEffect >= 116 && item->stEffect[2].cEffect <= 125)
	 {
		 if (item->stEffect[0].cEffect == EF_SANC || item->stEffect[0].cEffect >= 116 && item->stEffect[0].cEffect <= 125)
			 sanc = item->stEffect[0].cValue;
		 
		 else if (item->stEffect[1].cEffect == EF_SANC || item->stEffect[1].cEffect >= 116 && item->stEffect[1].cEffect <= 125)
			 sanc = item->stEffect[1].cValue;
		 
		 else
			 sanc = item->stEffect[2].cValue;		 
	 }

	 int isanc = sanc;
	 isanc /= 10;

	 return isanc;
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
	if(item->stEffect[0].cEffect == EF_AMOUNT || item->stEffect[1].cEffect == EF_AMOUNT || item->stEffect[2].cEffect == EF_AMOUNT)
	{
		if(item->stEffect[0].cEffect == EF_AMOUNT)
			item->stEffect[0].cValue = amount;
		
		else if(item->stEffect[1].cEffect == EF_AMOUNT)
			item->stEffect[1].cValue = amount;
		
		else
			item->stEffect[2].cValue = amount;
	}
	else if(item->stEffect[0].cEffect == 0 || item->stEffect[1].cEffect == 0 || item->stEffect[2].cEffect == 0)
	{
		
		if(item->stEffect[0].cEffect == 0)
		{
			item->stEffect[0].cEffect = EF_AMOUNT;
			item->stEffect[0].cValue = amount;
		}
		else if(item->stEffect[1].cEffect == 0 )
		{

			item->stEffect[1].cEffect = EF_AMOUNT;
			item->stEffect[1].cValue = amount;

		}
		else
		{
			item->stEffect[2].cEffect = EF_AMOUNT;
			item->stEffect[2].cValue = amount;	
		}
	}
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
int BASE_CanEquip(STRUCT_ITEM *item, STRUCT_SCORE *score, int Pos, int Class, STRUCT_ITEM *pBaseEquip, STRUCT_MOBEXTRA *extra)
{
	if(item->sIndex <= 0 || item->sIndex >= MAX_ITEMLIST)
		return FALSE;

	int nUnique = g_pItemList[item->sIndex].nUnique;

	if(Pos == 15)
		return FALSE;

	if(Pos != -1)
	{
		int tpos = BASE_GetItemAbility(item, EF_POS);
		int pos = (tpos >> Pos) & 1;

		if(pos == 0)
			return FALSE;

		if(Pos == 6 || Pos == 7)
		{
			int OtherPos = (Pos == 6) ? 7 : 6;
			int	OtherIdx = pBaseEquip[OtherPos].sIndex;

			if(OtherIdx > 0 && OtherIdx < MAX_ITEMLIST)
			{
				int nUnique2 = g_pItemList[OtherIdx].nUnique;
				int otherpos = BASE_GetItemAbility(&pBaseEquip[OtherPos], EF_POS);

				if(tpos == 64 || otherpos == 64)
				{
					if(nUnique == 46)
					{
						if(otherpos != 128)
							return FALSE;
					}
					else if(nUnique2 == 46)
					{
						if(tpos != 128)
							return FALSE;
					}
					else
						return FALSE;
				}
			}
		}
	}

	int cls = BASE_GetItemAbility(item, EF_CLASS);

	if (extra->ClassMaster != MORTAL)
		Class = (extra->MortalFace/10);

	cls = (cls >> Class) & 1;

	if (cls == 0 && extra->ClassMaster == MORTAL)
		return FALSE;

	if (cls == 0 && extra->ClassMaster != MORTAL && Pos != 6 && Pos != 7)
		return FALSE;

	if (BASE_GetItemAbility(item, EF_MOBTYPE) == ARCH && extra->ClassMaster == MORTAL)
		return FALSE;

	if (BASE_GetItemAbility(item, EF_MOBTYPE) == MORTAL && extra->ClassMaster != MORTAL)
		return FALSE;

	if (BASE_GetItemAbility(item, EF_MOBTYPE) == MORTAL && extra->ClassMaster != MORTAL)
		return FALSE;

	if (BASE_GetItemAbility(item, EF_MOBTYPE) == CELESTIAL && (extra->ClassMaster == MORTAL || extra->ClassMaster == ARCH))
		return FALSE;

	if(extra->ClassMaster != MORTAL && Pos == 1 && item->sIndex != 747 && (item->sIndex < 3500 || item->sIndex > 3507))
		return FALSE;

	int lvl = extra->ClassMaster == MORTAL ? BASE_GetItemAbility(item, EF_LEVEL) : 0;
	int str = extra->ClassMaster == MORTAL ? BASE_GetItemAbility(item, EF_REQ_STR) : 0;
	int spt = extra->ClassMaster == MORTAL ? BASE_GetItemAbility(item, EF_REQ_INT) : 0;
	int agi = extra->ClassMaster == MORTAL ? BASE_GetItemAbility(item, EF_REQ_DEX) : 0; 
	int con = extra->ClassMaster == MORTAL ? BASE_GetItemAbility(item, EF_REQ_CON) : 0;
    int weapontype = BASE_GetItemAbility(item, EF_WTYPE);

	weapontype %= 10;
	int modweapon = weapontype;
	int divweapon = weapontype / 10;

	if(Pos == 7 && weapontype != 0)
	{
		int rate = 100;

		if(divweapon == 0 && modweapon > 1)
			rate = 130;
		else if(divweapon == 6 && modweapon > 1)
			rate = 150;

		lvl = lvl * rate / 100;
		str = str * rate / 100;
		spt = spt * rate / 100;
		agi = agi * rate / 100;
		con = con * rate / 100;
	}

	if(lvl > score->Level)
		return FALSE;

	if(str > score->Str)
		return FALSE;

	if(spt > score->Int)
		return FALSE;

	if(agi > score->Dex)
		return FALSE;

	if(con > score->Con)
		return FALSE;

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
	int grid = ItemGridIndex(item);

	char SourGrid[8];

	memcpy(SourGrid, g_pItemGrid[grid], sizeof(SourGrid));

	char CargoGrid[MAX_CARGO];

	memset(CargoGrid, 0, MAX_CARGO-2);

	for(int i = 0; i < MAX_CARGO; i++)
	{
		if(cargo[i].sIndex == 0)
			continue;

		int grid = ItemGridIndex(&cargo[i]);

		char TempGrid[8];

		memcpy(TempGrid, g_pItemGrid[grid], sizeof(TempGrid));

		int tx = i % 9;
		int ty = i / 9;

		for(int yy = 0; yy < 4; yy++)
		{
			for(int xx = 0; xx < 2; xx++)
			{
				if(TempGrid[(yy * 2) + xx] == 0)
					continue;

				if(yy + ty < 0 || xx + tx < 0 || yy + ty >= CARGOGRIDY || xx + tx >= CARGOGRIDX)
					continue;

				CargoGrid[(yy + ty) * CARGOGRIDX + xx + tx] = 1;
			}
		}
	}

	for(int yy = 0; yy < 4; yy++)
	{
		for(int xx = 0; xx < 2; xx++)
		{
			if(SourGrid[(yy * 2) + xx] == 0)
				continue;

			if(yy + DestY < 0 || xx + DestX < 0 || yy + DestY >= CARGOGRIDY || xx + DestX >= CARGOGRIDX)
				return FALSE;

			if(CargoGrid[(yy + DestY) * CARGOGRIDX + xx + DestX] == 1)
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
		*(int*)((int)&ItemTemp[i] + 4) = *(int*)((int)&Item[MaxBufferIndex] + 4);
	}

	memcpy(Item, ItemTemp, sizeof(ItemTemp));
}

/// Valida se a troca entre dois inventarios possui espaco suficiente para ambos.
int BASE_CanTrade(STRUCT_ITEM *Dest, STRUCT_ITEM *Carry, unsigned char *MyTrade, STRUCT_ITEM *OpponentTrade, int MaxCarry)
{
	STRUCT_ITEM OpponentTemp[MAX_TRADE];

	memcpy(OpponentTemp, OpponentTrade, sizeof(OpponentTemp));

	memcpy(Dest, Carry, sizeof(STRUCT_ITEM)*MAX_CARRY);

	for (int i = 0; i < MAX_TRADE; i++)
	{
		int pos = MyTrade[i];

		if (pos == -1 || pos == 255)
			continue;

		if (pos >= 0 && pos < MAX_CARRY)
			BASE_ClearItem(&Dest[pos]);
	}

	BASE_SortTradeItem(OpponentTemp, 33);

	for (int i = 0; i < MAX_TRADE; i++)
	{
		if (!OpponentTemp[i].sIndex)
			continue;

		int j = 0;

		for (j = 0; j < MaxCarry; j++)
		{
			int x = j % 9;
			int y = j / 9;
			int error = -2;

			if (Dest[j].sIndex != 0)
				continue;

			memcpy(&Dest[j], &OpponentTemp[i], sizeof(STRUCT_ITEM));

			break;
		}

		if (j == MaxCarry)
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

	for(int y = 0; y <= MAX_GROUNDMASK-1; y++)
	{
		for(int x = 0; x <= MAX_GROUNDMASK-1; x++)
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
	int delta = 0;

	if(maskidx >= MAX_GROUNDMASK || maskidx < 0)
		return FALSE;

	for(int y = 0; y <= MAX_GROUNDMASK-1; y++)
	{
		for(int x = 0; x <= MAX_GROUNDMASK-1; x++)
		{
			int val = g_pGroundMask[maskidx][rotate][y][x];
			int xp = xx + x - 2;
			int yp = yy + y - 2;

			if((xp - g_HeightPosX < 1) || (yp - g_HeightPosY < 1) || ((xp - g_HeightPosX) > (g_HeightWidth - 2)) || ((yp - g_HeightPosY) > (g_HeightHeight - 2)))
				break;

			if(val != 0)
				pHeight[(((yp - g_HeightPosY) * g_HeightWidth) + xp) - g_HeightPosX] = height;
		}
	}

	return TRUE;
}

/// Codifica indice visual e refinacao de equipamento/montaria para envio aos clientes.
int  BASE_VisualItemCode(STRUCT_ITEM *Item, int mnt)
{
	int value = 0;

	if (mnt == 14)
	{// montaria
		if (Item->sIndex >= 3980 && Item->sIndex < 3995)
			return Item->sIndex;

		if (Item->sIndex >= 2360 && Item->sIndex < 2360)
		{
			if (Item->stEffect[0].sValue > 0)
			{
				value = Item->stEffect[1].cEffect/10;
			}
			else
				return 0;
		}

		return Item->sIndex | (value * 0x1000);
	}
	else
	{
		if (Item->stEffect[0].cEffect == 43)
			value = Item->stEffect[0].cValue;
		else if (Item->stEffect[1].cEffect == 43)
			value = Item->stEffect[1].cValue;
		else if (Item->stEffect[2].cEffect == 43)
			value = Item->stEffect[2].cValue;

		else if(Item->stEffect[0].cEffect >= 116 && Item->stEffect[0].cEffect <= 125)
				return Item->sIndex | (12 * 0x1000);

		else if(Item->stEffect[1].cEffect >= 116 && Item->stEffect[1].cEffect <= 125)
				return Item->sIndex | (12 * 0x1000);

		else if(Item->stEffect[2].cEffect >= 116 && Item->stEffect[2].cEffect <= 125)
				return Item->sIndex | (12 * 0x1000);

		else
			return Item->sIndex;
	}

	if (value < 230)
		value %= 10;

	else if (value < 234)
		value = 10;
	else if (value < 238)
		value = 11;
	else if (value < 242)
		value = 12;
	else if (value < 246)
		value = 13;
	else if (value < 250)
		value = 14;
	else if (value < 254)
		value = 15;
	else// value < 256
		value = 16;

	return Item->sIndex | (value * 0x1000);
}

/// Codifica o efeito visual de item ancient/tingido para exibicao.
int BASE_VisualAnctCode(STRUCT_ITEM *Item)
{
	int value = 0;

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
	if (Item->stEffect[0].cEffect == 43)
		value = Item->stEffect[0].cValue;
	else if (Item->stEffect[1].cEffect == 43)
		value = Item->stEffect[1].cValue;
	else if (Item->stEffect[2].cEffect == 43)
		value = Item->stEffect[2].cValue;


	if (Item->stEffect[0].cEffect >= 116 && Item->stEffect[0].cEffect <= 125)
		return value = Item->stEffect[0].cEffect-3;
	if (Item->stEffect[1].cEffect >= 116 && Item->stEffect[1].cEffect <= 125)
		return value = Item->stEffect[1].cEffect-3;
	if (Item->stEffect[2].cEffect >= 116 && Item->stEffect[2].cEffect <= 125)
		return value = Item->stEffect[2].cEffect-3;

	if (value == 0)
		return 0;

	if (value < 230)
		return 43;

	switch (value % 4)
	{
	case 0:  return 0x30;
	case 1:  return 0x40;
	case 2:  return 0x10;
	default: return 0x20;
	}
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
