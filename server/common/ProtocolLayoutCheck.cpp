#include "common/Basedef.h"

#include <cstddef>

#if defined(_WIN32) && !defined(_WIN64)
static_assert(sizeof(MSG_STANDARD) == 12, "MSG_STANDARD deve ter 12 bytes em Win32.");
static_assert(sizeof(STRUCT_ITEM) == 8, "STRUCT_ITEM deve ter 8 bytes em Win32.");
static_assert(sizeof(STRUCT_SCORE) == 48, "STRUCT_SCORE deve ter 48 bytes em Win32.");
static_assert(sizeof(STRUCT_MOB) == 1024, "STRUCT_MOB deve ter 1024 bytes em Win32 (layout do cliente).");
static_assert(sizeof(STRUCT_MOBEXTRA) == 536, "STRUCT_MOBEXTRA deve ter 536 bytes em Win32 (time_t de 64 bits).");
static_assert(sizeof(STRUCT_RANKING) == 52, "STRUCT_RANKING deve ter 52 bytes em Win32.");
static_assert(sizeof(STRUCT_ACCOUNTFILE) == 8640, "STRUCT_ACCOUNTFILE deve ter 8640 bytes em Win32.");

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

// Pacotes do login, na ordem do fluxo: conta, senha numérica, criação/exclusão e entrada.
static_assert(sizeof(MSG_AccountLogin) == 120, "MSG_AccountLogin deve ter 120 bytes (cliente).");
W2_FIELD(MSG_AccountLogin, AccountPassword, 12, 16);         // (AccountPass)
W2_FIELD(MSG_AccountLogin, AccountName, 28, 16);
W2_FIELD(MSG_AccountLogin, Zero, 44, 52);                    // (TID)
W2_FIELD(MSG_AccountLogin, ClientVersion, 96, 4);            // (Version)
W2_FIELD(MSG_AccountLogin, DBNeedSave, 100, 4);              // (Force)
W2_FIELD(MSG_AccountLogin, AdapterName, 104, 16);            // (Mac)

static_assert(sizeof(STRUCT_SELCHAR) == 840, "STRUCT_SELCHAR deve ter 840 bytes (cliente).");
static_assert(sizeof(MSG_DBCNFAccountLogin) == 1928, "MSG_DBCNFAccountLogin deve ter 1928 bytes (cliente).");
W2_FIELD(MSG_DBCNFAccountLogin, HashKeyTable, 12, 16);       // (SecretCode)
W2_FIELD(MSG_DBCNFAccountLogin, sel, 32, 840);               // (SelChar)
W2_FIELD(MSG_DBCNFAccountLogin, Cargo, 872, 1024);
W2_FIELD(MSG_DBCNFAccountLogin, Coin, 1896, 4);
W2_FIELD(MSG_DBCNFAccountLogin, AccountName, 1900, 16);

static_assert(sizeof(MSG_AccountSecure) == 32, "MSG_AccountSecure deve ter 32 bytes (MSG_CHARPASSWORD).");
W2_FIELD(MSG_AccountSecure, NumericToken, 12, 6);            // (ItemPassWord, até 6 dígitos)
W2_FIELD(MSG_AccountSecure, ChangeNumeric, 28, 1);           // (State)

static_assert(sizeof(MSG_CreateCharacter) == 36, "MSG_CreateCharacter deve ter 36 bytes (MSG_NewCharacter).");
W2_FIELD(MSG_CreateCharacter, Slot, 12, 4);
W2_FIELD(MSG_CreateCharacter, MobName, 16, 16);
W2_FIELD(MSG_CreateCharacter, MobClass, 32, 4);
static_assert(sizeof(MSG_CNFNewCharacter) == 856, "MSG_CNFNewCharacter deve ter 856 bytes (cliente).");
W2_FIELD(MSG_CNFNewCharacter, sel, 16, 840);                 // (SelChar)

static_assert(sizeof(MSG_DeleteCharacter) == 48, "MSG_DeleteCharacter deve ter 48 bytes (cliente).");
W2_FIELD(MSG_DeleteCharacter, Slot, 12, 4);
W2_FIELD(MSG_DeleteCharacter, MobName, 16, 16);
W2_FIELD(MSG_DeleteCharacter, Password, 32, 16);
static_assert(sizeof(MSG_CNFDeleteCharacter) == 856, "MSG_CNFDeleteCharacter deve ter 856 bytes (cliente).");

