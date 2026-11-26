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

/// @file w2pp_characterization_tests.cpp
/// @brief Testes de caracterização ("golden") das funções `BASE_*` de w2pp_common.
///
/// Cada caso alimenta uma função com milhares de entradas geradas por um LCG próprio
/// (semente fixa, independente de `rand()`), acumula todas as saídas num hash FNV-1a de
/// 64 bits e compara com o valor gravado em `kExpected`. As tabelas que o jogo carrega de
/// arquivo (`g_pItemList`, `g_pSpell`) são preenchidas com dados sintéticos determinísticos,
/// então o teste não depende de nenhum arquivo do jogo.
///
/// Os valores esperados foram gerados com o código da tag `pre-modularization` e conferidos
/// contra o código atual: um hash diferente indica mudança de comportamento da função.
///
/// Uso: `w2pp_characterization_tests` (compara) ou `w2pp_characterization_tests --print`
/// (imprime a tabela atual, para regravar `kExpected` após uma mudança intencional).

#include "common/Basedef.h"
#include "common/ItemEffect.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace
{

/// @brief Gerador congruencial linear próprio (não depende da CRT).
struct Lcg
{
	uint32_t s;
	explicit Lcg(uint32_t seed) : s(seed) {}
	uint32_t Next() { s = s * 1664525u + 1013904223u; return s >> 8; }
	/// @brief Inteiro uniforme em [lo, hi].
	int Range(int lo, int hi) { return lo + (int)(Next() % (uint32_t)(hi - lo + 1)); }
};

/// @brief Hash FNV-1a de 64 bits sobre valores inteiros e blocos de memória.
struct Hash
{
	uint64_t h = 1469598103934665603ull;
	void Bytes(const void* p, size_t n)
	{
		auto b = static_cast<const unsigned char*>(p);
		for (size_t i = 0; i < n; ++i)
		{
			h ^= b[i];
			h *= 1099511628211ull;
		}
	}
	void Int(long long v) { Bytes(&v, sizeof(v)); }
};

const int kSamples = 4000;

/// @brief Item sintético: índice e efeitos em faixas realistas, com viés para refinação e montarias.
STRUCT_ITEM RandomItem(Lcg& r)
{
	STRUCT_ITEM it{};
	int k = r.Range(0, 9);
	if (k == 0)
		it.sIndex = (short)r.Range(2360, 2390);   // montarias
	else if (k == 1)
		it.sIndex = (short)r.Range(3200, 3500);   // consumíveis/especiais
	else
		it.sIndex = (short)r.Range(0, MAX_ITEMLIST - 1);
	static const unsigned char effs[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 26, 27, 28, 42, 43, 44, 45, 46, 47, 48, 50,
		53, 54, 59, 60, 61, 62, 63, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 116, 117, 118, 119, 120, 121,
		122, 123, 124, 125 };
	for (int e = 0; e < 3; ++e)
	{
		it.stEffect[e].cEffect = effs[r.Range(0, (int)(sizeof(effs) / sizeof(effs[0])) - 1)];
		it.stEffect[e].cValue = (unsigned char)r.Range(0, 255);
	}
	// Domínio do jogo: em montarias, stEffect[1].cEffect / 10 indexa g_pSancRate[2][12]
	// (BASE_GetGrowthRate não valida o índice).
	if (it.sIndex >= 2360 && it.sIndex < 2390)
		it.stEffect[1].cEffect = (unsigned char)r.Range(0, 119);
	return it;
}

/// @brief Item do slot de montaria (Equip[14]): vazio ou montaria válida, como o jogo garante.
/// GetCurrentScore indexa g_pMountBonus[sIndex - 2360] sem validar o índice.
STRUCT_ITEM RandomMountSlot(Lcg& r)
{
	STRUCT_ITEM it = RandomItem(r);
	int k = r.Range(0, 2);
	if (k == 0)
		return STRUCT_ITEM{};
	it.sIndex = (short)(k == 1 ? r.Range(2360, 2389) : r.Range(3980, 3994));
	if (it.sIndex < 2390)
		it.stEffect[1].cEffect = (unsigned char)r.Range(0, 119);
	return it;
}

STRUCT_SCORE RandomScore(Lcg& r)
{
	STRUCT_SCORE s{};
	s.Level = r.Range(0, MAX_LEVEL);
	s.Ac = r.Range(0, 3000);
	s.Damage = r.Range(0, 5000);
	s.Merchant = (unsigned char)r.Range(0, 255);
	s.AttackRun = (unsigned char)r.Range(0, 255);
	s.Direction = (unsigned char)r.Range(0, 7);
	s.ChaosRate = (unsigned char)r.Range(0, 255);
	s.MaxHp = r.Range(1, 60000);
	s.MaxMp = r.Range(1, 60000);
	s.Hp = r.Range(0, s.MaxHp);
	s.Mp = r.Range(0, s.MaxMp);
	s.Str = (short)r.Range(0, 32000);
	s.Int = (short)r.Range(0, 32000);
	s.Dex = (short)r.Range(0, 32000);
	s.Con = (short)r.Range(0, 32000);
	for (int i = 0; i < 4; ++i)
		s.Special[i] = (short)r.Range(0, 255);
	return s;
}

STRUCT_MOB RandomMob(Lcg& r)
{
	STRUCT_MOB m{};
	for (int i = 0; i < NAME_LENGTH - 1; ++i)
		m.MobName[i] = (char)('a' + r.Range(0, 25));
	m.Clan = (char)r.Range(0, 7);
	m.Merchant = (unsigned char)r.Range(0, 255);
	m.Guild = (unsigned short)r.Range(0, 4095);
	m.Class = (unsigned char)r.Range(0, 3);
	m.Quest = (unsigned char)r.Range(0, 255);
	m.Coin = r.Range(0, 2000000000);
	m.Exp = (long long)r.Range(0, 2000000000) * r.Range(1, 3);
	m.SPX = (short)r.Range(0, 4095);
	m.SPY = (short)r.Range(0, 4095);
	m.BaseScore = RandomScore(r);
	m.CurrentScore = RandomScore(r);
	for (int i = 0; i < MAX_EQUIP; ++i)
		m.Equip[i] = r.Range(0, 3) ? RandomItem(r) : STRUCT_ITEM{};
	m.Equip[14] = RandomMountSlot(r);
	for (int i = 0; i < MAX_CARRY; ++i)
		m.Carry[i] = r.Range(0, 2) ? RandomItem(r) : STRUCT_ITEM{};
	m.LearnedSkill = (long)(r.Next() | (r.Next() << 24));
	m.Magic = r.Next() | (r.Next() << 24);
	m.ScoreBonus = (unsigned short)r.Range(0, 3000);
	m.SpecialBonus = (unsigned short)r.Range(0, 300);
	m.SkillBonus = (unsigned short)r.Range(0, 300);
	m.Critical = (unsigned char)r.Range(0, 255);
	m.SaveMana = (unsigned char)r.Range(0, 255);
	for (int i = 0; i < 4; ++i)
		m.SkillBar[i] = (unsigned char)r.Range(0, 255);
	m.GuildLevel = (unsigned char)r.Range(0, 9);
	m.RegenHP = (unsigned short)r.Range(0, 1000);
	m.RegenMP = (unsigned short)r.Range(0, 1000);
	for (int i = 0; i < 4; ++i)
		m.Resist[i] = (char)r.Range(0, 100);
	return m;
}

STRUCT_MOBEXTRA RandomExtra(Lcg& r)
{
	STRUCT_MOBEXTRA e{};
	auto b = reinterpret_cast<unsigned char*>(&e);
	for (size_t i = 0; i < sizeof(e); ++i)
		b[i] = (unsigned char)r.Range(0, 3);
	e.ClassMaster = (short)r.Range(0, 5);
	e.Citizen = (char)r.Range(0, 5);
	e.Fame = r.Range(0, 100000);
	e.Soul = (char)r.Range(0, 9);
	e.MortalFace = (short)r.Range(0, 30);
	e.QuestInfo.Celestial.ArchLevel = (short)r.Range(0, MAX_LEVEL);
	e.QuestInfo.Celestial.CelestialLevel = (short)r.Range(0, MAX_CLEVEL);
	e.QuestInfo.Celestial.SubCelestialLevel = (short)r.Range(0, MAX_CLEVEL);
	return e;
}

/// @brief Preenche as tabelas carregadas de arquivo com dados sintéticos determinísticos.
void FillTables()
{
	Lcg r(0xC0FFEEu);
	for (int i = 0; i < MAX_ITEMLIST; ++i)
	{
		STRUCT_ITEMLIST& it = g_pItemList[i];
		std::memset(&it, 0, sizeof(it));
		std::snprintf(it.Name, sizeof(it.Name), "item%d", i);
		it.IndexMesh = (short)r.Range(0, 200);
		it.IndexTexture = (short)r.Range(0, 200);
		it.IndexVisualEffect = (short)r.Range(0, 50);
		it.ReqLvl = (short)r.Range(0, MAX_LEVEL);
		it.ReqStr = (short)r.Range(0, 400);
		it.ReqInt = (short)r.Range(0, 400);
		it.ReqDex = (short)r.Range(0, 400);
		it.ReqCon = (short)r.Range(0, 400);
		for (int e = 0; e < MAX_STATICEFFECT; ++e)
		{
			it.stEffect[e].sEffect = (short)r.Range(0, 125);
			// EF_GRID indexa g_pItemGrid[8] em BASE_CanCarry/CanCargo sem validação.
			it.stEffect[e].sValue = (short)(it.stEffect[e].sEffect == EF_GRID ? r.Range(0, 7) : r.Range(0, 300));
		}
		it.Price = r.Range(0, 2000000000);
		it.nUnique = (short)r.Range(0, 80);
		it.nPos = (short)(1 << r.Range(0, 14));
		it.Extra = (short)r.Range(0, 200);
		it.Grade = (short)r.Range(0, 5);
	}
	for (int i = 0; i < MAX_SKILLINDEX; ++i)
	{
		STRUCT_SPELL& s = g_pSpell[i];
		std::memset(&s, 0, sizeof(s));
		s.SkillPoint = r.Range(0, 200);
		s.TargetType = r.Range(0, 6);
		s.ManaSpent = r.Range(0, 500);
		s.Delay = r.Range(0, 30);
		s.Range = r.Range(0, 20);
		s.InstanceType = r.Range(0, 11);
		s.InstanceValue = r.Range(0, 1000);
		s.TickType = r.Range(0, 11);
		s.TickValue = r.Range(0, 1000);
		s.AffectType = r.Range(0, 60);
		s.AffectValue = r.Range(0, 300);
		s.AffectTime = r.Range(0, 300);
		for (int a = 0; a < 8; ++a)
			s.Act[a] = (unsigned char)r.Range(0, 30);
		s.InstanceAttribute = r.Range(0, 4);
		s.TickAttribute = r.Range(0, 4);
		s.Aggressive = r.Range(0, 1);
		s.MaxTarget = r.Range(0, 13);
		s.bParty = r.Range(0, 1);
		s.AffectResist = r.Range(0, 4);
		s.Passive = r.Range(0, 1);
	}
	BASE_InitializeHitRate();
}

// ---------------------------------------------------------------------------------------
// Casos. Cada um devolve o hash das saídas para kSamples entradas.
// ---------------------------------------------------------------------------------------

uint64_t CaseItemAbility()
{
	Lcg r(1); Hash h;
	for (int i = 0; i < kSamples; ++i)
	{
		STRUCT_ITEM it = RandomItem(r);
		for (int t = 0; t < 256; t += 3)
		{
			h.Int(BASE_GetItemAbility(&it, (unsigned char)t));
			h.Int(BASE_GetItemAbilityNosanc(&it, (unsigned char)t));
			h.Int(BASE_GetStaticItemAbility(&it, (unsigned char)t));
			h.Int(BASE_GetBonusItemAbility(&it, (unsigned char)t));
			h.Int(BASE_GetBonusItemAbilityNosanc(&it, (unsigned char)t));
		}
	}
	return h.h;
}

uint64_t CaseItemSancAndAmount()
{
	Lcg r(2); Hash h;
	for (int i = 0; i < kSamples; ++i)
	{
		STRUCT_ITEM it = RandomItem(r);
		h.Int(BASE_GetItemSanc(&it));
		h.Int(BASE_GetItemSancSuccess(&it));
		h.Int(BASE_GetItemGem(&it));
		h.Int(BASE_GetGrowthRate(&it));
		h.Int(BASE_GetItemAmount(&it));
		h.Int(BASE_GetSuccessRate(&it, r.Range(0, 2)));
		h.Int(BASE_NeedLog(&it, r.Range(0, 2000000000)));
		h.Int(BASE_GetGuild(&it));
		h.Int(BASE_GetIndex(&it));
		STRUCT_ITEM a = it;
		BASE_SetItemAmount(&a, r.Range(0, 200));
		h.Bytes(&a, sizeof(a));
		STRUCT_ITEM s = it;
		h.Int(BASE_SetItemSanc(&s, r.Range(0, 15), r.Range(0, 255)));
		h.Bytes(&s, sizeof(s));
		STRUCT_ITEM c = it;
		BASE_ClearItem(&c);
		h.Bytes(&c, sizeof(c));
	}
	return h.h;
}

uint64_t CaseVisualCodes()
{
	Lcg r(3); Hash h;
	for (int i = 0; i < kSamples; ++i)
	{
		STRUCT_ITEM it = RandomItem(r);
		h.Int(BASE_VisualItemCode(&it, r.Range(0, 1)));
		h.Int(BASE_VisualAnctCode(&it));
		char code[256]{};
		BASE_GetItemCode(&it, code);
		h.Bytes(code, sizeof(code));
	}
	return h.h;
}

uint64_t CaseCanEquipCarry()
{
	Lcg r(4); Hash h;
	for (int i = 0; i < kSamples / 4; ++i)
	{
		STRUCT_MOB m = RandomMob(r);
		STRUCT_MOBEXTRA e = RandomExtra(r);
		STRUCT_ITEM it = RandomItem(r);
		for (int pos = 0; pos < MAX_EQUIP; ++pos)
			h.Int(BASE_CanEquip(&it, &m.CurrentScore, pos, m.Class, m.Equip, &e));
		int err = 0;
		h.Int(BASE_CanCarry(&it, m.Carry, r.Range(0, 8), r.Range(0, 8), &err));
		h.Int(err);
		STRUCT_ITEM cargo[MAX_CARGO]{};
		for (int k = 0; k < MAX_CARGO; ++k)
			cargo[k] = r.Range(0, 2) ? RandomItem(r) : STRUCT_ITEM{};
		// DestY <= 5: BASE_CanCargo lê CargoGrid[linha * CARGOGRIDY + coluna] (deveria ser CARGOGRIDX);
		// com linhas maiores a leitura passa do fim do buffer de 128 bytes na pilha.
		h.Int(BASE_CanCargo(&it, cargo, r.Range(0, 8), r.Range(0, 5)));
	}
	return h.h;
}

uint64_t CaseMobScores()
{
	Lcg r(5); Hash h;
	for (int i = 0; i < kSamples / 4; ++i)
	{
		STRUCT_MOB m = RandomMob(r);
		STRUCT_MOBEXTRA e = RandomExtra(r);
		h.Int(BASE_GetBonusSkillPoint(&m, &e));
		h.Int(BASE_GetBonusScorePoint(&m, &e));
		h.Int(BASE_GetHpMp(&m, &e));
		h.Int(BASE_GetMobCheckSum(&m));
		for (int t = 0; t < 256; t += 5)
		{
			h.Int(BASE_GetMobAbility(&m, (unsigned char)t));
			h.Int(BASE_GetMaxAbility(&m, (unsigned char)t));
		}
		h.Bytes(&m, sizeof(m));
	}
	return h.h;
}

uint64_t CaseCurrentScore()
{
	Lcg r(6); Hash h;
	for (int i = 0; i < kSamples / 4; ++i)
	{
		STRUCT_MOB m = RandomMob(r);
		STRUCT_MOBEXTRA e = RandomExtra(r);
		STRUCT_AFFECT aff[MAX_AFFECT]{};
		for (int a = 0; a < MAX_AFFECT; ++a)
		{
			if (r.Range(0, 2))
				continue;
			aff[a].Type = (unsigned char)r.Range(1, 60);
			aff[a].Value = (unsigned char)r.Range(0, 255);
			aff[a].Level = (unsigned short)r.Range(0, 400);
			aff[a].Time = (unsigned int)r.Range(0, 10000);
		}
		int expBonus = 0, forceMob = 0, accuracy = 0, absHp = 0, forceDamage = 0;
		BASE_GetCurrentScore(m, aff, &e, &expBonus, &forceMob, r.Range(0, 1), &accuracy, &absHp, &forceDamage);
		h.Bytes(&m, sizeof(m));
		h.Int(expBonus); h.Int(forceMob); h.Int(accuracy); h.Int(absHp); h.Int(forceDamage);
	}
	return h.h;
}

uint64_t CaseCombatFormulas()
{
	Lcg r(7); Hash h;
	std::srand(12345);
	for (int i = 0; i < kSamples; ++i)
	{
		int dam = r.Range(-200, 20000), ac = r.Range(0, 5000), combat = r.Range(0, 30);
		h.Int(BASE_GetDamage(dam, ac, combat));
		h.Int(BASE_GetSkillDamage(dam, ac, combat));
		h.Int(BASE_GetManaSpent(r.Range(0, MAX_SKILLINDEX - 1), r.Range(0, 255), r.Range(0, 255)));
		STRUCT_SCORE a = RandomScore(r), d = RandomScore(r);
		h.Int(BASE_GetHitRate(&a, &d));
		h.Int(BASE_GetDamageRate(&a, r.Range(0, MAX_SKILLINDEX - 1)));
		h.Int(BASE_GetAccuracyRate(&a));
		h.Int(BASE_GetSpeed(&a));
	}
	for (int i = 0; i < kSamples / 4; ++i)
	{
		STRUCT_MOB m = RandomMob(r);
		h.Int(BASE_GetSkillDamage(r.Range(0, MAX_SKILLINDEX - 1), &m, r.Range(0, 3), r.Range(0, 3000)));
		unsigned short sp = (unsigned short)r.Range(0, 1023), cp = (unsigned short)r.Range(0, 1023);
		unsigned char dc[4]{};
		h.Int(BASE_GetDoubleCritical(&m, &sp, &cp, dc));
		h.Int(sp); h.Int(cp); h.Bytes(dc, sizeof(dc));
	}
	return h.h;
}

uint64_t CaseWorld()
{
	Lcg r(8); Hash h;
	for (int i = 0; i < kSamples * 4; ++i)
	{
		int x = r.Range(0, 4095), y = r.Range(0, 4095);
		h.Int(BASE_GetVillage(x, y));
		h.Int(BASE_GetArena(x, y));
		h.Int(BASE_GetGuild(x, y));
		h.Int(BASE_GetDistance(x, y, x + r.Range(-40, 40), y + r.Range(-40, 40)));
	}
	for (int i = 0; i < 70000; i += 7)
		h.Int(BASE_GetSubGuild(i));
	return h.h;
}

uint64_t CaseStrings()
{
	Lcg r(9); Hash h;
	static const char alphabet[] = "abcXYZ019_ -!@\xB0\xA1\xC8\xFE";
	for (int i = 0; i < kSamples; ++i)
	{
		char s[32]{};
		int n = r.Range(0, 20);
		for (int k = 0; k < n; ++k)
			s[k] = alphabet[r.Range(0, (int)sizeof(alphabet) - 2)];
		char t[32];
		std::memcpy(t, s, sizeof(t));
		h.Int(BASE_CheckValidString(t));
		std::memcpy(t, s, sizeof(t));
		h.Int(BASE_GetEnglish(t));
		std::memcpy(t, s, sizeof(t));
		BASE_SpaceToUnderBar(t);
		h.Bytes(t, sizeof(t));
		BASE_UnderBarToSpace(t);
		h.Bytes(t, sizeof(t));
		h.Int(BASE_CheckHangul((unsigned short)r.Range(0, 65535)));
		h.Int(BASE_GetSum(s, sizeof(s)));
		h.Int(BASE_GetSum2(s, sizeof(s)));
	}
	return h.h;
}

struct Case
{
	const char* name;
	uint64_t (*fn)();
};

const Case kCases[] = {
	{ "ItemAbility", CaseItemAbility },
	{ "ItemSancAndAmount", CaseItemSancAndAmount },
	{ "VisualCodes", CaseVisualCodes },
	{ "CanEquipCarry", CaseCanEquipCarry },
	{ "MobScores", CaseMobScores },
	{ "CurrentScore", CaseCurrentScore },
	{ "CombatFormulas", CaseCombatFormulas },
	{ "World", CaseWorld },
	{ "Strings", CaseStrings },
};

struct Expected
{
	const char* name;
	uint64_t hash;
};

/// Valores gerados com o código da tag `pre-modularization` (ver @file), em ponto flutuante SSE2
/// (padrão do MSVC Win32; o mingw usa `-msse2 -mfpmath=sse`). Idênticos ao código atual.
const Expected kExpected[] = {
	{ "ItemAbility", 0x4be087df618daaaaull },
	{ "ItemSancAndAmount", 0xfcec5af4597c1fbeull },
	{ "VisualCodes", 0x6fcaf7a9a9970a97ull },
	{ "CanEquipCarry", 0xb10e393d032f681aull },
	{ "MobScores", 0xd7d142ebb152a56eull },
	{ "CurrentScore", 0x6d5c84b4dd23e63dull },
	{ "CombatFormulas", 0xc76e7e2a252190c8ull },
	{ "World", 0xa676176912029d1dull },
	{ "Strings", 0x3965cf4e379fe8e6ull },
};

} // namespace

int main(int argc, char** argv)
{
	const bool print = argc > 1 && std::strcmp(argv[1], "--print") == 0;
	FillTables();
	int failures = 0;
	for (const Case& c : kCases)
	{
		// Cada caso parte do mesmo estado das tabelas: alguns BASE_* gravam em globais.
		FillTables();
		uint64_t got = c.fn();
		if (print)
		{
			std::printf("\t{ \"%s\", 0x%016llxull },\n", c.name, (unsigned long long)got);
			continue;
		}
		uint64_t want = 0;
		for (const Expected& e : kExpected)
			if (std::strcmp(e.name, c.name) == 0)
				want = e.hash;
		if (got != want)
		{
			std::fprintf(stderr, "FALHA %s: hash 0x%016llx, esperado 0x%016llx\n", c.name,
				(unsigned long long)got, (unsigned long long)want);
			++failures;
		}
	}
	if (print)
		return 0;
	if (failures)
	{
		std::fprintf(stderr, "w2pp_characterization_tests: %d caso(s) divergente(s).\n", failures);
		return 1;
	}
	std::printf("w2pp_characterization_tests: %d casos conferem com a referencia.\n", (int)(sizeof(kCases) / sizeof(kCases[0])));
	return 0;
}
