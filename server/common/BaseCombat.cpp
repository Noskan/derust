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

#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "common/Basedef.h"
#include "common/ItemEffect.h"
#include "common/BasedefInternal.h"

/// @file BaseCombat.cpp
/// @brief Recalculo de atributos de combate, habilidades e pathfinding.

/// Calcula total de pontos de skill esperados para o nivel/evolucao do personagem.
int BASE_GetBonusSkillPoint(STRUCT_MOB *mob, STRUCT_MOBEXTRA *extra)
{
	int spellperlevel = mob->CurrentScore.Level * 3;

	if (extra->ClassMaster == ARCH)
		spellperlevel += 112;

	if (extra->ClassMaster != ARCH && extra->ClassMaster != MORTAL)
		spellperlevel += 1500;

	int mod = mob->CurrentScore.Level - 199;

	if(mod > 0)
		spellperlevel += mod;

	int quest = extra->QuestInfo.Mortal.PilulaOrc;

	if(quest != 0)
		spellperlevel += 9;

	int cls = mob->Class;

	if(cls < 0 || cls >= MAX_CLASS)
		return FALSE;

	int spelluse = 0;
	int skillbit = 1;

	for(int j = 0; j < MAX_SKILL; j++)
	{
		if((mob->LearnedSkill & skillbit) != 0)
			spelluse += g_pSpell[((cls * MAX_SKILL) + j)].SkillPoint;

		skillbit *= 2;
	}

	int rest = spellperlevel - spelluse;

	if(rest != 0)
	{
		mob->SkillBonus = rest;

		return FALSE;
	}
	
	return TRUE;
}

/// Calcula total de pontos de atributo disponiveis conforme classe, evolucao e nivel.
int BASE_GetBonusScorePoint(STRUCT_MOB *mob, STRUCT_MOBEXTRA *extra)
{
	int cls = mob->Class;

	if(cls < 0 || cls >= MAX_CLASS)
		return FALSE;

	int usestr = mob->BaseScore.Str - BaseSIDCHM[cls][0];
	int useint = mob->BaseScore.Int - BaseSIDCHM[cls][1];
	int usedex = mob->BaseScore.Dex - BaseSIDCHM[cls][2];
	int usecon = mob->BaseScore.Con - BaseSIDCHM[cls][3];

	if (extra->ClassMaster == MORTAL)
	{
		int totaluse = usestr + useint + usedex + usecon;

		int lvl = mob->BaseScore.Level;

		int leveluse = lvl * 5;

		if (lvl >= 254)
			leveluse += (lvl - 254) * 5;

		if (lvl >= 299)
			leveluse += (lvl - 299) * 10;

		if (lvl >= 354)
			leveluse += (lvl - 354) * -8;

		if(totaluse > leveluse)
		{
			int dif = totaluse - leveluse;

			if(mob->BaseScore.Str >= dif)
				mob->BaseScore.Str -= dif;

			else if(mob->BaseScore.Int >= dif)
				mob->BaseScore.Int -= dif;

			else if(mob->BaseScore.Dex >= dif)
				mob->BaseScore.Dex -= dif;

			else if(mob->BaseScore.Con >= dif)
				mob->BaseScore.Con -= dif;

			totaluse = mob->BaseScore.Str + mob->BaseScore.Int + mob->BaseScore.Dex + mob->BaseScore.Con - BaseSIDCHM[cls][0] - BaseSIDCHM[cls][1] - BaseSIDCHM[cls][2] - BaseSIDCHM[cls][3];
		}
		mob->ScoreBonus = leveluse - totaluse;
	}
	else if (extra->ClassMaster == ARCH)
	{
		int totaluse = usestr + useint + usedex + usecon;

		int lvl = mob->BaseScore.Level;

		int leveluse = lvl * 6;

		leveluse += extra->QuestInfo.Arch.MortalLevel * 6;

		if (lvl >= 354)
			leveluse += (lvl - 354) * 6;

		if(totaluse > leveluse)
		{
			int dif = totaluse - leveluse;

			if(mob->BaseScore.Str >= dif)
				mob->BaseScore.Str -= dif;

			else if(mob->BaseScore.Int >= dif)
				mob->BaseScore.Int -= dif;

			else if(mob->BaseScore.Dex >= dif)
				mob->BaseScore.Dex -= dif;

			else if(mob->BaseScore.Con >= dif)
				mob->BaseScore.Con -= dif;

			totaluse = mob->BaseScore.Str + mob->BaseScore.Int + mob->BaseScore.Dex + mob->BaseScore.Con - BaseSIDCHM[cls][0] - BaseSIDCHM[cls][1] - BaseSIDCHM[cls][2] - BaseSIDCHM[cls][3];
		}
		mob->ScoreBonus = leveluse - totaluse;
	}
	else if (extra->ClassMaster == CELESTIAL)
	{
		int totaluse = usestr + useint + usedex + usecon;

		int lvl = mob->BaseScore.Level;

		int leveluse = lvl * 6;

		leveluse += (extra->QuestInfo.Arch.Cristal * 100);
		leveluse += (extra->QuestInfo.Celestial.Reset * 200);

		leveluse += 1001;

		if (extra->QuestInfo.Celestial.ArchLevel == 1)
			leveluse += 100;

		else if (extra->QuestInfo.Celestial.ArchLevel == 2)
			leveluse += 300;

		else if (extra->QuestInfo.Celestial.ArchLevel == 3)
			leveluse += 600;

		else if (extra->QuestInfo.Celestial.ArchLevel == 4)
			leveluse += 900;

		else if (extra->QuestInfo.Celestial.ArchLevel == 5)
			leveluse += 1200;


		if (lvl >= 120)
			leveluse += (lvl - 119) * 6;

		if (lvl >= 150)
			leveluse += (lvl - 149) * 2;

		if (lvl >= 170)
			leveluse += (lvl - 169) * 2;

		if (lvl >= 180)
			leveluse += (lvl - 179) * 2;

		if (lvl >= 190)
			leveluse += (lvl - 189) * 2;

		if(totaluse > leveluse)
		{
			int dif = totaluse - leveluse;

			if(mob->BaseScore.Str >= dif)
				mob->BaseScore.Str -= dif;

			else if(mob->BaseScore.Int >= dif)
				mob->BaseScore.Int -= dif;

			else if(mob->BaseScore.Dex >= dif)
				mob->BaseScore.Dex -= dif;

			else if(mob->BaseScore.Con >= dif)
				mob->BaseScore.Con -= dif;

			totaluse = mob->BaseScore.Str + mob->BaseScore.Int + mob->BaseScore.Dex + mob->BaseScore.Con - BaseSIDCHM[cls][0] - BaseSIDCHM[cls][1] - BaseSIDCHM[cls][2] - BaseSIDCHM[cls][3];
		}
		mob->ScoreBonus = leveluse - totaluse;
	}
	else if (extra->ClassMaster == CELESTIALCS)
	{
		int totaluse = usestr + useint + usedex + usecon;

		int lvl = mob->BaseScore.Level;

		int leveluse = lvl * 6;

		leveluse += (extra->QuestInfo.Arch.Cristal * 100);
		leveluse += (extra->QuestInfo.Celestial.Reset * 200);

		leveluse += 1001;

		if (extra->QuestInfo.Celestial.ArchLevel == 1)
			leveluse += 100;

		else if (extra->QuestInfo.Celestial.ArchLevel == 2)
			leveluse += 300;

		else if (extra->QuestInfo.Celestial.ArchLevel == 3)
			leveluse += 600;

		else if (extra->QuestInfo.Celestial.ArchLevel == 4)
			leveluse += 900;

		else if (extra->QuestInfo.Celestial.ArchLevel == 5)
			leveluse += 1200;


		if (lvl >= 120)
			leveluse += (lvl - 119) * 6;

		if (lvl >= 150)
			leveluse += (lvl - 149) * 2;

		if (lvl >= 170)
			leveluse += (lvl - 169) * 2;

		if (lvl >= 180)
			leveluse += (lvl - 179) * 2;

		if (lvl >= 190)
			leveluse += (lvl - 189) * 2;

		lvl = extra->QuestInfo.Celestial.SubCelestialLevel;

		leveluse += (lvl * 6)/2;

		if (lvl >= 120)
			leveluse += ((lvl - 119) * 6);

		if (lvl >= 150)
			leveluse += ((lvl - 149) * 2);

		if (lvl >= 170)
			leveluse += ((lvl - 169) * 2);

		if (lvl >= 180)
			leveluse += ((lvl - 179) * 2);

		if (lvl >= 190)
			leveluse += ((lvl - 189) * 2);

		if(totaluse > leveluse)
		{
			int dif = totaluse - leveluse;

			if(mob->BaseScore.Str >= dif)
				mob->BaseScore.Str -= dif;

			else if(mob->BaseScore.Int >= dif)
				mob->BaseScore.Int -= dif;

			else if(mob->BaseScore.Dex >= dif)
				mob->BaseScore.Dex -= dif;

			else if(mob->BaseScore.Con >= dif)
				mob->BaseScore.Con -= dif;

			totaluse = mob->BaseScore.Str + mob->BaseScore.Int + mob->BaseScore.Dex + mob->BaseScore.Con - BaseSIDCHM[cls][0] - BaseSIDCHM[cls][1] - BaseSIDCHM[cls][2] - BaseSIDCHM[cls][3];
		}
		mob->ScoreBonus = leveluse - totaluse;
	}

	else if (extra->ClassMaster == SCELESTIAL)
	{
		int totaluse = usestr + useint + usedex + usecon;

		int lvl = mob->BaseScore.Level;

		int leveluse = lvl * 6;

		leveluse += (extra->QuestInfo.Arch.Cristal * 100);
		leveluse += (extra->QuestInfo.Celestial.Reset * 200);

		leveluse += 1001;

		if (extra->QuestInfo.Celestial.ArchLevel == 1)
			leveluse += 100;

		else if (extra->QuestInfo.Celestial.ArchLevel == 2)
			leveluse += 300;

		else if (extra->QuestInfo.Celestial.ArchLevel == 3)
			leveluse += 600;

		else if (extra->QuestInfo.Celestial.ArchLevel == 4)
			leveluse += 900;

		else if (extra->QuestInfo.Celestial.ArchLevel == 5)
			leveluse += 1200;


		if (lvl >= 120)
			leveluse += (lvl - 119) * 6;

		if (lvl >= 150)
			leveluse += (lvl - 149) * 2;

		if (lvl >= 170)
			leveluse += (lvl - 169) * 2;

		if (lvl >= 180)
			leveluse += (lvl - 179) * 2;

		if (lvl >= 190)
			leveluse += (lvl - 189) * 2;

		lvl = extra->QuestInfo.Celestial.CelestialLevel;
		leveluse += (lvl * 6)/2;

		if (lvl >= 120)
			leveluse += ((lvl - 119) * 6);

		if (lvl >= 150)
			leveluse += ((lvl - 149) * 2);

		if (lvl >= 170)
			leveluse += ((lvl - 169) * 2);

		if (lvl >= 180)
			leveluse += ((lvl - 179) * 2);

		if (lvl >= 190)
			leveluse += ((lvl - 189) * 2);


		if(totaluse > leveluse)
		{
			int dif = totaluse - leveluse;

			if(mob->BaseScore.Str >= dif)
				mob->BaseScore.Str -= dif;

			else if(mob->BaseScore.Int >= dif)
				mob->BaseScore.Int -= dif;

			else if(mob->BaseScore.Dex >= dif)
				mob->BaseScore.Dex -= dif;

			else if(mob->BaseScore.Con >= dif)
				mob->BaseScore.Con -= dif;

			totaluse = mob->BaseScore.Str + mob->BaseScore.Int + mob->BaseScore.Dex + mob->BaseScore.Con - BaseSIDCHM[cls][0] - BaseSIDCHM[cls][1] - BaseSIDCHM[cls][2] - BaseSIDCHM[cls][3];
		}
		mob->ScoreBonus = leveluse - totaluse;
	}
	return TRUE;
}