static_assert(sizeof(MSG_CharacterLogin) == 36, "MSG_CharacterLogin deve ter 36 bytes (cliente).");
W2_FIELD(MSG_CharacterLogin, Slot, 12, 4);
W2_FIELD(MSG_CharacterLogin, Force, 16, 4);
W2_FIELD(MSG_CharacterLogin, SecretCode, 20, 16);

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

// Pacotes de jogo (docs/DIVERGENCIAS.md, "Pacotes com campos em outros offsets").
static_assert(sizeof(MSG_MessageChat) == 140, "MSG_MessageChat deve ter 140 bytes (cliente).");
W2_FIELD(MSG_MessageChat, String, 12, 128);
static_assert(sizeof(MSG_MessageWhisper) == 160, "MSG_MessageWhisper deve ter 160 bytes (cliente).");
W2_FIELD(MSG_MessageWhisper, MobName, 12, 16);
W2_FIELD(MSG_MessageWhisper, String, 28, 128);
W2_FIELD(MSG_MessageWhisper, Color, 156, 2);
static_assert(sizeof(MSG_MagicTrumpet) == 160, "MSG_MagicTrumpet deve ter 160 bytes (MSG_MessageWhisper no cliente).");
W2_FIELD(MSG_MagicTrumpet, MobName, 12, 16);
W2_FIELD(MSG_MagicTrumpet, String, 28, 128);

static_assert(sizeof(STRUCT_PARTY) == 26, "STRUCT_PARTY deve ter 26 bytes (PARTY no cliente).");
W2_FIELD(STRUCT_PARTY, Class, 0, 1);
W2_FIELD(STRUCT_PARTY, PartyPos, 1, 1);                      // (PartyIndex)
W2_FIELD(STRUCT_PARTY, Level, 2, 2);
W2_FIELD(STRUCT_PARTY, MaxHp, 4, 2);
W2_FIELD(STRUCT_PARTY, Hp, 6, 2);
W2_FIELD(STRUCT_PARTY, PartyID, 8, 2);                       // (ID)
W2_FIELD(STRUCT_PARTY, MobName, 10, 16);                     // (Name)
static_assert(sizeof(MSG_SendReqParty) == 44, "MSG_SendReqParty deve ter 44 bytes (MSG_REQParty).");
W2_FIELD(MSG_SendReqParty, Leader, 12, 26);
W2_FIELD(MSG_SendReqParty, TargetID, 40, 4);
static_assert(sizeof(MSG_CNFAddParty) == 40, "MSG_CNFAddParty deve ter 40 bytes (MSG_AddParty).");
W2_FIELD(MSG_CNFAddParty, Party, 12, 26);
W2_FIELD(MSG_RemoveParty, Leaderconn, 12, 4);                // (MSG_STANDARDPARM::Parm)

static_assert(sizeof(STRUCT_DAM) == 8, "STRUCT_DAM deve ter 8 bytes (cliente).");
W2_FIELD(STRUCT_DAM, TargetID, 0, 2);
W2_FIELD(STRUCT_DAM, Damage, 4, 4);
#define W2_ATTACK(T, size, dam)                                                                           \
	static_assert(sizeof(T) == (size), #T " fora do tamanho do cliente");                              \
	W2_FIELD(T, FakeExp, 12, 4);                                                                       \
	W2_FIELD(T, ReqMp, 16, 4);                                                                         \
	W2_FIELD(T, CurrentExp, 24, 8);                                                                    \
	W2_FIELD(T, Rsv, 32, 2);                                                                           \
	W2_FIELD(T, PosX, 34, 2);                                                                          \
	W2_FIELD(T, AttackerID, 42, 2);                                                                    \
	W2_FIELD(T, Motion, 46, 1);                                                                        \
	W2_FIELD(T, FlagLocal, 47, 1);                                                                     \
	W2_FIELD(T, DoubleCritical, 48, 1);                                                                \
	W2_FIELD(T, SkillParm, 49, 1);                                                                     \
	W2_FIELD(T, CurrentMp, 52, 4);                                                                     \
	W2_FIELD(T, SkillIndex, 56, 2);                                                                    \
	W2_FIELD(T, Dam, 60, dam)
W2_ATTACK(MSG_Attack, 168, 104);
W2_ATTACK(MSG_AttackOne, 72, 8);
W2_ATTACK(MSG_AttackTwo, 80, 16);
#endif
