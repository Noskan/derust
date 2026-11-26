/// @file tmproject_characterization_tests.cpp
/// @brief Testes de caracterização ("golden") das funções `BASE_*` do cliente (TMProject).
///
/// Mesmo método de `w2pp_characterization_tests.cpp`: cada caso alimenta as funções com
/// milhares de entradas geradas por um LCG próprio (semente fixa), acumula as saídas num
/// hash FNV-1a de 64 bits e compara com `kExpected`. As tabelas que o jogo carrega de arquivo
/// (`g_pItemList`, `g_pSpell`, `g_pAttribute`, lista de palavrões) e o mapa de alturas são
/// preenchidos com dados sintéticos determinísticos: o teste não depende de arquivos do jogo.
///
/// Os valores esperados foram gerados com o código da tag `pre-modularization`
/// (`client/Projects/TMProject/Basedef.cpp`, que depois foi dividido em
/// `client/src/common/Base*.cpp`) e conferidos contra o código atual.
///
/// Ficam de fora as funções que leem arquivo, rede, relógio ou dependem do dispositivo D3D
/// e da janela (`BASE_Read*`, `BASE_GetHttpRequest`, `BASE_GetWeekNumber`,
/// `BASE_ScreenResize`, `BASE_Get3DTo2DPos`).
///
/// As entradas respeitam o domínio que o jogo garante. Fora dele, várias funções leem fora
/// dos limites (ver docs/BUGS_LATENTES.md) e o resultado passaria a depender do layout de
/// memória, não do código.
///
/// Uso: `tmproject_characterization_tests` (compara) ou `tmproject_characterization_tests --print`
/// (imprime a tabela atual, para regravar `kExpected` após uma mudança intencional).

// Os headers do cliente dependem do header pré-compilado (Windows, D3D9), como nos fontes do cliente.
#include "core/pch.h"
#include "common/Basedef.h"
#include "common/ItemEffect.h"
#include "core/TMGlobal.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

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
	void Float(float f) { Bytes(&f, sizeof(f)); }
};

const int kSamples = 3000;

// Mapa de alturas sintético e sua posição no mundo (g_HeightPosX/Y, g_HeightWidth/Height).
const int kHeightPos = 2000;
const int kHeightSize = 256;
std::vector<char> g_height;

const unsigned char kEffects[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 26, 27, 28, 42, 43, 44, 45, 46, 47, 48, 50,
	53, 54, 59, 60, 61, 62, 63, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 100, 101, 102, 103, 104, 105,
	106, 107, 108, 109, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127 };

/// @brief Item sintético com índice válido (1..MAX_ITEMLIST-1 ou vazio) e efeitos variados.
/// BASE_GetItemAbility aceita `sIndex == MAX_ITEMLIST` (lê fora de g_pItemList), então esse
/// valor fica de fora.
STRUCT_ITEM RandomItem(Lcg& r)
{
	STRUCT_ITEM it{};
	int k = r.Range(0, 9);
	if (k == 0)
		it.sIndex = (short)r.Range(2330, 2420);   // montarias e crias
	else if (k == 1)
		it.sIndex = (short)r.Range(3200, 4000);   // consumíveis/especiais
	else if (k == 2)
		it.sIndex = 0;
	else
		it.sIndex = (short)r.Range(1, MAX_ITEMLIST - 1);
	for (int e = 0; e < 3; ++e)
	{
		it.stEffect[e].cEffect = kEffects[r.Range(0, (int)sizeof(kEffects) - 1)];
		it.stEffect[e].cValue = (unsigned char)r.Range(0, 255);
	}
	return it;
}