/// Calcula HP e MP base acumulados do personagem sem buffs/equipamentos.
int BASE_GetHpMp(STRUCT_MOB *mob, STRUCT_MOBEXTRA *extra)
{
	int cls = mob->Class;

	if(cls < 0 || cls >= MAX_CLASS)
		return FALSE;


	int basehp = BaseSIDCHM[cls][4];	
	int stathp = (mob->BaseScore.Con - BaseSIDCHM[cls][3]) * 2;
	int levelhp = ((extra->ClassMaster == ARCH || extra->ClassMaster == MORTAL) ? mob->CurrentScore.Level : mob->CurrentScore.Level + MAX_LEVEL) * g_pIncrementHp[cls];

	int t_hp = basehp + stathp + levelhp;

	if (t_hp >= MAX_HP)
		t_hp = MAX_HP;

	mob->BaseScore.MaxHp = t_hp;
	mob->CurrentScore.MaxHp = t_hp;

	int basemp = BaseSIDCHM[cls][5];
	int statmp = (mob->BaseScore.Int - BaseSIDCHM[cls][1]) * 2;
	int levelmp = ((extra->ClassMaster == ARCH || extra->ClassMaster == MORTAL) ? mob->CurrentScore.Level : mob->CurrentScore.Level + MAX_LEVEL) * g_pIncrementMp[cls];

	int t_mp = basemp + statmp + levelmp;

	if (t_mp >= MAX_MP)
		t_mp = MAX_MP;


	mob->BaseScore.MaxMp = t_mp;
	mob->CurrentScore.MaxMp = t_mp;

	return TRUE;
}

/// Calcula velocidade de movimento efetiva limitada pelos caps do servidor.
int BASE_GetSpeed(STRUCT_SCORE *score)
{
	int Run = score->AttackRun;

	Run &= 15;

	if(Run < 1)
		Run = 1;

	if(Run > 6)
		Run = 6;

	return Run;
}

/// Calcula dano fisico final aplicando defesa do alvo e modificador de combate.
int BASE_GetDamage(int dam, int ac, int combat)
{
	long long tdam = dam - ac / 2;
	combat /= 2;

	if(combat > 7)
		combat = 7;

	int delta = 12 - combat;

	int rnd = rand() % delta + combat + 99;

	tdam = (long long)rnd * tdam / 100;

	if(tdam < -50)
		tdam = 0;
	else if(tdam >= -50 && tdam < 0)
	{
		tdam += 50;
		tdam /= 7;
	}
	else if(tdam >= 0 && tdam <= 50)
	{
		tdam = 5 * tdam / 4;
		tdam += 7;
	}

	if(tdam <= 0)
		tdam = 1;

	return (int)tdam;
}

/// Calcula dano de habilidade fisica/magica considerando atributos, arma e clima.
int BASE_GetSkillDamage(int dam, int ac, int combat)
{
	int tdam = dam - ((ac/ 2));

	if(combat > 15)
		combat = 15;

	int delta = 21 - combat;
	int rnd = rand() % delta + combat + 90;

	tdam = (tdam * rnd) / 100;

	if(tdam < -50)
		tdam = 0;

	else if(tdam > -50 && tdam < 0)
	{
		tdam += 50;
		tdam /= 10;
	}
	else if(tdam >= 0 && tdam <= 45)
	{
		tdam = 5 * tdam / 4;
		tdam += 5;
	}

	if(tdam <= 0)
		tdam = 1;


	return tdam;
}

/// Soma o valor de um efeito (EF_*) em todos os slots equipados do mob.
int BASE_GetMobAbility(STRUCT_MOB *mob, unsigned char Type)
{
	int value = 0;
	int nUnique[MAX_EQUIP];

	if(Type == EF_RANGE)
	{
		value = BASE_GetMaxAbility(mob, Type);

		int cls = mob->Equip[0].sIndex / 10;

		if(value < 2 && cls == 3)
		{
			if((mob->LearnedSkill & 0x100000) != 0)
                value = 2;
		}

		return value;
	}

	for(int i = 0; i < MAX_EQUIP; i++)
	{
		nUnique[i] = 0;

		int item = mob->Equip[i].sIndex;

		if(item == 0 && i != 7)
			continue;

		if(i >= 1 && i <= 5)
			nUnique[i] = g_pItemList[item].nUnique;

		if(Type == EF_DAMAGE && i == 6)
            continue;

		if(Type == EF_MAGIC && i == 7)
            continue;

		if(i == 7 && Type == EF_DAMAGE)
		{
			int ldam = BASE_GetItemAbility(&mob->Equip[6], Type) + BASE_GetItemAbility(&mob->Equip[6], EF_DAMAGE2);
			int rdam = BASE_GetItemAbility(&mob->Equip[7], Type) + BASE_GetItemAbility(&mob->Equip[7], EF_DAMAGE2);

			int lidx = mob->Equip[6].sIndex;
			int ridx = mob->Equip[7].sIndex;

			int ltype = 0;

			if(lidx > 0 && lidx < MAX_ITEMLIST)
                ltype = g_pItemList[lidx].nUnique;

            int rtype = 0;

            if(ridx > 0 && ridx < MAX_ITEMLIST)
                rtype = g_pItemList[ridx].nUnique;

			if(ltype != 0 && rtype != 0)
			{
				int multi = 0;

				if(ltype == rtype)
                    multi = 50;
				else
					multi = 30;

				//Pericia do caçador
				if(mob->LearnedSkill & (1 << 10) && mob->Class == 3)
					multi = 100;

				//Mestre das Armas
				if(mob->LearnedSkill & (1 << 9) && mob->Class == 0)
					multi = 100;

				if(ldam > rdam)
                    value = ((value + ldam) + ((rdam * multi) / 100));
                else
                    value = ((value + rdam) + ((ldam * multi) / 100));

				continue;
			}

			if(ldam > rdam)
                value += ldam;
            else
                value += rdam;

			continue;
		}

		int tvalue = 0;

		tvalue = BASE_GetItemAbility(&mob->Equip[i], Type);

		if(Type == EF_ATTSPEED && tvalue == 1)
            tvalue = 10;

		value += tvalue;
	}

	 if(Type == EF_AC && nUnique[1] != 0)
	 {
        if(nUnique[1] == nUnique[2] && nUnique[2] == nUnique[3] && nUnique[3] == nUnique[4] && nUnique[4] == nUnique[5])
            value = ((value * 105) / 100);
	 }

	 return value;
}

