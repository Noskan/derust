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

/// @file Quest_Kingdom.cpp
/// @brief Handlers de quests e serviços ligados a reino, dragões e contribuintes.

#include "handlers/world/QuestInternal.h"

bool HandleQuest_Kingdom(int conn, int npcIndex, int confirm, int npcMode, int npcMerc, int npcGrade, MSG_STANDARDPARM2* m)
{
	switch (npcMode)
	{
#pragma region KING
	case KING:
	{
				int ReiClan = pMob[npcIndex].MOB.Clan;

				int Clan = pMob[conn].MOB.Clan;
				int cLevel = pMob[conn].MOB.CurrentScore.Level;
				int CapeMode = 0;

				STRUCT_ITEM *Capa = &pMob[conn].MOB.Equip[15];

				if (Capa->sIndex == 543 || Capa->sIndex == 545)
					Clan = 7;

				if (Capa->sIndex == 544 || Capa->sIndex == 546)
					Clan = 8;

				if (Capa->sIndex == 734 || Capa->sIndex == 736)
					Clan = 7;

				if (Capa->sIndex == 735 || Capa->sIndex == 737)
					Clan = 8;

				if (Capa->sIndex == 3191 || Capa->sIndex == 3194)
					Clan = 7;

				if (Capa->sIndex == 3192 || Capa->sIndex == 3195)
					Clan = 8;

				if (Capa->sIndex == 3197)
					Clan = 7;

				if (Capa->sIndex == 3198)
					Clan = 8;

				if (Capa->sIndex == 543 || Capa->sIndex == 544 || Capa->sIndex == 3191 || Capa->sIndex == 3194 || Capa->sIndex == 3192 || Capa->sIndex == 3195 || Capa->sIndex == 3197 || Capa->sIndex == 3198)
					CapeMode = 2;

				if (Capa->sIndex == 545 || Capa->sIndex == 546 || Capa->sIndex == 549)
					CapeMode = 1;

				if(Capa->sIndex == 3193)
					CapeMode = 1;

				if(Capa->sIndex == 3196)
					CapeMode = 1;								

				if (Clan != 0 && Clan != ReiClan)
					break;


				int Saphire = Sapphire;

				if (ReiClan == 8)
				{
					if (Sapphire == 1)
						Saphire = 60;
					if (Sapphire == 2)
						Saphire = 32;
					if (Sapphire == 4)
						Saphire = 16;
					if (Sapphire == 16)
						Saphire = 4;
					if (Sapphire == 32)
						Saphire = 2;
					if (Sapphire == 64)
						Saphire = 1;
				}

				if (confirm == 0)
				{
					sprintf(temp, g_pMessageStringTable[_DN_Need_D_Sapphire], Saphire);
					SendClientMessage(conn, temp);
					break;
				}

				if (pMob[conn].MOB.Equip[10].sIndex == 1742 && pMob[conn].MOB.Equip[11].sIndex >= 1760 && pMob[conn].MOB.Equip[11].sIndex <= 1763)
				{
					int sAgua = 0;
					int sTerra = 0;
					int sSol = 0;
					int sVento = 0;


					for (int i = 0; i < MAX_CARRY - 4; i = BASE_NextCarry(pMob[conn].MOB.Carry, i))
					{
						if (pMob[conn].MOB.Carry[i].sIndex == 5334)
							sAgua = 1;

						if (pMob[conn].MOB.Carry[i].sIndex == 5335)
							sTerra = 1;

						if (pMob[conn].MOB.Carry[i].sIndex == 5336)
							sSol = 1;

						if (pMob[conn].MOB.Carry[i].sIndex == 5337)
							sVento = 1;
					}

					if (sAgua == 1 && sTerra == 1 && sSol == 1 && sVento == 1)
					{
						for (int i = 0; i < MAX_CARRY - 4; i = BASE_NextCarry(pMob[conn].MOB.Carry, i))
						{
							if (pMob[conn].MOB.Carry[i].sIndex == 5334 && sAgua == 1)
							{
								memset(&pMob[conn].MOB.Carry[i], 0, sizeof(STRUCT_ITEM));

								pMob[conn].MOB.Carry[i].sIndex = 5338;

								SendItem(conn, ITEM_PLACE_CARRY, i, &pMob[conn].MOB.Carry[i]);
								sAgua = 0;
							}

							if (pMob[conn].MOB.Carry[i].sIndex == 5335 && sTerra == 1)
							{
								memset(&pMob[conn].MOB.Carry[i], 0, sizeof(STRUCT_ITEM));
								SendItem(conn, ITEM_PLACE_CARRY, i, &pMob[conn].MOB.Carry[i]);
								sTerra = 0;
							}

							if (pMob[conn].MOB.Carry[i].sIndex == 5336 && sSol == 1)
							{
								memset(&pMob[conn].MOB.Carry[i], 0, sizeof(STRUCT_ITEM));
								SendItem(conn, ITEM_PLACE_CARRY, i, &pMob[conn].MOB.Carry[i]);
								sSol = 0;
							}

							if (pMob[conn].MOB.Carry[i].sIndex == 5337 && sVento == 1)
							{
								memset(&pMob[conn].MOB.Carry[i], 0, sizeof(STRUCT_ITEM));
								SendItem(conn, ITEM_PLACE_CARRY, i, &pMob[conn].MOB.Carry[i]);
								sVento = 0;
							}
						}

						memset(&pMob[conn].MOB.Equip[10], 0, sizeof(STRUCT_ITEM));
						SendItem(conn, ITEM_PLACE_EQUIP, 10, &pMob[conn].MOB.Equip[10]);

						memset(&pMob[conn].MOB.Equip[11], 0, sizeof(STRUCT_ITEM));
						SendItem(conn, ITEM_PLACE_EQUIP, 11, &pMob[conn].MOB.Equip[11]);

						SendSay(npcIndex, g_pMessageStringTable[_NN_My_King_Bless1]);

						sprintf(temp, "etc,king make ideal_stone name:%s level:%d", pMob[conn].MOB.MobName, pMob[conn].MOB.BaseScore.Level);
						Log(temp, "-system", 0);

						break;
					}

					if (pMob[conn].extra.ClassMaster == MORTAL && pMob[conn].MOB.CurrentScore.Level >= 299 && pMob[conn].Mode != MOB_WAITDB)
					{
									
						int cls = pMob[conn].MOB.Equip[11].sIndex - 1760;

						MSG_DBCreateArchCharacter sm_ca;
						memset(&sm_ca, 0, sizeof(MSG_DBCreateArchCharacter));

						sm_ca.Size = sizeof(MSG_DBCreateArchCharacter);
						sm_ca.Type = _MSG_DBCreateArchCharacter;
						sm_ca.ID = conn;

						sm_ca.MobClass = cls;
						sm_ca.MortalFace = pMob[conn].extra.MortalFace;
						sm_ca.MortalSlot = pUser[conn].Slot;

						strncpy(sm_ca.MobName, pMob[conn].MOB.MobName, NAME_LENGTH);

						pMob[conn].Mode = MOB_WAITDB;
						DBServerSocket.SendOneMessage((char*)&sm_ca, sizeof(MSG_DBCreateArchCharacter));

						sprintf(temp, "etc,king_arch name:%s class:%d mortalface:%d mortalslot:%d", pMob[conn].MOB.MobName, cls, pMob[conn].extra.MortalFace, pUser[conn].Slot);
						Log(temp, pUser[conn].AccountName, pUser[conn].IP);
						break;
					}
				}

				if (CapeMode >= 2)
				{
					if (ReiClan == 7)
						SendClientMessage(conn, g_pMessageStringTable[_NN_My_King_Bless1]);
					else
						SendClientMessage(conn, g_pMessageStringTable[_NN_My_King_Bless2]);

					break;
				}

				if ((pMob[conn].extra.ClassMaster == CELESTIAL || pMob[conn].extra.ClassMaster == CELESTIALCS || pMob[conn].extra.ClassMaster == SCELESTIAL))
				{
					if (Capa->sIndex != 3199)
						break;

					int NumSepInv = 0;

					for (int i = 0; i < MAX_CARRY - 4; i = BASE_NextCarry(pMob[conn].MOB.Carry, i))
					{
						if (pMob[conn].MOB.Carry[i].sIndex == 697)
							NumSepInv++;

						else if (pMob[conn].MOB.Carry[i].sIndex == 4131)
							NumSepInv += 10;
					}

					if (NumSepInv < Saphire)
					{
						sprintf(temp, g_pMessageStringTable[_DN_Need_D_Sapphire], Saphire);
						SendClientMessage(conn, temp);
						break;
					}

					MSG_STANDARDPARM sm_us;
					memset(&sm_us, 0, sizeof(MSG_STANDARDPARM));

					sm_us.ID = 0;
					sm_us.Type = _MSG_DBUpdateSapphire;

					sm_us.Size = sizeof(MSG_STANDARDPARM);

					sm_us.Parm = ReiClan == 7;

					DBServerSocket.SendOneMessage((char*)&sm_us, sizeof(MSG_STANDARD));

					for (int i = 0; i < MAX_CARRY - 4 && Saphire > 0; i = BASE_NextCarry(pMob[conn].MOB.Carry, i))
					{
						if (pMob[conn].MOB.Carry[i].sIndex == 697)
						{
							memset(&pMob[conn].MOB.Carry[i], 0, sizeof(STRUCT_ITEM));
							SendItem(conn, ITEM_PLACE_CARRY, i, &pMob[conn].MOB.Carry[i]);
							Saphire--;
						}

						else if (pMob[conn].MOB.Carry[i].sIndex == 4131)
						{
							memset(&pMob[conn].MOB.Carry[i], 0, sizeof(STRUCT_ITEM));
							SendItem(conn, ITEM_PLACE_CARRY, i, &pMob[conn].MOB.Carry[i]);

							if((Saphire-10) < 0)
								Saphire = 0;
												
							else
								Saphire -= 10;
						}
					}


					if (ReiClan == 7)
						pMob[conn].MOB.Equip[15].sIndex = 3197;
					else
						pMob[conn].MOB.Equip[15].sIndex = 3198;

					SendItem(conn, ITEM_PLACE_EQUIP, 15, &pMob[conn].MOB.Equip[15]);

					if (ReiClan == 7)
						SendClientMessage(conn, g_pMessageStringTable[_NN_My_King_Bless1]);
					else
						SendClientMessage(conn, g_pMessageStringTable[_NN_My_King_Bless2]);

					int Guild = pMob[conn].MOB.Guild;
					int Group = ServerGroup;
					int Server = Guild / MAX_GUILD;
					int usGuild = Guild & MAX_GUILD - 1;

					if(Guild && pMob[conn].MOB.GuildLevel == 9)
					{
						MSG_GuildInfo sm_gi;
						memset(&sm_gi, 0, sizeof(MSG_GuildInfo));

						sm_gi.Type = _MSG_GuildInfo;
						sm_gi.Size = sizeof(MSG_GuildInfo);
						sm_gi.ID = conn;

						sm_gi.Guild = Guild;

						GuildInfo[Guild].Clan = ReiClan;
						GuildInfo[Guild].Fame = 0;

						sm_gi.GuildInfo = GuildInfo[Guild];

						DBServerSocket.SendOneMessage((char*)&sm_gi, sizeof(MSG_GuildInfo));
					}

					sprintf(temp, "etc,get mantle %d - sapphire:%d", pMob[conn].MOB.Equip[15].sIndex, Saphire);
					Log(temp, pUser[conn].AccountName, pUser[conn].IP);
					break;
				}

				if (cLevel < 219)
				{
					SendClientMessage(conn, g_pMessageStringTable[_NN_Need_Level]);
					break;
				}

				if (CapeMode != 1 || cLevel >= 255)
				{
					int NumSepInv = 0;
										
					for (int i = 0; i < MAX_CARRY - 4; i = BASE_NextCarry(pMob[conn].MOB.Carry, i))
					{
						if (pMob[conn].MOB.Carry[i].sIndex == 697)
							NumSepInv++;

						else if (pMob[conn].MOB.Carry[i].sIndex == 4131)
							NumSepInv += 10;
					}

					if (NumSepInv < Saphire && pMob[conn].MOB.Equip[13].sIndex != 4081)
					{
						sprintf(temp, g_pMessageStringTable[_DN_Need_D_Sapphire], Saphire);
						SendClientMessage(conn, temp);
						break;
					}

					MSG_STANDARDPARM sm_us;
					memset(&sm_us, 0, sizeof(MSG_STANDARDPARM));

					sm_us.ID = 0;
					sm_us.Type = _MSG_DBUpdateSapphire;

					sm_us.Size = sizeof(MSG_STANDARDPARM);

					sm_us.Parm = ReiClan == 7;

					DBServerSocket.SendOneMessage((char*)&sm_us, sizeof(MSG_STANDARD));

					for (int i = 0; i < MAX_CARRY - 4 && Saphire > 0 && pMob[conn].MOB.Equip[13].sIndex != 4081; i = BASE_NextCarry(pMob[conn].MOB.Carry, i))
					{
						if (pMob[conn].MOB.Carry[i].sIndex == 697)
						{
							memset(&pMob[conn].MOB.Carry[i], 0, sizeof(STRUCT_ITEM));
							SendItem(conn, ITEM_PLACE_CARRY, i, &pMob[conn].MOB.Carry[i]);
							Saphire--;
						}

						else if (pMob[conn].MOB.Carry[i].sIndex == 4131)
						{
							memset(&pMob[conn].MOB.Carry[i], 0, sizeof(STRUCT_ITEM));
							SendItem(conn, ITEM_PLACE_CARRY, i, &pMob[conn].MOB.Carry[i]);

							if((Saphire-10) < 0)
								Saphire = 0;
												
							else
								Saphire -= 10;
						}
					}

					if(pMob[conn].MOB.Equip[13].sIndex == 4081)
					{
						memset(&pMob[conn].MOB.Equip[13], 0, sizeof(STRUCT_ITEM));
						SendItem(conn, ITEM_PLACE_EQUIP, 13, &pMob[conn].MOB.Equip[13]);
					}

					if (CapeMode == 0)
					{
						if (Capa->sIndex != 548)
							memset(&pMob[conn].MOB.Equip[15], 0, sizeof(STRUCT_ITEM));

						if (ReiClan == 7)
							pMob[conn].MOB.Equip[15].sIndex = 545;
						else
							pMob[conn].MOB.Equip[15].sIndex = 546;
					}

					if (CapeMode == 1)
					{
						if(Capa->sIndex == 3193)
						{
							if (ReiClan == 7)
								pMob[conn].MOB.Equip[15].sIndex = 3191;
							else
								pMob[conn].MOB.Equip[15].sIndex = 3192;
						}

						else if(Capa->sIndex == 3196)
						{
							if (ReiClan == 7)
								pMob[conn].MOB.Equip[15].sIndex = 3194;
							else
								pMob[conn].MOB.Equip[15].sIndex = 3195;
						}

						else
						{
							if (Capa->sIndex != 549)
								memset(&pMob[conn].MOB.Equip[15], 0, sizeof(STRUCT_ITEM));

							if (ReiClan == 7)
								pMob[conn].MOB.Equip[15].sIndex = 543;
							else
								pMob[conn].MOB.Equip[15].sIndex = 544;
						}
					}


					SendItem(conn, ITEM_PLACE_EQUIP, 15, &pMob[conn].MOB.Equip[15]);
										
					if (ReiClan == 7)
						SendClientMessage(conn, g_pMessageStringTable[_NN_My_King_Bless1]);
					else
						SendClientMessage(conn, g_pMessageStringTable[_NN_My_King_Bless2]);
										
					int Guild = pMob[conn].MOB.Guild;
					int Group = ServerGroup;
					int Server = Guild / MAX_GUILD;
					int usGuild = Guild & MAX_GUILD - 1;

					if(Guild && pMob[conn].MOB.GuildLevel == 9)
					{
						MSG_GuildInfo sm_gi;
						memset(&sm_gi, 0, sizeof(MSG_GuildInfo));

						sm_gi.Type = _MSG_GuildInfo;
						sm_gi.Size = sizeof(MSG_GuildInfo);
						sm_gi.ID = conn;

						sm_gi.Guild = Guild;

						GuildInfo[Guild].Clan = ReiClan;

						sm_gi.GuildInfo = GuildInfo[Guild];

						DBServerSocket.SendOneMessage((char*)&sm_gi, sizeof(MSG_GuildInfo));
					}

					sprintf(temp,"etc,get mantle %d - sapphire:%d", pMob[conn].MOB.Equip[15].sIndex, Saphire);
					Log(temp, pUser[conn].AccountName, pUser[conn].IP);
					break;
				}
				else
				{
					SendClientMessage(conn, g_pMessageStringTable[_NN_Need_Level]);
					break;
				}
	} break;
#pragma endregion
#pragma region KINGDOM BROKER
	case KINGDOM:
	{
					int Clan = pMob[conn].MOB.Clan;


					time_t rawtime;
					tm *timeinfo;

					time(&rawtime);
					timeinfo = localtime(&rawtime);

					if(timeinfo->tm_wday == 6 || timeinfo->tm_wday == 0)
					{
						SendClientMessage(conn, g_pMessageStringTable[_NN_NotEquip_Saturday]);
						break;
					}

					if (Clan != 7 && Clan != 8)
					{
						if (Clan == 7)
							SendClientMessage(conn, g_pMessageStringTable[_NN_My_King_Bless1]);
						else
							SendClientMessage(conn, g_pMessageStringTable[_NN_My_King_Bless2]);

						break;
					}

					if (confirm == 0)
					{
						SendClientMessage(conn, g_pMessageStringTable[_NN_Leaving_kingdom]);

						break;
					}
					int Saphire = 16;
					int NumSepInv = 0;

					for (int i = 0; i < MAX_CARRY - 4; i = BASE_NextCarry(pMob[conn].MOB.Carry, i))
					{
						if (pMob[conn].MOB.Carry[i].sIndex == 697)
							NumSepInv++;

						else if(pMob[conn].MOB.Carry[i].sIndex == 4131)
							NumSepInv += 10;
					}

					if (NumSepInv < Saphire)
					{
						sprintf(temp, g_pMessageStringTable[_DN_Need_D_Sapphire], Saphire);
						SendClientMessage(conn, temp);
						break;
					}



					for (int i = 0; i < MAX_CARRY - 4 && Saphire > 0; i = BASE_NextCarry(pMob[conn].MOB.Carry, i))
					{
						if (pMob[conn].MOB.Carry[i].sIndex == 697)
						{
							memset(&pMob[conn].MOB.Carry[i], 0, sizeof(STRUCT_ITEM));
							SendItem(conn, ITEM_PLACE_CARRY, i, &pMob[conn].MOB.Carry[i]);
							Saphire--;
						}

						else if (pMob[conn].MOB.Carry[i].sIndex == 4131)
						{
							memset(&pMob[conn].MOB.Carry[i], 0, sizeof(STRUCT_ITEM));
							SendItem(conn, ITEM_PLACE_CARRY, i, &pMob[conn].MOB.Carry[i]);

							if((Saphire - 10) < 0)
								Saphire = 0;
												
							else
								Saphire -= 10;
						}
					}


					if (pMob[conn].MOB.Equip[15].sIndex == 543 || pMob[conn].MOB.Equip[15].sIndex == 544)
						pMob[conn].MOB.Equip[15].sIndex = 549;

					if (pMob[conn].MOB.Equip[15].sIndex == 545 || pMob[conn].MOB.Equip[15].sIndex == 546)
						pMob[conn].MOB.Equip[15].sIndex = 548;

					if (pMob[conn].MOB.Equip[15].sIndex == 3191 || pMob[conn].MOB.Equip[15].sIndex == 3192)
						pMob[conn].MOB.Equip[15].sIndex = 3193;

					if (pMob[conn].MOB.Equip[15].sIndex == 3194 || pMob[conn].MOB.Equip[15].sIndex == 3195)
						pMob[conn].MOB.Equip[15].sIndex = 3196;

					if (pMob[conn].MOB.Equip[15].sIndex == 3197 || pMob[conn].MOB.Equip[15].sIndex == 3198)
						pMob[conn].MOB.Equip[15].sIndex = 3199;

					if (Clan == 7)
						SendClientMessage(conn, g_pMessageStringTable[_NN_My_King_Bless1]);
					else
						SendClientMessage(conn, g_pMessageStringTable[_NN_My_King_Bless2]);

					SendItem(conn, ITEM_PLACE_EQUIP, 15, &pMob[conn].MOB.Equip[15]);

					pMob[conn].GetCurrentScore(conn);

					MSG_STANDARDPARM sm_scl;
					memset(&sm_scl, 0, sizeof(MSG_STANDARDPARM));

					sm_scl.Type = _MSG_SetClan;
					sm_scl.Size = sizeof(MSG_STANDARDPARM);
					sm_scl.ID = conn;
					sm_scl.Parm = pMob[conn].MOB.Clan;
					m->ClientTick = CurrentTime;
									   
					GridMulticast(pMob[conn].TargetX, pMob[conn].TargetY, (MSG_STANDARD*)&sm_scl, 0);

					SendEquip(conn, conn);
					sprintf(temp, "etc,remove mantle");
					Log(temp, pUser[conn].AccountName, pUser[conn].IP);
					break;

	} break;
#pragma endregion
#pragma region URNAMMU
	case URNAMMU:
	{
					if (pMob[conn].extra.ClassMaster != CELESTIALCS)
					{
						sprintf(temp, g_pMessageStringTable[_SN_NEEDCLASS], "Sub Celestial");
						SendClientMessage(conn, temp);
						break;
					}				   
					if (pMob[conn].MOB.Equip[11].sIndex < 1760 || pMob[conn].MOB.Equip[11].sIndex > 1763)
					{
						sprintf(temp, g_pMessageStringTable[_SN_WANTEQUIPITEM], "o Sephirot da classe escolhida");
						SendClientMessage(conn, temp);
						break;
					}
					if (pMob[conn].extra.Fame < 2000)
					{
						sprintf(temp, g_pMessageStringTable[_DN_NEEDFAME], 2000);
						SendClientMessage(conn, temp);
						break;
					}

					sprintf(temp, "etc,change SubClass class:%d to %d name:%s level:%d", pMob[conn].MOB.Class, pMob[conn].MOB.Equip[11].sIndex - 1760, pMob[conn].MOB.MobName, pMob[conn].MOB.BaseScore.Level);
					Log(temp, "-system", 0);

					pMob[conn].extra.Fame -= 2000;

					pMob[conn].extra.SaveCelestial[1].Class = pMob[conn].MOB.Equip[11].sIndex - 1760;

					pMob[conn].extra.SaveCelestial[1].BaseScore.Str = BaseSIDCHM[pMob[conn].MOB.Class][0];
					pMob[conn].extra.SaveCelestial[1].BaseScore.Int = BaseSIDCHM[pMob[conn].MOB.Class][1];
					pMob[conn].extra.SaveCelestial[1].BaseScore.Dex = BaseSIDCHM[pMob[conn].MOB.Class][2];
					pMob[conn].extra.SaveCelestial[1].BaseScore.Con = BaseSIDCHM[pMob[conn].MOB.Class][3];

					memset(pMob[conn].extra.SaveCelestial[1].SkillBar1, 0, 4);
					memset(pMob[conn].extra.SaveCelestial[1].SkillBar2, 0, 16);

					int special = pMob[conn].extra.SaveCelestial[1].SpecialBonus;
					int specialreset = 55;

					for (int i = 0; i < 4; i++)
					{
						if (pMob[conn].extra.SaveCelestial[1].BaseScore.Special[i] <= 55)
						{
							special += pMob[conn].extra.SaveCelestial[1].BaseScore.Special[i];
							pMob[conn].extra.SaveCelestial[1].BaseScore.Special[i] = 0;
						}
						else
						{
							special += specialreset;
							pMob[conn].extra.SaveCelestial[1].BaseScore.Special[i] -= specialreset;
						}
					}

					pMob[conn].extra.SaveCelestial[1].SpecialBonus = special;

					pMob[conn].extra.SaveCelestial[1].LearnedSkill &= 0xFF000000;

					memset(&pMob[conn].MOB.Equip[11], 0, sizeof(STRUCT_ITEM));

					SendItem(conn, ITEM_PLACE_EQUIP, 11, &pMob[conn].MOB.Equip[11]);

					SendClientMessage(conn, g_pMessageStringTable[_NN_My_King_Bless1]);
					break;
	} break;
#pragma endregion
#pragma region UXMAL
	case UXMAL:
	{
		struct tm when;
		time_t now;
		time(&now);
		when = *localtime(&now);

		if(when.tm_min >= 0 && when.tm_min <= 15 || when.tm_min >= 20 && when.tm_min <= 35 || when.tm_min >= 40 && when.tm_min <= 55)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_Night_Already]);
			break;
		}

		if (pMob[conn].Leader != -1 && pMob[conn].Leader)
		{
				SendClientMessage(conn, g_pMessageStringTable[_NN_Party_Leader_Only]);
				break;
		}

		int i = 0;

		int sala = -1;

		for (i = 0; i < MAX_CARRY - 4; i = BASE_NextCarry(pMob[conn].MOB.Carry, i))
		{
			if (pMob[conn].MOB.Carry[i].sIndex != 5134)
				continue;

			sala = BASE_GetItemSanc(&pMob[conn].MOB.Carry[i]);

			if(sala > 6)
				sala = 6;
								
			break;
		}

		if (i == MAX_CARRY - 4)
		{
			sprintf(temp, g_pMessageStringTable[_SN_BRINGITEM], g_pItemList[5134].Name);
			SendClientMessage(conn, temp);
			break;
		}
		int j = 0;

		for(j = 0; j < (sala == 0 ? 2 : 3); j++)
		{
			if(Pista[sala].Party[j].LeaderID == 0)
				break;
		}

		if(j == (sala == 0 ? 2 : 3))
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_Night_Limited]);
			break;
		}

		int c = 0;
		for(c = 0; c < 3; c++)
		{
			if(Pista[sala].Party[c].LeaderID == conn)
				break;
		}
		if(c != 3)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_Night_Already]);
			break;
		}

		sprintf(temp, "etc,register RuneQuest type:%d conn:%d name:%s level:%d", sala, conn, pMob[conn].MOB.MobName, pMob[conn].MOB.BaseScore.Level);
		Log(temp, "-system", 0);

		Pista[sala].Party[j].LeaderID = conn;
		Pista[sala].Party[j].MobCount = 0;
		Pista[sala].Party[j].Sala = sala;
		strncpy(Pista[sala].Party[j].LeaderName, pMob[conn].MOB.MobName, NAME_LENGTH);

		BASE_ClearItem(&pMob[conn].MOB.Carry[i]);
		SendItem(conn, ITEM_PLACE_CARRY, i, &pMob[conn].MOB.Carry[i]);

		SendClientMessage(conn, g_pMessageStringTable[_NN_TicketUsed]);

	} break;
