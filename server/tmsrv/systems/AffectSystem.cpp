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

/// @file AffectSystem.cpp
/// @brief Gerenciamento de buffs/debuffs (affects), regeneracao e efeitos periodicos.

/// Regenera HP e MP do personagem/mob levando em conta buffs, montaria e estado de combate.
void RegenMob(int conn)
{
	if (conn > 0 && conn < MAX_USER)
	{
		int Guild = pMob[conn].MOB.Guild;
		if (CastleState <= 1 || pMob[conn].TargetX != 1046 || pMob[conn].TargetY != 1690 || Guild <= 0 || pMob[conn].MOB.GuildLevel != 9 || Guild == g_pGuildZone[4].ChargeGuild)
			pUser[conn].CastleStatus = 0;
		
		else
		{
			int Citys = 0;

			for(int i = 0; i < ValidGuild; i++)
			{
				if(g_pGuildZone[i].ChargeGuild == pMob[conn].MOB.Guild && i != 4)
					Citys++;
			}

			if(Citys != 0)
			{
				if (pUser[conn].CastleStatus == 0)
				{
					sprintf(temp, g_pMessageStringTable[_SN_S_is_charging_castle], pMob[conn].MOB.MobName);
					SendNotice(temp);
				}
				MSG_STANDARDPARM sm;
				memset(&sm, 0, sizeof(MSG_STANDARDPARM));

				sm.Type = _MSG_SendCastleState2;
				sm.Size = sizeof(MSG_STANDARDPARM);
				sm.ID = conn;
				sm.Parm = 1;

				GridMulticast(pMob[conn].TargetX, pMob[conn].TargetY, (MSG_STANDARD*)&sm, 0);
			
				pUser[conn].CastleStatus++;
			
				if (pUser[conn].CastleStatus > 22)
				{
					sprintf(temp, g_pMessageStringTable[_SN_S_charge_castle], pMob[conn].MOB.MobName);
					SendNotice(temp);
				
					g_pGuildZone[4].ChargeGuild = Guild;
					g_pGuildZone[4].Clan = pMob[conn].MOB.Clan;
				
					FinishCastleWar();
				
					for (int i = 0; i < MAX_USER; i++)
						ClearCrown(i);
				}
			}
		}

		pUser[conn].Unk_2736++;

		if (!(pUser[conn].Unk_2736 % 450))
		{
			int PKP = GetPKPoint(conn);
			if (PKP < 75)
			{
				PKP++;
				SetPKPoint(conn, PKP);
				sprintf(temp, g_pMessageStringTable[_DD_PKPointPlus], PKP - 75, 1);
				SendClientMessage(conn, temp);
			}

			STRUCT_ITEM *Mount = &pMob[conn].MOB.Equip[14];
			if (pMob[conn].MOB.Equip[14].sIndex >= 2300 && Mount->sIndex < 2330)
			{
				int Incubacao = BASE_GetBonusItemAbility(Mount, EF_INCUDELAY);
				if (Incubacao > 0)
				{
					Incubacao--;
					Mount->stEffect[2].cValue = Incubacao;
					SendItem(conn, ITEM_PLACE_EQUIP, 14, &pMob[conn].MOB.Equip[14]);
					SendClientMessage(conn, g_pMessageStringTable[_NN_Incu_Proceed]);
				}
			}
			if (Mount->sIndex >= 2330 && Mount->sIndex < 2390 && Mount->stEffect[0].sValue > 0)
			{
				int Feed = Mount->stEffect[2].cEffect;
				int MountDiv = Mount->sIndex - 2330 % 30;

				if (MountDiv > 15)
					Feed -= 4;
				else
					Feed -= 2;

				if (Feed > 1)
					Mount->stEffect[2].cEffect = Feed;

				else
				{
					Mount->stEffect[0].sValue = 0;
					Mount->stEffect[2].cEffect = 4;

					SendClientMessage(conn, g_pMessageStringTable[_NN_Mount_died]);

					memcpy(&pMob[conn].MOB.Equip[14], Mount, sizeof(STRUCT_ITEM));

					Mount->stEffect[2].cEffect = 0;

					if (Mount->sIndex >= 2360 && Mount->sIndex < 2390)
						ProcessAdultMount(conn, 0);

					if (Mount->sIndex >= 2360)
						pMob[conn].GetCurrentScore(conn);

					else
						MountProcess(conn, Mount);
				}
				SendItem(conn, ITEM_PLACE_EQUIP, 14, &pMob[conn].MOB.Equip[14]);
			}
		}
		int Guilty = GetGuilty(conn);

		if (Guilty > 0)
		{
			Guilty--;
			SetGuilty(conn, Guilty);

			if (!Guilty)
			{
				MSG_CreateMob CreateMob;
				GetCreateMob(conn, &CreateMob);
				GridMulticast(pMob[conn].TargetX, pMob[conn].TargetY, (MSG_STANDARD*)&CreateMob, 0);
				
				for (int j = MAX_USER; j < MAX_MOB; j++)
				{
					if (pMob[j].Mode == 5 && pMob[j].MOB.Clan == 4)
					{
						if (pMob[j].CurrentTarget == conn)
						{
							pMob[j].CurrentTarget = 0;
							pMob[j].Mode = 4;
						}
						for (int k = 0; k < MAX_ENEMY; k++)
						{
							if (pMob[j].EnemyList[k] == conn)
							{
								pMob[j].EnemyList[k] = 0;
								pMob[j].Mode = MOB_PEACE;
							}
						}
					}
				}
			}
		}
	}	
}