/// Retorna o maior valor de um efeito (EF_*) entre os equipamentos do mob.
int BASE_GetMaxAbility(STRUCT_MOB *mob, unsigned char Type)
{
	int value = 0;
	int tvalue = 0;

	for(int i = 0; i < MAX_EQUIP; i++)
	{
		int item = mob->Equip[i].sIndex;

		if(item == 0)
			continue;

		tvalue = BASE_GetItemAbility(&mob->Equip[i], Type);

		if(value < tvalue)
			value = tvalue;
	}

	return value;
}

/// Calcula checksum de integridade da estrutura STRUCT_MOB.
int BASE_GetMobCheckSum(STRUCT_MOB *mob)
{
	int sum = 0;
	int i;

	memset((void*)&mob->BaseScore, 0, sizeof(mob->BaseScore));

	for(i = 0; i < MAX_EQUIP; i++)   
		sum = sum + mob->Equip[i].sIndex * i;

	for(i = 0; i < MAX_CARRY; i++)    
		sum = sum + mob->Carry[i].sIndex * i;

	return sum;
}

/// Inicializa/zera uma estrutura STRUCT_MOB.
void BASE_ClearMob(STRUCT_MOB *mob)
{
	memset(mob, 0, sizeof(STRUCT_MOB));

	mob->SPX = 2112;
	mob->SPY = 2112;

	memset(&mob->BaseScore, 0, sizeof(STRUCT_SCORE));
	memset(&mob->CurrentScore, 0, sizeof(STRUCT_SCORE));

	for(int i = 0; i < MAX_EQUIP; i++)
		BASE_ClearItem(&mob->Equip[i]);

	for(int i = 0; i < MAX_CARRY; i++)
		BASE_ClearItem(&mob->Carry[i]);

	mob->SkillBar[0] = 0;
	mob->SkillBar[1] = 0;
	mob->SkillBar[2] = 0;
	mob->SkillBar[3] = 0;
}

/// Inicializa/zera uma estrutura STRUCT_MOBEXTRA.
void BASE_ClearMobExtra(STRUCT_MOBEXTRA *extra)
{
	memset(extra, 0, sizeof(STRUCT_MOBEXTRA));

	extra->ClassMaster = MORTAL;
	memset(&extra->QuestInfo, 0, sizeof(extra->QuestInfo));
}

