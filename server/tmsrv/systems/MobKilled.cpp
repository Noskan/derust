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
#include "systems/MobKilledInternal.h"

/// @brief Orquestra a morte de um mob ou jogador (Nyerds, crias, EXP PvE, drops e PvP).
void MobKilled(int target, int conn, int PosX, int PosY)
{
	if (conn <= 0 || conn >= MAX_MOB || target <= 0 || target >= MAX_MOB || pMob[target].Mode == USER_EMPTY)
		return;

	STRUCT_ITEM *FairySlot = &pMob[target].MOB.Equip[13];

	MSG_CreateMob sCreateMob;

	if (pMob[target].MOB.CurrentScore.Level >= 1000)
	{
		pMob[target].MOB.CurrentScore.Hp = pMob[target].MOB.CurrentScore.MaxHp;

		if (target < MAX_USER)
			SetReqHp(target);

		GetCreateMob(target, &sCreateMob);
		GridMulticast(pMob[target].TargetX, pMob[target].TargetY, (MSG_STANDARD*)&sCreateMob, 0);

		SendScore(conn);

		return;
	}

	if (pMob[target].MOB.Equip[13].sIndex == 769 && RvRState == 0 && GTorreState == 0 && CastleState == 0) // Nyerds
	{
		int sanc = BASE_GetItemSanc(FairySlot);

		if (sanc > 0)
		{
			sanc--;

			if (FairySlot->stEffect[0].cEffect == 43)
				FairySlot->stEffect[0].cValue = sanc;

			else if (FairySlot->stEffect[1].cEffect == 43)
				FairySlot->stEffect[1].cValue = sanc;

			else if (FairySlot->stEffect[2].cEffect == 43)
				FairySlot->stEffect[2].cValue = sanc;
		}
		else
			BASE_ClearItem(FairySlot);

		if (target > 0 && target <= MAX_USER)
		{
			SendItem(target, ITEM_PLACE_EQUIP, 13, &pMob[target].MOB.Equip[13]);
			SendEmotion(target, 14, 2);
		}

		SendEquip(target, 0);

		pMob[target].MOB.CurrentScore.Hp = pMob[target].MOB.CurrentScore.MaxHp;
		
		if (target < MAX_USER)
			SetReqHp(target);

		GetCreateMob(target, &sCreateMob);
		GridMulticast(pMob[target].TargetX, pMob[target].TargetY, (MSG_STANDARD*)&sCreateMob, 0);

		SendScore(conn);

		return;
	}
	if (CastleState > 1 && pMob[target].TargetX == 1046 && pMob[target].TargetY == 1690 && target > 0 && target < MAX_USER)
	{
		DoTeleport(target, 1066, 1717);
		return;
	}

	int Face = pMob[conn].MOB.Equip[0].sIndex;

#pragma region Crias EXP
	if (conn >= MAX_USER && pMob[conn].MOB.Clan == 4 && Face >= 315 && Face <= 345 && target > MAX_USER && pMob[target].MOB.Clan != 4)
	{
		int summoner = pMob[conn].Summoner;

		if (summoner > 0 && summoner < MAX_USER && pUser[summoner].Mode && pMob[summoner].Mode)
		{
			STRUCT_ITEM *Mount = &pMob[summoner].MOB.Equip[14];

			if (pMob[summoner].MOB.Equip[14].sIndex >= 2330 && pMob[summoner].MOB.Equip[14].sIndex < 2390)
			{
				unsigned char XP = Mount->stEffect[1].cEffect;
				unsigned char Crescimento = Mount->stEffect[2].cValue;

				unsigned char exp = XP + 100;

				if (Mount->sIndex == 2330)
					exp = XP + 25;

				else if (Mount->sIndex == 2331)
					exp = XP + 35;

				else if (Mount->sIndex == 2332)
					exp = XP + 45;

				else if (Mount->sIndex == 2333)
					exp = XP + 55;

				else if (Mount->sIndex == 2334)
					exp = XP + 65;

				else if (Mount->sIndex == 2335)
					exp = XP + 75;

				if (XP < pMob[target].MOB.CurrentScore.Level && XP < 100)
				{
					Crescimento = Crescimento + 1;

					if (Crescimento < exp)
					{
						Mount->stEffect[2].cValue = Crescimento;
						SendItem(summoner, ITEM_PLACE_EQUIP, 14, &pMob[summoner].MOB.Equip[14]);
					}
					else
					{
						Crescimento = 1;
						XP = XP + 1;
						Mount->stEffect[2].cValue = 1;
						Mount->stEffect[1].cEffect = XP;
						SendClientMessage(summoner, g_pMessageStringTable[_NN_Mount_Level]);
						SendItem(summoner, ITEM_PLACE_EQUIP, 14, &pMob[summoner].MOB.Equip[14]);
						MountProcess(summoner, 0);
					}
				}
			}
		}
	}

#pragma endregion

	MSG_CNFMobKill sm;
	memset(&sm, 0, sizeof(MSG_CNFMobKill));

	sm.Type = _MSG_CNFMobKill;
	sm.Size = sizeof(MSG_CNFMobKill);
	sm.ID = ESCENE_FIELD;

	sm.KilledMob = target;
	sm.Killer = conn;

	pMob[target].MOB.CurrentScore.Hp = 0;

	int Leader = pMob[conn].Leader;

	if (Leader == 0)
		Leader = conn;

	if (conn >= MAX_USER && pMob[conn].MOB.Clan == 4)
	{
		int Summoner = pMob[conn].Summoner;

		if (Summoner <= 0 || Summoner >= MAX_USER || pUser[Summoner].Mode != USER_PLAY)
		{
			GridMulticast(pMob[target].TargetX, pMob[target].TargetY, (MSG_STANDARD*)&sm, 0);

			if (target > MAX_USER)
				DeleteMob(target, 1);

			return;
		}

		conn = Summoner;
	}
// region PvE
	if (target >= MAX_USER || pMob[target].MOB.BaseScore.Level > MAX_LEVEL + 5)
	{
		if (target >= MAX_USER || pMob[target].MOB.BaseScore.Level <= MAX_LEVEL)
		{
			if (conn < MAX_USER && pMob[target].MOB.Clan != 4)
			{
				if (ProcessMobKilledExp(target, conn, Leader))
				{
					int GenerateIndex = pMob[target].GenerateIndex;
					int DieSay = W2Rand() % 4;

					if (GenerateIndex >= 0 && mNPCGen.pList[GenerateIndex].DieAction[DieSay][0] && pMob[target].Leader == 0)
						SendChat(target, mNPCGen.pList[GenerateIndex].DieAction[DieSay]);

					GridMulticast(pMob[target].TargetX, pMob[target].TargetY, (MSG_STANDARD*)&sm, 0);

					if (pMob[conn].MOB.Clan && pMob[conn].MOB.Clan != 4 && pMob[conn].MOB.Clan != 7 && pMob[conn].MOB.Clan != 8)
						DeleteMob(target, 1);

					else
						ProcessMobKilledDrops(target, conn, PosX, PosY, GenerateIndex);
				}

				else
				{
					GridMulticast(pMob[target].TargetX, pMob[target].TargetY, (MSG_STANDARD*)&sm, 0);
					DeleteMob(target, 1);
				}
			}

			else
			{
				GridMulticast(pMob[target].TargetX, pMob[target].TargetY, (MSG_STANDARD*)&sm, 0);
				DeleteMob(target, 1);
			}
		}
		else
		{
			pUser[target].ReqHp = pMob[target].MOB.CurrentScore.MaxHp;
			SetReqHp(target);

			pMob[target].MOB.CurrentScore.Hp = pMob[target].MOB.CurrentScore.MaxHp;
			SendScore(target);
		}
	}
// endregion

// region PvP
	else
		ProcessMobKilledPvP(target, conn, sm);
// endregion
}
