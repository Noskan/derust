#include "common/Basedef.h"

#include <cstddef>

#if defined(_WIN32) && !defined(_WIN64)
static_assert(sizeof(MSG_STANDARD) == 12, "MSG_STANDARD deve ter 12 bytes em Win32.");
static_assert(sizeof(STRUCT_ITEM) == 8, "STRUCT_ITEM deve ter 8 bytes em Win32.");
static_assert(sizeof(STRUCT_SCORE) == 48, "STRUCT_SCORE deve ter 48 bytes em Win32.");
static_assert(sizeof(STRUCT_MOB) == 1024, "STRUCT_MOB deve ter 1024 bytes em Win32 (layout do cliente).");
static_assert(sizeof(STRUCT_MOBEXTRA) == 536, "STRUCT_MOBEXTRA deve ter 536 bytes em Win32 (time_t de 64 bits).");
static_assert(sizeof(STRUCT_RANKING) == 52, "STRUCT_RANKING deve ter 52 bytes em Win32.");
static_assert(sizeof(STRUCT_ACCOUNTFILE) == 8632, "STRUCT_ACCOUNTFILE deve ter 8632 bytes em Win32.");

// Layout do cliente (client/src/common/Basedef.h), a versão alvo do protocolo: offset e tamanho
// de cada campo que vai no fio. Os nomes do servidor ficaram; entre parênteses, o nome no cliente
// quando difere.
#define W2_FIELD(T, f, off, size)                                                                      \
	static_assert(offsetof(T, f) == (off) && sizeof(((T*)nullptr)->f) == (size),                      \
		#T "::" #f " fora do layout do cliente")

W2_FIELD(STRUCT_MOB, MobName, 0, 16);
W2_FIELD(STRUCT_MOB, Clan, 16, 1);
W2_FIELD(STRUCT_MOB, Merchant, 17, 1);
W2_FIELD(STRUCT_MOB, Guild, 18, 2);
W2_FIELD(STRUCT_MOB, Class, 20, 1);
W2_FIELD(STRUCT_MOB, Rsv, 21, 1);
W2_FIELD(STRUCT_MOB, Quest, 22, 2);
W2_FIELD(STRUCT_MOB, Coin, 24, 4);
W2_FIELD(STRUCT_MOB, Exp, 32, 8);
W2_FIELD(STRUCT_MOB, SPX, 40, 2);                 // (HomeTownX)
W2_FIELD(STRUCT_MOB, SPY, 42, 2);                 // (HomeTownY)
W2_FIELD(STRUCT_MOB, BaseScore, 44, 48);
W2_FIELD(STRUCT_MOB, CurrentScore, 92, 48);
W2_FIELD(STRUCT_MOB, Equip, 140, 128);
W2_FIELD(STRUCT_MOB, Carry, 268, 512);
W2_FIELD(STRUCT_MOB, LearnedSkill, 780, 4);       // (LearnedSkill[0])
W2_FIELD(STRUCT_MOB, SecLearnedSkill, 784, 4);    // (LearnedSkill[1])
W2_FIELD(STRUCT_MOB, ScoreBonus, 788, 2);
W2_FIELD(STRUCT_MOB, SpecialBonus, 790, 2);
W2_FIELD(STRUCT_MOB, SkillBonus, 792, 2);
W2_FIELD(STRUCT_MOB, Critical, 794, 1);
W2_FIELD(STRUCT_MOB, SaveMana, 795, 1);
W2_FIELD(STRUCT_MOB, SkillBar, 796, 4);           // (ShortSkill)
W2_FIELD(STRUCT_MOB, GuildLevel, 800, 1);
W2_FIELD(STRUCT_MOB, Magic, 801, 1);
W2_FIELD(STRUCT_MOB, RegenHP, 802, 1);
W2_FIELD(STRUCT_MOB, RegenMP, 803, 1);
W2_FIELD(STRUCT_MOB, Resist, 804, 4);
W2_FIELD(STRUCT_MOB, Reserved, 808, 212);         // (dummy)
W2_FIELD(STRUCT_MOB, CurrentKill, 1020, 2);
W2_FIELD(STRUCT_MOB, TotalKill, 1022, 2);

// _MSG_CNFCharacterLogin (MSG_CNFCharacterLogin no cliente).
static_assert(sizeof(MSG_CNFClientCharacterLogin) == 1712, "MSG_CNFClientCharacterLogin deve ter 1712 bytes (cliente).");
W2_FIELD(MSG_CNFClientCharacterLogin, PosX, 12, 2);
W2_FIELD(MSG_CNFClientCharacterLogin, PosY, 14, 2);
W2_FIELD(MSG_CNFClientCharacterLogin, mob, 16, 1024);       // (MOB)
W2_FIELD(MSG_CNFClientCharacterLogin, Slot, 1040, 2);
W2_FIELD(MSG_CNFClientCharacterLogin, ClientID, 1042, 2);
W2_FIELD(MSG_CNFClientCharacterLogin, Weather, 1044, 2);
W2_FIELD(MSG_CNFClientCharacterLogin, ShortSkill, 1046, 16);
W2_FIELD(MSG_CNFClientCharacterLogin, Ext1, 1062, 288);
W2_FIELD(MSG_CNFClientCharacterLogin, Ext2, 1350, 362);
#endif