STRUCT_SCORE RandomScore(Lcg& r)
{
	STRUCT_SCORE s{};
	s.Level = (short)r.Range(0, 400);
	s.Ac = r.Range(0, 3000);
	s.Damage = r.Range(0, 5000);
	s.Reserved = (char)r.Range(0, 255);
	s.AttackRun = (char)r.Range(0, 255);
	s.MaxHp = r.Range(1, 60000);
	s.MaxMp = r.Range(1, 60000);
	s.Hp = r.Range(0, s.MaxHp);
	s.Mp = r.Range(0, s.MaxMp);
	s.Str = (short)r.Range(0, 32000);
	s.Int = (short)r.Range(0, 32000);
	s.Dex = (short)r.Range(0, 32000);
	s.Con = (short)r.Range(0, 32000);
	for (int i = 0; i < 4; ++i)
		s.Special[i] = (unsigned short)r.Range(0, 255);
	return s;
}

STRUCT_MOB RandomMob(Lcg& r)
{
	STRUCT_MOB m{};
	for (int i = 0; i < 15; ++i)
		m.MobName[i] = (char)('a' + r.Range(0, 25));
	m.Clan = (char)r.Range(0, 7);
	m.Merchant = (char)r.Range(0, 255);
	m.Guild = (unsigned short)r.Range(0, 4095);
	m.Class = (char)r.Range(0, 3);
	m.Quest = (unsigned short)r.Range(0, 255);
	m.Coin = r.Range(0, 2000000000);
	m.Exp = (long long)r.Range(0, 2000000000) * r.Range(1, 3);
	m.BaseScore = RandomScore(r);
	m.CurrentScore = RandomScore(r);
	for (int i = 0; i < 16; ++i)
		m.Equip[i] = r.Range(0, 3) ? RandomItem(r) : STRUCT_ITEM{};
	for (int i = 0; i < MAX_CARRY; ++i)
		m.Carry[i] = r.Range(0, 2) ? RandomItem(r) : STRUCT_ITEM{};
	m.LearnedSkill[0] = r.Next() | (r.Next() << 24);
	m.LearnedSkill[1] = r.Next() | (r.Next() << 24);
	m.ScoreBonus = (short)r.Range(0, 3000);
	m.SpecialBonus = (short)r.Range(0, 300);
	m.SkillBonus = (short)r.Range(0, 300);
	m.Critical = (char)r.Range(0, 255);
	m.SaveMana = (char)r.Range(0, 255);
	m.GuildLevel = (char)r.Range(0, 9);
	m.Magic = (char)r.Range(0, 255);
	for (int i = 0; i < 4; ++i)
		m.Resist[i] = (char)r.Range(0, 100);
	return m;
}

/// @brief Lista de palavrões sintética (ASCII, sem uma tradução conter o original).
stCurse g_curses[3];