/// Processa todos os buffs/debuffs ativos no mob/personagem e efeitos periodicos (Fada, Aura, Veneno, Trovao).
int  ProcessAffect(int idx)
{
	int FadaIndex = FadaIndex = pMob[idx].MOB.Equip[13].sIndex;
	int currentTarget = pMob[idx].CurrentTarget;

	if (idx >= MAX_USER || FadaIndex != 753 && FadaIndex != 769 && FadaIndex != 1726 || currentTarget == 0)
		goto LABEL_59;

	if (currentTarget == idx)
		pMob[idx].CurrentTarget = 0;

// region Fada attack
	else
	{
		if (currentTarget > 0 && currentTarget < MAX_MOB)
		{
			if (pMob[currentTarget].Mode)
			{
				unsigned char map_att = GetAttribute(pMob[idx].TargetX, pMob[idx].TargetY);

				if (map_att & 0x40 || currentTarget >= MAX_USER && pMob[currentTarget].MOB.Clan != 4)
				{
					if (pUser[idx].PKMode || currentTarget >= MAX_USER)
					{
						int leader = pMob[idx].Leader;
						if (!leader)
							leader = idx;

						int targetleader = pMob[currentTarget].Leader;
						if (!targetleader)
							targetleader = currentTarget;

						int guild = pMob[idx].MOB.Guild;
						if (pMob[idx].GuildDisable)
							guild = 0;

						int mobguild = pMob[currentTarget].MOB.Guild;
						if (pMob[currentTarget].GuildDisable)
							mobguild = 0;

						if (!guild && !mobguild)
							guild = -1;

						if (leader != targetleader && guild != mobguild)
						{
							if (currentTarget < MAX_USER)
							{
								unsigned char map_att2 = GetAttribute(pMob[currentTarget].TargetX, pMob[currentTarget].TargetY);

								if(!(map_att & 0x40))
								{
									pMob[idx].CurrentTarget = 0;
									goto LABEL_59;
								}
								if (pMob[currentTarget].MOB.Rsv & 0x10)
								{
									pMob[idx].CurrentTarget = 0;
									goto LABEL_59;
								}
								if (pMob[currentTarget].MOB.BaseScore.Level >= MAX_LEVEL && pMob[currentTarget].MOB.Merchant & 1)
								{
									pMob[idx].CurrentTarget = 0;
									goto LABEL_59;
								}
							}
							int sameclan = 0;
							int clan = pMob[idx].MOB.Clan;
							int mobclan = pMob[currentTarget].MOB.Clan;

							if (clan == 7 && mobclan == 7 || clan == 8 && mobclan == 8)
								sameclan = 1;

							if ((sameclan != 1 || pUser[idx].PKMode) && (sameclan != 1 || currentTarget < MAX_USER))
							{
								if (pMob[currentTarget].TargetX >= pMob[idx].TargetX - HALFGRIDX
									&& pMob[currentTarget].TargetX <= pMob[idx].TargetX + HALFGRIDX
									&& (pMob[currentTarget].TargetY >= pMob[idx].TargetY - HALFGRIDY || pMob[currentTarget].TargetY <= pMob[idx].TargetY + HALFGRIDY))
								{
									MSG_Attack sm;

									memset(&sm, 0, sizeof(MSG_AttackOne));

									sm.Type = _MSG_AttackOne;
									sm.ID = idx;
									sm.Size = sizeof(MSG_AttackOne);

									//check
									sm.ClientTick = 0xE0A1ACA;
									sm.Motion = 254;

									sm.TargetX = pMob[idx].TargetX;
									sm.TargetY = pMob[idx].TargetY;

									//check
									sm.CurrentMp = -2;
									//check
									sm.AttackerID = idx;

									//check
									sm.SkillIndex = 32;

									if (FadaIndex == 769)
										sm.SkillIndex = 34;
									if (FadaIndex == 1726)
										sm.SkillIndex = 36;

									sm.Dam[0].TargetID = currentTarget;
									sm.Dam[0].Damage = -1;

									ProcessClientMessage(idx, (char*)&sm, TRUE);
								}
								else
									pMob[idx].CurrentTarget = 0;
							}
							goto LABEL_59;
						}
						pMob[idx].CurrentTarget = 0;
					}
					else
						pMob[idx].CurrentTarget = 0;
				}
				else
					pMob[idx].CurrentTarget = 0;
			}
			else
				pMob[idx].CurrentTarget = 0;
		}
		else
			pMob[idx].CurrentTarget = 0;
	}
// endregion
LABEL_59:

	int regen = 0;
	int upScore = 0;
	int FaceChange = 0;
	int Damage = 0;

	for (int i = 0; i < MAX_AFFECT; i++)
	{
		int Type = pMob[idx].Affect[i].Type;

		if (Type <= 0)
			continue;


		int MaxHp = pMob[idx].MOB.CurrentScore.MaxHp;
		int Hp = pMob[idx].MOB.CurrentScore.Hp;
		int AffectValue = pMob[idx].Affect[i].Level;
		int AffectLevel = pMob[idx].Affect[i].Value;
		int Level = pMob[idx].MOB.CurrentScore.Level;

		if(pMob[idx].extra.ClassMaster != MORTAL && pMob[idx].extra.ClassMaster != ARCH)
			Level += MAX_LEVEL;

#pragma region Regen de HP Aura da Vida
		if (Type == 17)
		{
			Hp += AffectLevel / 2 + AffectValue;

			if (Hp < 1)
				Hp = 1;

			if (Hp > MaxHp)
				Hp = MaxHp;

			if (pMob[idx].MOB.CurrentScore.Hp != Hp)
				upScore = 1;

			Damage = Hp - pMob[idx].MOB.CurrentScore.Hp;

			pMob[idx].MOB.CurrentScore.Hp = Hp;
			regen = 1;
			goto LessAffect;
		}
#pragma endregion
#pragma region Veneno - Drenagem de HP
		if (Type == 20) //??? Possivelmente Veneno's
		{
			AffectValue = 100;

			if (idx >= MAX_USER)
				AffectValue -= pMob[idx].MOB.LearnedSkill / MAX_USER;

			AffectValue /= 10;

			int HpLose = (int)((100 - AffectValue) / 100.0);
			int Perda = 1000;

			if (idx >= MAX_USER)
				Perda = 10 * (100 - pMob[idx].MOB.LearnedSkill / MAX_USER);

			int Perdamax = Hp - Perda;

			Hp = (int)((double)Hp * HpLose);

			if (Hp < (signed int)Perdamax)
				Hp = Perdamax;

			if (Hp < 1)
				Hp = 1;

			if (Hp > MaxHp)
				Hp = MaxHp;

			if (pMob[idx].MOB.CurrentScore.Hp != Hp)
				upScore = 1;

			Damage = Hp - pMob[idx].MOB.CurrentScore.Hp;
			pMob[idx].MOB.CurrentScore.Hp = Hp;

			if (idx > 0 && idx < MAX_USER)
				pUser[idx].ReqHp += Damage;

			regen = 1;

			if (idx > MAX_USER && pMob[idx].MOB.Clan == 4)
				LinkMountHp(idx);

			goto LessAffect;
		}
#pragma endregion
#pragma region RegenHP Type21
/*		if (Type == 21)//Regen de HT
		{
			Hp += AffectValue * MaxHp / 100;

			if (Hp < 1)
				Hp = 1;

			if (Hp > MaxHp)
				Hp = MaxHp;

			if (pMob[idx].MOB.CurrentScore.Hp != Hp)
				upScore = 1;

			Damage = Hp - pMob[idx].MOB.CurrentScore.Hp;

			pMob[idx].MOB.CurrentScore.Hp = Hp;
			regen = 1;

			goto LessAffect;
		}*/
#pragma endregion
#pragma region Trovão
		if(Type == 22)
		{
			  int alvo1 = 0;
			  int alvo2 = 0;
			  int alvo3 = 0;
			  int alvo4 = 0;
			  int alvo6 = 0;
			  int alvo7 = 0;

			  unsigned char map_att = GetAttribute(pMob[idx].TargetX, pMob[idx].TargetY);

			  int Clan = pMob[idx].MOB.Clan;

			  if (map_att & 1)
				goto LessAffect;

			  int yv = pMob[idx].TargetY - 1;
			  int xv = pMob[idx].TargetX - 1;
			  int leader = pMob[idx].Leader;

			  if(leader <= 0)
				leader = idx;

			  for(int yy = yv; yy <= (yv + 1); yy++)
			  {
				  for(int xx = xv; xx <= (xv + 1); xx++)
				  {
					  if (xx < 0 || xx >= MAX_GRIDX || yy < 0 || yy >= MAX_GRIDY || pMobGrid[yy][xx] <= 0 || pMobGrid[yy][xx] >= MAX_MOB || !pMob[pMobGrid[yy][xx]].Mode || pMob[pMobGrid[yy][xx]].MOB.CurrentScore.Hp <= 0 || leader == pMob[pMobGrid[yy][xx]].Leader)
						  continue;

					  int tmob = pMobGrid[yy][xx];

					  int tclan = pMob[tmob].MOB.Clan;
					  int SameClan = 0;

					  if(Clan == 7 && tclan == 7 || Clan == 8 && tclan == 8)
						SameClan = 1;

					  if(pMob[tmob].MOB.Merchant & 1)
						  continue;

					  unsigned char map_attack = GetAttribute(xx, yy);

					  if(map_attack & 1)
						  continue;

					  if(idx >= MAX_USER)
						  break;

					  if ((pUser[idx].PKMode || tmob >= MAX_USER && tclan != 4 && SameClan != 1) && (map_att & 0x40 || tmob >= 100 && tclan != 4 && SameClan != 1) && !(pMob[tmob].MOB.Rsv & 0x10))
					  {
						  if (SameClan == 1 && tmob >= MAX_USER || tclan == 6 || tmob == idx)
							  continue;

						  int idxleader = pMob[idx].Leader;
						  
						  if(!idxleader)
							idxleader = idx;
						  
						  int mobleader = pMob[tmob].Leader;
						  if(!mobleader)
							mobleader = tmob;

						  int Guild = pMob[idx].MOB.Guild;
						  
						  if(pMob[idx].GuildDisable)
							Guild = 0;

						  int MobGuild = pMob[tmob].MOB.Guild;
						  
						  if(pMob[tmob].GuildDisable)
							MobGuild = 0;

						  int Ally = g_pGuildAlly[Guild];
						  
						  if(!Ally)
							Ally = -2;

						  if(!Guild && !MobGuild)
							Guild = -1;

						  if(idxleader == mobleader || Guild == MobGuild || Ally == MobGuild)
							continue;

						  if(!alvo1)
						  {
							alvo1 = tmob;
							continue;
						  }

						  if(alvo1 == tmob)
							 continue;

						  if(!alvo2)
						  {
							alvo2 = tmob;
							continue;
						  }
						  if(alvo2 == tmob)
							continue;

						  if(!alvo3)
						  {
							alvo3 = tmob;
							continue;
						  }
						  if(alvo3 == tmob)
							continue;

						  if(!alvo4)
						  {
							alvo4 = tmob;
							continue;
						  }
						  if(alvo4 == tmob)
							continue;

						  if(!alvo6)
						  {
							alvo6 = tmob;
							continue;
						  }

						  if(alvo6 == tmob || alvo7)
							 continue;

						  alvo7 = tmob;
					  }
				  }
			  }

			  yv = pMob[idx].TargetY - 4;
			  xv = pMob[idx].TargetX - 4;


			  for(int yy = yv; yy <= (yv + 4); yy++)
			  {
				  for(int xx = xv; xx <= (xv + 4); xx++)
				  {
					  if (xx < 0 || xx >= MAX_GRIDX || yy < 0 || yy >= MAX_GRIDY || pMobGrid[yy][xx] <= 0 || pMobGrid[yy][xx] >= MAX_MOB || !pMob[pMobGrid[yy][xx]].Mode || pMob[pMobGrid[yy][xx]].MOB.CurrentScore.Hp <= 0 || leader == pMob[pMobGrid[yy][xx]].Leader)
						  continue;

					  int tmob = pMobGrid[yy][xx];

					  int tclan = pMob[tmob].MOB.Clan;
					  int SameClan = 0;

					  if(Clan == 7 && tclan == 7 || Clan == 8 && tclan == 8)
						SameClan = 1;

					  if(pMob[tmob].MOB.Merchant & 1)
						  continue;

					  unsigned char map_attack = GetAttribute(xx, yy);

					  if(map_attack & 1)
						  continue;

					  if(idx >= MAX_USER)
						  break;

					  if ((pUser[idx].PKMode || tmob >= MAX_USER && tclan != 4 && SameClan != 1) && (map_att & 0x40 || tmob >= MAX_USER && tclan != 4 && SameClan != 1) && !(pMob[tmob].MOB.Rsv & 0x10))
					  {
						  if (SameClan == 1 && tmob >= MAX_USER || tclan == 6 || tmob == idx)
							  continue;

						  int idxleader = pMob[idx].Leader;
						  
						  if(!idxleader)
							idxleader = idx;
						  
						  int mobleader = pMob[tmob].Leader;
						  if(!mobleader)
							mobleader = tmob;

						  int Guild = pMob[idx].MOB.Guild;
						  
						  if(pMob[idx].GuildDisable)
							Guild = 0;

						  int MobGuild = pMob[tmob].MOB.Guild;
						  
						  if(pMob[tmob].GuildDisable)
							MobGuild = 0;

						  int Ally = g_pGuildAlly[Guild];
						  
						  if(!Ally)
							Ally = -2;

						  if(!Guild && !MobGuild)
							Guild = -1;

						  if(idxleader == mobleader || Guild == MobGuild || Ally == MobGuild)
							continue;

						  if(!alvo1)
						  {
							alvo1 = tmob;
							continue;
						  }

						  if(alvo1 == tmob)
							 continue;

						  if(!alvo2)
						  {
							alvo2 = tmob;
							continue;
						  }
						  if(alvo2 == tmob)
							continue;

						  if(!alvo3)
						  {
							alvo3 = tmob;
							continue;
						  }
						  if(alvo3 == tmob)
							continue;

						  if(!alvo4)
						  {
							alvo4 = tmob;
							continue;
						  }
						  if(alvo4 == tmob)
							continue;

						  if(!alvo6)
						  {
							alvo6 = tmob;
							continue;
						  }

						  if(alvo6 == tmob || alvo7)
							 continue;

						  alvo7 = tmob;
					  }
				  }
			  }

			  if(alvo1)
			  {
					MSG_Attack sm;

					memset(&sm, 0, sizeof(MSG_Attack));

					sm.Type = _MSG_Attack;
					sm.ID = idx;
					sm.Size = sizeof(MSG_Attack);

					sm.ClientTick = 0xE0A1ACA;
					sm.Motion = 254;

					sm.TargetX = pMob[idx].TargetX;
					sm.TargetY = pMob[idx].TargetY;
					
					sm.CurrentMp = -2;
					
					if(pMob[idx].MOB.Equip[0].sIndex == 219)
						sm.CurrentMp = -4;

					sm.AttackerID = idx;
					sm.SkillIndex = 33;

					int calc = 500 + rand()%100 + (pMob[idx].extra.ClassMaster == MORTAL ||  pMob[idx].extra.ClassMaster == MORTAL ? Level : Level + MAX_CLEVEL) + AffectLevel;

					sm.Dam[0].TargetID = alvo1;
					sm.Dam[0].Damage = -1;

					if(calc > 300)
					{
						sm.Dam[1].TargetID = alvo2;
						sm.Dam[1].Damage = -1;
					}

					if(calc > 350)
					{
						sm.Dam[2].TargetID = alvo3;
						sm.Dam[2].Damage = -1;
					}

					if(calc > 400)
					{
						sm.Dam[3].TargetID = alvo4;
						sm.Dam[3].Damage = -1;
					}

					if(calc > 450)
					{
						sm.Dam[4].TargetID = alvo6;
						sm.Dam[4].Damage = -1;
					}

					if(calc > 500)
					{
						sm.Dam[5].TargetID = alvo7;
						sm.Dam[5].Damage = -1;
					}


					if(idx < MAX_USER)
						ProcessClientMessage(idx, (char*)&sm, TRUE);
			  }
		}
#pragma endregion
#pragma region Aura Bestial
		if(Type == 23)
		{
			 int alvo1 = 0;
			  int alvo2 = 0;
			  int alvo3 = 0;
			  int alvo4 = 0;
			  int alvo6 = 0;
			  int alvo7 = 0;

			  unsigned char map_att = GetAttribute(pMob[idx].TargetX, pMob[idx].TargetY);

			  int Clan = pMob[idx].MOB.Clan;

			  if (map_att & 1)
				goto LessAffect;

			  int yv = pMob[idx].TargetY - 1;
			  int xv = pMob[idx].TargetX - 1;
			  int leader = pMob[idx].Leader;

			  if(leader <= 0)
				leader = idx;

			  for(int yy = yv; yy <= (yv + 1); yy++)
			  {
				  for(int xx = xv; xx <= (xv + 1); xx++)
				  {
					  if (xx < 0 || xx >= MAX_GRIDX || yy < 0 || yy >= MAX_GRIDY || pMobGrid[yy][xx] <= 0 || pMobGrid[yy][xx] >= MAX_MOB || !pMob[pMobGrid[yy][xx]].Mode || pMob[pMobGrid[yy][xx]].MOB.CurrentScore.Hp <= 0 || leader == pMob[pMobGrid[yy][xx]].Leader)
						  continue;

					  int tmob = pMobGrid[yy][xx];

					  int tclan = pMob[tmob].MOB.Clan;
					  int SameClan = 0;

					  if(Clan == 7 && tclan == 7 || Clan == 8 && tclan == 8)
						SameClan = 1;

					  if(pMob[tmob].MOB.Merchant & 1)
						  continue;

					  unsigned char map_attack = GetAttribute(xx, yy);

					  if(map_attack & 1)
						  continue;

					  if(idx >= MAX_USER)
						  break;

					  if ((pUser[idx].PKMode || tmob >= MAX_USER && tclan != 4 && SameClan != 1) && (map_att & 0x40 || tmob >= 100 && tclan != 4 && SameClan != 1) && !(pMob[tmob].MOB.Rsv & 0x10))
					  {
						  if (SameClan == 1 && tmob >= MAX_USER || tclan == 6 || tmob == idx)
							  continue;

						  int idxleader = pMob[idx].Leader;
						  
						  if(!idxleader)
							idxleader = idx;
						  
						  int mobleader = pMob[tmob].Leader;
						  if(!mobleader)
							mobleader = tmob;

						  int Guild = pMob[idx].MOB.Guild;
						  
						  if(pMob[idx].GuildDisable)
							Guild = 0;

						  int MobGuild = pMob[tmob].MOB.Guild;
						  
						  if(pMob[tmob].GuildDisable)
							MobGuild = 0;

						  int Ally = g_pGuildAlly[Guild];
						  
						  if(!Ally)
							Ally = -2;

						  if(!Guild && !MobGuild)
							Guild = -1;

						  if(idxleader == mobleader || Guild == MobGuild || Ally == MobGuild)
							continue;

						  if(!alvo1)
						  {
							alvo1 = tmob;
							continue;
						  }

						  if(alvo1 == tmob)
							 continue;

						  if(!alvo2)
						  {
							alvo2 = tmob;
							continue;
						  }
						  if(alvo2 == tmob)
							continue;

						  if(!alvo3)
						  {
							alvo3 = tmob;
							continue;
						  }
						  if(alvo3 == tmob)
							continue;

						  if(!alvo4)
						  {
							alvo4 = tmob;
							continue;
						  }
						  if(alvo4 == tmob)
							continue;

						  if(!alvo6)
						  {
							alvo6 = tmob;
							continue;
						  }

						  if(alvo6 == tmob || alvo7)
							 continue;

						  alvo7 = tmob;
					  }
				  }
			  }

			  yv = pMob[idx].TargetY - 4;
			  xv = pMob[idx].TargetX - 4;


			  for(int yy = yv; yy <= (yv + 4); yy++)
			  {
				  for(int xx = xv; xx <= (xv + 4); xx++)
				  {
					  if (xx < 0 || xx >= MAX_GRIDX || yy < 0 || yy >= MAX_GRIDY || pMobGrid[yy][xx] <= 0 || pMobGrid[yy][xx] >= MAX_MOB || !pMob[pMobGrid[yy][xx]].Mode || pMob[pMobGrid[yy][xx]].MOB.CurrentScore.Hp <= 0 || leader == pMob[pMobGrid[yy][xx]].Leader)
						  continue;

					  int tmob = pMobGrid[yy][xx];

					  int tclan = pMob[tmob].MOB.Clan;
					  int SameClan = 0;

					  if(Clan == 7 && tclan == 7 || Clan == 8 && tclan == 8)
						SameClan = 1;

					  if(pMob[tmob].MOB.Merchant & 1)
						  continue;

					  unsigned char map_attack = GetAttribute(xx, yy);

					  if(map_attack & 1)
						  continue;

					  if(idx >= MAX_USER)
						  break;

					  if ((pUser[idx].PKMode || tmob >= MAX_USER && tclan != 4 && SameClan != 1) && (map_att & 0x40 || tmob >= MAX_USER && tclan != 4 && SameClan != 1) && !(pMob[tmob].MOB.Rsv & 0x10))
					  {
						  if (SameClan == 1 && tmob >= MAX_USER || tclan == 6 || tmob == idx)
							  continue;

						  int idxleader = pMob[idx].Leader;
						  
						  if(!idxleader)
							idxleader = idx;
						  
						  int mobleader = pMob[tmob].Leader;
						  if(!mobleader)
							mobleader = tmob;

						  int Guild = pMob[idx].MOB.Guild;
						  
						  if(pMob[idx].GuildDisable)
							Guild = 0;

						  int MobGuild = pMob[tmob].MOB.Guild;
						  
						  if(pMob[tmob].GuildDisable)
							MobGuild = 0;

						  int Ally = g_pGuildAlly[Guild];
						  
						  if(!Ally)
							Ally = -2;

						  if(!Guild && !MobGuild)
							Guild = -1;

						  if(idxleader == mobleader || Guild == MobGuild || Ally == MobGuild)
							continue;

						  if(!alvo1)
						  {
							alvo1 = tmob;
							continue;
						  }

						  if(alvo1 == tmob)
							 continue;

						  if(!alvo2)
						  {
							alvo2 = tmob;
							continue;
						  }
						  if(alvo2 == tmob)
							continue;

						  if(!alvo3)
						  {
							alvo3 = tmob;
							continue;
						  }
						  if(alvo3 == tmob)
							continue;

						  if(!alvo4)
						  {
							alvo4 = tmob;
							continue;
						  }
						  if(alvo4 == tmob)
							continue;

						  if(!alvo6)
						  {
							alvo6 = tmob;
							continue;
						  }

						  if(alvo6 == tmob || alvo7)
							 continue;

						  alvo7 = tmob;
					  }
				  }
			  }

			  if(alvo1)
			  {
					MSG_Attack sm;

					memset(&sm, 0, sizeof(MSG_Attack));

					sm.Type = _MSG_Attack;
					sm.ID = idx;
					sm.Size = sizeof(MSG_Attack);

					sm.ClientTick = 0xE0A1ACA;
					sm.Motion = 254;

					sm.TargetX = pMob[idx].TargetX;
					sm.TargetY = pMob[idx].TargetY;
					
					sm.CurrentMp = -2;
					
					if(pMob[idx].MOB.Equip[0].sIndex == 219)
						sm.CurrentMp = -4;

					sm.AttackerID = idx;
					sm.SkillIndex = 52;

					int calc = 5000 + rand()%100 + (pMob[idx].extra.ClassMaster == MORTAL ||  pMob[idx].extra.ClassMaster == MORTAL ? Level : Level + MAX_CLEVEL) + AffectLevel;

					sm.Dam[0].TargetID = alvo1;
					sm.Dam[0].Damage = -1;

					if(calc > 300)
					{
						sm.Dam[1].TargetID = alvo2;
						sm.Dam[1].Damage = -1;
					}

					if(calc > 350)
					{
						sm.Dam[2].TargetID = alvo3;
						sm.Dam[2].Damage = -1;
					}

					if(calc > 400)
					{
						sm.Dam[3].TargetID = alvo4;
						sm.Dam[3].Damage = -1;
					}

					if(calc > 450)
					{
						sm.Dam[4].TargetID = alvo6;
						sm.Dam[4].Damage = -1;
					}

					if(calc > 500)
					{
						sm.Dam[5].TargetID = alvo7;
						sm.Dam[5].Damage = -1;
					}


					if(idx < MAX_USER)
						ProcessClientMessage(idx, (char*)&sm, TRUE);
			  }
		}
#pragma endregion
#pragma region Affects less time
	LessAffect :
		if (pMob[idx].Affect[i].Time < 32400000)
			pMob[idx].Affect[i].Time--;


		if (pMob[idx].Affect[i].Time <= 0)
		{
			if (pMob[idx].Affect[i].Type == 16)
				FaceChange = 1;

			int summonEnd = 0;

			if(pMob[idx].Affect[i].Type == 24 && idx >= MAX_USER)
				DeleteMob(idx, 3);


			pMob[idx].Affect[i].Type = 0;
			pMob[idx].Affect[i].Time = 0;
			pMob[idx].Affect[i].Level = 0;
			pMob[idx].Affect[i].Value = 0;
			upScore = 1;
		}

#pragma endregion
	}

	
	if (regen)
	{
		if (idx > 0 && idx < MAX_USER)
		{
			SetReqHp(idx);
			SetReqMp(idx);
		}
		MSG_SetHpDam dam;

		dam.Type = _MSG_SetHpDam;
		dam.Size = sizeof(MSG_SetHpDam);
		dam.ID = idx;

		dam.Hp = pMob[idx].MOB.CurrentScore.Hp;
		dam.Dam = SetHpDamValue(Damage);

		GridMulticast(pMob[idx].TargetX, pMob[idx].TargetY, (MSG_STANDARD*)&dam, 0);
	}
	if (upScore)
	{
		pMob[idx].GetCurrentScore(idx);
		SendScore(idx);
		
		if (FaceChange)
			SendEquip(idx, 0);

		return TRUE;
	}
	else
		return 0;

	return 0;
}

