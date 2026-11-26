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

/// @file MobKilled_PvP.cpp
/// @brief Regras de morte em PvP: perda de EXP, PK/CP, drops de PK e fadas protetoras.

#include "systems/MobKilledInternal.h"

void ProcessMobKilledPvP(int target, int conn, MSG_CNFMobKill& sm)
{
	MSG_CreateMob sCreateMob;
		int tlevel = pMob[target].MOB.BaseScore.Level;

		if (tlevel < 0 || tlevel > MAX_LEVEL)
			return;

#pragma region Lose EXP
		unsigned int curexp = (unsigned int)(pMob[target].extra.ClassMaster == MORTAL || pMob[target].extra.ClassMaster == ARCH ? g_pNextLevel[tlevel] : g_pNextLevel_2[tlevel]);
		unsigned int nextexp = (unsigned int)(pMob[target].extra.ClassMaster == MORTAL || pMob[target].extra.ClassMaster == ARCH ? g_pNextLevel[tlevel + 1] : g_pNextLevel_2[tlevel + 1]);
		unsigned int alphaexp = (nextexp - curexp);
		unsigned int deltaexp = (nextexp - curexp) / 20;

		if (tlevel >= 30)
			deltaexp = alphaexp / 22;

		if (tlevel >= 40)
			deltaexp = alphaexp / 25;

		if (tlevel >= 50)
			deltaexp = alphaexp / 30;

		if (tlevel >= 60)
			deltaexp = alphaexp / 35;

		if (tlevel >= 70)
			deltaexp = alphaexp / 40;

		if (tlevel >= 80)
			deltaexp = alphaexp / 45;

		if (tlevel >= 90)
			deltaexp = alphaexp / 50;

		if (tlevel >= 100)
			deltaexp = alphaexp / 55;

		if (tlevel >= 150)
			deltaexp = alphaexp / 70;

		if (tlevel >= 200)
			deltaexp = alphaexp / 85;

		if (tlevel >= 250)
			deltaexp = alphaexp / 100;

		if (deltaexp < 0)
			deltaexp = 0;
		if (deltaexp > 150000)
			deltaexp = 150000;
#pragma endregion

		int killer_curkill = GetCurKill(conn);
		int killer_totkill = GetTotKill(conn);
		int killer_pkpoint = GetPKPoint(conn);
		int killed_pkpoint = GetPKPoint(target);
		int killed_guilty = GetGuilty(target);
		int killed_curkill = GetCurKill(target);
		int killed_clan = pMob[target].MOB.Clan;
		int killer_clan = pMob[conn].MOB.Clan;
		int SameClan = 0;
		int AtWar = 0;

		if (killed_pkpoint > 10 && killed_pkpoint <= 25)
			deltaexp *= 3;
		else
			deltaexp *= 5;

		int killed_x = pMob[target].TargetX / 128;
		int killed_y = pMob[target].TargetY / 128;

#pragma region No lose At war
		if (conn < MAX_USER)
		{
			if (TESTSERVER)
				deltaexp /= 3;

			else
				deltaexp /= 6;

			if (killed_clan == 7 && killer_clan == 8 || killed_clan == 8 && killer_clan == 7)
				SameClan = 1;

			int killed_guild = pMob[target].MOB.Guild;
			int killer_guild = pMob[conn].MOB.Guild;
			int max_guild = 65536;

			if (killed_guild > 0 && killed_guild < max_guild && killer_guild > 0 && killer_guild < max_guild
				&& g_pGuildWar[killed_guild] == killer_guild && g_pGuildWar[killer_guild] == killed_guild)
				AtWar = 1;

			if (CastleState && killed_x == 8 && killed_y == 13)
				AtWar = 1;

			if (RvRState != 0)
				AtWar = 1;

			if (GTorreState != 0)
				AtWar = 1;
		}
#pragma endregion

		GridMulticast(pMob[target].TargetX, pMob[target].TargetY, (MSG_STANDARD*)&sm, 0);

		int arena = BASE_GetArena(pMob[target].TargetX, pMob[target].TargetY);
		int village = BASE_GetVillage(pMob[target].TargetX, pMob[target].TargetY);
		int ZoneUnk = 0;

		if (killed_x == 1 && killed_y == 31)
			ZoneUnk = 1;

		if (arena != 5 || village != 5 || ZoneUnk)
		{
			SendClientMessage(target, g_pMessageStringTable[_NN_In_Arena_No_Exp_Loss]);

			if (arena == 5 && village != 5)
			{
				FILE *fpp = NULL;

				fpp = fopen("TesteDie.txt", "a+");

				fprintf(fpp, "%s %s %d %d %d %d\n", pMob[conn].MOB.MobName, pMob[target].MOB.MobName, conn, target, pMob[target].TargetX, pMob[target].TargetY);

				fclose(fpp);
			}

			if (arena >= 0 && arena < MAX_GUILDZONE)
			{
				STRUCT_ITEM item;
				memset(&item, 0, sizeof(STRUCT_ITEM));

				item.sIndex = 431;
				SetItemBonus(&item, 0, 0, 0);

				CreateItem(pMob[target].TargetX, pMob[target].TargetY, &item, rand() % 4, 1);
			}
		}

		else
		{
			if (TESTSERVER)
				deltaexp /= 4;

			if (SameClan)
				deltaexp += killed_curkill * deltaexp / 4;

			if (deltaexp > 30000)
				deltaexp = 30000;

			if (deltaexp < 0)
				deltaexp = 0;

			
			if ((tlevel >= 35 || pMob[target].extra.ClassMaster != MORTAL) && NewbieEventServer == 0)
			{
				if (conn >= MAX_USER)
				{
					if (DEADPOINT)
						pMob[target].extra.Hold += (unsigned int)(pMob[target].MOB.Exp > deltaexp ? deltaexp : pMob[target].MOB.Exp);
				}
				else
					pMob[target].extra.Hold += (unsigned int)(pMob[target].MOB.Exp > deltaexp ? deltaexp : pMob[target].MOB.Exp);

				if (DEADPOINT || conn < MAX_USER)
				{
					sprintf(temp, g_pMessageStringTable[_DN_lose_D_exp], deltaexp);
					SendClientMessage(target, temp);
					SendEtc(target);
				}
			}
			else if (pMob[target].MOB.CurrentScore.Level < FREEEXP && pMob[target].extra.ClassMaster == MORTAL && NewbieEventServer == 0)
				SendClientMessage(target, g_pMessageStringTable[_NN_Below_lv20_No_Exp_Loss]);

			if (SameClan)
			{
				SendEtc(conn);

				++killer_curkill;
				SetCurKill(conn, killer_curkill);

				++killer_totkill;
				SetTotKill(conn, killer_totkill);

				if (!AtWar && !killed_guilty && NewbieEventServer == 0)
				{
					int Lostpk = 3 * killed_pkpoint / -20;
					if (Lostpk < -9)
						Lostpk = -9;

					if (Lostpk > 0)
						Lostpk = 0;

					if (killed_guilty > 0)
						Lostpk = 0;

					killer_pkpoint += Lostpk;
					SetPKPoint(conn, killer_pkpoint);
					sprintf(temp, g_pMessageStringTable[_DD_PKPointMinus], killer_pkpoint - 75, Lostpk);
					SendClientMessage(conn, temp);
				}

				if (pMob[target].TargetX >= 1017 && pMob[target].TargetX <= 1290 && pMob[target].TargetY >= 1911 && pMob[target].TargetY <= 2183)
				{
					if (pMob[target].MOB.Clan == 7)
					{
						pMob[target].MOB.CurrentScore.Hp = 2;
						DoTeleport(target, 1061 + rand() % 2, 2129 + rand() % 2);
						RvRRedPoint++;
					}

					else if (pMob[target].MOB.Clan == 8)
					{
						pMob[target].MOB.CurrentScore.Hp = 2;
						DoTeleport(target, 1237 + rand() % 2, 1966 + rand() % 2);
						RvRBluePoint++;
					}
				}
			}
#pragma region PK Drop - CP
			else
			{
				if (!AtWar && NewbieEventServer == 0)
				{
					int LostPk = 3 * killed_pkpoint / -25;

					if (LostPk < -9)
						LostPk = -9;

					if (LostPk > 0)
						LostPk = 0;

					if (killed_guilty > 0)
						LostPk = 0;

					if (pMob[target].MOB.Equip[15].sIndex == 548 || pMob[target].MOB.Equip[15].sIndex == 549 || pMob[conn].MOB.Equip[15].sIndex == 548 || pMob[conn].MOB.Equip[15].sIndex == 549)
						LostPk *= 3;

					killer_pkpoint += LostPk;

					SetPKPoint(conn, killer_pkpoint);

					if (LostPk)
					{
						sprintf(temp, g_pMessageStringTable[_DD_PKPointMinus], killer_pkpoint - 75, -LostPk);
						SendClientMessage(conn, temp);
					}
#ifdef PKDrop
					if (killed_pkpoint <= 60)
					{
						int killed_loseitem = (75 - killed_pkpoint) / 10;
						int killed_numitem = 0;

						for (int i = 0; i < MAX_CARRY - 1; i++)
						{
							if (rand() % 5)
								continue;

							STRUCT_ITEM *ItemLose = &pMob[target].MOB.Carry[i];

							if (pMob[target].MOB.Carry[i].sIndex <= 0)
								continue;

							if (ItemLose->sIndex > MAX_ITEMLIST || ItemLose->sIndex == 508 || ItemLose->sIndex == 509 || ItemLose->sIndex == 522 || ItemLose->sIndex >= 526 && ItemLose->sIndex <= 531)
								continue;

							if (ItemLose->sIndex == 446)
								continue;

							int create = CreateItem(pMob[target].TargetX, pMob[target].TargetY, ItemLose, rand() % 4);

							if (create >= MAX_ITEM || create <= 0)
								continue;

							if (BASE_NeedLog(ItemLose, 0))
							{
								char logitem[256];

								BASE_GetItemCode(ItemLose, logitem);

								sprintf(temp, "cho %s", logitem);

								Log(temp, pUser[target].AccountName, pUser[target].IP);
							}

							memset(ItemLose, 0, sizeof(STRUCT_ITEM));

							MSG_CNFDropItem cnfDrop;

							cnfDrop.Type = _MSG_CNFDropItem;
							cnfDrop.Size = sizeof(MSG_CNFDropItem);

							cnfDrop.SourType = ITEM_PLACE_CARRY;
							cnfDrop.SourPos = i;

							cnfDrop.GridX = pMob[target].TargetX;
							cnfDrop.GridY = pMob[target].TargetY;

							pUser[target].cSock.SendOneMessage((char*)&cnfDrop, sizeof(MSG_CNFDropItem));

							killed_numitem++;

							if (rand() % 2 && killed_numitem < killed_loseitem)
								continue;
						}
					}
					if (killed_pkpoint <= 35)
					{
						int killed_loseitem = (killed_pkpoint + 10) / 10;

						if (killed_loseitem <= 0)
							killed_loseitem = 1;


						int drop = rand() % killed_loseitem;

						while (!drop)
						{
							int slot = rand() % 14 + 1;
							if (slot != 12)
							{
								STRUCT_ITEM *ItemLose = &pMob[target].MOB.Equip[slot];
								if (pMob[target].MOB.Equip[slot].sIndex > 0 && ItemLose->sIndex <= MAX_ITEMLIST && ItemLose->sIndex != 508 && ItemLose->sIndex != 509 && ItemLose->sIndex != 522 && (ItemLose->sIndex < 526 || ItemLose->sIndex > 531))
								{
									if (ItemLose->sIndex != 446)
									{
										int create = CreateItem(pMob[target].TargetX, pMob[target].TargetY, ItemLose, rand() % 4);
										if (create > 0 && create < 5000)
										{
											if (BASE_NeedLog(ItemLose, 0))
											{
												char itemname[256];

												BASE_GetItemCode(ItemLose, itemname);
												sprintf(temp, "cho %s", itemname);

												Log(temp, pUser[target].AccountName, pUser[target].IP);
											}
											SendItem(target, ITEM_PLACE_EQUIP, slot, &pMob[target].MOB.Equip[slot]);
											memset(ItemLose, 0, sizeof(STRUCT_ITEM));

											MSG_CNFDropItem cnfDrop;

											cnfDrop.Type = _MSG_CNFDropItem;
											cnfDrop.Size = sizeof(MSG_CNFDropItem);

											cnfDrop.SourType = ITEM_PLACE_CARRY;
											cnfDrop.SourPos = slot;

											cnfDrop.GridX = pMob[target].TargetX;
											cnfDrop.GridY = pMob[target].TargetY;

											pUser[target].cSock.SendOneMessage((char*)&cnfDrop, sizeof(MSG_CNFDropItem));
										}
									}
								}
								break;
							}
						}
					}
#endif
				}
			}
#pragma endregion

			GetCreateMob(conn, &sCreateMob);
			GridMulticast(pMob[conn].TargetX, pMob[conn].TargetY, (MSG_STANDARD*)&sCreateMob, 0);

			if (SameClan)
				SetCurKill(target, 0);

			if (killed_pkpoint < 75 && !AtWar && conn < MAX_USER)
			{
				killed_pkpoint++;
				SetPKPoint(target, killed_pkpoint);
				sprintf(temp, g_pMessageStringTable[_DD_PKPointPlus], killed_pkpoint - 75, 1);
				SendClientMessage(target, temp);
			}
			GetCreateMob(target, &sCreateMob);
			GridMulticast(pMob[target].TargetX, pMob[target].TargetY, (MSG_STANDARD*)&sCreateMob, 0);

			sprintf(temp, "etc PK killer:%s(%d,%d) killed:%s(%d,%d) kingdom:%d war:%d",
				pMob[conn].MOB.MobName, pMob[conn].MOB.CurrentScore.Level, killer_pkpoint, pMob[target].MOB.MobName,
				pMob[target].MOB.CurrentScore.Level, killed_pkpoint, SameClan, AtWar);

			Log(temp, "-system", 0);
		}

		if (pMob[target].MOB.Equip[13].sIndex == 753 || pMob[target].MOB.Equip[13].sIndex == 1726)//10X HP MOB  - 20X HP MOB
		{
			int sanc = BASE_GetItemSanc(&pMob[target].MOB.Equip[13]);
			if (sanc > 0)
			{
				sanc--;

				if (pMob[target].MOB.Equip[13].stEffect[0].cEffect == 43)
					pMob[target].MOB.Equip[13].stEffect[0].cValue = sanc;

				else if (pMob[target].MOB.Equip[13].stEffect[1].cEffect == 43)
					pMob[target].MOB.Equip[13].stEffect[1].cValue = sanc;

				else if (pMob[target].MOB.Equip[13].stEffect[2].cEffect == 43)
					pMob[target].MOB.Equip[13].stEffect[2].cValue = sanc;
			}
			else
				BASE_ClearItem(&pMob[target].MOB.Equip[13]);

			SendItem(target, ITEM_PLACE_EQUIP, 13, &pMob[target].MOB.Equip[13]);
		}
}
