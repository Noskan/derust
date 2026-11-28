/// @file account_layout_dump.cpp
/// @brief Layout do arquivo de conta (STRUCT_ACCOUNTFILE e as estruturas dentro dele), campo a campo.
///
/// O codec do w2db (services/w2db/src/legacy.rs) lê e grava o mesmo arquivo sem os headers do C++.
/// Os dois lados conferem contra shared/protocol/account_layout.txt: este programa compara a saída
/// dele com o arquivo (teste do ctest), e o teste do Rust compara a tabela de campos do codec.
///
/// Uso: account_layout_dump <account_layout.txt>          confere
///      account_layout_dump --write <account_layout.txt>  regrava
#include "common/Basedef.h"

#include <cstddef>
#include <cstdio>
#include <cstring>
#include <string>

namespace
{
std::string g_out;

void Struct(const char* name, size_t size)
{
	char line[128];
	snprintf(line, sizeof(line), "struct %s %zu\n", name, size);
	g_out += line;
}

void Field(const char* name, size_t offset, size_t size)
{
	char line[160];
	snprintf(line, sizeof(line), "  %s %zu %zu\n", name, offset, size);
	g_out += line;
}

#define F(T, f) Field(#f, offsetof(T, f), sizeof(((T*)nullptr)->f))

void Dump()
{
	Struct("STRUCT_ITEM", sizeof(STRUCT_ITEM));
	F(STRUCT_ITEM, sIndex);
	F(STRUCT_ITEM, stEffect);

	Struct("STRUCT_AFFECT", sizeof(STRUCT_AFFECT));
	F(STRUCT_AFFECT, Type);
	F(STRUCT_AFFECT, Level);
	F(STRUCT_AFFECT, Value);
	F(STRUCT_AFFECT, Time);

	Struct("STRUCT_SCORE", sizeof(STRUCT_SCORE));
	F(STRUCT_SCORE, Level);
	F(STRUCT_SCORE, Ac);
	F(STRUCT_SCORE, Damage);
	F(STRUCT_SCORE, Merchant);
	F(STRUCT_SCORE, AttackRun);
	F(STRUCT_SCORE, Direction);
	F(STRUCT_SCORE, ChaosRate);
	F(STRUCT_SCORE, MaxHp);
	F(STRUCT_SCORE, MaxMp);
	F(STRUCT_SCORE, Hp);
	F(STRUCT_SCORE, Mp);
	F(STRUCT_SCORE, Str);
	F(STRUCT_SCORE, Int);
	F(STRUCT_SCORE, Dex);
	F(STRUCT_SCORE, Con);
	F(STRUCT_SCORE, Special);

	Struct("STRUCT_MOB", sizeof(STRUCT_MOB));
	F(STRUCT_MOB, MobName);
	F(STRUCT_MOB, Clan);
	F(STRUCT_MOB, Merchant);
	F(STRUCT_MOB, Guild);
	F(STRUCT_MOB, Class);
	F(STRUCT_MOB, Rsv);
	F(STRUCT_MOB, Quest);
	F(STRUCT_MOB, Coin);
	F(STRUCT_MOB, Exp);
	F(STRUCT_MOB, HomeTownX);
	F(STRUCT_MOB, HomeTownY);
	F(STRUCT_MOB, BaseScore);
	F(STRUCT_MOB, CurrentScore);
	F(STRUCT_MOB, Equip);
	F(STRUCT_MOB, Carry);
	F(STRUCT_MOB, LearnedSkill);
	F(STRUCT_MOB, ScoreBonus);
	F(STRUCT_MOB, SpecialBonus);
	F(STRUCT_MOB, SkillBonus);
	F(STRUCT_MOB, Critical);
	F(STRUCT_MOB, SaveMana);
	F(STRUCT_MOB, ShortSkill);
	F(STRUCT_MOB, GuildLevel);
	F(STRUCT_MOB, Magic);
	F(STRUCT_MOB, RegenHP);
	F(STRUCT_MOB, RegenMP);
	F(STRUCT_MOB, Resist);
	F(STRUCT_MOB, dummy);
	F(STRUCT_MOB, CurrentKill);
	F(STRUCT_MOB, TotalKill);

	Struct("STRUCT_MOBEXTRA", sizeof(STRUCT_MOBEXTRA));
	F(STRUCT_MOBEXTRA, ClassMaster);
	F(STRUCT_MOBEXTRA, Citizen);
	F(STRUCT_MOBEXTRA, Fame);
	F(STRUCT_MOBEXTRA, Soul);
	F(STRUCT_MOBEXTRA, MortalFace);
	F(STRUCT_MOBEXTRA, QuestInfo.Mortal.Newbie);
	F(STRUCT_MOBEXTRA, QuestInfo.Mortal.TerraMistica);
	F(STRUCT_MOBEXTRA, QuestInfo.Mortal.MolarGargula);
	F(STRUCT_MOBEXTRA, QuestInfo.Mortal.PilulaOrc);
	F(STRUCT_MOBEXTRA, QuestInfo.Mortal.EMPTY);
	F(STRUCT_MOBEXTRA, QuestInfo.Arch.MortalSlot);
	F(STRUCT_MOBEXTRA, QuestInfo.Arch.MortalLevel);
	F(STRUCT_MOBEXTRA, QuestInfo.Arch.Level355);
	F(STRUCT_MOBEXTRA, QuestInfo.Arch.Level370);
	F(STRUCT_MOBEXTRA, QuestInfo.Arch.Cristal);
	F(STRUCT_MOBEXTRA, QuestInfo.Arch.EMPTY);
	F(STRUCT_MOBEXTRA, QuestInfo.Celestial.ArchLevel);
	F(STRUCT_MOBEXTRA, QuestInfo.Celestial.CelestialLevel);
	F(STRUCT_MOBEXTRA, QuestInfo.Celestial.SubCelestialLevel);
	F(STRUCT_MOBEXTRA, QuestInfo.Celestial.Lv40);
	F(STRUCT_MOBEXTRA, QuestInfo.Celestial.Lv90);
	F(STRUCT_MOBEXTRA, QuestInfo.Celestial.Add120);
	F(STRUCT_MOBEXTRA, QuestInfo.Celestial.Add150);
	F(STRUCT_MOBEXTRA, QuestInfo.Celestial.Add180);
	F(STRUCT_MOBEXTRA, QuestInfo.Celestial.Add200);
	F(STRUCT_MOBEXTRA, QuestInfo.Celestial.Arcana);
	F(STRUCT_MOBEXTRA, QuestInfo.Celestial.Reset);
	F(STRUCT_MOBEXTRA, QuestInfo.Celestial.EMPTY);
	F(STRUCT_MOBEXTRA, QuestInfo.Circle);
	F(STRUCT_MOBEXTRA, QuestInfo.EMPTY);
#define SC(i)                                              \
	F(STRUCT_MOBEXTRA, SaveCelestial[i].Class);            \
	F(STRUCT_MOBEXTRA, SaveCelestial[i].Exp);              \
	F(STRUCT_MOBEXTRA, SaveCelestial[i].SPX);              \
	F(STRUCT_MOBEXTRA, SaveCelestial[i].SPY);              \
	F(STRUCT_MOBEXTRA, SaveCelestial[i].BaseScore);        \
	F(STRUCT_MOBEXTRA, SaveCelestial[i].LearnedSkill);     \
	F(STRUCT_MOBEXTRA, SaveCelestial[i].ScoreBonus);       \
	F(STRUCT_MOBEXTRA, SaveCelestial[i].SpecialBonus);     \
	F(STRUCT_MOBEXTRA, SaveCelestial[i].SkillBonus);       \
	F(STRUCT_MOBEXTRA, SaveCelestial[i].SkillBar1);        \
	F(STRUCT_MOBEXTRA, SaveCelestial[i].SkillBar2);        \
	F(STRUCT_MOBEXTRA, SaveCelestial[i].Soul);             \
	F(STRUCT_MOBEXTRA, SaveCelestial[i].EMPTY)
	SC(0);
	SC(1);
#undef SC
	F(STRUCT_MOBEXTRA, LastNT);
	F(STRUCT_MOBEXTRA, NT);
	F(STRUCT_MOBEXTRA, KefraTicket);
	F(STRUCT_MOBEXTRA, DivineEnd);
	F(STRUCT_MOBEXTRA, Hold);
	F(STRUCT_MOBEXTRA, DayLog.Exp);
	F(STRUCT_MOBEXTRA, DayLog.YearDay);
	F(STRUCT_MOBEXTRA, DonateInfo.LastTime);
	F(STRUCT_MOBEXTRA, DonateInfo.Count);
	F(STRUCT_MOBEXTRA, EMPTY);

	Struct("STRUCT_ACCOUNTINFO", sizeof(STRUCT_ACCOUNTINFO));
	F(STRUCT_ACCOUNTINFO, AccountName);
	F(STRUCT_ACCOUNTINFO, AccountPass);
	F(STRUCT_ACCOUNTINFO, RealName);
	F(STRUCT_ACCOUNTINFO, SSN1);
	F(STRUCT_ACCOUNTINFO, SSN2);
	F(STRUCT_ACCOUNTINFO, Email);
	F(STRUCT_ACCOUNTINFO, Telephone);
	F(STRUCT_ACCOUNTINFO, Address);
	F(STRUCT_ACCOUNTINFO, NumericToken);
	F(STRUCT_ACCOUNTINFO, Year);
	F(STRUCT_ACCOUNTINFO, YearDay);

	Struct("STRUCT_ACCOUNTFILE", sizeof(STRUCT_ACCOUNTFILE));
	F(STRUCT_ACCOUNTFILE, Info);
	F(STRUCT_ACCOUNTFILE, Char);
	F(STRUCT_ACCOUNTFILE, Cargo);
	F(STRUCT_ACCOUNTFILE, Coin);
	F(STRUCT_ACCOUNTFILE, ShortSkill);
	F(STRUCT_ACCOUNTFILE, affect);
	F(STRUCT_ACCOUNTFILE, mobExtra);
	F(STRUCT_ACCOUNTFILE, Donate);
	F(STRUCT_ACCOUNTFILE, TempKey);
}
}