/// Aplica ou renova um buff/debuff (Affect) no slot correspondente do personagem/mob.
BOOL SetAffect(int conn, int skillnum, int time, int Level)
{
	if (pMob[conn].MOB.Merchant == 1 || conn > MAX_USER)
		return FALSE;

	int sAffectType = g_pSpell[skillnum].AffectType;
	int sAggressive = g_pSpell[skillnum].Aggressive;

	if (pMob[conn].MOB.Rsv & RSV_BLOCK && sAggressive)
		return FALSE;

	int EmptyAffect = GetEmptyAffect(conn, sAffectType);

	if (sAffectType <= 0 || EmptyAffect  < 0 || EmptyAffect >= MAX_AFFECT)
		return FALSE;

	int sType = pMob[conn].Affect[EmptyAffect].Type;

	pMob[conn].Affect[EmptyAffect].Type = sAffectType;
	pMob[conn].Affect[EmptyAffect].Level = g_pSpell[skillnum].AffectValue;

	int sTime = (g_pSpell[skillnum].AffectTime + 1) * time / 100;

	if(sType == 1 || sType == 3 || sType == 10)
		sTime = 4;

	if (sType == sAffectType && (unsigned int)sTime > pMob[conn].Affect[EmptyAffect].Time)
		pMob[conn].Affect[EmptyAffect].Time = sTime;

	else
		pMob[conn].Affect[EmptyAffect].Time = sTime;

	if (sType == sAffectType && Level > pMob[conn].Affect[EmptyAffect].Value)
		pMob[conn].Affect[EmptyAffect].Value = Level;

	else
		pMob[conn].Affect[EmptyAffect].Value = Level;

	if (time >= 2139062143)
		pMob[conn].Affect[EmptyAffect].Time = 2139062143;

	pMob[conn].GetCurrentScore(conn);


	return TRUE;
}