#pragma endregion
#pragma region ARZAN_DRAGON
	case ARZAN_DRAGON:
	{
		if(pMob[conn].MOB.Coin < 2000000)
		{
			SendSay(npcIndex, g_pMessageStringTable[_NN_snowimp_need_2MGold]);
			break;
		}

		int Materiais[5] = {0, 0, 0, 0, 0};

		for (int i = 0; i < MAX_CARRY - 4; i = BASE_NextCarry(pMob[conn].MOB.Carry, i))
		{
			if(pMob[conn].MOB.Carry[i].sIndex < 1721 || pMob[conn].MOB.Carry[i].sIndex > 1725)
				continue;

			int Id = pMob[conn].MOB.Carry[i].sIndex - 1721;

			Materiais[Id]++;
		}

		if(Materiais[0] < 10 || Materiais[1] < 2 || Materiais[2] < 4 || Materiais[3] < 1 || Materiais[4] < 1)
		{
			SendSay(npcIndex, g_pMessageStringTable[_NN_Need_5_materials]);
			break;
		}

		if(Materiais[0] > 10)
			Materiais[0] = 10;

		if(Materiais[1] > 2)
			Materiais[1] = 2;

		if(Materiais[2] > 4)
			Materiais[2] = 4;

		if(Materiais[3] > 1)
			Materiais[3] = 1;

		if(Materiais[4] > 1)
			Materiais[4] = 1;

		for(int x = 0;x < 5; x++)
		{
			for (int i = 0; i < MAX_CARRY - 4 && Materiais[x] > 0; i = BASE_NextCarry(pMob[conn].MOB.Carry, i))
			{
				if (pMob[conn].MOB.Carry[i].sIndex == (1721 + x))
				{
					memset(&pMob[conn].MOB.Carry[i], 0, sizeof(STRUCT_ITEM));
					SendItem(conn, ITEM_PLACE_CARRY, i, &pMob[conn].MOB.Carry[i]);
					Materiais[x]--;
				}
			}
		}

		pMob[conn].MOB.Coin -= 2000000;
        SendEtc(conn);

		STRUCT_ITEM Item;

		memset(&Item, 0, sizeof(STRUCT_ITEM));

		Item.sIndex = 1726;
		Item.stEffect[0].cEffect = 43;
		Item.stEffect[0].cValue = rand()%4;

		PutItem(conn, &Item);

		SendClientMessage(conn, g_pMessageStringTable[_NN_snowimp_create_success]);
							
		sprintf(temp, "etc,arzan_dragon snowimp create name:%s level:%d", pMob[conn].MOB.MobName, pMob[conn].MOB.BaseScore.Level);
		Log(temp, "-system", 0);
	}break;
