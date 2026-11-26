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

/// @file MobKilled_Exp.cpp
/// @brief Cálculo e distribuição de experiência PvE ao jogador e grupo em MobKilled().

#include "systems/MobKilledInternal.h"

bool ProcessMobKilledExp(int target, int conn, int Leader)
{
#pragma region Distribuição da EXP
				int MobExp = GetExpApply(pMob[conn].extra, (int)pMob[target].MOB.Exp, pMob[conn].MOB.CurrentScore.Level, pMob[target].MOB.CurrentScore.Level);
				int FinalExp = 0;

				// TODO: Change this UNK_s to something else.
				int UNK_1 = 30;
				int UNK_2 = 0;
				int UNK_3 = pMob[conn].extra.ClassMaster;

				int tx = pMob[conn].TargetX;
				int ty = pMob[conn].TargetY;

				int party = 0;


				if (UNK_3> 0 && UNK_3 <= MAX_PARTY)
				{
					int NumMob = g_EmptyMob + UNK_3;

					if (UNK_3 > 1)
						NumMob = NumMob + PARTYBONUS - 100;

					int eMob = MobExp;
					int isExp = NumMob * MobExp / 100;

					struct tm when;
					time_t now;
					time(&now);
					when = *localtime(&now);

					for (int i = 0; i < MAX_PARTY + 1; ++i)
					{
						party = 0;

						if (Leader && i < MAX_PARTY)
							party = pMob[Leader].PartyList[i];
						else
							party = Leader;
// region Pesa A
						if (party > 0 && party < MAX_USER && pMob[party].MOB.CurrentScore.Hp > 0 && (tx / 128) == 9 && (pMob[party].TargetX / 128) == 9
							&& (ty / 128) == 1 && (pMob[party].TargetY / 128) == 1)
						{
							isExp = GetExpApply(pMob[party].extra, (int)pMob[target].MOB.Exp, pMob[party].MOB.CurrentScore.Level, pMob[target].MOB.CurrentScore.Level);

							int myLevel = pMob[party].MOB.CurrentScore.Level;

							if (pMob[party].extra.ClassMaster != MORTAL && pMob[party].extra.ClassMaster != ARCH)
								myLevel += MAX_LEVEL + 1;

							int exp = (UNK_1 + myLevel) * isExp / (UNK_1 + myLevel);
							if (exp > 0 && exp <= 10000000)
							{
								if (pMob[party].extra.ClassMaster != MORTAL && pMob[party].extra.ClassMaster != ARCH)
								{
									if (myLevel < 120)
										exp /= 10;

									else if (myLevel < 150)
										exp /= 20;

									else if (myLevel < 170)
										exp /= 40;

									else if (myLevel < 180)
										exp /= 80;

									else if (myLevel < 190)
										exp /= 160;

									else
										exp /= 320;
								}
								exp = 6 * exp / 10;

								//if (exp > eMob)
								//	exp = eMob;

								if (pMob[conn].ExpBonus > 0 && pMob[conn].ExpBonus < 500)
									exp += exp * pMob[conn].ExpBonus / 100;

								if (RvRBonus && RvRBonus == pMob[party].MOB.Clan)
									exp += (exp * 10) / 100;

								if (NewbieEventServer && pMob[party].MOB.CurrentScore.Level < 100 && pMob[party].extra.ClassMaster != CELESTIAL && pMob[party].extra.ClassMaster != CELESTIALCS && pMob[party].extra.ClassMaster != SCELESTIAL)
									exp += exp / 4;

								if (DOUBLEMODE)
									exp *= 2;

								if (KefraLive == 0)
									exp /= 2;

								if (NewbieEventServer)
									exp += (exp * 15) / 100;
								else
									exp -= (exp * 15) / 100;

#pragma region Log de Experiência diário
								if (when.tm_yday != pMob[party].extra.DayLog.YearDay)
									pMob[party].extra.DayLog.Exp = 0;

								pMob[party].extra.DayLog.YearDay = when.tm_yday;
								pMob[party].extra.DayLog.Exp += exp;
#pragma endregion

								if (pMob[party].extra.Hold > 0 && (unsigned int)exp >= pMob[party].extra.Hold)
								{
									int nhold = pMob[party].extra.Hold - exp;

									if (nhold < 0)
										nhold = 0;

									exp -= pMob[party].extra.Hold;
									pMob[party].extra.Hold = nhold;
								}

								else if (pMob[party].extra.Hold > 0 && (unsigned int)exp < pMob[party].extra.Hold)
								{
									pMob[party].extra.Hold -= exp;
									continue;
								}

								pMob[party].MOB.Exp += exp;

#pragma region RankingUpdateExp
								STRUCT_RANKING rankInfo = STRUCT_RANKING(pMob[party].MOB.MobName, pMob[party].MOB.Exp, pMob[party].extra.ClassMaster, pMob[party].MOB.CurrentScore.Level, pMob[party].MOB.Class);

								MSG_UpdateExpRanking expRankingPacket = MSG_UpdateExpRanking(party, rankInfo);
								DBServerSocket.SendOneMessage((char*)&expRankingPacket, sizeof(expRankingPacket));
#pragma endregion
							}
						}
// endregion
// region Pesa M
						else if (party > 0 && party < MAX_USER && pMob[party].MOB.CurrentScore.Hp > 0 && (tx / 128) == 8 && (pMob[party].TargetX / 128) == 8
							&& (ty / 128) == 2 && (pMob[party].TargetY / 128) == 2)
						{
							isExp = GetExpApply(pMob[party].extra, (int)pMob[target].MOB.Exp, pMob[party].MOB.CurrentScore.Level, pMob[target].MOB.CurrentScore.Level);

							int myLevel = pMob[party].MOB.CurrentScore.Level;

							if (pMob[party].extra.ClassMaster != MORTAL && pMob[party].extra.ClassMaster != ARCH)
								myLevel += MAX_LEVEL + 1;

							int exp = (UNK_1 + myLevel) * isExp / (UNK_1 + myLevel);
							if (exp > 0 && exp <= 10000000)
							{
								if (pMob[party].extra.ClassMaster != MORTAL && pMob[party].extra.ClassMaster != ARCH)
								{
									if (myLevel < 120)
										exp /= 10;

									else if (myLevel < 150)
										exp /= 20;

									else if (myLevel < 170)
										exp /= 40;

									else if (myLevel < 180)
										exp /= 80;

									else if (myLevel < 190)
										exp /= 160;

									else
										exp /= 320;
								}
								exp = 6 * exp / 10;

								//if (exp > eMob)
								//	exp = eMob;

								if (pMob[conn].ExpBonus > 0 && pMob[conn].ExpBonus < 500)
									exp += exp * pMob[conn].ExpBonus / 100;

								if (RvRBonus && RvRBonus == pMob[party].MOB.Clan)
									exp += (exp * 10) / 100;

								if (NewbieEventServer && pMob[party].MOB.CurrentScore.Level < 100 && pMob[party].extra.ClassMaster != CELESTIAL && pMob[party].extra.ClassMaster != CELESTIALCS && pMob[party].extra.ClassMaster != SCELESTIAL)
									exp += exp / 4;

								if (DOUBLEMODE)
									exp *= 2;

								if (KefraLive == 0)
									exp /= 2;

								if (NewbieEventServer)
									exp += (exp * 15) / 100;
								else
									exp -= (exp * 15) / 100;

#pragma region Log de Experiência diário
								if (when.tm_yday != pMob[party].extra.DayLog.YearDay)
									pMob[party].extra.DayLog.Exp = 0;

								pMob[party].extra.DayLog.YearDay = when.tm_yday;
								pMob[party].extra.DayLog.Exp += exp;
#pragma endregion

								if (pMob[party].extra.Hold > 0 && (unsigned int)exp >= pMob[party].extra.Hold)
								{
									int nhold = pMob[party].extra.Hold - exp;

									if (nhold < 0)
										nhold = 0;

									exp -= pMob[party].extra.Hold;
									pMob[party].extra.Hold = nhold;
								}

								else if (pMob[party].extra.Hold > 0 && (unsigned int)exp < pMob[party].extra.Hold)
								{
									pMob[party].extra.Hold -= exp;
									continue;
								}

								pMob[party].MOB.Exp += exp;
#pragma region RankingUpdateExp
								STRUCT_RANKING rankInfo = STRUCT_RANKING(pMob[party].MOB.MobName, pMob[party].MOB.Exp, pMob[party].extra.ClassMaster, pMob[party].MOB.CurrentScore.Level, pMob[party].MOB.Class);

								MSG_UpdateExpRanking expRankingPacket = MSG_UpdateExpRanking(party, rankInfo);
								DBServerSocket.SendOneMessage((char*)&expRankingPacket, sizeof(expRankingPacket));
#pragma endregion
							}
						}
// endregion
// region Pesa N
						else if (party > 0 && party < MAX_USER && pMob[party].MOB.CurrentScore.Hp > 0 && (tx / 128) == 10 && (pMob[party].TargetX / 128) == 10
							&& (ty / 128) == 2 && (pMob[party].TargetY / 128) == 2)
						{
							isExp = GetExpApply(pMob[party].extra, (int)pMob[target].MOB.Exp, pMob[party].MOB.CurrentScore.Level, pMob[target].MOB.CurrentScore.Level);

							int myLevel = pMob[party].MOB.CurrentScore.Level;

							if (pMob[party].extra.ClassMaster != MORTAL && pMob[party].extra.ClassMaster != ARCH)
								myLevel += MAX_LEVEL + 1;

							int exp = (UNK_1 + myLevel) * isExp / (UNK_1 + myLevel);
							if (exp > 0 && exp <= 10000000)
							{
								if (pMob[party].extra.ClassMaster != MORTAL && pMob[party].extra.ClassMaster != ARCH)
								{
									if (myLevel < 120)
										exp /= 10;

									else if (myLevel < 150)
										exp /= 20;

									else if (myLevel < 170)
										exp /= 40;

									else if (myLevel < 180)
										exp /= 80;

									else if (myLevel < 190)
										exp /= 160;

									else
										exp /= 320;
								}
								exp = 6 * exp / 10;

								//if (exp > eMob)
								//	exp = eMob;

								if (pMob[conn].ExpBonus > 0 && pMob[conn].ExpBonus < 500)
									exp += exp * pMob[conn].ExpBonus / 100;

								if (RvRBonus && RvRBonus == pMob[party].MOB.Clan)
									exp += (exp * 10) / 100;

								if (NewbieEventServer && pMob[party].MOB.CurrentScore.Level < 100 && pMob[party].extra.ClassMaster != CELESTIAL && pMob[party].extra.ClassMaster != CELESTIALCS && pMob[party].extra.ClassMaster != SCELESTIAL)
									exp += exp / 4;

								if (DOUBLEMODE)
									exp *= 2;

								if (KefraLive == 0)
									exp /= 2;

								if (NewbieEventServer)
									exp += (exp * 15) / 100;
								else
									exp -= (exp * 15) / 100;
#pragma region Log de Experiência diário
								if (when.tm_yday != pMob[party].extra.DayLog.YearDay)
									pMob[party].extra.DayLog.Exp = 0;

								pMob[party].extra.DayLog.YearDay = when.tm_yday;
								pMob[party].extra.DayLog.Exp += exp;
#pragma endregion

								if (pMob[party].extra.Hold > 0 && (unsigned int)exp >= pMob[party].extra.Hold)
								{
									int nhold = pMob[party].extra.Hold - exp;

									if (nhold < 0)
										nhold = 0;

									exp -= pMob[party].extra.Hold;
									pMob[party].extra.Hold = nhold;
								}

								else if (pMob[party].extra.Hold > 0 && (unsigned int)exp < pMob[party].extra.Hold)
								{
									pMob[party].extra.Hold -= exp;
									continue;
								}

								pMob[party].MOB.Exp += exp;
#pragma region RankingUpdateExp
								STRUCT_RANKING rankInfo = STRUCT_RANKING(pMob[party].MOB.MobName, pMob[party].MOB.Exp, pMob[party].extra.ClassMaster, pMob[party].MOB.CurrentScore.Level, pMob[party].MOB.Class);

								MSG_UpdateExpRanking expRankingPacket = MSG_UpdateExpRanking(party, rankInfo);
								DBServerSocket.SendOneMessage((char*)&expRankingPacket, sizeof(expRankingPacket));
#pragma endregion
							}
						}
// endregion
						else if (party > 0 && party < MAX_USER && pMob[party].MOB.CurrentScore.Hp > 0 && tx >= pMob[party].TargetX - HALFGRIDX
							&& tx <= pMob[party].TargetX + HALFGRIDX && ty >= pMob[party].TargetY - HALFGRIDY && ty <= pMob[party].TargetY + HALFGRIDY)
						{
							isExp = GetExpApply(pMob[party].extra, (int)pMob[target].MOB.Exp, pMob[party].MOB.CurrentScore.Level, pMob[target].MOB.CurrentScore.Level);

							int myLevel = pMob[party].MOB.CurrentScore.Level;

							if (pMob[party].extra.ClassMaster != MORTAL && pMob[party].extra.ClassMaster != ARCH)
								myLevel += MAX_LEVEL + 1;

							int exp = (UNK_1 + myLevel) * isExp / (UNK_1 + myLevel);
							if (exp > 0 && exp <= 10000000)
							{
								if (pMob[party].extra.ClassMaster != MORTAL && pMob[party].extra.ClassMaster != ARCH)
								{
									if (myLevel < 120)
										exp /= 10;

									else if (myLevel < 150)
										exp /= 20;

									else if (myLevel < 170)
										exp /= 40;

									else if (myLevel < 180)
										exp /= 80;

									else if (myLevel < 190)
										exp /= 160;

									else 
										exp /= 320;
									
								}
								exp = 6 * exp / 10;

								if (exp > eMob)
									exp = eMob;

								if (pMob[conn].ExpBonus > 0 && pMob[conn].ExpBonus < 500)
									exp += exp * pMob[conn].ExpBonus / 100;

								if (RvRBonus && RvRBonus == pMob[party].MOB.Clan)
									exp += (exp * 10) / 100;

								if (NewbieEventServer && pMob[party].MOB.CurrentScore.Level < 100 && pMob[party].extra.ClassMaster != CELESTIAL && pMob[party].extra.ClassMaster != CELESTIALCS && pMob[party].extra.ClassMaster != SCELESTIAL)
									exp += exp / 4;

								if (DOUBLEMODE)
									exp *= 2;

								if (KefraLive == 0)
									exp /= 2;

								if (NewbieEventServer)
									exp += (exp * 15) / 100;
								else
									exp -= (exp * 15) / 100;

#pragma region Log de Experiência diário
								if (when.tm_yday != pMob[party].extra.DayLog.YearDay)
									pMob[party].extra.DayLog.Exp = 0;

								pMob[party].extra.DayLog.YearDay = when.tm_yday;
								pMob[party].extra.DayLog.Exp += exp;
#pragma endregion
								if (pMob[party].extra.Hold > 0 && (unsigned int)exp >= pMob[party].extra.Hold)
								{
									int nhold = pMob[party].extra.Hold - exp;

									if (nhold < 0)
										nhold = 0;

									exp -= pMob[party].extra.Hold;
									pMob[party].extra.Hold = nhold;
								}

								else if (pMob[party].extra.Hold > 0 && (unsigned int)exp < pMob[party].extra.Hold)
								{
									pMob[party].extra.Hold -= exp;
									continue;
								}

								pMob[party].MOB.Exp += exp;

#pragma region RankingUpdateExp
								STRUCT_RANKING rankInfo = STRUCT_RANKING(pMob[party].MOB.MobName, pMob[party].MOB.Exp, pMob[party].extra.ClassMaster, pMob[party].MOB.CurrentScore.Level, pMob[party].MOB.Class);

								MSG_UpdateExpRanking expRankingPacket = MSG_UpdateExpRanking(party, rankInfo);
								DBServerSocket.SendOneMessage((char*)&expRankingPacket, sizeof(expRankingPacket));
#pragma endregion
							}
						}
					}
	return true;
}
#pragma endregion

	return false;
}