int main(int argc, char** argv)
{
	const bool write = argc == 3 && strcmp(argv[1], "--write") == 0;
	if (argc != 2 && !write)
	{
		fprintf(stderr, "uso: %s [--write] <account_layout.txt>\n", argv[0]);
		return 2;
	}
	const char* path = argv[argc - 1];

	Dump();

	if (write)
	{
		FILE* fp = fopen(path, "wb");
		if (fp == nullptr || fwrite(g_out.data(), 1, g_out.size(), fp) != g_out.size())
		{
			fprintf(stderr, "não gravou %s\n", path);
			return 1;
		}
		fclose(fp);
		return 0;
	}

	std::string ref;
	if (FILE* fp = fopen(path, "rb"))
	{
		char buf[4096];
		size_t n;
		while ((n = fread(buf, 1, sizeof(buf), fp)) > 0)
			ref.append(buf, n);
		fclose(fp);
	}
	if (ref != g_out)
	{
		fprintf(stderr, "o layout do arquivo de conta mudou em relação a %s; saída atual:\n%s", path, g_out.c_str());
		fprintf(stderr, "se a mudança é intencional, regrave com --write e ajuste services/w2db/src/legacy.rs\n");
		return 1;
	}
	printf("layout do arquivo de conta confere com %s\n", path);
	return 0;
}