/// @brief Preenche as tabelas carregadas de arquivo com dados sintéticos determinísticos.
void FillTables()
{
	Lcg r(0xC0FFEEu);
	static const short staticEffects[] = { EF_LEVEL, EF_REQ_STR, EF_REQ_INT, EF_REQ_DEX, EF_REQ_CON, EF_POS,
		EF_CLASS, EF_DAMAGE, EF_AC, EF_HP, EF_MP, EF_CRITICAL, EF_SANC, EF_AMOUNT, EF_UNIQUE, EF_RESIST1,
		EF_HPADD, EF_MPADD, EF_ACADD, EF_DAMAGEADD, EF_MAGICADD, EF_PARRY, EF_HITRATE, EF_RUNSPEED, EF_ATTSPEED,
		EF_SPECIALALL, EF_INCUBATE, EF_RANGE, EF_VOLATILE, EF_GRADE0, EF_REFLEVEL, EF_MAGIC };
	for (int i = 0; i < MAX_ITEMLIST; ++i)
	{
		STRUCT_ITEMLIST& it = g_pItemList[i];
		std::memset(&it, 0, sizeof(it));
		std::snprintf(it.Name, sizeof(it.Name), "item%d", i);
		it.nIndexMesh = (short)r.Range(0, 200);
		it.nIndexTexture = (short)r.Range(0, 20);
		it.nIndexVisualEffect = (short)r.Range(0, 20);
		it.nReqLvl = (short)r.Range(0, 400);
		it.nReqStr = (short)r.Range(0, 300);
		it.nReqInt = (short)r.Range(0, 300);
		it.nReqDex = (short)r.Range(0, 300);
		it.nReqCon = (short)r.Range(0, 300);
		for (int e = 0; e < 12; ++e)
		{
			if (r.Range(0, 2) == 0)
				continue;
			it.stEffect[e].sEffect = staticEffects[r.Range(0, (int)(sizeof(staticEffects) / sizeof(staticEffects[0])) - 1)];
			it.stEffect[e].sValue = (short)r.Range(0, 300);
		}
		// Domínio do jogo: EF_GRID indexa g_pItemGrid[8] sem validar.
		it.stEffect[11].sEffect = EF_GRID;
		it.stEffect[11].sValue = (short)r.Range(0, 7);
		it.nPrice = r.Range(0, 2000000);
		it.nUnique = (short)r.Range(0, 60);
		it.nPos = (short)(1 << r.Range(0, 15));
		it.nExtra = (short)r.Range(0, 3000);
		it.nGrade = (short)r.Range(0, 10);
	}
	for (int i = 0; i < MAX_SPELL_LIST; ++i)
	{
		STRUCT_SPELL& s = g_pSpell[i];
		std::memset(&s, 0, sizeof(s));
		s.SkillPoint = r.Range(0, 10);
		s.TargetType = r.Range(0, 5);
		s.ManaSpent = r.Range(0, 500);
		s.Delay = r.Range(0, 30);
		s.Range = r.Range(0, 10);
		s.InstanceType = r.Range(0, 12);
		s.InstanceValue = r.Range(0, 2000);
		s.TickType = r.Range(0, 5);
		s.TickValue = r.Range(0, 500);
		s.AffectType = r.Range(0, 40);
		s.AffectValue = r.Range(0, 500);
		s.AffectTime = r.Range(0, 600);
		s.InstanceAttribute = r.Range(0, 5);
		s.TickAttribute = r.Range(0, 5);
		s.Aggressive = r.Range(0, 1);
		s.MaxTarget = r.Range(0, 13);
		s.bParty = r.Range(0, 1);
		s.AffectResist = r.Range(0, 5);
		s.Passive = r.Range(0, 1);
		s.ForceDamage = r.Range(0, 3000);
	}
	for (int y = 0; y < 1024; ++y)
		for (int x = 0; x < 1024; ++x)
			g_pAttribute[y][x] = (char)r.Range(0, 255);

	g_HeightPosX = kHeightPos;
	g_HeightPosY = kHeightPos;
	g_HeightWidth = kHeightSize;
	g_HeightHeight = kHeightSize;
	g_height.resize(kHeightSize * kHeightSize);
	for (auto& h : g_height)
		h = (char)(r.Range(0, 9) ? r.Range(-20, 40) : 127);

	std::strcpy(g_curses[0].szOriginal, "abc");
	std::strcpy(g_curses[0].szTrans, "***");
	std::strcpy(g_curses[1].szOriginal, "zz");
	std::strcpy(g_curses[1].szTrans, "y");
	std::strcpy(g_curses[2].szOriginal, "qwe");
	std::strcpy(g_curses[2].szTrans, "Q");
	g_pCurseList.pCurseList = g_curses;
	g_pCurseList.dnum = 3;

	BASE_InitializeHitRate();
}

uint64_t CaseItemAbility()
{
	Lcg r(1);
	Hash h;
	for (int n = 0; n < kSamples; ++n)
	{
		STRUCT_ITEM it = RandomItem(r);
		for (int t = 0; t < 128; ++t)
		{
			h.Int(BASE_GetItemAbility(&it, (char)t));
			h.Int(BASE_GetStaticItemAbility(&it, (char)t));
			h.Int(BASE_GetBonusItemAbility(&it, (char)t));
			h.Int(BASE_GetBonusItemAbilityNosanc(&it, (char)t));
			h.Int(BASE_GetItemAbilityNosanc(&it, (char)t));
		}
	}
	return h.h;
}

