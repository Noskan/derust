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

/// @file MobKilled_Drops.cpp
/// @brief Eventos de boss (Kefra/Noatum), drops especiais/comuns e recompensas em MobKilled().

#include "systems/MobKilledInternal.h"

void ProcessMobKilledDrops(int target, int conn, int PosX, int PosY, int GenerateIndex)
					{
						int AlvoX = PosX;
						if (!PosX)
							AlvoX = pMob[target].TargetX;

						int AlvoY = PosY;
						if (!PosY)
							AlvoY = pMob[target].TargetY;

#pragma region Kefra
						if (pMob[target].GenerateIndex == KEFRA_BOSS)
						{
							if (pMob[conn].MOB.Guild)
							{
								int Group = ServerGroup;
								int Server = pMob[conn].MOB.Guild / MAX_GUILD;
								int usGuild = pMob[conn].MOB.Guild;

								char guildname[256];

								BASE_GetGuildName(Group, usGuild, guildname);

								KefraLive = usGuild;

								MSG_GuildInfo sm_gi;
								memset(&sm_gi, 0, sizeof(MSG_GuildInfo));

								sm_gi.Type = _MSG_GuildInfo;
								sm_gi.Size = sizeof(MSG_GuildInfo);
								sm_gi.ID = conn;

								sm_gi.Guild = usGuild;
								GuildInfo[usGuild].Fame += 100;

								sm_gi.GuildInfo = GuildInfo[usGuild];

								DBServerSocket.SendOneMessage((char*)&sm_gi, sizeof(MSG_GuildInfo));

								sprintf(temp, g_pMessageStringTable[_SN_End_Khepra], guildname);
								SendNotice(temp);

								DrawConfig(TRUE);
							}
							
							else
							{
								KefraLive = 1;
								sprintf(temp, g_pMessageStringTable[_SN_End_Khepra], "UoW");
								SendNotice(temp);

								DrawConfig(TRUE);
							}

							for (int xx = 2335; xx < 2395; xx++)
							{
								for (int yy = 3896; yy < 3955; yy++)
								{
									int tmob = pMobGrid[yy][xx];

									if (tmob < MAX_USER)
										continue;

									int itemrand = W2Rand() % 60;

									STRUCT_ITEM *KefraItem = &pMob[target].MOB.Carry[itemrand];

									PutItem(conn, KefraItem);
								}
							}

							sprintf(temp, "etc,kefra killed");
							Log(temp, pMob[conn].MOB.MobName, 0);
						}
#pragma endregion
						if (pHeightGrid[AlvoY][AlvoX] >= -50 && pHeightGrid[AlvoY][AlvoX] <= 92)
						{
							int GenerateID = pMob[target].GenerateIndex;

// region Portões de Noatum
							if (pMob[target].MOB.Equip[0].sIndex == 220 && CastleState)
							{
								if (pMob[target].TargetX < 0 || pMob[target].TargetX >= MAX_GRIDX || pMob[target].TargetY < 0 || pMob[target].TargetY >= MAX_GRIDY)
								{
									Log("err,no castle gate to open pos", "-system", 0);
									return;
								}

								int ItemID = pItemGrid[pMob[target].TargetY][pMob[target].TargetX];
								if (ItemID >= 0 && ItemID < MAX_ITEM && pItem[ItemID].Mode)
								{
									int heigth = 0;
									int isUpdate = UpdateItem(ItemID, 1, &heigth);
									if (isUpdate)
									{
										MSG_UpdateItem UpdateIt;

										UpdateIt.ID = ESCENE_FIELD;
										UpdateIt.Type = _MSG_UpdateItem;

										UpdateIt.ItemID = ItemID + 10000;

										UpdateIt.Size = sizeof(MSG_UpdateItem);

										//UpdateIt.Height = heigth;

										UpdateIt.State = pItem[ItemID].State;

										GridMulticast(pItem[ItemID].PosX, pItem[ItemID].PosY, (MSG_STANDARD*)&UpdateIt, 0);

										pItem[ItemID].Delay = 0;
									}
								}

								else
									Log("err,no castle gato to open", "-system", 0);
							}
// endregion
// region Drops especiais - IMP - NYERDS - AGUA - REI - ZAKUM
							else
							{
// region Agua M
								//AguaM
								if (GenerateID >= WATER_M_INITIAL && GenerateID <= WATER_M_INITIAL + 7)
								{
									int CurrentNumMob = mNPCGen.pList[GenerateID].CurrentNumMob;

									if (CurrentNumMob == 1)
									{
										int Sala = GenerateID - WATER_M_INITIAL;

										if (WaterClear1[1][Sala] > 15)
											WaterClear1[1][Sala] = 15;


										int partyleader = pMob[conn].Leader;

										if (partyleader <= 0)
											partyleader = conn;

										int slot_free = -1;

										STRUCT_ITEM Perga;
										memset(&Perga, 0, sizeof(STRUCT_ITEM));

										Perga.sIndex = 778 + Sala;

										if (partyleader > 0 && partyleader < MAX_USER)
											PutItem(partyleader, &Perga);

										SendClientSignalParm(partyleader, ESCENE_FIELD, _MSG_StartTime, WaterClear1[1][Sala] * 2);

										for (int i = 0; i < MAX_PARTY; i++)
										{
											int partymember = pMob[partyleader].PartyList[i];

											if (pUser[partymember].Mode != USER_PLAY)
												continue;

											SendClientSignalParm(partymember, ESCENE_FIELD, _MSG_StartTime, WaterClear1[1][Sala] * 2);
										}
									}
								}

								else if (GenerateID >= WATER_M_INITIAL + 8 && GenerateID <= WATER_M_INITIAL + 11)
								{
									int CurrentNumMob = mNPCGen.pList[GenerateID].CurrentNumMob;

									if (CurrentNumMob == 1)
									{
										int Sala = 9;

										if (WaterClear1[1][Sala] > 5)
											WaterClear1[1][Sala] = 5;


										int partyleader = pMob[conn].Leader;

										if (partyleader <= 0)
											partyleader = conn;

										SendClientSignalParm(partyleader, ESCENE_FIELD, _MSG_StartTime, WaterClear1[1][Sala] * 2);

										for (int i = 0; i < MAX_PARTY; i++)
										{
											int partymember = pMob[partyleader].PartyList[i];

											if (pUser[partymember].Mode != USER_PLAY)
												continue;

											SendClientSignalParm(partymember, ESCENE_FIELD, _MSG_StartTime, WaterClear1[1][Sala] * 2);
										}
									}
								}
// endregion
// region Agua N
								else if (GenerateID >= WATER_N_INITIAL && GenerateID <= WATER_N_INITIAL + 7)
								{
									int CurrentNumMob = mNPCGen.pList[GenerateID].CurrentNumMob;

									if (CurrentNumMob == 1)
									{
										int Sala = GenerateID - WATER_N_INITIAL;

										if (WaterClear1[0][Sala] > 15)
											WaterClear1[0][Sala] = 15;


										int partyleader = pMob[conn].Leader;

										if (partyleader <= 0)
											partyleader = conn;

										int slot_free = -1;

										STRUCT_ITEM Perga;
										memset(&Perga, 0, sizeof(STRUCT_ITEM));

										Perga.sIndex = 3174 + Sala;

										if (partyleader > 0 && partyleader < MAX_USER)
											PutItem(partyleader, &Perga);

										SendClientSignalParm(partyleader, ESCENE_FIELD, _MSG_StartTime, WaterClear1[0][Sala] * 2);

										for (int i = 0; i < MAX_PARTY; i++)
										{
											int partymember = pMob[partyleader].PartyList[i];

											if (pUser[partymember].Mode != USER_PLAY)
												continue;

											SendClientSignalParm(partymember, ESCENE_FIELD, _MSG_StartTime, WaterClear1[0][Sala] * 2);
										}
									}
								}

								else if (GenerateID >= WATER_N_INITIAL + 8 && GenerateID <= WATER_N_INITIAL + 11)
								{
									int CurrentNumMob = mNPCGen.pList[GenerateID].CurrentNumMob;

									if (CurrentNumMob == 1)
									{
										int Sala = 9;

										if (WaterClear1[0][Sala] > 5)
											WaterClear1[0][Sala] = 5;


										int partyleader = pMob[conn].Leader;

										if (partyleader <= 0)
											partyleader = conn;

										SendClientSignalParm(partyleader, ESCENE_FIELD, _MSG_StartTime, WaterClear1[0][Sala] * 2);

										for (int i = 0; i < MAX_PARTY; i++)
										{
											int partymember = pMob[partyleader].PartyList[i];

											if (pUser[partymember].Mode != USER_PLAY)
												continue;

											SendClientSignalParm(partymember, ESCENE_FIELD, _MSG_StartTime, WaterClear1[0][Sala] * 2);
										}
									}
								}
// endregion
// region Agua A
								else if (GenerateID >= WATER_A_INITIAL && GenerateID <= WATER_A_INITIAL + 7)
								{
									int CurrentNumMob = mNPCGen.pList[GenerateID].CurrentNumMob;

									if (CurrentNumMob == 1)
									{
										int Sala = GenerateID - WATER_A_INITIAL;

										if (WaterClear1[2][Sala] > 15)
											WaterClear1[2][Sala] = 15;


										int partyleader = pMob[conn].Leader;

										if (partyleader <= 0)
											partyleader = conn;

										int slot_free = -1;

										STRUCT_ITEM Perga;
										memset(&Perga, 0, sizeof(STRUCT_ITEM));

										Perga.sIndex = 3183 + Sala;

										if (partyleader > 0 && partyleader < MAX_USER)
											PutItem(partyleader, &Perga);

										SendClientSignalParm(partyleader, ESCENE_FIELD, _MSG_StartTime, WaterClear1[2][Sala] * 2);

										for (int i = 0; i < MAX_PARTY; i++)
										{
											int partymember = pMob[partyleader].PartyList[i];

											if (pUser[partymember].Mode != USER_PLAY)
												continue;

											SendClientSignalParm(partymember, ESCENE_FIELD, _MSG_StartTime, WaterClear1[2][Sala] * 2);
										}
									}
								}

								else if (GenerateID >= WATER_A_INITIAL + 8 && GenerateID <= WATER_A_INITIAL + 11)
								{
									int CurrentNumMob = mNPCGen.pList[GenerateID].CurrentNumMob;

									if (CurrentNumMob == 1)
									{
										int Sala = 9;

										if (WaterClear1[2][Sala] > 5)
											WaterClear1[2][Sala] = 5;


										int partyleader = pMob[conn].Leader;

										if (partyleader <= 0)
											partyleader = conn;

										SendClientSignalParm(partyleader, ESCENE_FIELD, _MSG_StartTime, WaterClear1[2][Sala] * 2);

										for (int i = 0; i < MAX_PARTY; i++)
										{
											int partymember = pMob[partyleader].PartyList[i];

											if (pUser[partymember].Mode != USER_PLAY)
												continue;

											SendClientSignalParm(partymember, ESCENE_FIELD, _MSG_StartTime, WaterClear1[2][Sala] * 2);
										}
									}
								}
// endregion
								CCastleZakum::MobKilled(target, conn, PosX, PosY);
								// CEncampment::MobKilled(target, conn, PosX, PosY);

								if (GenerateID == ORC_GUERREIRO)
									DoRecall(conn);

								else if (GenerateID >= TORRE_NOATUM1 && GenerateID <= TORRE_NOATUM3)
									LiveTower[GenerateID - TORRE_NOATUM1] = 0;
								

								else if (GenerateID == REI_HARABARD)
								{
									Kingdom1Clear = 1;
									SendNoticeArea(g_pMessageStringTable[_NN_King1_Killed], 1676, 1556, 1776, 1636);
								}
								else if (GenerateID == REI_GLANTUAR)
								{
									Kingdom2Clear = 1;
									SendNoticeArea(g_pMessageStringTable[_NN_King2_Killed], 1676, 1816, 1776, 1892);
								}
// region Boss R Lac R Ori Circulo divino
								else if (GenerateID == 0 || GenerateID == 1 || GenerateID == 2)
								{
									STRUCT_ITEM item;
									memset(&item, 0, sizeof(STRUCT_ITEM));

									int _rand = W2Rand() % 14;

									if (_rand == 0)
										item.sIndex = 419;

									else if (_rand == 1)
										item.sIndex = 420;

									if (item.sIndex)
									{
										SetItemBonus(&item, 0, 0, 0);

										PutItem(conn, &item);
									}
								}

								else if (GenerateID == 5 || GenerateID == 6 || GenerateID == 7)
								{
									STRUCT_ITEM item;
									memset(&item, 0, sizeof(STRUCT_ITEM));

									int _rand = W2Rand() % 14;

									if (_rand == 0)
										item.sIndex = 421 + (W2Rand() % 7);

									else if (_rand == 1)
										item.sIndex = 419;

									if (item.sIndex)
									{
										SetItemBonus(&item, 0, 0, 0);

										if (pHeightGrid[AlvoY][AlvoX] > -40 && pHeightGrid[AlvoY][AlvoX] < 36)
											PutItem(conn, &item);
										//CreateItem(AlvoX, AlvoY, &item, W2Rand() % 4, 1);
									}
								}
// endregion

								else if (GenerateID == 3)
								{
									STRUCT_ITEM item;
									memset(&item, 0, sizeof(STRUCT_ITEM));

									int _rand = W2Rand() % 7;

									if (_rand == 0)
										item.sIndex = 1106;

									else if (_rand == 1)
										item.sIndex = 1256;

									else if (_rand == 2)
										item.sIndex = 1418;

									else if (_rand == 3)
										item.sIndex = 1568;

									if (item.sIndex)
									{
										SetItemBonus(&item, 75, 1, 0);

										if (pHeightGrid[AlvoY][AlvoX] > -40 && pHeightGrid[AlvoY][AlvoX] < 36)
											PutItem(conn, &item);
										//CreateItem(AlvoX, AlvoY, &item, W2Rand() % 4, 1);
									}
								}
// region Pesas

								else if (GenerateIndex >= NIGHTMARE_M_INITIAL && GenerateIndex <= NIGHTMARE_M_END)
									GenerateMob(GenerateIndex, 0, 0);//Pesa M

								else if (GenerateIndex >= NIGHTMARE_N_INITIAL && GenerateIndex <= NIGHTMARE_N_END)
									GenerateMob(GenerateIndex, 0, 0);//Pesa N

								else if (GenerateIndex >= NIGHTMARE_A_INITIAL && GenerateIndex <= NIGHTMARE_A_END)
									GenerateMob(GenerateIndex, 0, 0);//Pesa A
// endregion
#pragma region Carta de duelo N
								//Carta N
								if (GenerateID >= SECRET_ROOM_N_SALA1_MOB_1 && GenerateID <= SECRET_ROOM_N_SALA1_MOB_2)
								{
									int CurrentNumMob = mNPCGen.pList[SECRET_ROOM_N_SALA1_MOB_1].CurrentNumMob;
									CurrentNumMob += mNPCGen.pList[SECRET_ROOM_N_SALA1_MOB_2].CurrentNumMob;

									if (CurrentNumMob == 1)
									{
										CartaTime = 60;

										ClearAreaTeleport(778, 3652, 832, 3698, CartaPos[1][0], CartaPos[1][1]);

										CartaSala++;

										MSG_STANDARDPARM sm;

										sm.Type = _MSG_StartTime;
										sm.ID = ESCENE_FIELD;
										sm.Size = sizeof(MSG_STANDARDPARM);
										sm.Parm = CartaTime * 2;

										MapaMulticast(6, 28, (MSG_STANDARD*)&sm, 0);
									}
								}

								if (GenerateID >= SECRET_ROOM_N_SALA2_MOB_1 && GenerateID <= SECRET_ROOM_N_SALA2_MOB_2)
								{
									int CurrentNumMob = mNPCGen.pList[SECRET_ROOM_N_SALA2_MOB_1].CurrentNumMob;
									CurrentNumMob += mNPCGen.pList[SECRET_ROOM_N_SALA2_MOB_2].CurrentNumMob;

									if (CurrentNumMob == 1)
									{
										CartaTime = 60;

										ClearAreaTeleport(836, 3652, 890, 3698, CartaPos[2][0], CartaPos[2][1]);

										CartaSala++;

										MSG_STANDARDPARM sm;

										sm.Type = _MSG_StartTime;
										sm.ID = ESCENE_FIELD;
										sm.Size = sizeof(MSG_STANDARDPARM);
										sm.Parm = CartaTime * 2;

										MapaMulticast(6, 28, (MSG_STANDARD*)&sm, 0);
									}
								}

								if (GenerateID >= SECRET_ROOM_N_SALA3_MOB_1 && GenerateID <= SECRET_ROOM_N_SALA3_MOB_2)
								{
									int CurrentNumMob = mNPCGen.pList[SECRET_ROOM_N_SALA3_MOB_1].CurrentNumMob;
									CurrentNumMob += mNPCGen.pList[SECRET_ROOM_N_SALA3_MOB_2].CurrentNumMob;

									if (CurrentNumMob == 1)
									{
										CartaTime = 60;

										ClearAreaTeleport(834, 3595, 889, 3645, CartaPos[3][0], CartaPos[3][1]);

										CartaSala++;

										MSG_STANDARDPARM sm;

										sm.Type = _MSG_StartTime;
										sm.ID = ESCENE_FIELD;
										sm.Size = sizeof(MSG_STANDARDPARM);
										sm.Parm = CartaTime * 2;

										MapaMulticast(6, 28, (MSG_STANDARD*)&sm, 0);
									}
								}

								if (GenerateID >= SECRET_ROOM_N_SALA4_MOB_1 && GenerateID <= SECRET_ROOM_N_SALA4_MOB_4)
								{
									int CurrentNumMob = mNPCGen.pList[SECRET_ROOM_N_SALA4_MOB_1].CurrentNumMob;
									CurrentNumMob += mNPCGen.pList[SECRET_ROOM_N_SALA4_MOB_2].CurrentNumMob;
									CurrentNumMob += mNPCGen.pList[SECRET_ROOM_N_SALA4_MOB_3].CurrentNumMob;
									CurrentNumMob += mNPCGen.pList[SECRET_ROOM_N_SALA4_MOB_4].CurrentNumMob;

									if (CurrentNumMob == 1)
									{
										//ClearArea(776, 3595, 834, 3648);
										CartaTime = 3;
										CartaSala = 4;
									}
								}
#pragma endregion
#pragma region Carta de duelo M
								//Carta M
								if (GenerateID >= SECRET_ROOM_M_SALA1_MOB_1 && GenerateID <= SECRET_ROOM_M_SALA1_MOB_2)
								{
									int CurrentNumMob = mNPCGen.pList[SECRET_ROOM_M_SALA1_MOB_1].CurrentNumMob;
									CurrentNumMob += mNPCGen.pList[SECRET_ROOM_M_SALA1_MOB_2].CurrentNumMob;

									if (CurrentNumMob == 1)
									{
										CartaTime = 60;

										ClearAreaTeleport(778, 3652, 832, 3698, CartaPos[1][0], CartaPos[1][1]);

										CartaSala++;

										MSG_STANDARDPARM sm;

										sm.Type = _MSG_StartTime;
										sm.ID = ESCENE_FIELD;
										sm.Size = sizeof(MSG_STANDARDPARM);
										sm.Parm = CartaTime * 2;

										MapaMulticast(6, 28, (MSG_STANDARD*)&sm, 0);
									}
								}

								if (GenerateID >= SECRET_ROOM_M_SALA2_MOB_1 && GenerateID <= SECRET_ROOM_M_SALA2_MOB_2)
								{
									int CurrentNumMob = mNPCGen.pList[SECRET_ROOM_M_SALA2_MOB_1].CurrentNumMob;
									CurrentNumMob += mNPCGen.pList[SECRET_ROOM_M_SALA2_MOB_2].CurrentNumMob;

									if (CurrentNumMob == 1)
									{
										CartaTime = 60;

										ClearAreaTeleport(836, 3652, 890, 3698, CartaPos[2][0], CartaPos[2][1]);

										CartaSala++;

										MSG_STANDARDPARM sm;

										sm.Type = _MSG_StartTime;
										sm.ID = ESCENE_FIELD;
										sm.Size = sizeof(MSG_STANDARDPARM);
										sm.Parm = CartaTime * 2;

										MapaMulticast(6, 28, (MSG_STANDARD*)&sm, 0);
									}
								}

								if (GenerateID >= SECRET_ROOM_M_SALA3_MOB_1 && GenerateID <= SECRET_ROOM_M_SALA3_MOB_2)
								{
									int CurrentNumMob = mNPCGen.pList[SECRET_ROOM_M_SALA3_MOB_1].CurrentNumMob;
									CurrentNumMob += mNPCGen.pList[SECRET_ROOM_M_SALA3_MOB_2].CurrentNumMob;

									if (CurrentNumMob == 1)
									{
										CartaTime = 60;

										ClearAreaTeleport(834, 3595, 889, 3645, CartaPos[3][0], CartaPos[3][1]);

										CartaSala++;

										MSG_STANDARDPARM sm;

										sm.Type = _MSG_StartTime;
										sm.ID = ESCENE_FIELD;
										sm.Size = sizeof(MSG_STANDARDPARM);
										sm.Parm = CartaTime * 2;

										MapaMulticast(6, 28, (MSG_STANDARD*)&sm, 0);
									}
								}

								if (GenerateID >= SECRET_ROOM_M_SALA4_MOB_1 && GenerateID <= SECRET_ROOM_M_SALA4_MOB_4)
								{
									int CurrentNumMob = mNPCGen.pList[SECRET_ROOM_M_SALA4_MOB_1].CurrentNumMob;
									CurrentNumMob += mNPCGen.pList[SECRET_ROOM_M_SALA4_MOB_2].CurrentNumMob;
									CurrentNumMob += mNPCGen.pList[SECRET_ROOM_M_SALA4_MOB_3].CurrentNumMob;
									CurrentNumMob += mNPCGen.pList[SECRET_ROOM_M_SALA4_MOB_4].CurrentNumMob;

									if (CurrentNumMob == 1)
									{
										//ClearArea(776, 3595, 834, 3648);
										CartaTime = 3;
										CartaSala = 4;
									}
								}
#pragma endregion
#pragma region Carta de duelo A
								//Carta A
								if (GenerateID >= SECRET_ROOM_A_SALA1_MOB_1 && GenerateID <= SECRET_ROOM_A_SALA1_MOB_2)
								{
									int CurrentNumMob = mNPCGen.pList[SECRET_ROOM_A_SALA1_MOB_1].CurrentNumMob;
									CurrentNumMob += mNPCGen.pList[SECRET_ROOM_A_SALA1_MOB_2].CurrentNumMob;

									if (CurrentNumMob == 1)
									{
										CartaTime = 60;

										ClearAreaTeleport(778, 3652, 832, 3698, CartaPos[1][0], CartaPos[1][1]);

										CartaSala++;

										MSG_STANDARDPARM sm;

										sm.Type = _MSG_StartTime;
										sm.ID = ESCENE_FIELD;
										sm.Size = sizeof(MSG_STANDARDPARM);
										sm.Parm = CartaTime * 2;

										MapaMulticast(6, 28, (MSG_STANDARD*)&sm, 0);
									}
								}

								if (GenerateID >= SECRET_ROOM_A_SALA2_MOB_1 && GenerateID <= SECRET_ROOM_A_SALA2_MOB_2)
								{
									int CurrentNumMob = mNPCGen.pList[SECRET_ROOM_A_SALA2_MOB_1].CurrentNumMob;
									CurrentNumMob += mNPCGen.pList[SECRET_ROOM_A_SALA2_MOB_2].CurrentNumMob;

									if (CurrentNumMob == 1)
									{
										CartaTime = 60;

										ClearAreaTeleport(836, 3652, 890, 3698, CartaPos[2][0], CartaPos[2][1]);

										CartaSala++;

										MSG_STANDARDPARM sm;

										sm.Type = _MSG_StartTime;
										sm.ID = ESCENE_FIELD;
										sm.Size = sizeof(MSG_STANDARDPARM);
										sm.Parm = CartaTime * 2;

										MapaMulticast(6, 28, (MSG_STANDARD*)&sm, 0);
									}
								}

								if (GenerateID >= SECRET_ROOM_A_SALA3_MOB_1 && GenerateID <= SECRET_ROOM_A_SALA3_MOB_2)
								{
									int CurrentNumMob = mNPCGen.pList[SECRET_ROOM_A_SALA3_MOB_1].CurrentNumMob;
									CurrentNumMob += mNPCGen.pList[SECRET_ROOM_A_SALA3_MOB_2].CurrentNumMob;

									if (CurrentNumMob == 1)
									{
										CartaTime = 60;

										ClearAreaTeleport(834, 3595, 889, 3645, CartaPos[3][0], CartaPos[3][1]);

										CartaSala++;

										MSG_STANDARDPARM sm;

										sm.Type = _MSG_StartTime;
										sm.ID = ESCENE_FIELD;
										sm.Size = sizeof(MSG_STANDARDPARM);
										sm.Parm = CartaTime * 2;

										MapaMulticast(6, 28, (MSG_STANDARD*)&sm, 0);
									}
								}

								if (GenerateID >= SECRET_ROOM_A_SALA4_MOB_1 && GenerateID <= SECRET_ROOM_A_SALA4_MOB_4)
								{
									int CurrentNumMob = mNPCGen.pList[SECRET_ROOM_A_SALA4_MOB_1].CurrentNumMob;
									CurrentNumMob += mNPCGen.pList[SECRET_ROOM_A_SALA4_MOB_2].CurrentNumMob;
									CurrentNumMob += mNPCGen.pList[SECRET_ROOM_A_SALA4_MOB_3].CurrentNumMob;
									CurrentNumMob += mNPCGen.pList[SECRET_ROOM_A_SALA4_MOB_4].CurrentNumMob;

									if (CurrentNumMob == 1)
									{
										//ClearArea(776, 3595, 834, 3648);
										CartaTime = 3;
										CartaSala = 4;
									}
								}
#pragma endregion
#pragma region Pista de Runas
#pragma region Pista Nv0 Lich_Crunt
								if (GenerateID == RUNEQUEST_LV0_LICH1 || GenerateID == RUNEQUEST_LV0_LICH2)
								{
									int _rand = W2Rand() % 100;

									if (_rand < 20)
									{
										for (int c = MAX_USER; c < MAX_MOB; c++)
										{
											if (pMob[c].Mode == USER_EMPTY)
												continue;

											if (pMob[c].GenerateIndex != RUNEQUEST_LV0_LICH1 && pMob[c].GenerateIndex != RUNEQUEST_LV0_LICH2)
												continue;

											DeleteMob(c, 3);
										}

										int partyleader = pMob[conn].Leader;

										if (partyleader <= 0)
											partyleader = conn;

										STRUCT_ITEM Runa;
										memset(&Runa, 0, sizeof(STRUCT_ITEM));

										Runa.sIndex = PistaRune[0][W2Rand() % 4];

										if (partyleader > 0 && partyleader < MAX_USER)
											PutItem(partyleader, &Runa);

										for (int i = 0; i < MAX_PARTY; i++)
										{
											int partymember = pMob[partyleader].PartyList[i];

											if (pUser[partymember].Mode != USER_PLAY)
												continue;

											Runa.sIndex = PistaRune[0][W2Rand() % 4];
											PutItem(partymember, &Runa);
										}

										STRUCT_ITEM NextPista;
										memset(&NextPista, 0, sizeof(STRUCT_ITEM));

										NextPista.sIndex = 5134;
										NextPista.stEffect[0].cEffect = 43;
										NextPista.stEffect[0].cValue = 1;

										PutItem(partyleader, &NextPista);

										sprintf(temp, "etc,questRune +0 complete leader:%s", pMob[partyleader].MOB.MobName);
										Log(temp, "-system", 0);
									}
									else
									{
										for (int c = MAX_USER; c < MAX_MOB; c++)
										{
											if (pMob[c].Mode == USER_EMPTY)
												continue;

											if (pMob[c].GenerateIndex != RUNEQUEST_LV0_LICH1 && pMob[c].GenerateIndex != RUNEQUEST_LV0_LICH2)
												continue;

											DeleteMob(c, 3);
										}

										GenerateMob(RUNEQUEST_LV0_LICH1, 0, 0);
										GenerateMob(RUNEQUEST_LV0_LICH1, 0, 0);

										GenerateMob(RUNEQUEST_LV0_LICH2, 0, 0);
										GenerateMob(RUNEQUEST_LV0_LICH2, 0, 0);
									}
								}
#pragma endregion
#pragma region Pista Nv1 Torre
								if (GenerateID == RUNEQUEST_LV1_TORRE1 || GenerateID == RUNEQUEST_LV1_TORRE2 || GenerateID == RUNEQUEST_LV1_TORRE3)
								{
									int party = GenerateID - RUNEQUEST_LV1_TORRE1;

									Pista[1].Party[party].MobCount = 0;
								}

								if (GenerateID >= RUNEQUEST_LV1_MOB_INITIAL && GenerateID <= RUNEQUEST_LV1_MOB_END)
								{
									int partyleader = pMob[conn].Leader;

									if (partyleader <= 0)
										partyleader = conn;

									if (mNPCGen.pList[RUNEQUEST_LV1_TORRE1].CurrentNumMob >= 1 && Pista[1].Party[0].LeaderID == partyleader)
										Pista[1].Party[0].MobCount++;

									else if (mNPCGen.pList[RUNEQUEST_LV1_TORRE2].CurrentNumMob >= 1 && Pista[1].Party[1].LeaderID == partyleader)
										Pista[1].Party[1].MobCount++;

									else if (mNPCGen.pList[RUNEQUEST_LV1_TORRE3].CurrentNumMob >= 1 && Pista[1].Party[2].LeaderID == partyleader)
										Pista[1].Party[2].MobCount++;
								}
#pragma endregion
#pragma region Pista Nv2 Amon
								if (GenerateID == RUNEQUEST_LV2_MOB_BOSS)
								{
									int partyleader = pMob[conn].Leader;

									if (partyleader <= 0)
										partyleader = conn;

									STRUCT_ITEM Runa;
									memset(&Runa, 0, sizeof(STRUCT_ITEM));

									Runa.sIndex = PistaRune[2][W2Rand() % 5];

									if (partyleader > 0 && partyleader < MAX_USER)
										PutItem(partyleader, &Runa);

									for (int i = 0; i < MAX_PARTY; i++)
									{
										int partymember = pMob[partyleader].PartyList[i];

										if (pUser[partymember].Mode != USER_PLAY)
											continue;

										Runa.sIndex = PistaRune[2][W2Rand() % 5];
										PutItem(partymember, &Runa);
									}

									STRUCT_ITEM NextPista;
									memset(&NextPista, 0, sizeof(STRUCT_ITEM));

									NextPista.sIndex = 5134;
									NextPista.stEffect[0].cEffect = 43;
									NextPista.stEffect[0].cValue = 3;

									PutItem(partyleader, &NextPista);

									sprintf(temp, "etc,questRune +2 complete leader:%s", pMob[partyleader].MOB.MobName);
									Log(temp, "-system", 0);
								}
#pragma endregion
#pragma region Pista Nv3 Sulrang
								if (GenerateID >= RUNEQUEST_LV3_MOB_SULRANG_INITIAL && GenerateID <= RUNEQUEST_LV3_MOB_SULRANG_END && Pista[3].Party[0].LeaderID != 0 && Pista[3].Party[0].MobCount == 0)
								{
									Pista[3].Party[0].MobCount = 1;

									GenerateMob(RUNEQUEST_LV3_MOB_BOSS_INITIAL + W2Rand() % 7, 0, 0);
								}
#pragma endregion
#pragma region Pista Nv4 Labirinto
								if (GenerateID >= RUNEQUEST_LV4_MOB_INITIAL && GenerateID <= RUNEQUEST_LV4_MOB_END && mNPCGen.pList[GenerateID].CurrentNumMob == 1)
								{
									int leader = pMob[conn].Leader;

									int partyleader = pMob[conn].Leader;

									if (partyleader <= 0)
										partyleader = conn;

									if (partyleader == Pista[4].Party[0].LeaderID && Pista[4].Party[1].MobCount >= 1 && Pista[4].Party[2].MobCount >= 1)
									{
										Pista[4].Party[0].MobCount--;

										if (Pista[4].Party[0].MobCount == 0)
										{
											DoTeleport(partyleader, 3351, 1334);

											for (int i = 0; i < MAX_PARTY; i++)
											{
												int partyconn = pMob[partyleader].PartyList[i];

												if (partyconn > 0 && partyconn < MAX_USER && partyconn != partyleader && pUser[partyconn].Mode == USER_PLAY)
													DoTeleport(partyconn, 3352, 1334);
											}

											GenerateMob(RUNEQUEST_LV4_MOB_BOSS, 0, 0);
										}
									}
									else if (partyleader == Pista[4].Party[1].LeaderID && Pista[4].Party[0].MobCount >= 1 && Pista[4].Party[2].MobCount >= 1)
									{
										Pista[4].Party[1].MobCount--;

										if (Pista[4].Party[1].MobCount == 0)
										{
											DoTeleport(partyleader, 3351, 1334);

											for (int i = 0; i < MAX_PARTY; i++)
											{
												int partyconn = pMob[partyleader].PartyList[i];

												if (partyconn > 0 && partyconn < MAX_USER && partyconn != partyleader && pUser[partyconn].Mode == USER_PLAY)
													DoTeleport(partyconn, 3351, 1334);
											}

											GenerateMob(RUNEQUEST_LV4_MOB_BOSS, 0, 0);
										}
									}
									else if (partyleader == Pista[4].Party[2].LeaderID && Pista[4].Party[1].MobCount >= 1 && Pista[4].Party[0].MobCount >= 1)
									{
										Pista[4].Party[2].MobCount--;

										if (Pista[4].Party[2].MobCount == 0)
										{
											DoTeleport(partyleader, 3351, 1334);

											for (int i = 0; i < MAX_PARTY; i++)
											{
												int partyconn = pMob[partyleader].PartyList[i];

												if (partyconn > 0 && partyconn < MAX_USER && partyconn != partyleader && pUser[partyconn].Mode == USER_PLAY)
													DoTeleport(partyconn, 3351, 1334);
											}

											GenerateMob(RUNEQUEST_LV4_MOB_BOSS, 0, 0);
										}
									}
								}

								if (GenerateID == RUNEQUEST_LV4_MOB_BOSS)
								{
									int partyleader = pMob[conn].Leader;

									if (partyleader <= 0)
										partyleader = conn;

									STRUCT_ITEM Runa;
									memset(&Runa, 0, sizeof(STRUCT_ITEM));

									Runa.sIndex = PistaRune[4][W2Rand() % 3];

									if (partyleader > 0 && partyleader < MAX_USER)
										PutItem(partyleader, &Runa);

									for (int i = 0; i < MAX_PARTY; i++)
									{
										int partymember = pMob[partyleader].PartyList[i];

										if (pUser[partymember].Mode != USER_PLAY)
											continue;

										Runa.sIndex = PistaRune[4][W2Rand() % 3];
										PutItem(partymember, &Runa);
									}

									STRUCT_ITEM NextPista;
									memset(&NextPista, 0, sizeof(STRUCT_ITEM));

									NextPista.sIndex = 5134;
									NextPista.stEffect[0].cEffect = 43;
									NextPista.stEffect[0].cValue = 5;

									PutItem(partyleader, &NextPista);

									sprintf(temp, "etc,questRune +4 complete leader:%s", pMob[partyleader].MOB.MobName);
									Log(temp, "-system", 0);
								}
#pragma endregion
#pragma region Pista Nv5 Balrog
								if (GenerateID == RUNEQUEST_LV5_MOB_BOSS)
								{
									Pista[5].Party[0].MobCount = 1;

									int partyleader = pMob[conn].Leader;

									if (partyleader <= 0)
										partyleader = conn;

									STRUCT_ITEM Runa;
									memset(&Runa, 0, sizeof(STRUCT_ITEM));

									Runa.sIndex = PistaRune[5][W2Rand() % 6];

									if (partyleader > 0 && partyleader < MAX_USER)
										PutItem(partyleader, &Runa);

									for (int i = 0; i < MAX_PARTY; i++)
									{
										int partymember = pMob[partyleader].PartyList[i];

										if (pUser[partymember].Mode != USER_PLAY)
											continue;

										Runa.sIndex = PistaRune[5][W2Rand() % 6];
										PutItem(partymember, &Runa);
									}

									STRUCT_ITEM NextPista;
									memset(&NextPista, 0, sizeof(STRUCT_ITEM));

									NextPista.sIndex = 5134;
									NextPista.stEffect[0].cEffect = 43;
									NextPista.stEffect[0].cValue = 6;

									PutItem(partyleader, &NextPista);

									sprintf(temp, "etc,questRune +5 complete leader:%s", pMob[partyleader].MOB.MobName);
									Log(temp, "-system", 0);
								}
#pragma endregion
#pragma region Pista Nv6 Coelho
								if (GenerateID >= RUNEQUEST_LV6_MOB_INITIAL && GenerateID <= RUNEQUEST_LV6_MOB_END)
								{
									if (Pista[6].Party[0].MobCount == 1)
									{
										Pista[6].Party[0].MobCount--;

										GenerateMob(RUNEQUEST_LV6_MOB_BOSS, 0, 0);
									}

									else if (Pista[6].Party[0].MobCount > 0)
										Pista[6].Party[0].MobCount--;
								}

								if (GenerateID == RUNEQUEST_LV6_MOB_BOSS)
								{
									int partyleader = pMob[conn].Leader;

									if (partyleader <= 0)
										partyleader = conn;

									STRUCT_ITEM Runa;
									memset(&Runa, 0, sizeof(STRUCT_ITEM));

									Runa.sIndex = PistaRune[6][W2Rand() % 8];

									if (partyleader > 0 && partyleader < MAX_USER)
										PutItem(partyleader, &Runa);

									for (int i = 0; i < MAX_PARTY; i++)
									{
										int partymember = pMob[partyleader].PartyList[i];

										if (pUser[partymember].Mode != USER_PLAY)
											continue;

										Runa.sIndex = PistaRune[6][W2Rand() % 8];
										PutItem(partymember, &Runa);
									}

									sprintf(temp, "etc,questRune +6 complete leader:%s", pMob[partyleader].MOB.MobName);
									Log(temp, "-system", 0);
								}
#pragma endregion
#pragma endregion
#pragma region RvR
								if (GenerateID == RVRTORRE_1 || GenerateID == RVRTORRE_2)
								{
									ClearArea(1020, 1916, 1286, 2178);

									for (int i = MAX_USER; i < MAX_MOB; i++)
									{
										if (pMob[i].GenerateIndex == RVRTORRE_1 || pMob[i].GenerateIndex == RVRTORRE_2)
											DeleteMob(i, 1);
									}

									if (pMob[conn].MOB.Clan)
									{
										if (pMob[conn].MOB.Clan == 8)
											sprintf(temp, g_pMessageStringTable[_SN_KINGDOMWAR_DROP_], g_pMessageStringTable[_NN_KINGDOM_RED]);

										else if (pMob[conn].MOB.Clan == 7)
											sprintf(temp, g_pMessageStringTable[_SN_KINGDOMWAR_DROP_], g_pMessageStringTable[_NN_KINGDOM_BLUE]);

										SendNotice(temp);
										RvRBonus = pMob[conn].MOB.Clan;
									}

									sprintf(temp, "etc,RvR clan winner:%d", pMob[conn].MOB.Clan);
									Log(temp, "-system", 0);

									RvRState = 0;
								}
#pragma endregion
								
							}
// endregion
#pragma region Drop Gold
							int MobCoin = pMob[target].MOB.Coin;
							int UNKGOLD = 18;

							if (pMob[target].MOB.BaseScore.Level < 10)
								UNKGOLD = 2;
							else if (pMob[target].MOB.BaseScore.Level < 20)
								UNKGOLD = 4;
							else if (pMob[target].MOB.BaseScore.Level < 30)
								UNKGOLD = 6;
							else if (pMob[target].MOB.BaseScore.Level < 50)
								UNKGOLD = 9;

							UNKGOLD = W2Rand() % (UNKGOLD+1);

							if (MobCoin && UNKGOLD == 0)
							{
								MobCoin = 4 * (W2Rand() % (((MobCoin + 1) / 4)+1) + (MobCoin + 1) / 4 + MobCoin);

								if (MobCoin > 2000)
									MobCoin = 2000;

								if (MobCoin + pMob[conn].MOB.Coin > 2000000000)
									SendClientMessage(conn, g_pMessageStringTable[_NN_Cant_get_more_than_2G]);
								else
								{
									pMob[conn].MOB.Coin += MobCoin;
									SendEtc(conn);
								}
							}
#pragma endregion
#pragma region Drop Evento em todos os mobs
							if (evOn && evStartIndex && evEndIndex && evItem && evRate && evCurrentIndex < evEndIndex && W2Rand() % evRate == 0)
							{
								STRUCT_ITEM item;
								memset(&item, 0, sizeof(STRUCT_ITEM));

								item.sIndex = evItem;

								if (evIndex)
								{
									item.stEffect[0].cEffect = 62;
									item.stEffect[0].cValue = evCurrentIndex / 256;
									item.stEffect[1].cEffect = 63;
									item.stEffect[1].cValue = evCurrentIndex;
									item.stEffect[2].cEffect = 59;
									item.stEffect[2].cValue = W2Rand();

									sprintf(temp, g_pMessageStringTable[_SSD_S_get_S_D], pMob[conn].MOB.MobName, g_pItemList[evItem].Name, evCurrentIndex);
								}
								else
									sprintf(temp, g_pMessageStringTable[_SSD_S_get_S], pMob[conn].MOB.MobName, g_pItemList[evItem].Name);

								if (evNotice)
									SendNotice(temp);

								evCurrentIndex++;

								SetItemBonus(&item, pMob[target].MOB.CurrentScore.Level, 0, 0);
								PutItem(conn, &item);
								DrawConfig(1);
							}
#pragma endregion
							int target_level = pMob[target].MOB.CurrentScore.Level;
#pragma region Drop comum
							for (int i = 0; i < MAX_CARRY; i++)
							{
								if (pMob[target].MOB.Carry[i].sIndex == 0)
									continue;

								int droprate = g_pDropRate[i];
								int dropbonus = g_pDropBonus[i] + pMob[conn].DropBonus;

								if (dropbonus != 100)
								{
									dropbonus = 10000 / (dropbonus + 1);
									droprate = dropbonus * droprate / 100;
								}

								int pos = i / 8;

								if (i < 60)
								{
									if (pos == 0 || pos == 1 || pos == 2)
									{
										if (target_level < 10)
											droprate = 4 * droprate / 100;

										else if (target_level < 20)
											droprate = 5 * droprate / 100;

										else if (target_level < 30)
											droprate = 6 * droprate / 100;

										else if (target_level < 40)
											droprate = 7 * droprate / 100;

										else if (target_level < 60)
											droprate = 8 * droprate / 100;

										else
											droprate = 99 * droprate / 100;
									}
								}

								else
								{
									if (target_level < 170)
										droprate = 90 * droprate / 100;

									else if (target_level < 200)
										droprate = 60 * droprate / 100;

									else if (target_level < 230)
										droprate = 50 * droprate / 100;

									else if (target_level < 255)
										droprate = 43 * droprate / 100;

									else if (target_level < 320)
										droprate = 38 * droprate / 100;

									else
										droprate = 50 * droprate / 100;
								}

								if (TESTSERVER)
									droprate /= 2;

								if (LOCALSERVER)
									droprate /= 100;

								if (pMob[conn].MOB.Rsv & 4)
								{
									int special2 = pMob[conn].MOB.CurrentScore.Special[2];
									special2 = special2 / 10 + 10;
									special2 = 100 - special2;
									droprate = special2 * droprate / 100;
								}

								if (i == 8 || i == 9 || i == 10)
									droprate = 4;

								if (i == 11)
									droprate = 1;

								if (droprate >= 32000)
									droprate = 32000;

								if (droprate <= 0)
									droprate = 0;
								else
									droprate = W2Rand() % droprate;

								if (droprate == 0 || i == 11)
								{
									STRUCT_ITEM *item = &pMob[target].MOB.Carry[i];

									if (item->sIndex <= 390 || item->sIndex >= MAX_ITEMLIST)
										continue;

									if (item->sIndex == 454)
										continue;

									int reqlv = g_pItemList[item->sIndex].nReqLvl;

									if (reqlv < 140 || (droprate % 2) != 1)
									{
										int bonus = 0;

										if (conn > 0 && conn < MAX_USER)
											bonus = pMob[conn].DropBonus;

										SetItemBonus(item, pMob[target].MOB.CurrentScore.Level, 0, bonus);


										if(CCastleZakum::KeyDrop(target, conn, PosX, PosY, item) == TRUE)
											PutItem(conn, item);

										sprintf(temp, "MobName:%s dropou o item: %s:%d %d.%d.%d.%d.%d.%d do mob:%s", pMob[conn].MOB.MobName, g_pItemList[item->sIndex].Name, item->sIndex, item->stEffect[0].cEffect, item->stEffect[0].cValue, item->stEffect[1].cEffect, item->stEffect[1].cValue, item->stEffect[2].cEffect, item->stEffect[2].cValue, pMob[target].MOB.MobName);

										if (conn > 0 && conn < MAX_USER)
											Log(temp, pUser[conn].AccountName, pUser[conn].IP);

										MSG_STANDARDPARM updateItemDayLog;
										memset(&updateItemDayLog, 0, sizeof(MSG_STANDARDPARM));

										updateItemDayLog.Size = sizeof(MSG_STANDARDPARM);
										updateItemDayLog.Type = _MSG_DBItemDayLog;
										updateItemDayLog.ID = 0;
										updateItemDayLog.Parm = item->sIndex;

										DBServerSocket.SendOneMessage((char*)&updateItemDayLog, sizeof(MSG_STANDARDPARM));
										

										if (LOCALSERVER)
										{
											int item_index = item->sIndex;

											int reqlv = g_pItemList[item_index].nReqLvl;
											int itempos = g_pItemList[item_index].nPos;

											if (itempos & 0xFE && itempos != 128 && conn < MAX_USER)
											{
												sprintf(temp, "%-12s - %s(%d:%d) %d(%d:%d)",
													pMob[target].MOB.MobName, g_pItemList[item_index].Name,
													pMob[target].MOB.CurrentScore.Level, reqlv, g_dLevel, g_dLevel1,
													g_dLevel2);

												SendSay(conn, temp);
											}
										}

										if (i == 8 || i == 9 || i == 10)
											i = 11;
									}
								}
							}
#pragma endregion
#pragma region Quest Amuleto mistico
							if (conn < MAX_USER && (pMob[target].MOB.Equip[0].sIndex == 239 || pMob[target].MOB.Equip[0].sIndex == 241) && !(W2Rand() % 20))
							{
								if (pMob[conn].extra.QuestInfo.Mortal.TerraMistica == 1)
								{
									SendClientMessage(conn, g_pMessageStringTable[_NN_Watching_Town_Success]);
									pMob[conn].extra.QuestInfo.Mortal.TerraMistica = 2;
								}
							}
#pragma endregion
#pragma region Drenagem de HP do mob com skill de HT
							/*							if (pMob[conn].MOB.Rsv & 2)
							{
							int mob_maxhp = pMob[target].MOB.CurrentScore.MaxHp;
							int myspecial = pMob[conn].MOB.CurrentScore.Special[3];

							mob_maxhp = myspecial + mob_maxhp / 30;
							int myhp = pMob[conn].MOB.CurrentScore.Hp;

							pMob[conn].MOB.CurrentScore.Hp += mob_maxhp;

							if (pMob[conn].MOB.CurrentScore.Hp > pMob[conn].MOB.CurrentScore.MaxHp)
							pMob[conn].MOB.CurrentScore.Hp = pMob[conn].MOB.CurrentScore.MaxHp;

							if (myhp != pMob[conn].MOB.CurrentScore.Hp)
							{
							if (conn > 0 && conn < MAX_USER)
							{
							SetReqHp(conn);
							SetReqMp(conn);
							}
							MSG_SetHpDam hpDam;

							hpDam.Type = _MSG_SetHpDam;
							hpDam.Size = sizeof(MSG_SetHpDam);
							hpDam.ID = conn;
							hpDam.Hp = pMob[conn].MOB.CurrentScore.Hp;
							hpDam.Dam = SetHpDamValue(pMob[conn].MOB.CurrentScore.Hp - myhp);

							GridMulticast(pMob[conn].TargetX, pMob[conn].TargetY, (MSG_STANDARD*)&hpDam, 0);
							}
							}*/
#pragma endregion
							DeleteMob(target, 1);

#pragma region Pista +1 - Lugefer_Inf
							if (GenerateID >= RUNEQUEST_LV1_MOB_INITIAL && GenerateID <= RUNEQUEST_LV1_MOB_END)
								GenerateMob(GenerateID, 0, 0);
#pragma endregion
#pragma region Pista +2 - Amon_Inf
							if (GenerateID >= RUNEQUEST_LV2_MOB_INITIAL && GenerateID <= RUNEQUEST_LV2_MOB_END)
								GenerateMob(GenerateID, 0, 0);
#pragma endregion
#pragma region Pista +3 - Kalintz_Mago
							if (GenerateID >= RUNEQUEST_LV3_MOB_BOSS_INITIAL && GenerateID <= RUNEQUEST_LV3_MOB_BOSS_END && Pista[3].Party[0].LeaderID != 0 && Pista[3].Party[0].MobCount != 0)
							{
								GenerateMob(RUNEQUEST_LV3_MOB_BOSS_INITIAL + W2Rand() % 7, 0, 0);

								int leader = pMob[conn].Leader;

								int partyleader = pMob[conn].Leader;

								if (partyleader <= 0)
									partyleader = conn;

								if (partyleader == Pista[3].Party[0].LeaderID)
									Pista[3].Party[0].MobCount++;

								else if (partyleader == Pista[3].Party[1].LeaderID)
									Pista[3].Party[1].MobCount++;

								else if (partyleader == Pista[3].Party[2].LeaderID)
									Pista[3].Party[2].MobCount++;
							}
#pragma endregion
#pragma region Kefra - Mago_Negro
							if (GenerateIndex >= KEFRA_MOB_INITIAL && GenerateIndex <= KEFRA_MOB_END && KefraLive == 0)
								GenerateMob(GenerateIndex, 0, 0);
#pragma endregion

							CWarTower::MobKilled(target, conn, PosX, PosY);
						}
						else
							DeleteMob(target, 1);
					}
