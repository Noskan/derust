/// @file base_compare.cpp
/// @brief Roda as mesmas entradas nas funções `BASE_*` que o servidor e o cliente têm em comum.
///
/// Compilado duas vezes por tools/divergence_map.py: com `-DDIV_SERVER` contra
/// server/common e com `-DDIV_CLIENT` contra client/src/common. As tabelas que o jogo carrega
/// de arquivo (`g_pItemList`, `g_pSpell`, `g_pAttribute`, mapa de alturas) recebem os mesmos
/// valores sintéticos nos dois lados, campo a campo, mesmo onde o nome ou o offset do campo
/// difere. Cada chamada grava uma linha `função<TAB>entrada<TAB>saída`; o script compara as
/// saídas linha a linha.
///
/// Onde as assinaturas diferem, o parâmetro que só um lado tem recebe um valor neutro, anotado
/// em `NOTE` e repetido no relatório.

#if defined(DIV_CLIENT)
#include "core/pch.h"
#include "common/Basedef.h"
#include "common/ItemEffect.h"
#include "core/TMGlobal.h"
#elif defined(DIV_SERVER)
#include "common/Basedef.h"
#include "common/ItemEffect.h"
#else
#error defina DIV_SERVER ou DIV_CLIENT
#endif

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

namespace
{

#if defined(DIV_CLIENT)
using TypeArg = char;
#else
using TypeArg = unsigned char;
#endif
const int kSpellCount = MAX_SPELL_LIST;	// 248 entradas nos dois lados

struct Lcg
{
	uint32_t s;
	explicit Lcg(uint32_t seed) : s(seed) {}
	uint32_t Next() { s = s * 1664525u + 1013904223u; return s >> 8; }
	int Range(int lo, int hi) { return lo + (int)(Next() % (uint32_t)(hi - lo + 1)); }
};

FILE* g_out = nullptr;

/// @brief Uma linha de saída: função, descrição da entrada, saída.
void Emit(const char* fn, const std::string& in, const std::string& out)
{
	std::fprintf(g_out, "%s\t%s\t%s\n", fn, in.c_str(), out.c_str());
}

std::string Hex(const void* p, size_t n)
{
	static const char d[] = "0123456789abcdef";
	std::string s;
	auto b = static_cast<const unsigned char*>(p);
	for (size_t i = 0; i < n; ++i)
	{
		s += d[b[i] >> 4];
		s += d[b[i] & 15];
	}
	return s;
}

std::string I(long long v) { return std::to_string(v); }

std::string ItemStr(const STRUCT_ITEM& it)
{
	char b[64];
	std::snprintf(b, sizeof(b), "item(%d;%d:%d,%d:%d,%d:%d)", it.sIndex, it.stEffect[0].cEffect, it.stEffect[0].cValue,
		it.stEffect[1].cEffect, it.stEffect[1].cValue, it.stEffect[2].cEffect, it.stEffect[2].cValue);
	return b;
}

const unsigned char kEffects[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 26, 27, 28, 42, 43, 44, 45, 46, 47, 48, 50,
	53, 54, 59, 60, 61, 62, 63, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 100, 101, 102, 103, 104, 105,
	106, 107, 108, 109, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127 };

/// @brief Item com índice válido (1..MAX_ITEMLIST-1 ou vazio) e efeitos variados, como nos testes de caracterização.
STRUCT_ITEM RandomItem(Lcg& r)
{
	STRUCT_ITEM it{};
	int k = r.Range(0, 9);
	if (k == 0)
		it.sIndex = (short)r.Range(2330, 2420);
	else if (k == 1)
		it.sIndex = (short)r.Range(3200, 4000);
	else if (k == 2)
		it.sIndex = 0;
	else
		it.sIndex = (short)r.Range(1, MAX_ITEMLIST - 1);
	for (int e = 0; e < 3; ++e)
	{
		it.stEffect[e].cEffect = kEffects[r.Range(0, (int)sizeof(kEffects) - 1)];
		it.stEffect[e].cValue = (unsigned char)r.Range(0, 255);
	}
	// Domínio do jogo: em montarias, stEffect[1].cEffect / 10 indexa g_pSancRate[2][12] no servidor.
	if (it.sIndex >= 2360 && it.sIndex < 2390)
		it.stEffect[1].cEffect = (unsigned char)r.Range(0, 119);
	return it;
}

/// @brief STRUCT_SCORE com os campos que existem nos dois lados (o servidor tem Direction e
/// ChaosRate nos bytes 14 e 15, que no cliente são preenchimento: ficam zerados).
STRUCT_SCORE RandomScore(Lcg& r)
{
	STRUCT_SCORE s{};
	s.Level = (short)r.Range(0, 400);
	s.Ac = r.Range(0, 3000);
	s.Damage = r.Range(0, 5000);
	s.Merchant = (unsigned char)r.Range(0, 255);
	s.AttackRun = (unsigned char)r.Range(0, 255);
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

/// @brief STRUCT_MOB preenchida campo a campo pelo nome (os nomes e alguns tipos diferem).
/// Campos que só um lado tem ficam zerados; campos mais largos de um lado recebem valores que
/// cabem no mais estreito.
STRUCT_MOB RandomMob(Lcg& r)
{
	STRUCT_MOB m{};
	for (int i = 0; i < 15; ++i)
		m.MobName[i] = (char)('a' + r.Range(0, 25));
	m.Clan = (char)r.Range(0, 7);
	m.Merchant = (unsigned char)r.Range(0, 255);
	m.Guild = (unsigned short)r.Range(0, 4095);
	m.Class = (unsigned char)r.Range(0, 3);
	m.Quest = (unsigned char)r.Range(0, 255);
	m.Coin = r.Range(0, 2000000000);
	m.Exp = (long long)r.Range(0, 2000000000) * r.Range(1, 3);
	short spx = (short)r.Range(0, 4095), spy = (short)r.Range(0, 4095);
	m.HomeTownX = spx;
	m.HomeTownY = spy;
	m.BaseScore = RandomScore(r);
	m.CurrentScore = RandomScore(r);
	for (int i = 0; i < 16; ++i)
		m.Equip[i] = r.Range(0, 3) ? RandomItem(r) : STRUCT_ITEM{};
	for (int i = 0; i < MAX_CARRY; ++i)
		m.Carry[i] = r.Range(0, 2) ? RandomItem(r) : STRUCT_ITEM{};
	unsigned int learned = r.Next() | (r.Next() << 24);
	m.LearnedSkill[0] = learned;
	m.Magic = (unsigned char)r.Range(0, 255);
	m.ScoreBonus = (unsigned short)r.Range(0, 3000);
	m.SpecialBonus = (unsigned short)r.Range(0, 300);
	m.SkillBonus = (unsigned short)r.Range(0, 300);
	m.Critical = (unsigned char)r.Range(0, 255);
	m.SaveMana = (unsigned char)r.Range(0, 255);
	for (int i = 0; i < 4; ++i)
	{
		unsigned char sb = (unsigned char)r.Range(0, 255);
		m.ShortSkill[i] = sb;
	}
	m.GuildLevel = (unsigned char)r.Range(0, 9);
	m.RegenHP = (unsigned char)r.Range(0, 255);
	m.RegenMP = (unsigned char)r.Range(0, 255);
	for (int i = 0; i < 4; ++i)
		m.Resist[i] = (char)r.Range(0, 100);
	return m;
}

std::string MobStr(const STRUCT_MOB& m)
{
	char b[96];
	std::snprintf(b, sizeof(b), "mob(class %d, lvl %d, str %d, int %d, dex %d, con %d, special %d/%d/%d/%d)", m.Class,
		m.CurrentScore.Level, m.CurrentScore.Str, m.CurrentScore.Int, m.CurrentScore.Dex, m.CurrentScore.Con,
		m.CurrentScore.Special[0], m.CurrentScore.Special[1], m.CurrentScore.Special[2], m.CurrentScore.Special[3]);
	return b;
}

const int kHeightPos = 2000;
const int kHeightSize = 256;
std::vector<char> g_height;

/// @brief Mesmos valores nos dois lados; cada índice usa o próprio gerador.
void FillTables()
{
	static const short staticEffects[] = { EF_LEVEL, EF_REQ_STR, EF_REQ_INT, EF_REQ_DEX, EF_REQ_CON, EF_POS,
		EF_CLASS, EF_DAMAGE, EF_AC, EF_HP, EF_MP, EF_CRITICAL, EF_SANC, EF_AMOUNT, EF_UNIQUE, EF_RESIST1,
		EF_HPADD, EF_MPADD, EF_ACADD, EF_DAMAGEADD, EF_MAGICADD, EF_PARRY, EF_HITRATE, EF_RUNSPEED, EF_ATTSPEED,
		EF_SPECIALALL, EF_INCUBATE, EF_RANGE, EF_VOLATILE, EF_GRADE0, EF_MAGIC };
	for (int i = 0; i < MAX_ITEMLIST; ++i)
	{
		Lcg r(0xC0FFEEu + (uint32_t)i * 7919u);
		STRUCT_ITEMLIST& it = g_pItemList[i];
		std::memset(&it, 0, sizeof(it));
		std::snprintf(it.Name, sizeof(it.Name), "item%d", i);
		short mesh = (short)r.Range(0, 200), tex = (short)r.Range(0, 20), vis = (short)r.Range(0, 20);
		short lvl = (short)r.Range(0, 400), str = (short)r.Range(0, 300), in = (short)r.Range(0, 300);
		short dex = (short)r.Range(0, 300), con = (short)r.Range(0, 300);
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
		int price = r.Range(0, 2000000);
		short uniq = (short)r.Range(0, 60), pos = (short)(1 << r.Range(0, 15));
		short extra = (short)r.Range(0, 3000), grade = (short)r.Range(0, 10);
		it.nIndexMesh = mesh; it.nIndexTexture = tex; it.nIndexVisualEffect = vis;
		it.nReqLvl = lvl; it.nReqStr = str; it.nReqInt = in; it.nReqDex = dex; it.nReqCon = con;
		it.nPrice = price; it.nExtra = extra; it.nGrade = grade;
		it.nUnique = uniq;
		it.nPos = pos;
	}
	for (int i = 0; i < kSpellCount; ++i)
	{
		Lcg r(0xBEEFu + (uint32_t)i * 104729u);
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
		for (int a = 0; a < 8; ++a)
			s.Act1[a] = (unsigned char)r.Range(0, 30);
		s.InstanceAttribute = r.Range(0, 5);
		s.TickAttribute = r.Range(0, 5);
		s.Aggressive = r.Range(0, 1);
		s.MaxTarget = r.Range(0, 13);
		s.bParty = r.Range(0, 1);
		s.AffectResist = r.Range(0, 5);
		s.Passive = r.Range(0, 1);
	}
	Lcg r(0xA77Bu);
	for (int y = 0; y < 1024; ++y)
		for (int x = 0; x < 1024; ++x)
			g_pAttribute[y][x] = (unsigned char)r.Range(0, 255);

	g_HeightPosX = kHeightPos;
	g_HeightPosY = kHeightPos;
	g_HeightWidth = kHeightSize;
	g_HeightHeight = kHeightSize;
	g_height.resize(kHeightSize * kHeightSize);
	for (auto& h : g_height)
		h = (char)(r.Range(0, 9) ? r.Range(-20, 40) : 127);

	BASE_InitializeHitRate();
}

const int kSamples = 3000;

void CaseItemAbility()
{
	Lcg r(1);
	for (int n = 0; n < kSamples; ++n)
	{
		STRUCT_ITEM it = RandomItem(r);
		for (int t = 0; t < 128; ++t)
		{
			std::string in = ItemStr(it) + " type " + I(t);
			Emit("BASE_GetItemAbility", in, I(BASE_GetItemAbility(&it, (TypeArg)t)));
			Emit("BASE_GetStaticItemAbility", in, I(BASE_GetStaticItemAbility(&it, (TypeArg)t)));
			Emit("BASE_GetBonusItemAbility", in, I(BASE_GetBonusItemAbility(&it, (TypeArg)t)));
			Emit("BASE_GetBonusItemAbilityNosanc", in, I(BASE_GetBonusItemAbilityNosanc(&it, (TypeArg)t)));
			Emit("BASE_GetItemAbilityNosanc", in, I(BASE_GetItemAbilityNosanc(&it, (TypeArg)t)));
		}
	}
}

void CaseItemState()
{
	Lcg r(2);
	for (int n = 0; n < kSamples * 4; ++n)
	{
		STRUCT_ITEM it = RandomItem(r);
		std::string in = ItemStr(it);
		Emit("BASE_GetItemSanc", in, I(BASE_GetItemSanc(&it)));
		Emit("BASE_GetItemSancSuccess", in, I(BASE_GetItemSancSuccess(&it)));
		Emit("BASE_GetItemAmount", in, I(BASE_GetItemAmount(&it)));
		int amount = r.Range(0, 300);
		STRUCT_ITEM a = it;
		BASE_SetItemAmount(&a, amount);
		Emit("BASE_SetItemAmount", in + " amount " + I(amount), Hex(&a, sizeof(a)));
		STRUCT_ITEM c = it;
		BASE_ClearItem(&c);
		Emit("BASE_ClearItem", in, Hex(&c, sizeof(c)));
	}
}

void CaseEquipCargoTrade()
{
	Lcg r(3);
	for (int n = 0; n < kSamples; ++n)
	{
		STRUCT_MOB mob = RandomMob(r);
		STRUCT_ITEM it = RandomItem(r);
		STRUCT_SCORE score = RandomScore(r);
		int pos = r.Range(0, 15);
		// Rosto: dezena = classe, unidade 1..4 mortal, 6..9 evoluído; 22..25 e 32 são transformações.
		static const int faces[] = { 1, 4, 6, 9, 11, 16, 21, 26, 31, 36, 22, 25, 32 };
		int face = faces[r.Range(0, (int)(sizeof(faces) / sizeof(faces[0])) - 1)];
		int orig = faces[r.Range(0, 9)];
		bool soul = r.Range(0, 1) != 0;
		std::string in = ItemStr(it) + " pos " + I(pos) + " face " + I(face) + "/" + I(orig) + " soul " + I(soul) + " lvl " + I(score.Level);
		int eq = BASE_CanEquip(&it, &score, pos, face, mob.Equip, orig, soul);
		Emit("BASE_CanEquip", in, I(eq));

		STRUCT_ITEM cargo[MAX_CARGO]{};
		for (int i = 0; i < MAX_CARGO; ++i)
			cargo[i] = r.Range(0, 2) ? RandomItem(r) : STRUCT_ITEM{};
		int cx = r.Range(0, 8), cy = r.Range(0, 13);
		Emit("BASE_CanCargo", ItemStr(it) + " x " + I(cx) + " y " + I(cy), I(BASE_CanCargo(&it, cargo, cx, cy)));

		unsigned char myTrade[MAX_TRADE];
		for (int i = 0; i < MAX_TRADE; ++i)
			myTrade[i] = (unsigned char)r.Range(0, MAX_CARRY - 1);
		STRUCT_ITEM opponent[MAX_TRADE];
		for (int i = 0; i < MAX_TRADE; ++i)
			opponent[i] = RandomItem(r);
		STRUCT_ITEM dest[MAX_CARRY]{};
#if defined(DIV_CLIENT)
		int tr = BASE_CanTrade(dest, mob.Carry, (char*)myTrade, opponent);
#else
		int tr = BASE_CanTrade(dest, mob.Carry, myTrade, opponent);
#endif
		Emit("BASE_CanTrade", "amostra " + I(n), I(tr) + " " + Hex(dest, sizeof(dest)));

		int type = r.Range(0, 1) ? EF_GRID : EF_LEVEL;
		BASE_SortTradeItem(opponent, type);
		Emit("BASE_SortTradeItem", "amostra " + I(n) + " type " + I(type), Hex(opponent, sizeof(opponent)));
	}
}

void CaseCombat()
{
	Lcg r(4);
#ifdef DIV_SERVER
	W2Srand(1234);	// o servidor usa W2Rand (common/Platform.h), o mesmo algoritmo do rand() do CRT
#else
	std::srand(1234);
#endif
	for (int n = 0; n < kSamples; ++n)
	{
		STRUCT_MOB mob = RandomMob(r);
		std::string in = MobStr(mob);
		Emit("BASE_GetSpeed", in, I(BASE_GetSpeed(&mob.CurrentScore)));
		int sk = r.Range(0, 102), sm = r.Range(0, 100), sp = r.Range(0, 255);
		Emit("BASE_GetManaSpent", "skill " + I(sk) + " savemana " + I(sm) + " special " + I(sp), I(BASE_GetManaSpent(sk, sm, sp)));
		int skill = r.Range(0, 102), weather = r.Range(0, 3), wd = r.Range(0, 3000);
		std::string sin = in + " skill " + I(skill) + " weather " + I(weather) + " weapon " + I(wd) + " face " + I(mob.Equip[0].sIndex);
		int dmg = BASE_GetSkillDamage(skill, &mob, weather, wd, mob.Equip[0].sIndex);
		Emit("BASE_GetSkillDamage", sin, I(dmg));
		for (int t = 0; t < 128; ++t)
		{
			Emit("BASE_GetMobAbility", in + " type " + I(t), I(BASE_GetMobAbility(&mob, (TypeArg)t)));
			Emit("BASE_GetMaxAbility", in + " type " + I(t), I(BASE_GetMaxAbility(&mob, (TypeArg)t)));
		}
		unsigned short sprog = (unsigned short)r.Range(0, 2000), cprog = (unsigned short)r.Range(0, 2000);
		std::string din = in + " progress " + I(sprog) + "/" + I(cprog);
		TypeArg dc[4]{};
		int crit = BASE_GetDoubleCritical(&mob, &sprog, &cprog, dc);
		Emit("BASE_GetDoubleCritical", din, I(crit) + " " + I(sprog) + " " + I(cprog) + " " + I((unsigned char)dc[0]));
	}
}

void CaseWorld()
{
	Lcg r(5);
	const int lo = kHeightPos + 40;
	const int hi = kHeightPos + kHeightSize - 40;
	for (int n = 0; n < kSamples * 4; ++n)
	{
		int x1 = r.Range(0, 4095), y1 = r.Range(0, 4095), x2 = x1 + r.Range(-40, 40), y2 = y1 + r.Range(-40, 40);
		Emit("BASE_GetDistance", I(x1) + "," + I(y1) + " -> " + I(x2) + "," + I(y2), I(BASE_GetDistance(x1, y1, x2, y2)));
		int vx = r.Range(0, 4095), vy = r.Range(0, 4095);
		Emit("BASE_GetVillage", I(vx) + "," + I(vy), I(BASE_GetVillage(vx, vy)));
		int gl = r.Range(-2, 12);
		Emit("BASE_GetSubGuild", I(gl), I(BASE_GetSubGuild(gl)));

		int sx = r.Range(lo, hi), sy = r.Range(lo, hi);
		int tx = sx + r.Range(-30, 30), ty = sy + r.Range(-30, 30);
		int dist = r.Range(0, 23);
		char route[24]{};
		int rtx = tx, rty = ty;
		int rr = BASE_GetRoute(sx, sy, &rtx, &rty, route, dist, g_height.data(), 8); // MH do servidor
		Emit("BASE_GetRoute", I(sx) + "," + I(sy) + " -> " + I(tx) + "," + I(ty) + " dist " + I(dist),
			I(rr) + " " + I(rtx) + "," + I(rty) + " " + Hex(route, sizeof(route)));
	}

	// Cada chamada parte do mesmo mapa, para que uma diferença não se propague às seguintes.
	for (int n = 0; n < kSamples; ++n)
	{
		std::vector<char> height = g_height;
		int mask = r.Range(0, 10), cur = r.Range(0, 3), next = r.Range(0, 3);
		int xx = r.Range(kHeightPos + 8, kHeightPos + kHeightSize - 8), yy = r.Range(kHeightPos + 8, kHeightPos + kHeightSize - 8);
		int rot = r.Range(0, 3), hgt = r.Range(-20, 60);
		int res = BASE_UpdateItem2(mask, cur, next, xx, yy, height.data(), rot, hgt);
		std::string changed;
		for (size_t k = 0; k < height.size(); ++k)
			if (height[k] != g_height[k])
				changed += " " + I((long long)k) + "=" + I(height[k]);
		Emit("BASE_UpdateItem2", "mask " + I(mask) + " " + I(cur) + "->" + I(next) + " at " + I(xx) + "," + I(yy) + " rot " + I(rot) + " h " + I(hgt),
			I(res) + changed);
	}

	std::vector<char> att = g_height;
	BASE_ApplyAttribute(att.data(), kHeightSize);
	for (int y = 0; y < kHeightSize; y += 16)
		Emit("BASE_ApplyAttribute", "linha " + I(y), Hex(att.data() + y * kHeightSize, kHeightSize));
}

void CaseStrings()
{
	Lcg r(6);
	static const char alphabet[] = "abcqwezAZ09-_ !\xb0\xc1";
	for (int n = 0; n < kSamples * 4; ++n)
	{
		char s[64]{};
		int len = r.Range(0, 20);
		for (int i = 0; i < len; ++i)
			s[i] = alphabet[r.Range(0, (int)sizeof(alphabet) - 2)];
		std::string in = "\"" + Hex(s, len) + "\"";
		Emit("BASE_GetSum", in, I(BASE_GetSum(s, len)));
		Emit("BASE_GetSum2", in, I(BASE_GetSum2(s, len)));
		char t[64];
		std::memcpy(t, s, sizeof(t));
		Emit("BASE_CheckValidString", in, I((int)BASE_CheckValidString(t)));
		std::memcpy(t, s, sizeof(t));
		BASE_UnderBarToSpace(t);
		Emit("BASE_UnderBarToSpace", in, Hex(t, sizeof(t)));
	}
}

} // namespace

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		std::fprintf(stderr, "uso: base_compare <saida.tsv>\n");
		return 2;
	}
	g_out = std::fopen(argv[1], "wb");
	if (!g_out)
		return 3;
	FillTables();
	CaseItemAbility();
	CaseItemState();
	CaseEquipCargoTrade();
	CaseCombat();
	CaseWorld();
	CaseStrings();
	std::fclose(g_out);
	return 0;
}