uint64_t CaseItemSancAndColor()
{
	Lcg r(2);
	Hash h;
	for (int n = 0; n < kSamples * 4; ++n)
	{
		STRUCT_ITEM it = RandomItem(r);
		h.Int(BASE_GetItemSanc(&it));
		h.Int(BASE_GetItemTenColor(&it));
		h.Int(BASE_GetItemColorEffect(&it));
		unsigned int color = BASE_GetItemColor(&it);
		h.Int(color);
		h.Int(BASE_GetColorCount(color));
		h.Int(BASE_GetColorCount(r.Next() | (r.Next() << 24)));
		h.Int(BASE_GetItemSancSuccess(&it));
		h.Int(BASE_GetSancEffValue(it));
		h.Int(BASE_HasSancAdd(it.stEffect[r.Range(0, 2)]));
		h.Int(BASE_CanRefine(&it));
		h.Int(BASE_GetItemAmount(&it));
		h.Int(BASE_GetEffectValue(&it, kEffects[r.Range(0, (int)sizeof(kEffects) - 1)]));
		h.Int(BASE_GetSubGuild(r.Range(-2, 12)));
		h.Int(BASE_GetMeshIndex((short)r.Range(0, MAX_ITEMLIST - 1)));
		h.Int(BASE_GetOptionColor(r.Range(0, 255), (unsigned int)kEffects[r.Range(0, (int)sizeof(kEffects) - 1)],
			r.Range(0, 255)));

		STRUCT_ITEM copy = it;
		BASE_SetItemAmount(&copy, r.Range(0, 300));
		h.Bytes(&copy, sizeof(copy));
		copy = it;
		BASE_ChangeOrAddEffectValue(&copy, kEffects[r.Range(0, (int)sizeof(kEffects) - 1)], r.Range(0, 300));
		h.Bytes(&copy, sizeof(copy));
		copy = it;
		BASE_RemoveEffect(&copy, it.stEffect[r.Range(0, 2)].cEffect);
		h.Bytes(&copy, sizeof(copy));
		copy = it;
		BASE_ClearItem(&copy);
		h.Bytes(&copy, sizeof(copy));
	}
	return h.h;
}

uint64_t CaseEquipCarryTrade()
{
	Lcg r(3);
	Hash h;
	for (int n = 0; n < kSamples; ++n)
	{
		STRUCT_MOB mob = RandomMob(r);
		STRUCT_ITEM it = RandomItem(r);
		STRUCT_SCORE score = RandomScore(r);
		int pos = r.Range(-1, 15);
		int cls = r.Range(0, 3);
		int face = r.Range(0, 40);
		bool soul = r.Range(0, 1) != 0;
		h.Int(BASE_CanEquip(&it, &score, pos, cls, mob.Equip, face, soul));

		STRUCT_REQ req{};
		h.Int(BASE_CanEquip_RecvRes(&req, &it, &score, pos, cls, mob.Equip, face));
		h.Bytes(&req, sizeof(req));

		if (r.Range(0, 3) == 0)
			mob.Carry[60].sIndex = 3467;
		if (r.Range(0, 3) == 0)
			mob.Carry[61].sIndex = 3467;
		h.Int(BASE_CanCarry(mob.Carry, r.Range(0, MAX_CARRY - 1)));

		STRUCT_ITEM cargo[MAX_CARGO]{};
		for (int i = 0; i < MAX_CARGO; ++i)
			cargo[i] = r.Range(0, 2) ? RandomItem(r) : STRUCT_ITEM{};
		h.Int(BASE_CanCargo(&it, cargo, r.Range(0, 8), r.Range(0, 13)));

		// Domínio do jogo: cada entrada de MyTrade é um slot do inventário (CanTrade não valida).
		char myTrade[MAX_TRADE];
		for (int i = 0; i < MAX_TRADE; ++i)
			myTrade[i] = (char)r.Range(0, MAX_CARRY - 1);
		STRUCT_ITEM opponent[MAX_TRADE];
		for (int i = 0; i < MAX_TRADE; ++i)
			opponent[i] = RandomItem(r);
		STRUCT_ITEM dest[MAX_CARRY]{};
		h.Int(BASE_CanTrade(dest, mob.Carry, myTrade, opponent));
		h.Bytes(dest, sizeof(dest));

		BASE_SortTradeItem(opponent, r.Range(0, 1) ? EF_GRID : EF_LEVEL);
		h.Bytes(opponent, sizeof(opponent));
	}
	return h.h;
}

