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

/// @file MobAISystem.cpp
/// @brief "IA" por segundo dos mobs/NPCs: movimento e fala dos NPCs ociosos/em patrulha e
///        o processador de batalha dos mobs em combate (ataques, skills, Kefra).
///
/// Extraído de `core/ProcessSecMinTimer.cpp` na Fase 2 (Onda 3) sem alteração de comportamento.

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
#include "systems/CWarTower.h"
#include "systems/SecTimerInternal.h"

/// Processa movimento, fala e estados ociosos dos mobs não-jogadores.
///
/// Para distribuir a carga, cada segundo processa apenas 1/6 dos índices de mob:
/// começa em `initial` (= `SecCounter % 6 + MAX_USER`) e avança de 6 em 6 até MAX_MOB.
/// @param initial primeiro índice de mob a processar neste tick.
void ProcessNpcMovementTick(int initial)
{
#pragma region Movimento / fala dos NPCS 
	for (int index = initial; index < MAX_MOB; index += 6)
	{
		int Mode = pMob[index].Mode;
		int Clan = pMob[index].MOB.Clan;

		if (Mode)
		{
			if (Mode == MOB_IDLE)
			{
				if (pMob[index].MOB.CurrentScore.Hp <= 0)
				{
					Log("err,standingby processer delete hp zero mob", "-system", 0);
					DeleteMob(index, 1);
					continue;
				}
				ProcessAffect(index);
				pMobGrid[pMob[index].TargetY][pMob[index].TargetX] = index;
			}

			if (Mode == MOB_PEACE)
			{
				int chp = pMob[index].MOB.CurrentScore.Hp;
				if (chp <= 0)
				{
					Log("err,standingby processer delete hp zero mob", "-system", 0);
					DeleteMob(index, 1);
					continue;
				}

				ProcessAffect(index);

				if(pMob[index].GenerateIndex != RUNEQUEST_LV6_MOB_BOSS && pMob[index].TargetX >= 3423 && pMob[index].TargetX <= 3442 && pMob[index].TargetY >= 1492 && pMob[index].TargetY <= 1511)
				{
					if(pMob[index].GenerateIndex >= RUNEQUEST_LV6_MOB_INITIAL2 && pMob[index].GenerateIndex <= RUNEQUEST_LV6_MOB_END2)
					{
						if(Pista[6].Party[0].MobCount < 100 && Pista[6].Party[0].MobCount != 0)
							Pista[6].Party[0].MobCount++;
					}

					DeleteMob(index, 3);
					continue;
				}
				int Progress = pMob[index].SegmentProgress;

				if (Progress < 0 || Progress > MAX_SEGMENT)
				{
					pMob[index].SegmentProgress = 0;
					Progress = 0;
				}
				int Processor = pMob[index].StandingByProcessor();

				if (Processor & 0x10000000)
				{		
					int Target = Processor & 0xFFFFFFF;

					SetBattle(index, Processor & 0xFFFFFFF);

					int Leader = pMob[index].Leader;
					if (Leader <= 0)
						Leader = index;

					for (int l = 0; l < MAX_PARTY; ++l)
					{
						int party = pMob[Leader].PartyList[l];

						if (party > MAX_USER)
						{
							if (pMob[party].Mode && pMob[party].MOB.CurrentScore.Hp > 0)
								SetBattle(party, Target);

							else
							{
								if (pMob[party].Mode)
									DeleteMob(party, 1);

								pMob[Leader].PartyList[l] = 0;
							}
						}
					}

					if (Target < MAX_USER)
					{
						int leader = pMob[Target].Leader;

						if (leader <= 0)
							leader = Target;

						for (int n = 0; n < MAX_PARTY; n++)
						{
							int party = pMob[leader].PartyList[n];
							if (party > MAX_USER)
							{
								if (pMob[party].Mode && pMob[party].MOB.CurrentScore.Hp > 0)
									SetBattle(party, index);

								else
								{
									if (pMob[party].Mode)
										DeleteMob(party, 1);

									pMob[leader].PartyList[n] = 0;
								}
							}
						}
					}
				}
				else
				{
					if (Processor & 1)
					{
						MSG_Action sm;

						if (GetEmptyMobGrid(index, &pMob[index].NextX, &pMob[index].NextY) == FALSE)
							continue;

						GetAction(index, pMob[index].NextX, pMob[index].NextY, (MSG_Action*)&sm);

						if (pMob[index].NextX == pMob[index].TargetX && pMob[index].NextY == pMob[index].TargetY)
							continue;

						sm.Speed = BASE_GetSpeed(&pMob[index].MOB.CurrentScore);
						sm.Effect = 0;

						GridMulticast(index, pMob[index].NextX, pMob[index].NextY, (MSG_STANDARD*)&sm);

						int progres = pMob[index].SegmentProgress;

						if (Progress != progres)
						{
							if (progres >= 0 && progres < MAX_SEGMENT && pMob[index].GenerateIndex >= 0 && pMob[index].GenerateIndex < MAX_NPCGENERATOR && mNPCGen.pList[pMob[index].GenerateIndex].SegmentAction[Progress][0])
								SendSay(index, mNPCGen.pList[pMob[index].GenerateIndex].SegmentAction[Progress]);
						}

						if (pMob[index].GenerateIndex >= RUNEQUEST_LV2_MOB_INITIAL && pMob[index].GenerateIndex <= RUNEQUEST_LV2_MOB_END)
						{
							if (pMob[index].TargetX >= 3373 && pMob[index].TargetX <= 3407 && pMob[index].TargetY >= 1420 && pMob[index].TargetY <= 1453)
								DeleteMob(index, 3);
						}
					}
					if (Processor & 0x10)
					{
						int progres = pMob[index].SegmentProgress;

						if (Progress != progres)
						{
							if (progres >= 0 && progres < MAX_SEGMENT && pMob[index].GenerateIndex >= 0 && pMob[index].GenerateIndex < MAX_NPCGENERATOR && mNPCGen.pList[pMob[index].GenerateIndex].FleeAction[Progress][0])
								SendSay(index, mNPCGen.pList[pMob[index].GenerateIndex].FleeAction[Progress]);
						}
					}
					else
					{
						if (Processor & 0x100)
						{
							DeleteMob(index, 3);
							continue;
						}
						if (Processor & 0x1000)
						{
							pMob[index].GetRandomPos();

							if (pMob[index].NextX == pMob[index].TargetX && pMob[index].NextY == pMob[index].TargetY)
								continue;

							MSG_Action sm;

							if (GetEmptyMobGrid(index, &pMob[index].NextX, &pMob[index].NextY) == FALSE)
								continue;


							GetAction(index, pMob[index].NextX, pMob[index].NextY, (MSG_Action*)&sm);

							sm.Speed = BASE_GetSpeed(&pMob[index].MOB.CurrentScore);
							sm.Effect = 0;

							GridMulticast(index, pMob[index].NextX, pMob[index].NextY, (MSG_STANDARD*)&sm);
						}
						if (Processor & 2)
							DoTeleport(index, pMob[index].NextX, pMob[index].NextY);

						if (Processor & 0x10000)
						{
							if(pMob[index].GenerateIndex >= RUNEQUEST_LV6_MOB_INITIAL && pMob[index].GenerateIndex <= RUNEQUEST_LV6_MOB_END)
							{
								if(Pista[6].Party[0].MobCount < 100 && Pista[6].Party[0].MobCount != 0)
									Pista[6].Party[0].MobCount++;
							}

							DeleteMob(index, 3);
							continue;
						}
					}
				}
			}
		}
	}
#pragma endregion
}