/// Recalcula todos os atributos atuais (CurrentScore), dano, defesa, resistencias e HP/MP do mob.
void BASE_GetCurrentScore(STRUCT_MOB & MOB, STRUCT_AFFECT *Affect, STRUCT_MOBEXTRA *extra, int *ExpBonus, int *ForceMobDamage, int isSummon, int *Accuracy, int *AbsHp, int *ForceDamage)
{
	MOB.Rsv = 0;

	int hp = MOB.CurrentScore.Hp;
	int mp = MOB.CurrentScore.Mp;
	MOB.CurrentScore = MOB.BaseScore;

	MOB.CurrentScore.Hp = hp;
	MOB.CurrentScore.Mp = mp;

	int acadd = BASE_GetMobAbility(&MOB, EF_AC) + BASE_GetMobAbility(&MOB, EF_ACADD);

	MOB.CurrentScore.Ac += acadd;
	MOB.CurrentScore.Damage += BASE_GetMobAbility(&MOB, EF_DAMAGE);

	MOB.CurrentScore.MaxHp += BASE_GetMobAbility(&MOB, EF_HP);
	MOB.CurrentScore.MaxMp += BASE_GetMobAbility(&MOB, EF_MP);

	MOB.CurrentScore.Str += BASE_GetMobAbility(&MOB, EF_STR);
	MOB.CurrentScore.Int += BASE_GetMobAbility(&MOB, EF_INT);
	MOB.CurrentScore.Dex += BASE_GetMobAbility(&MOB, EF_DEX);
	MOB.CurrentScore.Con += BASE_GetMobAbility(&MOB, EF_CON);

	int isKibitaSoul = 0;

	if(isSummon == 0)
	{
#pragma region Soul Hp/Mp add
		int bInt = MOB.CurrentScore.Int;
		int bCon = MOB.CurrentScore.Con;

		for (int i = 0; i < MAX_AFFECT; i++)
		{
			int Type = Affect[i].Type;

			if (Type == 29 && extra->ClassMaster != MORTAL && extra->ClassMaster != ARCH)//Soul
			{
				if (extra->Soul == SOUL_I)
					bInt = (int)(bInt * 2.2f);

				if (extra->Soul == SOUL_C)
					bCon = (int)(bCon * 2.2f);

				else if (extra->Soul == SOUL_FI)
					bInt = (int)(bInt * 1.40f);

				else if (extra->Soul == SOUL_FC)
					bCon = (int)(bCon * 1.40f);


				else if (extra->Soul == SOUL_IF)
					bInt = (int)(bInt * 1.80f);

				else if (extra->Soul == SOUL_ID)
					bInt = (int)(bInt * 1.80f);


				else if (extra->Soul == SOUL_IC)
				{
					bInt = (int)(bInt * 1.80f);
					bCon = (int)(bCon * 1.40f);
				}

				else if (extra->Soul == SOUL_DI)
					bInt = (int)(bInt * 1.40f);

				else if (extra->Soul == SOUL_DC)
					bCon = (int)(bCon * 1.40f);


				else if (extra->Soul == SOUL_CF)
					bCon = (int)(bCon * 1.80f);


				else if (extra->Soul == SOUL_CI)
				{
					bCon = (int)(bCon * 1.80f);
					bInt = (int)(bInt * 1.40f);
				}

				else if (extra->Soul == SOUL_CD)
					bCon = (int)(bCon * 1.80f);
			}

			else if (Type == 29 && extra->ClassMaster == MORTAL && MOB.CurrentScore.Level < 369)//Soul Kibita
				isKibitaSoul = 1;

			else if (Type == 29 && extra->ClassMaster == MORTAL)//Soul
			{
				if (extra->Soul == SOUL_I)
					bInt = (int)(bInt * 1.8f);

				if (extra->Soul == SOUL_C)
					bCon = (int)(bCon * 1.8f);

				else if (extra->Soul == SOUL_FI)
					bInt = (int)(bInt * 1.40f);

				else if (extra->Soul == SOUL_FC)
					bCon = (int)(bCon * 1.40f);


				else if (extra->Soul == SOUL_IF)
					bInt = (int)(bInt * 1.60f);

				else if (extra->Soul == SOUL_ID)
					bInt = (int)(bInt * 1.60f);


				else if (extra->Soul == SOUL_IC)
				{
					bInt = (int)(bInt * 1.60f);
					bCon = (int)(bCon * 1.40f);
				}

				else if (extra->Soul == SOUL_DI)
					bInt = (int)(bInt * 1.40f);

				else if (extra->Soul == SOUL_DC)
					bCon = (int)(bCon * 1.40f);


				else if (extra->Soul == SOUL_CF)
					bCon = (int)(bCon * 1.60f);


				else if (extra->Soul == SOUL_CI)
				{
					bCon = (int)(bCon * 1.60f);
					bInt = (int)(bInt * 1.40f);
				}

				else if (extra->Soul == SOUL_CD)
					bCon = (int)(bCon * 1.60f);
			}
		}

		int ItemInt = bInt - MOB.BaseScore.Int;
		int ItemCon = bCon - MOB.BaseScore.Con;

		int MPDelta = ItemInt * 2;
		int HPDelta = ItemCon * 2;

		unsigned int mhp = MOB.CurrentScore.MaxHp + HPDelta;
		unsigned int mmp = MOB.CurrentScore.MaxMp + MPDelta;


		MOB.CurrentScore.MaxHp += mhp;
		MOB.CurrentScore.MaxMp += mmp;
#pragma endregion
	}

	int special0 = MOB.CurrentScore.Special[0] + BASE_GetMobAbility(&MOB, EF_SPECIAL1);

	if(special0 > 320)
		special0 = 320;

	int special1 = MOB.CurrentScore.Special[1] + BASE_GetMobAbility(&MOB, EF_SPECIAL2) + BASE_GetMobAbility(&MOB, EF_SPECIALALL);

	if(special1 > 320)
		special1 = 320;

	int special2 = MOB.CurrentScore.Special[2] + BASE_GetMobAbility(&MOB, EF_SPECIAL3) + BASE_GetMobAbility(&MOB, EF_SPECIALALL); 

	if(special2 > 320)
		special2 = 320;

	int special3 = MOB.CurrentScore.Special[3] + BASE_GetMobAbility(&MOB, EF_SPECIAL4) + BASE_GetMobAbility(&MOB, EF_SPECIALALL); 

	if(special3 > 320)
		special3 = 320;

	MOB.CurrentScore.Special[0] = special0;
	MOB.CurrentScore.Special[1] = special1;
	MOB.CurrentScore.Special[2] = special2;
	MOB.CurrentScore.Special[3] = special3;

	MOB.SaveMana = BASE_GetMobAbility(&MOB, EF_SAVEMANA);

	int magic = BASE_GetMobAbility(&MOB, EF_MAGIC) + BASE_GetMobAbility(&MOB, EF_MAGICADD);
	magic = (magic + 1) / 4;

	int Run = MOB.CurrentScore.AttackRun & 15; 
	Run += BASE_GetMobAbility(&MOB, EF_RUNSPEED);

	int Att = MOB.BaseScore.AttackRun / 16 * 10;
	Att += BASE_GetMobAbility(&MOB, EF_ATTSPEED);

	if(Run > 6)
		Run = 6;

	int RegenHP =  BASE_GetMobAbility(&MOB, EF_REGENHP);
	int RegenMP = BASE_GetMobAbility(&MOB, EF_REGENMP);

	int Critical = (BASE_GetMobAbility(&MOB, EF_CRITICAL)/4);

	int face = MOB.Equip[0].sIndex / 10;

	if(face < 4)
	{
		MOB.Equip[0].stEffect[0].cEffect = 43;
		MOB.Equip[0].stEffect[0].cValue = 0;

		if(MOB.Clan == 7 || MOB.Clan == 8 || MOB.Clan == 9)
			MOB.Clan = 0;
	}

	int mantle = MOB.Equip[15].sIndex;

	if(MOB.Clan != 4 && (mantle == 543 || mantle == 545))
		MOB.Clan = 7;

	if(MOB.Clan != 4 && (mantle == 544 || mantle == 546))
		MOB.Clan = 8;

	if (MOB.Clan != 4 && (mantle == 734 || mantle == 736))
		MOB.Clan = 7;

	if (MOB.Clan != 4 && (mantle == 735 || mantle == 737))
		MOB.Clan = 8;

	if (MOB.Clan != 4 && (mantle == 3191 || mantle == 3194))
		MOB.Clan = 7;

	if (MOB.Clan != 4 && (mantle == 3192 || mantle == 3195))
		MOB.Clan = 8;

	if (MOB.Clan != 4 && (mantle == 3197))
		MOB.Clan = 7;

	if (MOB.Clan != 4 && (mantle == 3198))
		MOB.Clan = 8;

	if(MOB.Class == 0)
	{
		//Armadura Crítica
		if(MOB.LearnedSkill & (1 << 15))
		{
			MOB.CurrentScore.Ac = (int)(MOB.CurrentScore.Ac * 1.1f);

			Critical += 24;
		}
	}


	int HpAdd = BASE_GetMobAbility(&MOB, EF_HPADD) + BASE_GetMobAbility(&MOB, EF_HPADD2) + 100;

	long long curHP = MOB.CurrentScore.MaxHp;
	long long maxHP = curHP * HpAdd / 100;

	if (maxHP >= MAX_HP)
		MOB.CurrentScore.MaxHp = MAX_HP;

	else
		MOB.CurrentScore.MaxHp = (int)maxHP;
	
	int MpAdd = BASE_GetMobAbility(&MOB, EF_MPADD) + BASE_GetMobAbility(&MOB, EF_MPADD2) + 100;

	long long curMP = MOB.CurrentScore.MaxMp;
	long long maxMP = curMP * MpAdd / 100;

	if (maxMP >= MAX_MP)
		MOB.CurrentScore.MaxMp = MAX_MP;

	else
		MOB.CurrentScore.MaxMp = (int)maxMP;

	if ((MOB.Equip[0].sIndex >= 20 && MOB.Equip[0].sIndex < 30 || MOB.Equip[0].sIndex == 32))
		MOB.Equip[0].sIndex = extra->ClassMaster == MORTAL ? extra->MortalFace : extra->MortalFace + 5 + MOB.Class;



	int AttackSpeedBonus = 0;
	int RunSpeedBonus = 0;
	int DAMAGEMULTI = 100;

	int Sagrado = MOB.Resist[0];
	int Trovao = MOB.Resist[1];
	int Fogo = MOB.Resist[2];
	int Gelo = MOB.Resist[3];

#pragma region Buff Loop
	for(int i = 0; i < MAX_AFFECT; i++)
	{
		if(Affect == 0)
			continue;

		int Type = Affect[i].Type;

		if(Type == 0)
			continue;

		int Value = Affect[i].Value;
		int Level = Affect[i].Level;

		if(Type == 1) // Lentidão (?)
		{
			Run -= Value;

			int tmp = 30;

			Att -= tmp;

			if(MOB.Equip[0].sIndex > 50)
			{
				int tempint = MOB.CurrentScore.Int;
				tempint -= tmp + 10;
				MOB.CurrentScore.Int = tempint;
			}
		}
		else if(Type == 2)
		{
			Run += Value;

			MOB.Rsv |= RSV_HASTE;
		}
		else if(Type == 3)
		{
			int val1 = 0;
			int tval = Value;

			if(MOB.Equip[0].sIndex < 50)
				tval /= 2;
			else
				tval -= 10;

			Sagrado -= tval;
			Fogo -= tval;
			Trovao -= tval;
			Gelo -= tval;
		}
		else if(Type == 4)
		{
			DAMAGEMULTI += 4;
			MOB.CurrentScore.Damage += 30;
			magic += 5;
		}
		else if(Type == 5)
		{
			float fValue = (100 - Value) / 100.0f;
			MOB.CurrentScore.Dex = (short)(MOB.CurrentScore.Dex * fValue);
		}
		else if(Type == 6)
		{
			float fValue = (Value + 100) / 100.0f;
			MOB.CurrentScore.Dex = (short)(MOB.CurrentScore.Dex * fValue);
		}
		else if(Type == 7)
		{
			int tmp = Level / 10 + 10;
			Att -= tmp;

			if(MOB.Equip[0].sIndex > 50)
			{
				int tempint = MOB.CurrentScore.Int;
				tempint -= (tmp + 10);
				MOB.CurrentScore.Int = tempint;
			}
		}
		
		else if(Type == 9)
		{
			int add = Level * 5 / 20 + Value;
			add = add * 3 / 2;

			DAMAGEMULTI += 5;

			if(MOB.Class == 1)
			{
				if(MOB.LearnedSkill & 0x80000)
				{
					add *= 3;
					DAMAGEMULTI += 10;
				}
			}

			MOB.CurrentScore.Damage += add;
		}
		else if(Type == 10)
		{
			int sub = Level / 5 + Value;

			MOB.CurrentScore.Damage -= sub;
		}
		else if(Type == 11)
		{
			int add = Level / 3 + Value;

			MOB.CurrentScore.Ac += add;
		}
		else if(Type == 12)
		{
			float fvalue = (100 - Value) / 100.0f;

			MOB.CurrentScore.Ac = (int)(MOB.CurrentScore.Ac * fvalue);
		}
		else if(Type == 13)
		{
			int tv = Level / 10 + Value;

			DAMAGEMULTI += tv;

			tv = MOB.CurrentScore.MaxHp;
			tv = tv * 9 / 10;

			MOB.CurrentScore.MaxHp = tv;
		}
		else if(Type == 14)
		{
			int value = Level * 3 / 4 + Value;

			if(extra->ClassMaster != ARCH && extra->ClassMaster != MORTAL)
				value *= 3;

			MOB.CurrentScore.MaxHp += value * 2;

			int tv = MOB.CurrentScore.Con + value;

			MOB.CurrentScore.Con = tv;
		}
		else if(Type == 15)
		{
			int value = Level / 10 + Value;

			int tv = MOB.CurrentScore.Special[0] + value;

			if(tv > 400)
				tv = 400;

			MOB.CurrentScore.Special[0] = tv;

			tv = MOB.CurrentScore.Special[1] + value;

			if(tv > 400)
				tv = 400;

			MOB.CurrentScore.Special[1] = tv;

			tv = MOB.CurrentScore.Special[2] + value;

			if(tv > 400)
				tv = 400;

			MOB.CurrentScore.Special[2] = tv;

			tv = MOB.CurrentScore.Special[3] + value;

			if(tv > 400)
				tv = 400;

			MOB.CurrentScore.Special[3] = tv;	
		}
		else if(Type == 16)
		{
			int value = Value - 1;

			if(value < 0 || value >= 5 || MOB.Class != 2)
				continue;

			MOB.Equip[0].sIndex = value == 4 ? 32 : value + 22;

			 int DamAdd = 0;
			 int HpAdd = 0;
			 int AcAdd = 0;
			 int AttAdd = 0;
			 int IsWolf = MOB.LearnedSkill & 0x20000;
			 int IsBear = MOB.LearnedSkill & 0x80000;
			 int IsAsta = MOB.LearnedSkill & 0x200000;
			 int RegAdd = 0;

			 if(MOB.Equip[0].sIndex == 22 && IsWolf != 0)
			 {
				 DamAdd = 10;
				 //RegAdd += 15;
				 Critical += 5;
			 }
			 else if(MOB.Equip[0].sIndex == 23 && IsBear != 0)
			 {
				 HpAdd = 10;
				 RegAdd += 20;//Old 30
				 AttAdd += 20;
			 }
			 else if(MOB.Equip[0].sIndex == 24 && IsAsta != 0)
			 {
				 DamAdd = 10;
				 AcAdd = 5;
				 HpAdd = 5;
				 RegAdd += 20;//15
				 AttAdd += 20;
			 }			
			 else if(MOB.Equip[0].sIndex == 25)
				 AttAdd += 30;

			 else if (MOB.Equip[0].sIndex == 32)
			 {
				 DamAdd = 10;
				 AcAdd = 5;
				 HpAdd = 10;
				 RegAdd += 10;
				 AttAdd += 20;
			 }

			 int sanc = (special3 + (extra->ClassMaster != ARCH && extra->ClassMaster != MORTAL ? MOB.CurrentScore.Level+MAX_LEVEL : MOB.CurrentScore.Level) * 2) / 3;
			 sanc = (sanc - pTransBonus[value].Unk17) / 12;

			 if(sanc <= 0)
				 sanc = 0;

			 if(sanc > 9)
				 sanc = 9;

			 MOB.Equip[0].stEffect[0].cEffect = EF_SANC;
			 MOB.Equip[0].stEffect[0].cValue = sanc;

			 int min = DamAdd + pTransBonus[value].Unk;
			 int max = DamAdd + pTransBonus[value].Unk2;
			 int delta = max - min;
			 int multi = delta * Level / 200 + min;
			 int tv = MOB.CurrentScore.Damage;

			 if(MOB.Equip[0].sIndex == 22)
				 tv += 10;

			 MOB.CurrentScore.Damage = tv;

			 DAMAGEMULTI = DAMAGEMULTI + multi - 100;

			 min = AcAdd + pTransBonus[value].Unk3;
			 max = AcAdd + pTransBonus[value].Unk4;
			 delta = max - min;
			 multi = delta * Level / 200 + min;
			 tv = MOB.CurrentScore.Ac;
			 tv = tv * multi / 100;

			 if(MOB.Equip[0].sIndex == 22)
				 tv += 5;

			 MOB.CurrentScore.Ac = tv;

			 min = HpAdd + pTransBonus[value].Unk5;
			 max = HpAdd + pTransBonus[value].Unk6;
			 delta = max - min;
			 multi = delta * Level / 200 + min;
			 tv = MOB.CurrentScore.MaxHp;
			 tv = tv * multi / 100;
			 MOB.CurrentScore.MaxHp = tv;

			 Sagrado += RegAdd;
			 Fogo += RegAdd;
			 Trovao += RegAdd;
			 Gelo += RegAdd;

			 AttackSpeedBonus = pTransBonus[value].Unk9 + AttAdd;
			 RunSpeedBonus = pTransBonus[value].Unk7;
		}
		else if(Type == 42)
		{
			int mana = MOB.CurrentScore.MaxMp / 5 + (((extra->ClassMaster != ARCH && extra->ClassMaster != MORTAL ? MOB.CurrentScore.Level + MAX_LEVEL : MOB.CurrentScore.Level) + Level) / 2);

			MOB.CurrentScore.MaxHp += mana;
			MOB.CurrentScore.MaxMp -= mana;
		}
		else if(Type == 19)
			MOB.Rsv |= RSV_BLOCK;

		else if(Type == 21)
		{
			int add = Level / 3 + 10;

			MOB.CurrentScore.Ac -= add;

			int tv = Level / 10 + Value;

			DAMAGEMULTI += tv;
		}
		else if(Type == 24)
		{
			int add = MOB.CurrentScore.Ac / 4 + Value;

			MOB.CurrentScore.Ac += add;
		}
		else if(Type == 25)
		{
			int add = (Value + Level / 4) / 10;

			if (Level >= 255)
				add += 20;

			Fogo += add;
			Trovao += add;
			Gelo += add;
		}
		else if(Type == 26)
			MOB.Rsv |= RSV_PARRY;
		else if(Type == 27 && BASE_GetItemAbility(&MOB.Equip[6], EF_WTYPE) == 101)
			MOB.Rsv |= RSV_FROST;
		else if(Type == 28)
			MOB.Rsv |= RSV_HIDE;
		
		else if(Type == 36 && BASE_GetItemAbility(&MOB.Equip[6], EF_WTYPE) == 41)
		    MOB.Rsv |= RSV_DRAIN;

		//else if (Type == 30)
		//	MOB.Rsv |= RSV_VISION;

		else if (Type == 29 && extra->ClassMaster != MORTAL)//Soul
		{
			if (extra->Soul == SOUL_F)
				MOB.CurrentScore.Str = (int)(MOB.CurrentScore.Str * 2.2f);

			else if (extra->Soul == SOUL_I)
				MOB.CurrentScore.Int = (int)(MOB.CurrentScore.Int * 2.2f);

			else if (extra->Soul == SOUL_D)
				MOB.CurrentScore.Dex = (int)(MOB.CurrentScore.Dex * 2.2f);

			else if (extra->Soul == SOUL_C)
				MOB.CurrentScore.Con = (int)(MOB.CurrentScore.Con * 2.2f);
			
			else if (extra->Soul == SOUL_FI)
			{
				MOB.CurrentScore.Str = (int)(MOB.CurrentScore.Str * 1.80f);
				MOB.CurrentScore.Int = (int)(MOB.CurrentScore.Int * 1.40f);
			}

			else if (extra->Soul == SOUL_FD)
			{
				MOB.CurrentScore.Str = (int)(MOB.CurrentScore.Str * 1.80f);
				MOB.CurrentScore.Dex = (int)(MOB.CurrentScore.Dex * 1.40f);
			}

			else if (extra->Soul == SOUL_FC)
			{
				MOB.CurrentScore.Str = (int)(MOB.CurrentScore.Str * 1.80f);
				MOB.CurrentScore.Con = (int)(MOB.CurrentScore.Con * 1.40f);
			}

			else if (extra->Soul == SOUL_IF)
			{
				MOB.CurrentScore.Int = (int)(MOB.CurrentScore.Int * 1.80f);
				MOB.CurrentScore.Str = (int)(MOB.CurrentScore.Str * 1.40f);
			}

			else if (extra->Soul == SOUL_ID)
			{
				MOB.CurrentScore.Int = (int)(MOB.CurrentScore.Int * 1.80f);
				MOB.CurrentScore.Dex = (int)(MOB.CurrentScore.Dex * 1.40f);
			}

			else if (extra->Soul == SOUL_IC)
			{
				MOB.CurrentScore.Int = (int)(MOB.CurrentScore.Int * 1.80f);
				MOB.CurrentScore.Con = (int)(MOB.CurrentScore.Con * 1.40f);
			}

			else if (extra->Soul == SOUL_DF)
			{
				MOB.CurrentScore.Dex = (int)(MOB.CurrentScore.Dex * 1.80f);
				MOB.CurrentScore.Str = (int)(MOB.CurrentScore.Str * 1.40f);
			}

			else if (extra->Soul == SOUL_DI)
			{
				MOB.CurrentScore.Dex = (int)(MOB.CurrentScore.Dex * 1.80f);
				MOB.CurrentScore.Int = (int)(MOB.CurrentScore.Int * 1.40f);
			}

			else if (extra->Soul == SOUL_DC)
			{
				MOB.CurrentScore.Dex = (int)(MOB.CurrentScore.Dex * 1.80f);
				MOB.CurrentScore.Con = (int)(MOB.CurrentScore.Con * 1.40f);
			}

			else if (extra->Soul == SOUL_CF)
			{
				MOB.CurrentScore.Con = (int)(MOB.CurrentScore.Con * 1.80f);
				MOB.CurrentScore.Str = (int)(MOB.CurrentScore.Str * 1.40f);
			}

			else if (extra->Soul == SOUL_CI)
			{
				MOB.CurrentScore.Con = (int)(MOB.CurrentScore.Con * 1.80f);
				MOB.CurrentScore.Int = (int)(MOB.CurrentScore.Int * 1.40f);
			}

			else if (extra->Soul == SOUL_CD)
			{
				MOB.CurrentScore.Con = (int)(MOB.CurrentScore.Con * 1.80f);
				MOB.CurrentScore.Dex = (int)(MOB.CurrentScore.Dex * 1.40f);
			}
		}

		else if (Type == 29 && extra->ClassMaster == MORTAL)//Soul
		{
			if (extra->Soul == SOUL_F)
				MOB.CurrentScore.Str = (int)(MOB.CurrentScore.Str * 1.8f);

			else if (extra->Soul == SOUL_I)
				MOB.CurrentScore.Int = (int)(MOB.CurrentScore.Int * 1.8f);

			else if (extra->Soul == SOUL_D)
				MOB.CurrentScore.Dex = (int)(MOB.CurrentScore.Dex * 1.8f);

			else if (extra->Soul == SOUL_C)
				MOB.CurrentScore.Con = (int)(MOB.CurrentScore.Con * 1.8f);
			
			else if (extra->Soul == SOUL_FI)
			{
				MOB.CurrentScore.Str = (int)(MOB.CurrentScore.Str * 1.60f);
				MOB.CurrentScore.Int = (int)(MOB.CurrentScore.Int * 1.40f);
			}

			else if (extra->Soul == SOUL_FD)
			{
				MOB.CurrentScore.Str = (int)(MOB.CurrentScore.Str * 1.60f);
				MOB.CurrentScore.Dex = (int)(MOB.CurrentScore.Dex * 1.40f);
			}

			else if (extra->Soul == SOUL_FC)
			{
				MOB.CurrentScore.Str = (int)(MOB.CurrentScore.Str * 1.60f);
				MOB.CurrentScore.Con = (int)(MOB.CurrentScore.Con * 1.40f);
			}

			else if (extra->Soul == SOUL_IF)
			{
				MOB.CurrentScore.Int = (int)(MOB.CurrentScore.Int * 1.60f);
				MOB.CurrentScore.Str = (int)(MOB.CurrentScore.Str * 1.40f);
			}

			else if (extra->Soul == SOUL_ID)
			{
				MOB.CurrentScore.Int = (int)(MOB.CurrentScore.Int * 1.60f);
				MOB.CurrentScore.Dex = (int)(MOB.CurrentScore.Dex * 1.40f);
			}

			else if (extra->Soul == SOUL_IC)
			{
				MOB.CurrentScore.Int = (int)(MOB.CurrentScore.Int * 1.60f);
				MOB.CurrentScore.Con = (int)(MOB.CurrentScore.Con * 1.40f);
			}

			else if (extra->Soul == SOUL_DF)
			{
				MOB.CurrentScore.Dex = (int)(MOB.CurrentScore.Dex * 1.60f);
				MOB.CurrentScore.Str = (int)(MOB.CurrentScore.Str * 1.40f);
			}

			else if (extra->Soul == SOUL_DI)
			{
				MOB.CurrentScore.Dex = (int)(MOB.CurrentScore.Dex * 1.60f);
				MOB.CurrentScore.Int = (int)(MOB.CurrentScore.Int * 1.40f);
			}

			else if (extra->Soul == SOUL_DC)
			{
				MOB.CurrentScore.Dex = (int)(MOB.CurrentScore.Dex * 1.60f);
				MOB.CurrentScore.Con = (int)(MOB.CurrentScore.Con * 1.40f);
			}

			else if (extra->Soul == SOUL_CF)
			{
				MOB.CurrentScore.Con = (int)(MOB.CurrentScore.Con * 1.60f);
				MOB.CurrentScore.Str = (int)(MOB.CurrentScore.Str * 1.40f);
			}

			else if (extra->Soul == SOUL_CI)
			{
				MOB.CurrentScore.Con = (int)(MOB.CurrentScore.Con * 1.60f);
				MOB.CurrentScore.Int = (int)(MOB.CurrentScore.Int * 1.40f);
			}

			else if (extra->Soul == SOUL_CD)
			{
				MOB.CurrentScore.Con = (int)(MOB.CurrentScore.Con * 1.60f);
				MOB.CurrentScore.Dex = (int)(MOB.CurrentScore.Dex * 1.40f);
			}
		}

		else if (Type == 30)
			*ForceMobDamage += Level;

		else if (Type == 37)
			*ForceDamage += special2;

		else if (Type == 31)
		{
			int add = Level / 2 + Value;

			MOB.CurrentScore.Ac += add;
		}
		
		else if(Type == 38)
		{
			int mana = MOB.CurrentScore.MaxMp / 2;

			MOB.CurrentScore.MaxHp += mana;
			MOB.CurrentScore.MaxMp -= mana;
		}

		else if (Type == 39)
			*ExpBonus += 100;
	}
#pragma endregion


#pragma region Loop affect2
	for(int i = 0; i < MAX_AFFECT; i++)
	{
		if(Affect == 0)
			continue;

		int Type = Affect[i].Type;

		if(Type == 0)
			continue;

		int Value = Affect[i].Value;
		int Level = Affect[i].Level;

		if(Type == 8)
		{
			//Resistencia
			if((Level & (1 << 1)) != 0)
			{
				Sagrado += 25;
				Fogo += 25;
				Trovao += 25;
				Gelo += 25;
			}

			//Abs
			if((Level & (1 << 3)) != 0)
				*AbsHp += 20;

			//+HP DEF
			if((Level & (1 << 4)) != 0)
			{
				int totalhp = ((MOB.CurrentScore.MaxHp / 100) * 10) + MOB.CurrentScore.MaxHp;

				if (totalhp >= MAX_HP)
					MOB.CurrentScore.MaxHp = MAX_HP;
				else
					MOB.CurrentScore.MaxHp = totalhp;

				int totalac = ((MOB.CurrentScore.Ac / 100) * 10) + MOB.CurrentScore.Ac;

				MOB.CurrentScore.Ac = totalac;
			}

			//+HP ATK
			if((Level & (1 << 5)) != 0)
			{
				int totalhp = ((MOB.CurrentScore.MaxHp / 100) * 10) + MOB.CurrentScore.MaxHp;

				if (totalhp >= MAX_HP)
					MOB.CurrentScore.MaxHp = MAX_HP;
				else
					MOB.CurrentScore.MaxHp = totalhp;

				int totaldam = ((MOB.CurrentScore.Damage / 100) * 10) + MOB.CurrentScore.Damage;

				MOB.CurrentScore.Damage = totaldam;
			}

			//Precisão
			if((Level & (1 << 6)) != 0)
				*Accuracy += 50;

			//MP = HP
			if((Level & (1 << 7)) != 0)
			{
				int mana = ((MOB.CurrentScore.MaxMp + 1) / 2);

				MOB.CurrentScore.MaxHp += mana;
				MOB.CurrentScore.MaxMp -= mana;
			}
		}
		else if (Type == 34)//Divina
		{
			int totalhp = ((MOB.CurrentScore.MaxHp / 100) * 20) + MOB.CurrentScore.MaxHp;

			if (totalhp >= MAX_HP)
				MOB.CurrentScore.MaxHp = MAX_HP;
			else
				MOB.CurrentScore.MaxHp = totalhp;

			int totalmp = ((MOB.CurrentScore.MaxMp / 100) * 20) + MOB.CurrentScore.MaxMp;

			if (totalmp >= MAX_MP)
				MOB.CurrentScore.MaxMp = MAX_MP;
			else
				MOB.CurrentScore.MaxMp = totalmp;

			int totaldn = ((MOB.CurrentScore.Damage / 100) * 20) + MOB.CurrentScore.Damage;

			if (totaldn >= MAX_DAMAGE)
				MOB.CurrentScore.Damage = MAX_DAMAGE;
			else
				MOB.CurrentScore.Damage = totaldn;

			int totalmg = ((MOB.Magic / 100) * 10) + MOB.Magic;

			if (totalmg >= MAX_DAMAGE_MG)
				MOB.Magic = MAX_DAMAGE_MG;
			else
				MOB.Magic = totalmg;
		}

		else if (Type == 35)//Vigor
		{
			int totalhp = ((MOB.CurrentScore.MaxHp / 100) * 10) + MOB.CurrentScore.MaxHp;

			if (totalhp >= MAX_HP)
				MOB.CurrentScore.MaxHp = MAX_HP;
			else
				MOB.CurrentScore.MaxHp = totalhp;

			int totalmp = ((MOB.CurrentScore.MaxMp / 100) * 10) + MOB.CurrentScore.MaxMp;

			if (totalmp >= MAX_MP)
				MOB.CurrentScore.MaxMp = MAX_MP;
			else
				MOB.CurrentScore.MaxMp = totalmp;
		}
	}
#pragma endregion

	for(int p = 1; p <= 7; p++)
	{
		int idx = MOB.Equip[p].sIndex;

		if(idx <= 0 || idx >= MAX_ITEMLIST)
			continue;

		int sa = BASE_GetItemSanc(&MOB.Equip[p]);

		if(sa < 9)
			continue;

		int nPos = g_pItemList[idx].nPos;

		 if(nPos == 4)
			MOB.CurrentScore.Ac += 25;
		else if(nPos == 8)
			MOB.CurrentScore.Ac += 25;
		else if(nPos == 16)
			MOB.Rsv |= RSV_CAST;
		else if(nPos == 128)
			MOB.CurrentScore.Ac += 25;
		
		else if(nPos == 64 || nPos == 192)
		{
			int nu = g_pItemList[idx].nUnique;

			if(nu == 47)
				magic += 8;
			else
				MOB.CurrentScore.Damage += 40;
		}
	}

	if(MOB.Class == 3)
	{
		//Aggressividade
		if(MOB.LearnedSkill & (1 << 2))
		{
			int lidx = MOB.Equip[6].sIndex;

			if(lidx > 0 && lidx < MAX_ITEMLIST)
			{
				int ltype = g_pItemList[lidx].nUnique;

				if(ltype == 64)
				{
					int fightadd = special1 + 10;
					MOB.CurrentScore.Damage += fightadd;
				}
			}
		}

		//Tempestade de Raios Bonus em Dano
		if(MOB.LearnedSkill & (1 << 7))
			MOB.CurrentScore.Damage += special1 * 2;


		//Visão do Caçador
		if(MOB.LearnedSkill & (1 << 18))
		{
			int criticaladd = ((special3+1)/10) + (MOB.CurrentScore.Dex / 75);
			
			if(criticaladd < 4)
				criticaladd = 4;
			
			Critical += criticaladd;
		}

		//Proteção das sombras
		if(MOB.LearnedSkill & (1 << 22))
		{
			int sombraadd = (special3 / 3) + 10;
			MOB.CurrentScore.Ac += sombraadd;
		}
	}

	if (isKibitaSoul)
	{
		MOB.CurrentScore.Str = 2000;
		MOB.CurrentScore.Int = 2000;

		MOB.CurrentScore.MaxHp = 10000;

		int ItemInt = MOB.CurrentScore.Int - MOB.BaseScore.Int > 2000 ? 2000 : MOB.BaseScore.Int;

		int MPDelta = ItemInt * 2;
		unsigned int mmp = MOB.CurrentScore.MaxMp + MPDelta;

		MOB.CurrentScore.MaxMp += mmp;
	}


	if (face < 4)
		MOB.CurrentScore.Damage += (MOB.CurrentScore.Str / 2) + (MOB.CurrentScore.Dex / 3) + MOB.CurrentScore.Special[0] + (extra->ClassMaster == ARCH || extra->ClassMaster == MORTAL ? MOB.CurrentScore.Level : MOB.CurrentScore.Level + MAX_LEVEL);

	if(DAMAGEMULTI != 100)
		MOB.CurrentScore.Damage = MOB.CurrentScore.Damage * DAMAGEMULTI  / 100;

	if(RegenHP < 0)
		RegenHP = 0;

	if(RegenHP > 255)
		RegenHP = 255;

	MOB.RegenHP = RegenHP;

	if(RegenMP < 0)
		RegenMP = 0;

	if(RegenMP > 255)
		RegenMP = 255;

	MOB.RegenMP = RegenMP;

	MOB.CurrentScore.Merchant = (MOB.CurrentScore.Merchant & 240) | MOB.Merchant;

	Att += AttackSpeedBonus + MOB.CurrentScore.Dex / 5;
	Run += RunSpeedBonus;

	int mount = MOB.Equip[14].sIndex;

	// g_pMountBonus tem 30 linhas (2360..2389); 2390 lia além do fim.
	if(face <= 4 && (mount >= 2360 && mount < 2390 || mount >= 3980 && mount <= 3994))
	{
		if (mount >= 3980 && mount <= 3994)
		{
			int code = mount - 3980;

			if (Run < g_pMountBonus2[code][4])
				Run = g_pMountBonus2[code][4];
		}

		else if(MOB.Equip[14].stEffect[0].sValue > 0 )
		{
			int code = mount - 2360;

			if(Run < g_pMountBonus[code][4])
				Run = g_pMountBonus[code][4];
		}
	}

	if(Run <= 0)
		Run = 0;
	if(Run > 6)
		Run = 6;

	if(Att < 0)
		Att = 0;
	if(Att > 150)
		Att = 150;

	Att /= 10;

	MOB.CurrentScore.AttackRun = Att * 16 + Run;


	if (MOB.CurrentScore.Hp < 0)
		MOB.CurrentScore.Hp = 0;

	if (MOB.CurrentScore.Mp < 0)
		MOB.CurrentScore.Mp = 0;

	if(MOB.CurrentScore.Hp > MOB.CurrentScore.MaxHp)
		MOB.CurrentScore.Hp = MOB.CurrentScore.MaxHp;

	if(MOB.CurrentScore.Mp > MOB.CurrentScore.MaxMp)
		MOB.CurrentScore.Mp = MOB.CurrentScore.MaxMp;

	if (magic >= MAX_DAMAGE_MG)
		magic = MAX_DAMAGE_MG;

	MOB.Magic = magic;

	if (Critical >= 255)
		Critical = 255;

	MOB.Critical = Critical;


	if (Sagrado < 0)
		Sagrado = 0;

	if (Sagrado > 100)
		Sagrado = 100;

	MOB.Resist[0] = Sagrado;

	if (Trovao < 0)
		Trovao = 0;

	if (Trovao > 100)
		Trovao = 100;

	MOB.Resist[1] = Trovao;

	if (Fogo < 0)
		Fogo = 0;

	if (Fogo > 100)
		Fogo = 100;

	MOB.Resist[2] = Fogo;

	if (Gelo < 0)
		Gelo = 0;

	if (Gelo > 100)
		Gelo = 100;

	MOB.Resist[3] = Gelo;
}

