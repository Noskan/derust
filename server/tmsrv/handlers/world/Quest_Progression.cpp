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

/// @file Quest_Progression.cpp
/// @brief Handlers de quests de progressão, treinamento inicial e NPCs clássicos.

#include "handlers/world/QuestInternal.h"

bool HandleQuest_Progression(int conn, int npcIndex, int confirm, int npcMode, int npcMerc, int npcGrade)
{
	switch (npcMode)
	{
#pragma region MOUNT_MASTER
	case MOUNT_MASTER:
	{
						int mountIndex = pMob[conn].MOB.Equip[14].sIndex;

						if (mountIndex < 2330 || mountIndex >= 2390)
						{
							SendSay(npcIndex, g_pMessageStringTable[_NN_Cure_animals]);
							break;
						}

						if (pMob[conn].MOB.Equip[14].stEffect[0].sValue > 0)
						{
							SendSay(npcIndex, g_pMessageStringTable[_NN_Cure_animals]);
							break;
						}

						int price = g_pItemList[mountIndex].Price;

						if (confirm == 0)
						{
							sprintf(temp, g_pMessageStringTable[_DS_S_cure_price_D], g_pItemList[mountIndex].Name, price);
							SendSay(npcIndex, temp);
							break;
						}

						if (pMob[conn].MOB.Coin < price)
						{
							SendClientMessage(conn, g_pMessageStringTable[_NN_Not_Enough_Money]);
							break;
						}

						if (price < 0 || price > 2000000000)
							break;

						pMob[conn].MOB.Coin -= price;

						int vit = BASE_GetItemAbility(&pMob[conn].MOB.Equip[14], 79);

						vit -= rand() % 3;

						if (vit > 0)
						{
							SendSay(npcIndex, g_pMessageStringTable[_NN_Cured]);
							pMob[conn].MOB.Equip[14].stEffect[1].cValue = vit;
							pMob[conn].MOB.Equip[14].stEffect[0].sValue = 20;
							pMob[conn].MOB.Equip[14].stEffect[2].cEffect = 5;
						}
						else
						{
							SendSay(npcIndex, g_pMessageStringTable[_NN_Cure_failed]);
							memset(&pMob[conn].MOB.Equip[14], 0, sizeof(STRUCT_ITEM));
						}
						

						pMob[conn].GetCurrentScore(conn);
						SendScore(conn);

						SendItem(conn, ITEM_PLACE_EQUIP, 14, &pMob[conn].MOB.Equip[14]);

						MountProcess(conn, 0);

						SendEtc(conn);

						sprintf(temp, "etc,mount ressurect index:%d vit:%d - %d", pMob[conn].MOB.Equip[14].sIndex, vit, pMob[conn].MOB.Equip[14].stEffect[1].cEffect);
						Log(temp, "-system", 0);
	} break;
#pragma endregion
#pragma region ZAKUM
	case ZAKUM:
	{
					char UserNameArea[128];

					int count = GetUserInArea(2180, 1160, 2296, 1270, UserNameArea);

					if (count < 1)
						sprintf(temp, g_pMessageStringTable[_NN_Zakum_Quest_empty]);
					else
						sprintf(temp, g_pMessageStringTable[_SD_Zakum_Quest_by_S_N], UserNameArea, count - 1);

					SendSay(npcIndex, temp);
	} break;
#pragma endregion
#pragma region AMU_MISTICO
	case AMU_MISTICO:
	{
						if (pMob[conn].extra.QuestInfo.Mortal.TerraMistica != 0 || pMob[conn].extra.ClassMaster != MORTAL)
						{
							SendSay(npcIndex, g_pMessageStringTable[_NN_Hurry_Helping_them]);
							break;
						}

						if (confirm == 0)
						{
							SendSay(npcIndex, g_pMessageStringTable[_NN_Monster_Attaking_Us_Help]);
							break;
						}

						if (pMob[conn].Leader)
						{
							SendSay(npcIndex, g_pMessageStringTable[_NN_Party_Only]);
							break;
						}

						int partycont = 0;

						for (int i = 0; i < MAX_PARTY; i++)
						if (pMob[conn].PartyList[i] > 0 && pMob[conn].PartyList[i] < MAX_USER)
							partycont++;

						if (partycont == 0)
						{
							SendSay(npcIndex, g_pMessageStringTable[_NN_Party_Only]);
							break;
						}

						pMob[conn].extra.QuestInfo.Mortal.TerraMistica = 1;
						SendClientMessage(conn, g_pMessageStringTable[_NN_Get_Watching_Town_Mission]);

						sprintf(temp, "etc,mysticalland complete. name:%s", pMob[conn].MOB.MobName);
						Log(temp, "-system", 0);
	} break;
#pragma endregion
#pragma region QUEST_COVEIRO
	case QUEST_COVEIRO:
	{
							if (pMob[conn].extra.ClassMaster != MORTAL && pMob[conn].extra.ClassMaster != ARCH)
							{
								SendSay(npcIndex, g_pMessageStringTable[_NN_Level_Limit2]);
								break;
							}

							int minlevel = pMob[conn].extra.ClassMaster == MORTAL ? 39 : 39;
							int maxlevel = pMob[conn].extra.ClassMaster == MORTAL ? 115 : 115;

							if (pMob[conn].MOB.CurrentScore.Level < minlevel || pMob[conn].MOB.CurrentScore.Level >= maxlevel)
							{
								SendSay(npcIndex, g_pMessageStringTable[_NN_Level_limit]);
								break;
							}

							int i = 0;

							for (i = 0; i < pMob[conn].MaxCarry; i++)
							{
								if (pMob[conn].MOB.Carry[i].sIndex != 4038)
									continue;

								BASE_ClearItem(&pMob[conn].MOB.Carry[i]);
								SendItem(conn, ITEM_PLACE_CARRY, i, &pMob[conn].MOB.Carry[i]);
								break;
							}

							if (i == pMob[conn].MaxCarry)
							{
								sprintf(temp, g_pMessageStringTable[_SN_BRINGITEM], g_pItemList[4038].Name);
								SendSay(npcIndex, temp);
								break;
							}

							pMob[conn].QuestFlag = 1;

							DoTeleport(conn, 2398 + rand() % 5 - 3, 2105 + rand() % 5 - 3);

							sprintf(temp, "etc,quest256 ticket coveiro name:%s level:%d", pMob[conn].MOB.MobName, pMob[conn].MOB.BaseScore.Level);
							Log(temp, "-system", 0);
	} break;
#pragma endregion
#pragma region QUEST_JARDINEIRO
	case QUEST_JARDINEIRO:
	{
							if (pMob[conn].extra.ClassMaster != MORTAL && pMob[conn].extra.ClassMaster != ARCH)
							{
								SendSay(npcIndex, g_pMessageStringTable[_NN_Level_Limit2]);
								break;
							}

							int minlevel = pMob[conn].extra.ClassMaster == MORTAL ? 115 : 115;
							int maxlevel = pMob[conn].extra.ClassMaster == MORTAL ? 190 : 190;

							if (pMob[conn].MOB.CurrentScore.Level < minlevel || pMob[conn].MOB.CurrentScore.Level >= maxlevel)
							{
									SendSay(npcIndex, g_pMessageStringTable[_NN_Level_limit]);
									break;
							}

							int i = 0;

							for (i = 0; i < pMob[conn].MaxCarry; i++)
							{
								if (pMob[conn].MOB.Carry[i].sIndex != 4039)
									continue;

								BASE_ClearItem(&pMob[conn].MOB.Carry[i]);
								SendItem(conn, ITEM_PLACE_CARRY, i, &pMob[conn].MOB.Carry[i]);
								break;
							}

							if (i == pMob[conn].MaxCarry)
							{
								sprintf(temp, g_pMessageStringTable[_SN_BRINGITEM], g_pItemList[4039].Name);
								SendSay(npcIndex, temp);
								break;
							}
							pMob[conn].QuestFlag = 2;

							DoTeleport(conn, 2234 + rand() % 5 - 3, 1714 + rand() % 5 - 3);

							sprintf(temp, "etc,quest256 ticket jardineiro name:%s level:%d", pMob[conn].MOB.MobName, pMob[conn].MOB.BaseScore.Level);
							Log(temp, "-system", 0);
	} break;
#pragma endregion
#pragma region QUEST_KAIZEN
	case QUEST_KAIZEN:
	{
						if (pMob[conn].extra.ClassMaster != MORTAL && pMob[conn].extra.ClassMaster != ARCH)
						{
							SendSay(npcIndex, g_pMessageStringTable[_NN_Level_Limit2]);
							break;
						}

						int minlevel = pMob[conn].extra.ClassMaster == MORTAL ? 190 : 190;
						int maxlevel = pMob[conn].extra.ClassMaster == MORTAL ? 265 : 265;

						if (pMob[conn].MOB.CurrentScore.Level < minlevel || pMob[conn].MOB.CurrentScore.Level >= maxlevel)
						{
								SendSay(npcIndex, g_pMessageStringTable[_NN_Level_limit]);
								break;
						}

						int i = 0;

						for (i = 0; i < pMob[conn].MaxCarry; i++)
						{
							if (pMob[conn].MOB.Carry[i].sIndex != 4040)
								continue;

							BASE_ClearItem(&pMob[conn].MOB.Carry[i]);
							SendItem(conn, ITEM_PLACE_CARRY, i, &pMob[conn].MOB.Carry[i]);
							break;
						}

						if (i == pMob[conn].MaxCarry)
						{
							sprintf(temp, g_pMessageStringTable[_SN_BRINGITEM], g_pItemList[4040].Name);
							SendSay(npcIndex, temp);
							break;
						}
						pMob[conn].QuestFlag = 3;

						DoTeleport(conn, 464 + rand() % 5 - 3, 3902 + rand() % 5 - 3);

						sprintf(temp, "etc,quest256 ticket kaizen name:%s level:%d", pMob[conn].MOB.MobName, pMob[conn].MOB.BaseScore.Level);
						Log(temp, "-system", 0);
	} break;
#pragma endregion
#pragma region QUEST_HIDRA
	case QUEST_HIDRA:
	{
						if (pMob[conn].extra.ClassMaster != MORTAL && pMob[conn].extra.ClassMaster != ARCH)
						{
							SendSay(npcIndex, g_pMessageStringTable[_NN_Level_Limit2]);
							break;
						}

						int minlevel = pMob[conn].extra.ClassMaster == MORTAL ? 265 : 265;
						int maxlevel = pMob[conn].extra.ClassMaster == MORTAL ? 320 : 320;

						if (pMob[conn].MOB.CurrentScore.Level < minlevel || pMob[conn].MOB.CurrentScore.Level >= maxlevel)
						{
								SendSay(npcIndex, g_pMessageStringTable[_NN_Level_limit]);
								break;
						}

						int i = 0;

						for (i = 0; i < pMob[conn].MaxCarry; i++)
						{
							if (pMob[conn].MOB.Carry[i].sIndex != 4041)
								continue;

							BASE_ClearItem(&pMob[conn].MOB.Carry[i]);
							SendItem(conn, ITEM_PLACE_CARRY, i, &pMob[conn].MOB.Carry[i]);
							break;
						}

						if (i == pMob[conn].MaxCarry)
						{
							sprintf(temp, g_pMessageStringTable[_SN_BRINGITEM], g_pItemList[4041].Name);
							SendSay(npcIndex, temp);
							break;
						}
						pMob[conn].QuestFlag = 4;

						DoTeleport(conn, 668 + rand() % 5 - 3, 3756 + rand() % 5 - 3);

						sprintf(temp, "etc,quest256 ticket hidra name:%s level:%d", pMob[conn].MOB.MobName, pMob[conn].MOB.BaseScore.Level);
						Log(temp, "-system", 0);
	} break;
#pragma endregion
#pragma region QUEST_ELFOS
	case QUEST_ELFOS:
	{
						if (pMob[conn].extra.ClassMaster != MORTAL && pMob[conn].extra.ClassMaster != ARCH)
						{
							SendSay(npcIndex, g_pMessageStringTable[_NN_Level_Limit2]);
							break;
						}

						int minlevel = pMob[conn].extra.ClassMaster == MORTAL ? 320 : 320;
						int maxlevel = pMob[conn].extra.ClassMaster == MORTAL ? 350 : 350;

						if (pMob[conn].MOB.CurrentScore.Level < minlevel || pMob[conn].MOB.CurrentScore.Level >= maxlevel)
						{
								SendSay(npcIndex, g_pMessageStringTable[_NN_Level_limit]);
								break;
						}

						int i = 0;

						for (i = 0; i < pMob[conn].MaxCarry; i++)
						{
							if (pMob[conn].MOB.Carry[i].sIndex != 4042)
								continue;

							BASE_ClearItem(&pMob[conn].MOB.Carry[i]);
							SendItem(conn, ITEM_PLACE_CARRY, i, &pMob[conn].MOB.Carry[i]);
							break;
						}

						if (i == pMob[conn].MaxCarry)
						{
							sprintf(temp, g_pMessageStringTable[_SN_BRINGITEM], g_pItemList[4042].Name);
							SendSay(npcIndex, temp);
							break;
						}
						pMob[conn].QuestFlag = 5;

						DoTeleport(conn, 1322 + rand() % 5 - 3, 4041 + rand() % 5 - 3);

						sprintf(temp, "etc,quest256 ticket elfos name:%s level:%d", pMob[conn].MOB.MobName, pMob[conn].MOB.BaseScore.Level);
						Log(temp, "-system", 0);
	} break;
#pragma endregion
#pragma region JEFFI
	case JEFFI:
	{
					if (pMob[conn].MOB.Equip[13].sIndex == 447 || pMob[conn].MOB.Equip[13].sIndex == 692)
					{
						int price = 0;
						if (pMob[conn].MOB.Equip[13].sIndex == 447)
							price = 1000000;

						else
						{
							if (pMob[conn].MOB.Equip[13].sIndex != 692)
								break;

							price = 5000000;
						}

						if (pMob[conn].MOB.Coin >= price)
						{
							if (price == 1000000)
							{
								pMob[conn].MOB.Coin -= 1000000;
								pMob[conn].MOB.Equip[13].sIndex = rand() % 3 + 448;
							}
							else
							{
								pMob[conn].MOB.Coin -= 5000000;
								pMob[conn].MOB.Equip[13].sIndex = rand() % 3 + 693;
							}

							SendItem(conn, ITEM_PLACE_EQUIP, 13, &pMob[conn].MOB.Equip[13]);
							SendEtc(conn);
							SendSay(npcIndex, g_pMessageStringTable[_NN_Processing_Complete]);
							SetAffect(conn, 44, 20, 20);
							SendScore(conn);

							sprintf(temp, "etc,jeffi make reset50-100 name:%s level:%d", pMob[conn].MOB.MobName, pMob[conn].MOB.BaseScore.Level);
							Log(temp, "-system", 0);
						}
						else
						{
							if (price == 1000000)
								SendSay(npcIndex, g_pMessageStringTable[_NN_Need_1000000_Gold]);
							else
								SendSay(npcIndex, g_pMessageStringTable[_NN_Need_5000000_Gold]);
						}
					}
					else
					{
						int Ori = 0;
						int Lac = 0;

						for (int i = 0; i < pMob[conn].MaxCarry; i++)
						{
							if (pMob[conn].MOB.Carry[i].sIndex == 419)
								Ori += pMob[conn].MOB.Carry[i].stEffect[0].cEffect == 61 ? pMob[conn].MOB.Carry[i].stEffect[0].cValue : 1;
							
							if (pMob[conn].MOB.Carry[i].sIndex == 420)
								Lac += pMob[conn].MOB.Carry[i].stEffect[0].cEffect == 61 ? pMob[conn].MOB.Carry[i].stEffect[0].cValue : 1;
						}

						if (Ori >= 10 || Lac >= 10)
						{
							if (pMob[conn].MOB.Coin >= 1000000)
							{
								for (; Ori >= 10 || Lac >= 10;)
								{
									if (Ori >= 10)
									{
										Combine(conn, 419, 412);
										Ori -= 10;
									}
									if (Lac >= 10)
									{
										Combine(conn, 420, 413);
										Lac -= 10;
									}
								}
								SendSay(npcIndex, g_pMessageStringTable[_NN_Processing_Complete]);
								pMob[conn].MOB.Coin -= 1000000;
								SetAffect(conn, 44, 20, 20);
								SendScore(conn);
								SendCarry(conn);
								SendEtc(conn);

								sprintf(temp, "etc,jeffi make lac_ori name:%s level:%d", pMob[conn].MOB.MobName, pMob[conn].MOB.BaseScore.Level);
								Log(temp, "-system", 0);
							}
							else
								SendSay(npcIndex, g_pMessageStringTable[_NN_Need_1000000_Gold]);

						}
						else
							SendSay(npcIndex, g_pMessageStringTable[_NN_Need_10_Particle]);
					}

	} break;
#pragma endregion
#pragma region SHAMA
	case SHAMA:
	{
					int circle = pMob[conn].MOB.Equip[13].sIndex;

					int ncircle = circle;
					int circlemode = 0;

					if (circle != 448 && circle != 449 && circle != 450)
					{
						if (circle != 693 && circle != 694 && circle != 695)
						{
							SendSay(npcIndex, g_pMessageStringTable[_NN_Need_Pure_Divine]);
							break;
						}
						ncircle -= 693;
						circlemode = 2;
					}
					else
					{
						ncircle -= 448;
						circlemode = 1;
					}

					if (circlemode == 1 && pMob[conn].extra.QuestInfo.Circle == 1)
					{
						SendSay(npcIndex, g_pMessageStringTable[_NN_Youve_Done_It_Already]);
						break;
					}

					int specialreset = 50;

					if (circlemode == 2)
						specialreset = 100;

					int special = pMob[conn].MOB.SpecialBonus;

					for (int i = 0; i < 4; i++)
					{
						if (pMob[conn].MOB.BaseScore.Special[i] <= specialreset)
						{
							special += pMob[conn].MOB.BaseScore.Special[i];
							pMob[conn].MOB.BaseScore.Special[i] = 0;
						}
						else
						{
							special += specialreset;
							pMob[conn].MOB.BaseScore.Special[i] -= specialreset;
						}
					}

					pMob[conn].MOB.SpecialBonus = special;

					if (circlemode == 1)
						pMob[conn].extra.QuestInfo.Circle = 1;

					if (ncircle == 0)
						pMob[conn].MOB.LearnedSkill &= 0xFFFFFF00;

					if (ncircle == 1)
						pMob[conn].MOB.LearnedSkill &= 0xFFFF00FF;

					if (ncircle == 2)
						pMob[conn].MOB.LearnedSkill &= 0xFF00FFFF;

					BASE_GetBonusSkillPoint(&pMob[conn].MOB, &pMob[conn].extra);

					memset(&pMob[conn].MOB.Equip[13], 0, sizeof(STRUCT_ITEM));

					SendItem(conn, ITEM_PLACE_EQUIP, 13, &pMob[conn].MOB.Equip[13]);
					SendEtc(conn);
					SendSay(npcIndex, g_pMessageStringTable[_NN_Initialize_Skill]);
					SetAffect(conn, 44, 20, 20);
					SendScore(conn);

					SendClientMessage(conn, g_pMessageStringTable[_NN_Qest06Complete]);
					sprintf(temp, "etc,device circle quest item:%d mode:%d type:%d", circle, circlemode, ncircle);
					Log(temp, pUser[conn].AccountName, pUser[conn].IP);
					break;
	} break;
#pragma endregion
#pragma region TREINADORNEWBIE1
	case TREINADORNEWBIE1:
	{
		if(pMob[conn].extra.ClassMaster != MORTAL || pMob[conn].MOB.CurrentScore.Level >= FREEEXP || pMob[conn].extra.QuestInfo.Mortal.Newbie != 0)
		{
			SendSay(npcIndex, g_pMessageStringTable[_NN_NewbieQuest_Already1]);
			break;
		}
		int i = 0;

		for (i = 0; i < pMob[conn].MaxCarry; i++)
		{
			if(pMob[conn].MOB.Carry[i].sIndex == 451)
				break;
		}

		if (i == pMob[conn].MaxCarry)
		{
			SendSay(npcIndex, g_pMessageStringTable[_NN_NewbieQuest_Cheerup1]);
			break;
		}
		sprintf(temp, "etc,newbiequest1 complete name:%s conn:%d", pMob[conn].MOB.MobName, conn);
		Log(temp, "-system", 0);

		pMob[conn].extra.QuestInfo.Mortal.Newbie = 1;
		SendSay(npcIndex, g_pMessageStringTable[_NN_NewbieQuest_Complete1]);
		SendClientMessage(conn, g_pMessageStringTable[_NN_NewbieQuest_Reward1]);

		STRUCT_ITEM Item;
		memset(&Item, 0, sizeof(STRUCT_ITEM));

		//Pot com 20
		Item.sIndex = 682;
		Item.stEffect[0].cEffect = 61;
		Item.stEffect[0].cValue = 20;

		PutItem(conn, &Item);

		SetAffect(conn, 44, 200, 200);
		SendScore(conn);
		SendCarry(conn);
	} break;
#pragma endregion
#pragma region TREINADORNEWBIE2
	case TREINADORNEWBIE2:
	{
		if(pMob[conn].extra.ClassMaster != MORTAL || pMob[conn].MOB.CurrentScore.Level >= FREEEXP || pMob[conn].extra.QuestInfo.Mortal.Newbie != 1)
		{
			SendSay(npcIndex, g_pMessageStringTable[_NN_NewbieQuest_Already2]);
			break;
		}
		int i = 0;

		for (i = 0; i < pMob[conn].MaxCarry; i++)
		{
			if(pMob[conn].MOB.Carry[i].sIndex == 452)
				break;
		}

		if (i == pMob[conn].MaxCarry)
		{
			SendSay(npcIndex, g_pMessageStringTable[_NN_NewbieQuest_Cheerup2]);
			break;
		}
		sprintf(temp, "etc,newbiequest2 complete name:%s conn:%d", pMob[conn].MOB.MobName, conn);
		Log(temp, "-system", 0);

		pMob[conn].extra.QuestInfo.Mortal.Newbie++;
		SendSay(npcIndex, g_pMessageStringTable[_NN_NewbieQuest_Complete2]);
		SendClientMessage(conn, g_pMessageStringTable[_NN_NewbieQuest_Reward2]);

		int WeaponId = pMob[conn].MOB.Equip[6].sIndex;
		int cls = pMob[conn].MOB.Class;

		if(WeaponId > 0 && WeaponId < MAX_ITEMLIST)
		{
			int ReqLv = g_pItemList[WeaponId].ReqLvl;

			if(ReqLv > 39)
				break;

			pMob[conn].MOB.Equip[6].stEffect[0].cEffect = 0;
			pMob[conn].MOB.Equip[6].stEffect[0].cValue = 0;
			pMob[conn].MOB.Equip[6].stEffect[1].cEffect = 0;
			pMob[conn].MOB.Equip[6].stEffect[1].cValue = 0;
			pMob[conn].MOB.Equip[6].stEffect[2].cEffect = 0;
			pMob[conn].MOB.Equip[6].stEffect[2].cValue = 0;

			SetItemBonus(&pMob[conn].MOB.Equip[6], 50 + ReqLv, 1, 0);
			SendItem(conn, ITEM_PLACE_EQUIP, 6, &pMob[conn].MOB.Equip[6]);
		}

		SetAffect(conn, 44, 200, 200);
		SendScore(conn);
		SendCarry(conn);
	} break;
#pragma endregion
#pragma region TREINADORNEWBIE3
	case TREINADORNEWBIE3:
	{
		if(pMob[conn].extra.ClassMaster != MORTAL || pMob[conn].MOB.CurrentScore.Level >= FREEEXP || pMob[conn].extra.QuestInfo.Mortal.Newbie != 2)
		{
			SendSay(npcIndex, g_pMessageStringTable[_NN_NewbieQuest_Already3]);
			break;
		}
		int i = 0;

		for (i = 0; i < pMob[conn].MaxCarry; i++)
		{
			if(pMob[conn].MOB.Carry[i].sIndex == 453)
				break;
		}

		if (i == pMob[conn].MaxCarry)
		{
			SendSay(npcIndex, g_pMessageStringTable[_NN_NewbieQuest_Cheerup3]);
			break;
		}
		sprintf(temp, "etc,newbiequest3 complete name:%s conn:%d", pMob[conn].MOB.MobName, conn);
		Log(temp, "-system", 0);

		pMob[conn].extra.QuestInfo.Mortal.Newbie++;
		SendSay(npcIndex, g_pMessageStringTable[_NN_NewbieQuest_Complete3]);
		SendClientMessage(conn, g_pMessageStringTable[_NN_NewbieQuest_Reward3]);

		for (int j = 1; j < 8; j++)
		{
			int itemId = pMob[conn].MOB.Equip[6].sIndex;

			if (itemId > 0 && itemId < MAX_ITEMLIST)
			{
				int ReqLv = g_pItemList[itemId].ReqLvl;

				pMob[conn].MOB.Equip[j].stEffect[0].cEffect = 0;
				pMob[conn].MOB.Equip[j].stEffect[0].cValue = 0;
				pMob[conn].MOB.Equip[j].stEffect[1].cEffect = 0;
				pMob[conn].MOB.Equip[j].stEffect[1].cValue = 0;
				pMob[conn].MOB.Equip[j].stEffect[2].cEffect = 0;
				pMob[conn].MOB.Equip[j].stEffect[2].cValue = 0;

				SetItemBonus(&pMob[conn].MOB.Equip[j], 50 + ReqLv, 1, 0);
				BASE_SetItemSanc(&pMob[conn].MOB.Equip[j], 3, 0);

				SendItem(conn, ITEM_PLACE_EQUIP, j, &pMob[conn].MOB.Equip[j]);
			}
		}
		SetAffect(conn, 44, 200, 200);
		SendScore(conn);
		SendCarry(conn);
	} break;
#pragma endregion
#pragma region TREINADORNEWBIE4
	case TREINADORNEWBIE4:
	{
		if(pMob[conn].extra.ClassMaster != MORTAL || pMob[conn].MOB.CurrentScore.Level >= FREEEXP || pMob[conn].extra.QuestInfo.Mortal.Newbie != 3)
		{
			SendSay(npcIndex, g_pMessageStringTable[_NN_NewbieQuest_Already4]);
			break;
		}
		int i = 0;

		for (i = 0; i < pMob[conn].MaxCarry; i++)
		{
			if(pMob[conn].MOB.Carry[i].sIndex == 524)
				break;
		}

		if (i == pMob[conn].MaxCarry)
		{
			SendSay(npcIndex, g_pMessageStringTable[_NN_NewbieQuest_Cheerup4]);
			break;
		}
		sprintf(temp, "etc,newbiequest4 complete name:%s conn:%d", pMob[conn].MOB.MobName, conn);
		Log(temp, "-system", 0);

		pMob[conn].extra.QuestInfo.Mortal.Newbie = 4;
		SendSay(npcIndex, g_pMessageStringTable[_NN_NewbieQuest_Complete4]);
		SendClientMessage(conn, g_pMessageStringTable[_NN_NewbieQuest_Reward4]);

		STRUCT_ITEM Item;
		memset(&Item, 0, sizeof(STRUCT_ITEM));

		BASE_ClearItem(&pMob[conn].MOB.Carry[i]);

		int rd = rand() % 3;

		if (rd == 0)
		{
			Item.sIndex = 682;
			Item.stEffect[0].cEffect = 61;
			Item.stEffect[0].cValue = 20;
		}

		else if (rd == 1)
			Item.sIndex = 481;

		else
			Item.sIndex = 652;


		PutItem(conn, &Item);

		SetAffect(conn, 44, 200, 200);
		SendScore(conn);
		SendCarry(conn);
	} break;
#pragma endregion
#pragma region CAPAVERDE_TELEPORT
	case CAPAVERDE_TELEPORT:
	{
		if (pMob[conn].extra.ClassMaster != MORTAL)
		{
			SendSay(npcIndex, g_pMessageStringTable[_NN_Level_Limit2]);
			break;
		}

		if (pMob[conn].MOB.CurrentScore.Level < 99 || pMob[conn].MOB.CurrentScore.Level >= 150)
		{
			SendSay(npcIndex, g_pMessageStringTable[_NN_Level_limit]);
			break;
		}

		DoTeleport(conn, 2245 + rand() % 5 - 3, 1576 + rand() % 5 - 3);

		sprintf(temp, "etc,quest_aprendiz1 npcteleport name:%s conn:%d", pMob[conn].MOB.MobName, conn);
		Log(temp, "-system", 0);
	} break;
#pragma endregion
#pragma region CAPAVERDE_TRADE
	case CAPAVERDE_TRADE:
	{
		if (pMob[conn].extra.ClassMaster != MORTAL)
		{
			SendSay(npcIndex, g_pMessageStringTable[_NN_Level_Limit2]);
			break;
		}

		if (pMob[conn].MOB.CurrentScore.Level < 99 || pMob[conn].MOB.CurrentScore.Level >= 150)
		{
			SendSay(npcIndex, g_pMessageStringTable[_NN_Level_limit]);
			break;
		}

		if (pMob[conn].MOB.Equip[13].sIndex != 4080)
		{
			sprintf(temp, g_pMessageStringTable[_SN_WANTEQUIPITEM], g_pItemList[4080].Name);
			SendSay(npcIndex, temp);
			break;
		}

		if (pMob[conn].MOB.Equip[15].sIndex != 0)
		{
			SendSay(npcIndex, g_pMessageStringTable[_NN_Youve_Done_It_Already]);
			break;
		}

		sprintf(temp, "etc,quest_aprendiz2 complete name:%s conn:%d", pMob[conn].MOB.MobName, conn);
		Log(temp, "-system", 0);

		BASE_ClearItem(&pMob[conn].MOB.Equip[13]);
		SendItem(conn, ITEM_PLACE_EQUIP, 13, &pMob[conn].MOB.Equip[13]);

		memset(&pMob[conn].MOB.Equip[15], 0, sizeof(STRUCT_ITEM));
		pMob[conn].MOB.Equip[15].sIndex = 4006;

		SendItem(conn, ITEM_PLACE_EQUIP, 15, &pMob[conn].MOB.Equip[15]);

		pMob[conn].GetCurrentScore(conn);
		SendScore(conn);

		SendEmotion(conn, 14, 3);

		sprintf(temp, g_pMessageStringTable[_DN_Play_Quest], "Formatura Aprendiz");
		SendClientMessage(conn, temp);
	} break;
#pragma endregion
#pragma region MOLARGARGULA
	case MOLARGARGULA:
	{
		if (pMob[conn].extra.ClassMaster != MORTAL)
		{
			SendSay(npcIndex, g_pMessageStringTable[_NN_Level_Limit2]);
			break;
		}

		if (pMob[conn].MOB.CurrentScore.Level < 199 || pMob[conn].MOB.CurrentScore.Level >= 254)
		{
			SendSay(npcIndex, g_pMessageStringTable[_NN_Level_limit]);
			break;
		}

		sprintf(temp, "etc,quest_molar1 npcteleport name:%s conn:%d", pMob[conn].MOB.MobName, conn);
		Log(temp, "-system", 0);

		DoTeleport(conn, 817 + rand() % 5 - 3, 4062 + rand() % 5 - 3);
	} break;
#pragma endregion
#pragma region QUEST_CAPAREAL
	case QUEST_CAPAREAL:
	{
		if (pMob[conn].extra.ClassMaster != MORTAL)
		{
			SendSay(npcIndex, g_pMessageStringTable[_NN_Level_Limit2]);
			break;
		}

		if (pMob[conn].MOB.CurrentScore.Level < 199 || pMob[conn].MOB.CurrentScore.Level >= 254)
		{
			SendSay(npcIndex, g_pMessageStringTable[_NN_Level_limit]);
			break;
		}

		DoTeleport(conn, 1740 + rand() % 5 - 3, 1725 + rand() % 5 - 3);

		sprintf(temp, "etc,questreal npcteleport name:%s conn:%d", pMob[conn].MOB.MobName, conn);
		Log(temp, "-system", 0);
	} break;
#pragma endregion
#pragma region LIDER_APRENDIZ
	case LIDER_APRENDIZ:
	{
		if (pMob[conn].extra.ClassMaster != ARCH)
		{
			sprintf(temp, g_pMessageStringTable[_SN_NEEDCLASS], "Arch");
			SendSay(npcIndex, temp);
			break;
		}

		if (pMob[conn].MOB.BaseScore.Level < 99 || pMob[conn].MOB.BaseScore.Level > 149)
		{
			SendSay(npcIndex, g_pMessageStringTable[_NN_Level_Limit2]);
			break;
		}

		if (pMob[conn].MOB.Equip[15].sIndex)
		{
			SendSay(npcIndex, g_pMessageStringTable[_NN_Youve_Done_It_Already]);
			break;
		}

		sprintf(temp, "etc,quest_aprendiz_arch complete name:%s conn:%d", pMob[conn].MOB.MobName, conn);
		Log(temp, "-system", 0);

		memset(&pMob[conn].MOB.Equip[15], 0, sizeof(STRUCT_ITEM));
		pMob[conn].MOB.Equip[15].sIndex = 4006;

		SendItem(conn, ITEM_PLACE_EQUIP, 15, &pMob[conn].MOB.Equip[15]);

		pMob[conn].GetCurrentScore(conn);
		SendScore(conn);

		SendEmotion(conn, 14, 3);

		sprintf(temp, g_pMessageStringTable[_DN_Play_Quest], "Formatura Aprendiz");
		SendClientMessage(conn, temp);
	} break;
#pragma endregion
	default:
		return false;
	}

	return true;
}