#pragma endregion
#pragma region GOLD_DRAGON
	case GOLD_DRAGON:
	{
		int treasure = CombineTreasureMap(conn);

		if(treasure)
			break;

		for(int i = 0; i < 8; i++)
		{
			if(g_pTreasure[i].Source == 0)
				continue;

			int x = 0;

			for (x = 0; x < MAX_CARRY - 4; x = BASE_NextCarry(pMob[conn].MOB.Carry, x))
			{
				if(pMob[conn].MOB.Carry[x].sIndex == g_pTreasure[i].Source)
					break;
			}

			if (x != MAX_CARRY - 4)
			{
				char itemlog[1024];

				sprintf(temp, "tra,_gold_ 0");
									
				BASE_GetItemCode(&pMob[conn].MOB.Carry[x], itemlog);
				strcat(temp, itemlog);

				Log(temp, pUser[conn].AccountName, pUser[conn].IP);
									
				memset(&pMob[conn].MOB.Carry[x], 0, sizeof(STRUCT_ITEM));

				SendItem(conn, ITEM_PLACE_CARRY, x, &pMob[conn].MOB.Carry[x]);

				int _rand = rand() % 1000;

				STRUCT_ITEM Item;

				memset(&Item, 0, sizeof(STRUCT_ITEM));

				if(_rand < g_pTreasure[i].Rate[0])
					memcpy(&Item, &g_pTreasure[i].Target[0], sizeof(STRUCT_ITEM));
									
				else if(_rand < g_pTreasure[x].Rate[1])
					memcpy(&Item, &g_pTreasure[i].Target[1], sizeof(STRUCT_ITEM));

				else if(_rand < g_pTreasure[i].Rate[2])
					memcpy(&Item, &g_pTreasure[i].Target[2], sizeof(STRUCT_ITEM));

				else if(_rand < g_pTreasure[i].Rate[3])
					memcpy(&Item, &g_pTreasure[i].Target[3], sizeof(STRUCT_ITEM));

				else if(_rand < g_pTreasure[i].Rate[4])
					memcpy(&Item, &g_pTreasure[i].Target[4], sizeof(STRUCT_ITEM));

				if(Item.sIndex == 0)
				{
					SendSay(npcIndex, g_pMessageStringTable[_NN_Next_Chance]);
					return true;
				}

				sprintf(temp, "tra,%s 0", pUser[conn].AccountName);
									
				BASE_GetItemCode(&Item, itemlog);
				strcat(temp, itemlog);
									
				Log(temp, "_gold_", pUser[conn].IP);

				PutItem(conn, &Item);

				SendSay(npcIndex, g_pMessageStringTable[_NN_Congratulations]);
				return true;
			}
		}

		int Cristais[7] = {-1, -1, -1, -1, -1, -1, -1};
		int NumCristal = 0;

		for (int i = 0; i < MAX_CARRY - 4; i = BASE_NextCarry(pMob[conn].MOB.Carry, i))
		{
			int ItemId = pMob[conn].MOB.Carry[i].sIndex;
								
			if(ItemId >= 421 && ItemId <= 427 && Cristais[ItemId-421] == -1)
			{
				Cristais[ItemId-421] = i;
				NumCristal++;
			}
		}

		if(Cristais[0] != -1 && Cristais[1] != -1 && Cristais[2] != -1 && Cristais[3] != -1 && Cristais[4] != -1 && Cristais[5] != -1 && Cristais[6] != -1)
		{
			if(pMob[conn].MOB.Equip[6].sIndex == 0)
			{
				SendSay(npcIndex, g_pMessageStringTable[_NN_Equip_Weapon_To_Enchant]);
				break;
			}

			int sanc = BASE_GetItemSanc(&pMob[conn].MOB.Equip[6]);

			if(sanc > 0)
			{
				SendSay(npcIndex, g_pMessageStringTable[_NN_Only_Nomal_Weapons]);
				break;
			}

			if((pMob[conn].MOB.Equip[0].sIndex/10) == 0)
				SendEmotion(conn, 23, 0);

			else
				SendEmotion(conn, 15, 0);

			int cEffect1 = pMob[conn].MOB.Equip[6].stEffect[0].cEffect;
			int cEffect2 = pMob[conn].MOB.Equip[6].stEffect[1].cEffect;
			int cEffect3 = pMob[conn].MOB.Equip[6].stEffect[2].cEffect;

			int cValue1 = pMob[conn].MOB.Equip[6].stEffect[0].cValue;
			int cValue2 = pMob[conn].MOB.Equip[6].stEffect[1].cValue;
			int cValue3 = pMob[conn].MOB.Equip[6].stEffect[2].cValue;

			pMob[conn].MOB.Equip[6].stEffect[0].cEffect = 0;
			pMob[conn].MOB.Equip[6].stEffect[0].cValue = 0;

			pMob[conn].MOB.Equip[6].stEffect[1].cEffect = 0;
			pMob[conn].MOB.Equip[6].stEffect[1].cValue = 0;

			pMob[conn].MOB.Equip[6].stEffect[2].cEffect = 0;
			pMob[conn].MOB.Equip[6].stEffect[2].cValue = 0;

			SetItemBonus(&pMob[conn].MOB.Equip[6], (25 * NumCristal / 10) + g_pItemList[pMob[conn].MOB.Equip[6].sIndex].nReqLvl, 1, pMob[conn].DropBonus);

			for(int i = 0; i < 7; i++)
				memset(&pMob[conn].MOB.Carry[Cristais[i]], 0, sizeof(STRUCT_ITEM));
								
			SendCarry(conn);
			SendClientMessage(conn, g_pMessageStringTable[_SN_I_Wwill_Enchant_Your_Weapon]);

			SendItem(conn, ITEM_PLACE_EQUIP, 6, &pMob[conn].MOB.Equip[6]);

			sprintf(temp, g_pMessageStringTable[_SN_Now_I_Will_Enchant_Your], pMob[conn].MOB.MobName);
			SendSay(npcIndex, temp);

			SetAffect(conn, 44, 200, 200);
			SendScore(conn);

			sprintf(temp, "etc,crystal %d-%d from %d:%d %d:%d %d:%d to %d:%d %d:%d %d:%d", NumCristal, pMob[conn].MOB.Equip[6].sIndex, cEffect1, cValue1, cEffect2, cValue2, cEffect3, cValue3, pMob[conn].MOB.Equip[6].stEffect[0].cEffect, pMob[conn].MOB.Equip[6].stEffect[0].cValue, pMob[conn].MOB.Equip[6].stEffect[1].cEffect, pMob[conn].MOB.Equip[6].stEffect[1].cValue, pMob[conn].MOB.Equip[6].stEffect[2].cEffect, pMob[conn].MOB.Equip[6].stEffect[2].cValue);
			Log(temp, pUser[conn].AccountName, pUser[conn].IP);

			break;
		}

		SendSay(npcIndex, g_pMessageStringTable[_NN_Gather_7_Clistals]);
	} break;
