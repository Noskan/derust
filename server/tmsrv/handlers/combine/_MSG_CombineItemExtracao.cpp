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
#include "net/ProcessClientMessage.h"

void Exec_MSG_CombineItemExtracao(int conn, char *pMsg)
{
	// O cliente envia MSG_Sell com a posição do item em MyPos (short, offset 16). Antes o servidor lia
	// MSG_STANDARDPARM2.Parm2, um int no mesmo offset, que só dava certo porque os 2 bytes de cima são
	// o preenchimento da struct, zerado pelo cliente.
	MSG_Sell *m = (MSG_Sell*)pMsg;

	int ItemSlot = m->MyPos;

	if (!BASE_CarryUsable(pMob[conn].MOB.Carry, ItemSlot))
		return;

	int item = pMob[conn].MOB.Carry[ItemSlot].sIndex;

	if(item <= 0 || item >= MAX_ITEMLIST)
		return;

	int itemlevel = BASE_GetItemAbility(&pMob[conn].MOB.Carry[ItemSlot], EF_ITEMLEVEL);

	if(itemlevel >= 5)
		return;

	if(BASE_GetItemSanc(&pMob[conn].MOB.Carry[ItemSlot]) < 9)
		return;

	if((g_pItemList[item].nPos != 2 && g_pItemList[item].nPos != 4 && g_pItemList[item].nPos != 8 && g_pItemList[item].nPos != 16 && g_pItemList[item].nPos != 32) || BASE_GetItemAbility(&pMob[conn].MOB.Carry[ItemSlot], EF_MOBTYPE) != 0)
		return;

	int i = 0;

	for (i = 0; i < MAX_CARRY - 4; i = BASE_NextCarry(pMob[conn].MOB.Carry, i))
	{
		if (pMob[conn].MOB.Carry[i].sIndex != 1774)
			continue;

		BASE_ClearItem(&pMob[conn].MOB.Carry[i]);
		SendItem(conn, ITEM_PLACE_CARRY, i, &pMob[conn].MOB.Carry[i]);
		break;
	}

	if (i == MAX_CARRY - 4)
		return;
		
	int _rnd = W2Rand()%115;

	if(_rnd > 100)
		_rnd -= 15;

	int rate = (pMob[conn].MOB.CurrentScore.Special[2] + 1) / 6;

	if(_rnd < rate)
	{
		pMob[conn].MOB.Carry[ItemSlot].stEffect[0].cEffect = EF_ITEMLEVEL;
		pMob[conn].MOB.Carry[ItemSlot].stEffect[0].cValue = itemlevel;

		if(pMob[conn].MOB.Carry[ItemSlot].stEffect[1].cEffect == EF_DAMAGE)
			pMob[conn].MOB.Carry[ItemSlot].stEffect[1].cValue += BASE_GetStaticItemAbility(&pMob[conn].MOB.Carry[ItemSlot], EF_DAMAGE);

		if(pMob[conn].MOB.Carry[ItemSlot].stEffect[2].cEffect == EF_DAMAGE)
			pMob[conn].MOB.Carry[ItemSlot].stEffect[2].cValue += BASE_GetStaticItemAbility(&pMob[conn].MOB.Carry[ItemSlot], EF_DAMAGE);

		pMob[conn].MOB.Carry[ItemSlot].sIndex = 3021;

		if(g_pItemList[item].nPos == 4)
			pMob[conn].MOB.Carry[ItemSlot].sIndex++;

		else if(g_pItemList[item].nPos == 8)
			pMob[conn].MOB.Carry[ItemSlot].sIndex += 2;

		else if(g_pItemList[item].nPos == 16)
			pMob[conn].MOB.Carry[ItemSlot].sIndex += 3;

		else if(g_pItemList[item].nPos == 32)
			pMob[conn].MOB.Carry[ItemSlot].sIndex += 4;

		SendItem(conn, ITEM_PLACE_CARRY, ItemSlot, &pMob[conn].MOB.Carry[ItemSlot]);

		char tt[256];
		sprintf(temp, "%s ", pUser[conn].AccountName);
		BASE_GetItemCode(&pMob[conn].MOB.Carry[ItemSlot], tt);
		strcat(temp, tt);

		ItemLog(temp, "*** Combine ht_skill_extração sucess ***", pUser[conn].IP);
	}
	else
	{
		BASE_ClearItem(&pMob[conn].MOB.Carry[ItemSlot]);
		SendItem(conn, ITEM_PLACE_CARRY, ItemSlot, &pMob[conn].MOB.Carry[ItemSlot]);

		ItemLog("*** Combine ht_skill_extração fail ***", pMob[conn].MOB.MobName, pUser[conn].IP);
	}
}