/// Calcula coordenada final (X, Y) a partir da sequencia de passos de MSG_Action.
void BASE_GetDestByAction(unsigned short *x, unsigned short *y, MSG_Action *mv)
{
	*x = mv->PosX;
	*y = mv->PosY;

	for(int i = 0; i < MAX_ROUTE; i++)
	{
		if(mv->Route[i] == 0)
			break;

		if(mv->Route[i] == '6')
			*x++;

		if(mv->Route[i] == '4')
			*x--;

		if(mv->Route[i] == '8')
			*y--;

		if(mv->Route[i] == '2')
			*y++;

		if(mv->Route[i] == '9')
		{
			*x++;
			*y--;
		}

		if(mv->Route[i] == '3')
		{
			*x++;
			*y++;
		}

		if(mv->Route[i] == '7')
		{
			*x--;
			*y--;
		}

		if(mv->Route[i] == '1')
		{
			*x--;
			*y++;
		}
	}
}

/// Calcula gasto de mana da habilidade considerando reducao SaveMana e especial.
int BASE_GetManaSpent(int SkillNumber, int SaveMana, int Special)
{
	int manaspent = g_pSpell[SkillNumber].ManaSpent * (Special / 2 + 100) / 100;
	manaspent = (100 - SaveMana) * manaspent / 100;

	return manaspent;
}

