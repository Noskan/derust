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

#include <Windows.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <math.h>
#include <io.h>
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

/// @file MountSystem.cpp
/// @brief Processamento de montarias, vinculo de HP, alimentacao e crescimento.

/// Atualiza estado da montaria equipada (vitalidade/crescimento) e sincroniza atributos.
void MountProcess(int conn, STRUCT_ITEM *Mount)
{
	STRUCT_ITEM *MItem = &pMob[conn].MOB.Equip[14];

	int IsEqual = 1;

	if (Mount != NULL)
		IsEqual = memcmp(Mount, MItem, sizeof(STRUCT_ITEM));

	if (IsEqual != 0)
		return;

	int leaderid = pMob[conn].Leader;

	if (leaderid == 0)
		leaderid = conn;

	for (int i = 0; i < MAX_PARTY; i++)
	{
		int partyconn = pMob[leaderid].PartyList[i];

		if (partyconn <= 0 || partyconn > MAX_MOB)
			continue;

		int face = pMob[partyconn].MOB.Equip[0].sIndex;

		if (pMob[partyconn].Summoner == conn)
		{
			if (face >= 315 && face < 345)
				DeleteMob(partyconn, 3);
		}
	}

	int MBabyId = MItem->sIndex - 2320;


	if (MBabyId >= 10 && MBabyId < 40)
	{
		int MHp = BASE_GetItemAbility(MItem, EF_MOUNTHP);

		if (MHp > 0)
			GenerateSummon(conn, MBabyId, MItem, 1);
	}
}

/// Sincroniza o HP da montaria equipada com a entidade evocada no mundo.
void LinkMountHp(int mobConn)
{
	if (mobConn < MAX_USER || mobConn >= MAX_MOB)
		return;

	if (pMob[mobConn].MOB.Clan != 4)
		return;

	int face = pMob[mobConn].MOB.Equip[0].sIndex;

	if (face < 315 || face >= 345)
		return;

	int Summoner = pMob[mobConn].Summoner;

	if (Summoner <= 0 || Summoner >= MAX_USER)
		return;

	if (pMob[Summoner].Mode == USER_EMPTY)
		return;

	if (pUser[Summoner].Mode != USER_PLAY)
		return;

	int mountId = pMob[Summoner].MOB.Equip[14].sIndex - 2330;
	int mountFace = face - 315;

	if ((face - 315) != mountId)
		return;

	int mountHpItem = pMob[Summoner].MOB.Equip[14].stEffect[0].sValue;
	int mountHp = pMob[mobConn].MOB.CurrentScore.Hp;

	if (mountHpItem == mountHp)
		return;

	pMob[Summoner].MOB.Equip[14].stEffect[0].sValue = mountHp;
	SendItem(Summoner, ITEM_PLACE_EQUIP, 14, &pMob[Summoner].MOB.Equip[14]);
}

/// Processa perda de HP/vitalidade da montaria adulta e reverte/mata quando esgotada.
void ProcessAdultMount(int conn, int HpLost)
{
	STRUCT_ITEM *MItem = &pMob[conn].MOB.Equip[14];

	if (MItem->sIndex < 2360 || MItem->sIndex >= 2390)
		return;

	int midx = MItem->sIndex - 2360;
	int MMaxHp = mSummon.Mob[midx + 10].CurrentScore.MaxHp;
	int MFeed = MItem->stEffect[2].cEffect;

	if (MFeed <= 0 && MItem->stEffect[0].sValue > 0)
	{
		MItem->stEffect[0].sValue = 0;
		MFeed = 0;
	}

	int MHp = MItem->stEffect[0].sValue;
	int MHp2 = MItem->stEffect[0].sValue - HpLost;

	if (MHp2 >= MMaxHp)
		MHp2 = MMaxHp;

	int NMHp = MHp2;

	MItem->stEffect[0].sValue = NMHp;

	if (MHp2 <= 0)
		MItem->stEffect[2].cEffect = 0;

	if ((MHp <= 0 && MHp2 > 0) || (MHp2 <= 0 && MHp > 0))
		SendEquip(conn, 0);

	if (MHp != MHp2 && conn < MAX_USER)
		SendItem(conn, ITEM_PLACE_EQUIP, 14, &pMob[conn].MOB.Equip[14]);
}
