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
#include "handlers/items/UseItemInternal.h"

/// @brief Processa o pacote _MSG_UseItem aplicando validações de estado/segurança
///        e despachando o uso do item para os sub-handlers por categoria de EF_VOLATILE.
void Exec_MSG_UseItem(int conn, char *pMsg)
{
	MSG_UseItem *m = (MSG_UseItem*)pMsg;

	if (pUser[conn].Mode != USER_PLAY)
	{
		SendHpMode(conn);
		return;
	}

	if (pUser[conn].Trade.OpponentID)
	{
		RemoveTrade(pUser[conn].Trade.OpponentID);
		RemoveTrade(conn);
		return;
	}

	if (pUser[conn].TradeMode)
	{
		RemoveTrade(conn);
		return;
	}

	if (pMob[conn].MOB.CurrentScore.Hp == 0)
	{
		SendHpMode(conn);
		AddCrackError(conn, 1, 15);

		if (m->SourType == ITEM_PLACE_CARRY)
			SendItem(conn, m->SourType, m->SourPos, &pMob[conn].MOB.Carry[m->SourPos]);

		else if (m->SourType == ITEM_PLACE_CARGO)
			SendItem(conn, m->SourType, m->SourPos, &pUser[conn].Cargo[m->SourPos]);

		else
			SendItem(conn, m->SourType, m->SourPos, &pMob[conn].MOB.Equip[m->SourPos]);

		return;
	}

	if (m->GridX >= MAX_GRIDX || m->GridY >= MAX_GRIDY)
	{
		CrackLog(conn, " Drop grid, out of range");
		return;
	}
	STRUCT_ITEM *item = GetItemPointer(&pMob[conn].MOB, pUser[conn].Cargo, m->SourType, m->SourPos);

	if (item == NULL)
	{
		sprintf(temp, "err,use item fail source %d %d", m->SourType, m->SourPos);
		ItemLog(temp, "-system", 0);
		return;
	}				 
	int Vol = BASE_GetItemAbility(item, EF_VOLATILE);
	int amount = BASE_GetItemAmount(item);


	// Avalia sequencialmente os grupos de voláteis preservando a ordem histórica de prioridade.
	if (UseItem_PotionsAndRefine(conn, m, item, Vol, amount))
		return;
	if (UseItem_MountsAndScrolls(conn, m, item, Vol, amount))
		return;
	if (UseItem_QuestsAndDungeons(conn, m, item, Vol, amount))
		return;
	if (UseItem_GemsAndMithril(conn, m, item, Vol, amount))
		return;
	if (UseItem_ExtractionsAndSpecials(conn, m, item, Vol, amount))
		return;
}