uint64_t CaseCombat()
{
	Lcg r(4);
	Hash h;
	std::srand(1234);
	for (int n = 0; n < kSamples; ++n)
	{
		STRUCT_MOB mob = RandomMob(r);
		h.Int(BASE_GetSpeed(&mob.CurrentScore));
		h.Int(BASE_GetManaSpent(r.Range(0, MAX_SPELL_LIST - 1), r.Range(0, 100), r.Range(0, 255)));
		h.Int(BASE_GetSkillDamage(r.Range(0, 5000), r.Range(0, 3000), r.Range(-5, 30)));
		h.Int(BASE_GetSkillDamage(r.Range(0, MAX_SPELL_LIST - 1), &mob, r.Range(0, 3), r.Range(0, 3000), r.Range(0, 40)));
		for (int t = 0; t < 128; t += 3)
		{
			h.Int(BASE_GetMobAbility(&mob, (char)t));
			h.Int(BASE_GetMaxAbility(&mob, (char)t));
		}
		unsigned short sp = (unsigned short)r.Range(0, 2000);
		unsigned short cp = (unsigned short)r.Range(0, 2000);
		char dc = 0;
		h.Int(BASE_GetDoubleCritical(&mob, r.Range(0, 1) ? &sp : nullptr, &cp, &dc));
		h.Int(sp);
		h.Int(cp);
		h.Int(dc);
	}
	return h.h;
}

uint64_t CaseWorld()
{
	Lcg r(5);
	Hash h;
	const int lo = kHeightPos + 40;
	const int hi = kHeightPos + kHeightSize - 40;
	for (int n = 0; n < kSamples * 4; ++n)
	{
		h.Int(BASE_DefineSkinMeshType(r.Range(-1, 40)));
		h.Float(BASE_GetMountScale(r.Range(0, 40), r.Range(0, 10)));
		h.Int(BASE_GetDistance(r.Range(0, 4095), r.Range(0, 4095), r.Range(0, 4095), r.Range(0, 4095)));
		int x1 = r.Range(0, 4095), y1 = r.Range(0, 4095);
		h.Int(BASE_GetDistance(x1, y1, x1 + r.Range(-8, 8), y1 + r.Range(-8, 8)));
		h.Int(BASE_GetVillage(r.Range(0, 4095), r.Range(0, 4095)));
		h.Int(BASE_GetAttribute(r.Range(-10, 4200), r.Range(-10, 4200)));
		// Domínio: GetAttr não valida e coordenadas negativas leem fora de g_pAttribute.
		h.Int(BASE_GetAttr(r.Range(0, 8191), r.Range(0, 8191)));
		h.Int(BASE_IsInLowZone(r.Range(0, 4095), r.Range(0, 4095)));

		int sx = r.Range(lo, hi), sy = r.Range(lo, hi);
		int tx = sx + r.Range(-30, 30), ty = sy + r.Range(-30, 30);
		char route[24];
		int rtx = tx, rty = ty;
		h.Int(BASE_GetRoute(sx, sy, &rtx, &rty, route, r.Range(0, 30), g_height.data(), r.Range(1, 30)));
		h.Bytes(route, sizeof(route));
		h.Int(rtx);
		h.Int(rty);

		int htx = tx, hty = ty;
		BASE_GetHitPosition(sx, sy, &htx, &hty, g_height.data(), r.Range(1, 30));
		h.Int(htx);
		h.Int(hty);
		htx = tx;
		hty = ty;
		BASE_GetHitPosition2(sx, sy, &htx, &hty, g_height.data(), r.Range(1, 30));
		h.Int(htx);
		h.Int(hty);

		char bits[16]{};
		BASE_SetBit(bits, r.Range(0, 127));
		BASE_SetBit(bits, r.Range(0, 127));
		h.Bytes(bits, sizeof(bits));
	}

	std::vector<char> height = g_height;
	for (int n = 0; n < kSamples; ++n)
		h.Int(BASE_UpdateItem2(r.Range(-1, 10), r.Range(0, 3), r.Range(0, 3), r.Range(kHeightPos - 4, kHeightPos + kHeightSize + 4),
			r.Range(kHeightPos - 4, kHeightPos + kHeightSize + 4), height.data(), r.Range(0, 3), r.Range(-20, 60)));
	h.Bytes(height.data(), height.size());
	return h.h;
}