/// Calcula probabilidade de acerto fisico entre atacante e defensor.
int BASE_GetHitRate(STRUCT_SCORE *att, STRUCT_SCORE *def)
{
	if(def->Dex == 0 || def->Level == 0)
		return 95;

	if(att->Dex < 0 || def->Dex < 0 || att->Level < 0 || def->Level < 0)
		return 0;

	int parta = 60 * att->Dex / (def->Dex / 2 + att->Dex);
	int partb = 100 * (def->Level + att->Level) / def->Level;
	int rate = partb * parta / 100;

	if(rate >= 95)
		rate = 95;
	if(rate < 40)
		rate = 40;

	return rate;
}

/// Retorna multiplicador de dano por habilidade.
int BASE_GetDamageRate(STRUCT_SCORE* att, int Skill)
{
	int rate = Skill + att->Special[0];

	if(rate >= 200)
		rate = 200;
	if(rate < 0)
		rate = 0;

	return rate;
}

/// Calcula taxa de precisao do atacante.
int BASE_GetAccuracyRate(STRUCT_SCORE *att)
{
	int ret = att->Dex;
	ret = ret / 2 + 50;

	if(ret > 100)
		ret = 100;

	return ret;
}

/// Determina ocorrencia de ataque duplo e/ou critico e atualiza contadores de progresso.
int BASE_GetDoubleCritical(STRUCT_MOB *mob, unsigned short *sProgress, unsigned short *cProgress, unsigned char *bDoubleCritical)
{
	*bDoubleCritical = 0;

	if(cProgress == 0)
		return FALSE;

	if(*cProgress >= 1024)
	{	
		if((*cProgress & 0x3FF) < 0)
			*cProgress = ((*cProgress & 0x3FF) - 1 | 0xFC00) + 1;		
	}

	if(sProgress != 0 && *sProgress >= 1024)
	{
		int val = *sProgress & 0x3FF;

		if(val < 0)
			val = ((val - 1) | 0xFC00) + 1;	

		*sProgress = val;
	}

	int ret = 1;

	int hitvalue[2];

	hitvalue[0] = 100 * ((mob->CurrentScore.AttackRun >> 4) - 5);
	hitvalue[1] = mob->Critical;

	if((sProgress != NULL && cProgress != NULL) && *cProgress != *sProgress)
	{
		*sProgress = *cProgress;

		int tProgress = *cProgress;

		if(tProgress < *sProgress)
			tProgress += 256;

		if(tProgress < *sProgress && tProgress < *sProgress + 5)
			ret = 1;
	}

	int value = g_pHitRate[*cProgress];

	for(int i = 0; i < 2; i++)
	{
		int bit = 0;

		if(i == 0)
		{
			if(value < hitvalue[0])
				bit = 1;
		}

		if(i == 1)
		{
			if (rand() % 255 < hitvalue[1])
				bit = 1;
		}

		*bDoubleCritical |= bit << i;
	}

	if(sProgress)
		++*sProgress;

	++*cProgress;

	return ret;
}