/// Aplica um efeito periodico com intervalo de tick configurado.
BOOL SetTick(int mob, int skillnum, int Delay, int Level)
{
	if (pMob[mob].MOB.Merchant == 1 && mob > MAX_USER)
		return FALSE;

	int sTickType = g_pSpell[skillnum].TickType;
	int sAggressive = g_pSpell[skillnum].Aggressive;

	if ((pMob[mob].MOB.Rsv & RSV_BLOCK) && (sAggressive != 0))
		return 0;

	int EmptyAffect = GetEmptyAffect(mob, sTickType);

	if (sTickType > 0 && EmptyAffect >= 0 && EmptyAffect < MAX_AFFECT)
	{
		pMob[mob].Affect[EmptyAffect].Type = sTickType;
		pMob[mob].Affect[EmptyAffect].Level = g_pSpell[skillnum].TickValue;
		pMob[mob].Affect[EmptyAffect].Time = (Delay * (g_pSpell[skillnum].AffectTime + 1)) / 100;

		if (Delay >= 500000000)
			pMob[mob].Affect[EmptyAffect].Time = 500000000;

		if(pMob[mob].Affect[EmptyAffect].Time >= 3 && (sTickType == 1 || sTickType == 3 || sTickType == 10))
			pMob[mob].Affect[EmptyAffect].Time = 2;

		pMob[mob].Affect[EmptyAffect].Value = Level;

		pMob[mob].GetCurrentScore(mob);

		
		return TRUE;
	}

	return FALSE;
}