/// Processador de batalha dos mobs em estado MOB_COMBAT.
///
/// Cada segundo processa 1/4 dos mobs (índices `Sec4 + MAX_USER`, de 4 em 4). Mobs com HP zerado
/// são removidos, afetos (buffs/debuffs) são aplicados a 1/16 dos mobs por tick (`index % 16 == Sec16`)
/// e mobs da Pista +6 que saem da área são descartados contando para `Pista[6]`.
/// O `BattleMode` do mob define o comportamento (bits 0x1, 0x2, 0x10, 0x20, 0x100, 0x1000);
/// os ataques especiais do Kefra (skills 109/110/111) usam o rótulo `KefraAttackLabel`.
/// @param Sec4  `SecCounter % 4`: deslocamento do subconjunto de mobs processado neste tick.
/// @param Sec16 `SecCounter % 16`: subconjunto que recebe ProcessAffect() neste tick.
void ProcessMobCombatTick(int Sec4, int Sec16)
{
#pragma region Ataque / fala em ação dos mobs
	for (int index = Sec4 + MAX_USER; index < MAX_MOB; index += 4)
	{
		if (pMob[index].Mode != MOB_COMBAT)
			continue;

		int flag = 0;

		if (pMob[index].MOB.Clan == 4)
			flag = FALSE;

		if (pMob[index].MOB.CurrentScore.Hp <= 0)
		{
			pMob[index].MOB.CurrentScore.Hp = 0;
			
			sprintf(temp, "err, battleprocessor delete hp 0 idx:%d leader:%d fol0:%d fol1:%d", index, pMob[index].Leader, pMob[index].PartyList[0], pMob[index].PartyList[1]);
			Log(temp, "-system", NULL);
			
			DeleteMob(index, 1);
			continue;
		}

		if (index % 16 == Sec16)
			ProcessAffect(index);

		if(pMob[index].GenerateIndex != RUNEQUEST_LV6_MOB_BOSS && pMob[index].TargetX >= 3423 && pMob[index].TargetX <= 3442 && pMob[index].TargetY >= 1492 && pMob[index].TargetY <= 1511)
		{
			if(pMob[index].GenerateIndex >= RUNEQUEST_LV6_MOB_INITIAL2 && pMob[index].GenerateIndex <= RUNEQUEST_LV6_MOB_END2)
			{
				if(Pista[6].Party[0].MobCount < 100 && Pista[6].Party[0].MobCount != 0)
					Pista[6].Party[0].MobCount++;
			}

			DeleteMob(index, 3);
			continue;
		}

		int Target = pMob[index].CurrentTarget;

		if (Target > 0 && Target < MAX_MOB)
		{
			int leader = pMob[index].Leader;
			if (leader == MOB_EMPTY)
				leader = index;

			int targetLeader = pMob[Target].Leader;
			
			if (targetLeader == MOB_EMPTY)
				targetLeader = Target;

			int indexguild = pMob[index].MOB.Guild;
			if (pMob[index].GuildDisable)
				indexguild = 0;

			int targetguild = pMob[Target].MOB.Guild;
			if (pMob[Target].GuildDisable)
				targetguild = 0;

			if (indexguild == 0 && targetguild == 0)
				indexguild = -1;

			if (leader == targetLeader || indexguild == targetguild)
				pMob[index].RemoveEnemyList(Target);
		}
		
		int BattleMode = pMob[index].BattleProcessor();
		
		if (BattleMode & 0x20)
		{
			DeleteMob(index, 3);
			continue;
		}

		if (BattleMode & 0x1000)
		{
			if (Target <= MOB_EMPTY || Target >= MAX_MOB)
				continue;

			
			if (pMob[Target].MOB.CurrentScore.Hp <= 0)
				continue;

			MSG_AttackOne sm;

			//

			int village = BASE_GetVillage(pMob[Target].TargetX, pMob[Target].TargetY);

			unsigned char mapatt = GetAttribute(pMob[Target].TargetX, pMob[Target].TargetY);

			if (village >= 0 && village <= 4 && Target < MAX_USER && index >= MAX_USER)
			{
				if ((mapatt & 4) == 0 && (mapatt & 0x40) == 0)
				{
					FILE *fpp = NULL;

					fpp = fopen("AttackDieTeste.txt", "a+");

					fprintf(fpp, "nome:%s x:%d y:%d conn:%d mode:%d atname:%s atx:%d aty:%d idx:%d\n", pMob[Target].MOB.MobName, pMob[Target].TargetX, pMob[Target].TargetY, Target, pUser[Target].Mode, pMob[index].MOB.MobName, pMob[index].TargetX, pMob[index].TargetY, index);

					fclose(fpp);
				}
			}

			//

			GetAttack(index, Target, &sm);

			sm.FlagLocal = 0;

			int targetbackup = Target;

			Target = sm.Dam[0].TargetID;

			if (GetInHalf(Target, index) == 0)
			{
				FILE *fp = NULL;

				fp = fopen("Gethalf.txt", "a+");

				if (fp != NULL)
				{
					fprintf(fp, "Attacker(%s, %d, %d, %d, %d, %d, %d) Targetbackup(%s, %d, %d, %d, %d, %d, %d) Target(%s, %d, %d, %d, %d, %d, %d)\n",
						pMob[index].MOB.MobName, index, pMob[index].TargetX, pMob[index].TargetY, targetbackup, Target, pMob[index].CurrentTarget,
						pMob[targetbackup].MOB.MobName, targetbackup, pMob[targetbackup].TargetX, pMob[targetbackup].TargetY, index, Target, pMob[targetbackup].CurrentTarget,
						pMob[Target].MOB.MobName, Target, pMob[Target].TargetX, pMob[Target].TargetY, targetbackup, Target, pMob[Target].CurrentTarget);
					
					int sx1 = pMob[index].TargetX - HALFGRIDX;
					int sy1 = pMob[index].TargetY - HALFGRIDY;
					int sx2 = pMob[index].TargetX + HALFGRIDX;
					int sy2 = pMob[index].TargetY + HALFGRIDY;

					for (int y = sy1; y < sy2; y++)
					{
						for (int x = sx1; x < sx2; x++)
						{
							if (x < 0 || x >= MAX_GRIDX || y < 0 || y >= MAX_GRIDY)
								continue;

							int tmob = pMobGrid[y][x];

							if (tmob <= 0 || tmob >= MAX_MOB)
								continue;

							fprintf(fp, "\tindex:%d nome:%s \r\n", tmob, pMob[tmob].MOB.MobName);
						}
					}

					fclose(fp);
				}

				continue;
			}

			int skill = sm.SkillIndex;

			//109 : Raio Vermelho
			//110 : Empurrão
			//111 : Chão de espinhos
			if(skill == 109 || skill == 110 || skill == 111)
				goto KefraAttackLabel;
			
			if (skill >= 0 && skill < MAX_SKILLINDEX && sm.SkillParm == 0)
			{
				if (skill == 33)
					sm.Motion = 253;

				int kind = ((skill % MAX_SKILL / 8) + 1);
				
				if (kind >= 1 && kind <= 3)
				{
					int special = pMob[index].MOB.CurrentScore.Special[kind];
					BOOL NeedUpdate = FALSE;
					
					if (SetAffect(Target, skill, 100, special))
						NeedUpdate = TRUE;

					if (SetTick(Target, skill, 100, special))
						NeedUpdate = TRUE;

					if (NeedUpdate == TRUE)
						SendScore(Target);
				}
			}

			if (Target < MAX_USER && sm.Dam[0].Damage > 0)
			{
				int attackdex = pMob[index].MOB.CurrentScore.Dex / 5;

				if (pMob[index].MOB.LearnedSkill & 0x1000000)
					attackdex += 100;

				if (pMob[index].MOB.Rsv & 0x40)
					attackdex += 500;

				int parryretn = GetParryRate(&pMob[Target].MOB, pMob[Target].Parry, attackdex, pMob[index].MOB.Rsv);

				if (sm.SkillIndex == 79 || sm.SkillIndex == 22)
					parryretn = 30 * parryretn / 100;

				int rd = rand() % 1000 + 1;

				if (rd < parryretn)
				{
					sm.Dam[0].Damage = -3;

					if ((pMob[Target].MOB.Rsv & 0x200) != 0 && rd < 100)
						sm.Dam[0].Damage = -4;
				}

				else if (pMob[index].MOB.Clan == 4)
					sm.Dam[0].Damage = (2 * sm.Dam[0].Damage) / 5;
			}

			int TargetLeader = pMob[Target].Leader;
			
			if (sm.Dam[0].Damage > 0)
			{
				if (TargetLeader <= MOB_EMPTY)
					TargetLeader = Target;

				SetBattle(TargetLeader, index);

				if (pMob[index].MOB.Clan != 4)
					SetBattle(index, TargetLeader);

				for (int z = 0; z < MAX_PARTY; z++)
				{
					int partyconn = pMob[TargetLeader].PartyList[z];
					if (partyconn <= MAX_USER)
						continue;

					if (pMob[partyconn].Mode == MOB_EMPTY || pMob[partyconn].MOB.CurrentScore.Hp <= 0)
					{
						if (pMob[partyconn].Mode != MOB_EMPTY)
						{
							pMob[partyconn].MOB.CurrentScore.Hp = 0;
							DeleteMob(partyconn, 1);
						}

						pMob[TargetLeader].PartyList[z] = MOB_EMPTY;
						continue;
					}

					SetBattle(partyconn, index);
					SetBattle(index, partyconn);
				}

				int Summoner = pMob[index].Summoner;
				
				if (pMob[index].MOB.Clan == 4 && Target >= MAX_USER && Summoner > 0 && Summoner < MAX_USER && pUser[Summoner].Mode == USER_PLAY)
				{
					int posX = pMob[Summoner].TargetX;
					int posY = pMob[Summoner].TargetY;
					int num = 46;
					int dam = 0;
					int exp = 0;
					int summdam = 0;

					if (pMob[index].TargetX > posX - num && pMob[index].TargetX < posX + num && pMob[index].TargetY > posY - num && pMob[index].TargetY < posY + num)
					{
						if (pMob[Target].MOB.CurrentScore.Hp < sm.Dam[0].Damage)
							summdam = pMob[Target].MOB.CurrentScore.Hp;
						
						else
							summdam = sm.Dam[0].Damage;
					}
				}
			}

			if (sm.Dam[0].Damage > 0 || sm.Dam[0].Damage <= -5)
			{
				int DamageNow = sm.Dam[0].Damage;
				int damage = 0;
				int mountindex = pMob[Target].MOB.Equip[14].sIndex;

				if (Target < MAX_USER && mountindex >= 2360 && mountindex < 2390 && pMob[Target].MOB.Equip[14].stEffect[0].sValue > 0)
				{
					DamageNow = (3 * sm.Dam[0].Damage) / 4;
					damage = sm.Dam[0].Damage - DamageNow;

					if (DamageNow <= 0)
						DamageNow = 1;

					sm.Dam[0].Damage = DamageNow;
				}

				for (int c = 0; c < MAX_AFFECT && Target < MAX_USER; c++)
				{
					if (pMob[Target].Affect[c].Type == 18)
					{
						if (pMob[Target].MOB.CurrentScore.Mp > ((pMob[Target].MOB.CurrentScore.MaxMp / 100) * 10))
						{
							int mana = pMob[Target].MOB.CurrentScore.Mp - (DamageNow >> 1);

							if (mana < 0)
							{
								DamageNow -= mana;
								mana = 0;
							}

							pMob[Target].MOB.CurrentScore.Mp = mana;

							pUser[Target].ReqMp = pUser[Target].ReqMp - (DamageNow >> 1);
							SetReqMp(Target);

							SendSetHpMp(Target);

							DamageNow = ((DamageNow >> 1) + (DamageNow << 4)) / 80;
							sm.Dam[0].Damage = DamageNow;
						}
					}
				}


				if (pMob[Target].MOB.Equip[13].sIndex == 786)
				{
					int sanc = pMob[Target].MOB.Equip[13].stEffect[0].cValue;
					
					if (sanc < 2)
						sanc = 2;

					sanc *= 2;

					pMob[Target].MOB.CurrentScore.Hp = pMob[Target].MOB.CurrentScore.Hp - (sm.Dam[0].Damage / sanc);
				}
				else if (pMob[Target].MOB.Equip[13].sIndex == 1936)
				{
					int sanc = pMob[Target].MOB.Equip[13].stEffect[0].cValue;
					
					if (sanc < 2)
						sanc = 2;

					sanc *= 10;

					pMob[Target].MOB.CurrentScore.Hp = pMob[Target].MOB.CurrentScore.Hp - (sm.Dam[0].Damage / sanc);
				}
				else if (pMob[Target].MOB.Equip[13].sIndex == 1937)
				{
					int sanc = pMob[Target].MOB.Equip[13].stEffect[0].cValue;
					
					if (sanc < 2)
						sanc = 2;

					sanc *= 1000;

					pMob[Target].MOB.CurrentScore.Hp = pMob[Target].MOB.CurrentScore.Hp - (sm.Dam[0].Damage / sanc);
				}
				else
				{
					if (pMob[Target].MOB.CurrentScore.Hp < sm.Dam[0].Damage)
						pMob[Target].MOB.CurrentScore.Hp = 0;

					else
						pMob[Target].MOB.CurrentScore.Hp = pMob[Target].MOB.CurrentScore.Hp - sm.Dam[0].Damage;
				}

				if (Target > MAX_USER && pMob[Target].MOB.Clan == 4)
					LinkMountHp(Target);

				if (damage > 0)
					ProcessAdultMount(Target, damage);
			}

			GridMulticast(pMob[Target].TargetX, pMob[Target].TargetY, (MSG_STANDARD*)&sm, 0);

			if (Target > 0 && Target < MAX_USER)
			{
				if (pUser[Target].ReqHp < sm.Dam[0].Damage)
					pUser[Target].ReqHp = 0;

				else
					pUser[Target].ReqHp = pUser[Target].ReqHp - sm.Dam[0].Damage;

				SetReqHp(Target);
				SendSetHpMp(Target);
			}
			else
				SendScore(Target);

			if (pMob[Target].MOB.CurrentScore.Hp <= 0)
			{
				pMob[Target].MOB.CurrentScore.Hp = 0;
				
				MobKilled(Target, index, 0, 0);
			}
		}

		//Kefra
		if (0 == 1)
		{
KefraAttackLabel:
			MSG_Attack sm;
			memset(&sm, 0, sizeof(MSG_Attack));

			GetAttackArea(index, &sm);
			MobAttack(index, sm);
		}

		if (BattleMode & 0x100)
		{
			int TargetID = pMob[index].CurrentTarget;
			pMob[index].GetTargetPosDistance(TargetID);

			if (pMob[index].NextX == pMob[index].TargetX && pMob[index].NextY == pMob[index].TargetY)
				continue;

			MSG_Action sm;
			if (GetEmptyMobGrid(index, &pMob[index].NextX, &pMob[index].NextY) == FALSE)
				continue;

			GetAction(index, pMob[index].NextX, pMob[index].NextY, (MSG_Action*)&sm);

			sm.Speed = BASE_GetSpeed(&pMob[index].MOB.CurrentScore);
			sm.Effect = 0;

			GridMulticast(index, pMob[index].NextX, pMob[index].NextY, (MSG_STANDARD*)&sm);
		}

		if (BattleMode & 1)
		{
			Target = pMob[index].CurrentTarget;
			pMob[index].GetTargetPos(Target);

			if (pMob[index].NextX == pMob[index].TargetX && pMob[index].NextY == pMob[index].TargetY)
				continue;

			MSG_Action sm;
			if (GetEmptyMobGrid(index, &pMob[index].NextX, &pMob[index].NextY) == FALSE)
				continue;

			GetAction(index, pMob[index].NextX, pMob[index].NextY, (MSG_Action*)&sm);

			sm.Speed = BASE_GetSpeed(&pMob[index].MOB.CurrentScore);
			sm.Effect = 0;

			GridMulticast(index, pMob[index].NextX, pMob[index].NextY, (MSG_STANDARD*)&sm);
		}

		if (BattleMode & 2)
			DoTeleport(index, pMob[index].NextX, pMob[index].NextY);

		if (BattleMode & 0x10)
		{
			MSG_Action sm;
			
			if (GetEmptyMobGrid(index, &pMob[index].NextX, &pMob[index].NextY) == FALSE)
				continue;

			GetAction(index, pMob[index].NextX, pMob[index].NextY, (MSG_Action*)&sm);

			if (pMob[index].NextX == pMob[index].TargetX && pMob[index].NextY == pMob[index].TargetY)
				continue;

			sm.Speed = BASE_GetSpeed(&pMob[index].MOB.CurrentScore);
			sm.Effect = 0;

			GridMulticast(index, pMob[index].NextX, pMob[index].NextY, (MSG_STANDARD*)&sm);
		}
	}
#pragma endregion
}