/// Calcula rota de deslocamento no grid de altura ate a coordenada alvo.
int BASE_GetRoute(int x, int y, int *targetx, int *targety, char *Route, int distance, char *pHeight)
{
	int lastx = x;
	int lasty = y;
	int tx = *targetx;
	int ty = *targety;

	memset(Route, 0, MAX_ROUTE);

	for(int i = 0; i < distance && i < MAX_ROUTE-1; i++)
	{
		if(x - g_HeightPosX < 1 || y - g_HeightPosY < 1 || x - g_HeightPosX > g_HeightWidth - 2 || y - g_HeightPosY > g_HeightHeight - 2)
		{
			Route[i] = 0;

			break;
		}

		int cul = pHeight[(y - g_HeightPosY) * g_HeightWidth + x - g_HeightPosX];

		if(cul > MH)
			int kk = 0;

		 int n  = pHeight[(y - g_HeightPosY - 1) * g_HeightWidth + x - g_HeightPosX    ];
		 int ne = pHeight[(y - g_HeightPosY - 1) * g_HeightWidth + x - g_HeightPosX + 1];
		 int e  = pHeight[(y - g_HeightPosY    ) * g_HeightWidth + x - g_HeightPosX + 1];
		 int se = pHeight[(y - g_HeightPosY + 1) * g_HeightWidth + x - g_HeightPosX + 1];
		 int s  = pHeight[(y - g_HeightPosY + 1) * g_HeightWidth + x - g_HeightPosX    ];
		 int sw = pHeight[(y - g_HeightPosY + 1) * g_HeightWidth + x - g_HeightPosX - 1];
		 int w  = pHeight[(y - g_HeightPosY    ) * g_HeightWidth + x - g_HeightPosX - 1];
		 int nw = pHeight[(y - g_HeightPosY - 1) * g_HeightWidth + x - g_HeightPosX - 1];

		 if(tx == x && ty == y) 
		 {
			 Route[i] = 0;
			 break;
		 }

		 if(tx == x && ty < y && n < cul + MH && n > cul - MH) 
		 {
			 Route[i] = '2'; 
			 y--;

			 continue;
		 }

		 if(tx > x && ty < y && ne < cul + MH && ne > cul - MH) 
		 {
			 Route[i] = '3';

			 x++;
			 y--;

			 continue;
		 }

		 if(tx > x && ty == y && e < cul + MH && e > cul - MH) 
		 {
			 Route[i] = '6';

			 x++;

			 continue;
		 }

		 if(tx > x && ty > y && se < cul + MH && se > cul - MH) 
		 {
			 Route[i] = '9';
			 
			 x++;
			 y++;
			 
			 continue;
		 }   

		 if(tx == x && ty > y && s < cul + MH && s > cul - MH) 
		 {
			 Route[i] = '8';
			 
			 y++;
			 
			 continue;
		 }

		 if(tx < x && ty > y && sw < cul + MH && sw > cul - MH) 
		 {
			 Route[i] = '7';
			 
			 x--;
			 y++;
			 
			 continue;
		 }

		 if(tx < x && ty == y && w < cul + MH && w > cul - MH) 
		 {
			 Route[i] = '4';
			 
			 x--;
			 
			 continue;
		 }

		 if(tx < x && ty < y && nw < cul + MH && nw > cul - MH) 
		 {
			 Route[i] = '1';
			 
			 x--;
			 y--;
			 
			 continue;
		 }

		 if(tx > x && ty < y && e < cul + MH && e > cul - MH) 
		 {
			 Route[i] = '6';

			 x++;
			 
			 continue;
		 }

		 if(tx > x && ty < y && n < cul + MH && n > cul - MH) 
		 {
			 Route[i] = '2';
			 
			 y--;
			 
			 continue;
		 }

		 if(tx > x && ty > y && e < cul + MH && e > cul - MH) 
		 {
			 Route[i] = '6';
			 
			 x++;
			 
			 continue;
		 }         

		 if(tx > x && ty > y && s < cul + MH && s > cul - MH) 
		 {
			 Route[i] = '8';

			 y++;
			 
			 continue;
		 }   

		 if(tx < x && ty > y && w < cul + MH && w > cul - MH) 
		 {
			 Route[i] = '4';
			 
			 x--;
			 
			 continue;
		 }

		 if(tx < x && ty > y && s < cul + MH && s > cul - MH) 
		 {
			 Route[i] = '8';

			 y++;

			 continue;
		 }

		 if(tx < x && ty < y && w < cul + MH && w > cul - MH) 
		 {
			 Route[i] = '4';

			 x--;

			 continue;
		 }

		 if(tx < x && ty < y && n < cul + MH && n > cul - MH) 
		 {
			 Route[i] = '2';
			 
			 y--;
			 
			 continue;
		 }

		 if(tx == x + 1 || ty == y + 1 || tx == x - 1 || ty == y - 1) 
		 {
			 Route[i] = 0;

			 break;
		 }

		 if(tx == x && ty > y && se < cul + MH && se > cul - MH) 
		 {
			 Route[i] = '9';
			 
			 x++;			 
			 y++;
			 
			 continue;
		 }         

		 if(tx == x && ty > y && sw < cul + MH && sw > cul - MH) 
		 {
			 Route[i] = '7';
			 
			 x--;
			 y++;
			 
			 continue;
		 }

		 if(tx == x && ty < y && ne < cul + MH && ne > cul - MH) 
		 {
			 Route[i] = '3';

			 x++;
			 y--;

			 continue;
		 }

		 if(tx == x && ty < y && nw < cul + MH && nw > cul - MH) 
		 {
			 Route[i] = '1';
			 
			 x--;
			 y--;
			 
			 continue;
		 }

		 if(tx < x && ty == y && sw < cul + MH && sw > cul - MH) 
		 {
			 Route[i] = '7';
			 
			 x--;
			 y++;
			 
			 continue;
		 }

		 if(tx < x && ty == y && nw < cul + MH && nw > cul - MH) 
		 {
			 Route[i] = '1';
			 
			 x--;
			 y--;
			 
			 continue;
		 }

		 if(tx > x && ty == y && se < cul + MH && se > cul - MH) 
		 {
			 Route[i] = '9';
			 
			 x++;
			 y++;
			 
			 continue;
		 }         

		 if(tx > x && ty == y && ne < cul + MH && ne > cul - MH) 
		 {
			 Route[i] = '3';

			 x++;
			 y--;

			 continue;
		 }

		 Route[i] = 0;
		 break; 
	}

	if(lastx == x && lasty == y)
		return 0;

	int kk = 0;

	*targetx = x;
	*targety = y;

	if(lastx == x && lasty == y) 
		return 0;

	return 1;
}