uint64_t CaseStrings()
{
	Lcg r(6);
	Hash h;
	static const char alphabet[] = "abcqwezAZ09-_ !\xb0\xc1";
	for (int n = 0; n < kSamples * 4; ++n)
	{
		char s[64]{};
		int len = r.Range(0, 20);
		for (int i = 0; i < len; ++i)
			s[i] = alphabet[r.Range(0, (int)sizeof(alphabet) - 2)];
		h.Int(BASE_GetSum(s, len));
		h.Int(BASE_GetSum2(s, len));
		h.Int(BASE_CheckValidString(s));
		h.Int(BASE_CheckChatValid(s));

		char u[64];
		std::memcpy(u, s, sizeof(u));
		BASE_UnderBarToSpace(u);
		h.Bytes(u, sizeof(u));

		// TransCurse usa CharNext/CharPrev: só ASCII, para não depender da página de código.
		char c[128]{};
		int clen = r.Range(0, 30);
		static const char ascii[] = "abcqwez xyz";
		for (int i = 0; i < clen; ++i)
			c[i] = ascii[r.Range(0, (int)sizeof(ascii) - 2)];
		h.Int(BASE_TransCurse(c) == c);
		h.Bytes(c, sizeof(c));
	}
	return h.h;
}

/// @brief Entradas fora do domínio do jogo que antes liam ou gravavam fora de uma tabela.
/// @details Conferidas com valores exatos (definidos pela correção), não por hash. Ver a seção
/// "Corrigidos" de docs/BUGS_LATENTES.md.
int CheckOutOfDomainInputs()
{
	int failures = 0;
	auto check = [&](bool ok, const char* what) {
		if (!ok)
		{
			std::printf("FALHA fora do dominio: %s\n", what);
			++failures;
		}
	};

	// sIndex == MAX_ITEMLIST: a guarda era `> MAX_ITEMLIST` e lia g_pItemList[MAX_ITEMLIST].
	STRUCT_ITEM last{};
	last.sIndex = MAX_ITEMLIST;
	last.stEffect[0].cEffect = EF_DAMAGE;
	last.stEffect[0].cValue = 50;
	for (int t = 0; t < 128; ++t)
	{
		if (BASE_GetItemAbility(&last, (char)t) || BASE_GetStaticItemAbility(&last, (char)t) ||
			BASE_GetBonusItemAbility(&last, (char)t) || BASE_GetBonusItemAbilityNosanc(&last, (char)t))
		{
			check(false, "habilidade de item com sIndex == MAX_ITEMLIST");
			break;
		}
	}

	// Coordenada negativa: o resto da divisão ficava negativo.
	check(BASE_GetAttr(-4, 100) == 0, "BASE_GetAttr(-4, 100)");
	check(BASE_GetAttr(100, -4) == 0, "BASE_GetAttr(100, -4)");
	check(BASE_GetAttr(100, 100) == g_pAttribute[25][25], "BASE_GetAttr(100, 100)");

	// MyTrade com -1 (slot vazio): antes limpava Dest[255], fora do inventário.
	STRUCT_ITEM carry[MAX_CARRY]{};
	for (int i = 0; i < MAX_CARRY; ++i)
		carry[i].sIndex = (short)(i < 30 ? 1 + i : 0);
	char myTrade[MAX_TRADE];
	std::memset(myTrade, -1, sizeof(myTrade));
	myTrade[0] = 3;
	STRUCT_ITEM opponent[MAX_TRADE]{};
	STRUCT_ITEM dest[MAX_CARRY]{};
	check(BASE_CanTrade(dest, carry, myTrade, opponent) == 1, "BASE_CanTrade com -1");
	check(dest[3].sIndex == 0 && dest[4].sIndex == 5 && dest[0].sIndex == 1, "BASE_CanTrade limpa só os slots marcados");

	// EF_GRID fora de 0..7 num item do baú: vale como item de uma célula (o resultado de
	// BASE_CanCargo do cliente é sempre 1; o que se confere aqui é não ler além de g_pItemGrid).
	STRUCT_ITEMLIST saved = g_pItemList[2];
	g_pItemList[2].stEffect[0].sEffect = EF_GRID;
	g_pItemList[2].stEffect[0].sValue = 200;
	STRUCT_ITEM cargo[MAX_CARGO]{};
	cargo[9].sIndex = 2;
	check(BASE_CanCargo(&cargo[9], cargo, 0, 0) == 1, "BASE_CanCargo com EF_GRID 200");
	g_pItemList[2] = saved;

	return failures;
}