#pragma endregion
#pragma region EXPLOIT_LEADER
	case EXPLOIT_LEADER:
	{
		if(pMob[conn].extra.QuestInfo.Mortal.TerraMistica == 0)
		{
			BASE_GetLanguage(temp, _NN_Guard_This_Village);
			SendSay(npcIndex, temp);
			break;
		}

		if(pMob[conn].extra.QuestInfo.Mortal.TerraMistica != 2)
		{
			BASE_GetLanguage(temp, _SN_All_Villagers_Thanks_Your, pMob[conn].MOB.MobName);
			SendSay(npcIndex, temp);
			break;
		}

		int _Level = pMob[conn].MOB.CurrentScore.Level;
        int _ItemID = 551;
        int _ItemType = 1;

        if (_Level >= 50 )
        {
            if (_Level >= 80 )
            {
				_ItemID = rand() % 4 + 559;
				_ItemType = 0;
            }
            else
            {
				_ItemID = rand() % 4 + 555;
				_ItemType = 1;
            }
        }
        else
        {
            _ItemID = rand() % 4 + 551;
            _ItemType = 1;
        }

		STRUCT_ITEM Item;
		memset(&Item, 0, sizeof(STRUCT_ITEM));

		Item.sIndex = _ItemID;

		int _rd = rand()%10;

		int bonustype = g_pBonusType[_rd];

		int bonusvalue1 = g_pBonusValue[_rd][_ItemType][0];
		int bonusvalue2 = g_pBonusValue[_rd][_ItemType][1];
		int fvalue = bonusvalue2 + 1 - bonusvalue1;
		int tvalue = rand() % fvalue + bonusvalue1;

		Item.stEffect[0].cEffect = 43;

		for (int z = 1; z <= 2; ++z )
        {
            int rd_ = rand() % 8;

            if (rd_)
            {
				switch ( rd_ )
				{
				case 1:
					Item.stEffect[z].cEffect = 2;
					Item.stEffect[z].cValue = rand() % 21 + 5;
					break;

				case 2:
					Item.stEffect[z].cEffect = 26;
					Item.stEffect[z].cValue = rand() % 11 + 5;
					break;

				case 3:
					Item.stEffect[z].cEffect = 5;
					Item.stEffect[z].cValue = rand() % 51 + 20;
					break;
				
				case 4:
					Item.stEffect[z].cEffect = 60;
					Item.stEffect[z].cValue = rand() % 7 + 2;
					break;
				
				case 5:
					Item.stEffect[z].cEffect = 7;
					Item.stEffect[z].cValue = rand() % 16 + 5;
					break;
				
				case 6:
					Item.stEffect[z].cEffect = 8;
					Item.stEffect[z].cValue = rand() % 16 + 5;
					break;
				
				case 7:
					Item.stEffect[z].cEffect = 9;
					Item.stEffect[z].cValue = rand() % 16 + 5;
					break;
				}
            }
            else
            {
				Item.stEffect[z].cEffect = 4;
				Item.stEffect[z].cValue = rand() % 41 + 20;
            }
        }

		PutItem(conn, &Item);

        BASE_GetLanguage(temp, _NN_Give_You_Some_Reward);
        SendSay(npcIndex, temp);

		pMob[conn].extra.QuestInfo.Mortal.TerraMistica = 3;
        SendClientMessage(conn, g_pMessageStringTable[_NN_Watching_Town_Awarded]);

		sprintf(temp, "etc,mystical_land complete %d-%d:%d:%d:%d:%d:%d name:%s conn:%d", Item.sIndex, Item.stEffect[0].cEffect, Item.stEffect[0].cValue, Item.stEffect[1].cEffect, Item.stEffect[1].cValue, Item.stEffect[2].cEffect, Item.stEffect[2].cValue, pMob[conn].MOB.MobName, conn);
		Log(temp, "-system", 0);
	} break;
#pragma endregion
#pragma region GODGOVERNMENT
	case GODGOVERNMENT:
	{
		int curkill = GetCurKill(conn);
		int PKPoint = GetPKPoint(conn);

		if(curkill >= 100)
		{
			PKPoint += 20;

			if(PKPoint >= 150)
				PKPoint = 150;

			SetCurKill(conn, curkill-100);
			SetPKPoint(conn, PKPoint);

			sprintf(temp, "etc,resetpk curpk:%d name:%s conn:%d", PKPoint, pMob[conn].MOB.MobName, conn);
			Log(temp, "-system", 0);

			MSG_CreateMob sm_rpk;
			memset(&sm_rpk, 0, sizeof(MSG_CreateMob));
			GetCreateMob(conn, &sm_rpk);

			GridMulticast(pMob[conn].TargetX, pMob[conn].TargetY, (MSG_STANDARD*)&sm_rpk, 0);

			SendEmotion(conn, 14, 3);
			break;
		}

		SendSay(npcIndex, g_pMessageStringTable[_NN_MOREKILLPOINT]);
	} break;
#pragma endregion
	default:
		return false;
	}

	return true;
}