/// Calcula distancia de tabuleiro (Chebyshev) entre duas posicoes no mapa.
int BASE_GetDistance(int x1, int y1, int x2, int y2)
{
	int	dx, dy;

	if(x1 > x2) 
		dx = x1 - x2;
	else	
		dx = x2 - x1;

	if(y1 > y2) 
		dy = y1 - y2;
	else       
		dy = y2 - y1;

	if(dx <= 6 && dy <= 6)
		return g_pDistanceTable[dy][dx];

	if(dx > dy) 
		return dx+1;
	else       
		return dy+1;
}

/// Calcula dano de habilidade fisica/magica considerando atributos, arma e clima.
int BASE_GetSkillDamage(int skillnum, STRUCT_MOB *mob, int weather, int weapondamage)
{
	int instanceindex = g_pSpell[skillnum].InstanceType;

	int kind = skillnum % MAX_SKILL / 8 + 1;

	int level = mob->CurrentScore.Level;

	if(level < 0)
		level = 0;

	if(level >= MAX_LEVEL)
		level = MAX_LEVEL;

	int special = mob->CurrentScore.Special[kind];

	int base = g_pSpell[skillnum].InstanceValue;
	int affectbase = g_pSpell[skillnum].AffectValue;

	int dam = 0;

	if(instanceindex == 0)
	{
		if(skillnum == 11)		
			dam = special / 10 + affectbase;

		else if(skillnum == 13)		
			dam = 3 * special / 4 + affectbase;

		else if(skillnum == 41)		
			dam = special / 25 + 2;

		else if(skillnum == 43)		
			dam = special / 3 + affectbase;

		else if(skillnum == 44)		
			dam = 2 * (3 * special / 20 + affectbase);

		else if(skillnum == 45)
			dam = special / 10 + affectbase;
	}

	else if(instanceindex >= 1 && instanceindex <= 5)
	{
		int skind = skillnum / 8;
		
		if(skillnum == 97)
			dam = 15 * level + base;
		
		else if(mob->Class == 0 && skind == 1) // TK
			dam =  3 * weapondamage + 3 * mob->CurrentScore.Str + level + special + base;
		
		else if(mob->Class == 0 && skind != 1) // TK
			dam = special + base + weapondamage + level + mob->CurrentScore.Int / 4 + mob->CurrentScore.Int / 40;
		
		else if(mob->Class == 1)//Foema
			dam = mob->CurrentScore.Int / 30 + mob->CurrentScore.Int / 3 + level + base + 2 * special;
		
		else if(mob->Class == 2)//BeastMaster
			dam = mob->CurrentScore.Int / 30 + mob->CurrentScore.Int / 3 + level + base + 2 * special;
		
		else if(mob->Class == 3)//Huntress
			dam = 3 * weapondamage + 3 * mob->CurrentScore.Str + level / 2 + special + base;		

		if(weather == 1)
		{
			if(instanceindex == 2)
				dam = 90 * dam / 100;
			
			if(instanceindex == 5)
				dam = 130 * dam / 100;
		}
		
		else if(weather == 2 && instanceindex == 3)
			dam = 120 * dam / 100;


		if ((mob->Class || skind != 1) && mob->Class != 3)
        {
			dam = (4 * mob->Magic + 100) * dam / 100;
			dam = 5 * dam / 4;
        }
        else
			dam = 5 * dam / 4;
	}

	else if(instanceindex == 6)
		dam = 3 * special / 2 + base;
	
	else if(instanceindex == 11)
		dam = base;
	
	else
		dam = mob->Magic;


	if(skillnum == 79)
		dam = mob->CurrentScore.Damage;

	return dam;
}