struct Case
{
	const char* name;
	uint64_t (*fn)();
};

const Case kCases[] = {
	{ "ItemAbility", CaseItemAbility },
	{ "ItemSancAndColor", CaseItemSancAndColor },
	{ "EquipCarryTrade", CaseEquipCarryTrade },
	{ "Combat", CaseCombat },
	{ "World", CaseWorld },
	{ "Strings", CaseStrings },
};

/// Valores gerados com o código da tag `pre-modularization` (ver @file), em ponto flutuante SSE2
/// (padrão do MSVC Win32; o mingw usa `-msse2 -mfpmath=sse`). Conferem também com `-O0` e x87.
/// Mudança intencional: `ItemSancAndColor` e `EquipCarryTrade` foram regravados quando
/// `BASE_ClearItem` passou a zerar o item inteiro (antes, `sizeof` do ponteiro: 4 de 8 bytes).
const struct
{
	const char* name;
	uint64_t hash;
} kExpected[] = {
	{ "ItemAbility", 0xba651a92727573d2ull },
	{ "ItemSancAndColor", 0x17120ba33e0148ceull },
	{ "EquipCarryTrade", 0xe7dcd9fdb0faea97ull },
	{ "Combat", 0x3bae9037bdb69946ull },
	{ "World", 0x9f42c7c0d5518942ull },
	{ "Strings", 0xff7b3c1c5a0c6e8eull },
};

} // namespace

int main(int argc, char** argv)
{
	bool print = argc > 1 && std::strcmp(argv[1], "--print") == 0;
	FillTables();

	int failures = print ? 0 : CheckOutOfDomainInputs();
	for (size_t i = 0; i < sizeof(kCases) / sizeof(kCases[0]); ++i)
	{
		uint64_t got = kCases[i].fn();
		if (print)
		{
			std::printf("\t{ \"%s\", 0x%016llxull },\n", kCases[i].name, (unsigned long long)got);
			continue;
		}
		if (got != kExpected[i].hash)
		{
			std::printf("FALHA %s: hash 0x%016llx, esperado 0x%016llx\n", kCases[i].name, (unsigned long long)got,
				(unsigned long long)kExpected[i].hash);
			++failures;
		}
	}
	if (print)
		return 0;
	if (failures)
	{
		std::printf("tmproject_characterization_tests: %d caso(s) divergente(s).\n", failures);
		return 1;
	}
	std::printf("tmproject_characterization_tests: %zu casos conferem com a referencia.\n",
		sizeof(kCases) / sizeof(kCases[0]));
	return 0;
}
