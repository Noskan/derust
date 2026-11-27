#pragma once

/// @file GameStructs.h
/// @brief Structs de jogo com a mesma definição no servidor (W2PP) e no cliente (TMProject).
/// @details Os dois lados incluem este arquivo por `common/Basedef.h`. Uma struct só entra aqui quando
/// nome, layout (Win32, sem `#pragma pack`) e tipo de cada campo são iguais nos dois lados, com os
/// nomes do cliente. Onde só o sinal de um campo difere, o tipo escolhido tem de dar o mesmo resultado
/// nos dois lados para os valores que de fato circulam; cada caso está anotado na struct.
/// Este cabeçalho não depende de nada de `server/` nem de `client/`.

/// @brief Par efeito/valor de um item, lido como um short ou como dois bytes.
union STRUCT_BONUSEFFECT
{
	struct
	{
		unsigned char cEffect;
		unsigned char cValue;
	};
	short sValue;
};

/// @brief Item (8 bytes): índice no ItemList e três pares efeito/valor.
struct STRUCT_ITEM
{
	short sIndex;
	STRUCT_BONUSEFFECT stEffect[3];
};

/// @brief Troca de um item de origem por um de cinco destinos, cada um com sua chance.
struct STRUCT_TREASURE
{
	short Source;
	STRUCT_ITEM Target[5];
	short Rate[5];
};

/// @brief Dano num alvo dentro dos pacotes de ataque (o alvo tem 2 bytes; os 2 seguintes são alinhamento).
struct STRUCT_DAM
{
	unsigned short TargetID;
	int Damage;
};

/// @brief Skill (96 bytes): um registro do SkillData.bin. O servidor a carrega do SkillData.csv.
struct STRUCT_SPELL
{
	int SkillPoint;
	int TargetType;
	int ManaSpent;
	int Delay;
	int Range;
	int InstanceType;
	int InstanceValue;
	int TickType;
	int TickValue;
	int AffectType;
	int AffectValue;
	int AffectTime;
	char Act1[8];	// Movimentos da skill, um por posição (o cliente usa Act2 para outro tipo de malha)
	char Act2[8];
	int InstanceAttribute;
	int TickAttribute;
	int Aggressive;
	int MaxTarget;
	int bParty;
	int AffectResist;
	int Passive;
	int ForceDamage;	// O SkillData.csv do servidor não tem a coluna; lá fica 0
};

/// @brief Efeito ativo (buff/debuff) de um personagem, em MOB, conta e pacotes (8 bytes).
/// @details Os tipos são os do servidor, sem sinal. No cliente, que declarava os campos com sinal, isso
/// não muda nada para o que o servidor envia: Type < 128 (o cliente já faz cast para unsigned char na
/// maioria dos usos), Value < 32768 (nível da skill, 500/2000 dos remédios, bits das joias), Time < 2^31
/// (as contas do cliente com Time já misturam o DWORD do relógio), e Level o cliente não lê. No
/// servidor, Level guarda valores até 255 (AffectValue da skill) e Time decresce até 0.
struct STRUCT_AFFECT
{
	unsigned char Type;
	unsigned char Level;	// 1 byte; no servidor, o AffectValue da skill
	unsigned short Value;	// 2 bytes: nível do efeito; no tipo 8, os bits das joias
	unsigned int Time;
};

/// @brief Atributos de combate, defesa, HP/MP, pontos base e maestrias de um mob/personagem (48 bytes).
/// @details Tipos do servidor. No cliente, que declarava `Merchant` e `AttackRun` como `char` e
/// `Special` sem sinal, nada muda para o que o servidor envia: os dois bytes são sempre mascarados ou
/// convertidos para `unsigned char` (e `Merchant == 76` é igual com ou sem sinal), e as maestrias ficam
/// abaixo de 32768. `Direction` e `ChaosRate` ocupam bytes que no cliente eram alinhamento.
struct STRUCT_SCORE
{
	short Level;			// Os bytes 2-3 são alinhamento de Ac
	int Ac;
	int Damage;

	unsigned char Merchant;	// Nibble baixo: tipo de comerciante (15 = NPC agressivo, bit 1 = escondido); alto: direção
	unsigned char AttackRun;	// Nibble baixo: velocidade de corrida; alto: velocidade de ataque
	unsigned char Direction;	// Só o servidor usa
	unsigned char ChaosRate;	// Só o servidor usa

	int MaxHp;
	int MaxMp;
	int Hp;
	int Mp;

	short Str;
	short Int;
	short Dex;
	short Con;

	short Special[4];		// Maestrias
};

/// @brief Personagem ou NPC em memória, nos arquivos e nos pacotes de login (1024 bytes).
/// @details Tipos do servidor, nomes do cliente. No cliente, os campos de 1 byte eram `char` e os
/// bônus de pontos `short`. Para os valores que o servidor envia (classe 0..3, posições < 4096,
/// bônus < 32768), nenhum uso do cliente muda. `Critical` e `SaveMana` o cliente já lia como
/// `unsigned char` em quase todos os usos; os poucos sem cast passam a concordar com eles.
struct STRUCT_MOB
{
	char MobName[16];
	char Clan;
	unsigned char Merchant;
	unsigned short Guild;
	unsigned char Class;
	unsigned char Rsv;			// Flags RSV_* do servidor
	unsigned short Quest;

	int Coin;

	long long Exp;

	short HomeTownX;			// Posição salva (pedra estelar), para o pergaminho de retorno
	short HomeTownY;

	STRUCT_SCORE BaseScore;
	STRUCT_SCORE CurrentScore;

	STRUCT_ITEM Equip[16];
	STRUCT_ITEM Carry[64];

	// Skills aprendidas: [0] em quatro blocos de 8 bits (um por categoria) e o bit 0x40000000 do
	// limite da alma; [1] são as skills 200+ do cliente, que o servidor não tem e envia 0.
	unsigned int LearnedSkill[2];

	unsigned short ScoreBonus;	// Pontos para Str/Int/Dex/Con
	unsigned short SpecialBonus;	// Pontos para as maestrias
	unsigned short SkillBonus;	// Pontos para comprar skills

	unsigned char Critical;
	unsigned char SaveMana;

	unsigned char ShortSkill[4];	// As 4 primeiras posições da barra de skills

	unsigned char GuildLevel;	// Membro ou líder da guilda

	unsigned char Magic;		// Bônus de dano mágico, limitado a 254 no servidor

	unsigned char RegenHP;
	unsigned char RegenMP;

	char Resist[4];				// Fogo, gelo, raio e magia

	char dummy[212];			// Reservado; o servidor envia zerado
	unsigned short CurrentKill;	// Contadores de kills do cliente; o servidor não os usa
	unsigned short TotalKill;

	bool HasSoulSkill() const
	{
		return this->LearnedSkill[0] & 0x40000000;
	}
};

/// @brief Os 4 personagens de uma conta na tela de seleção (840 bytes).
/// @details Tipos do servidor; no cliente as posições eram sem sinal, o que não muda nada abaixo de 4096.
struct STRUCT_SELCHAR
{
	short HomeTownX[4];
	short HomeTownY[4];
	char MobName[4][16];
	STRUCT_SCORE Score[4];
	STRUCT_ITEM Equip[4][16];
	unsigned short Guild[4];
	int Coin[4];
	long long Exp[4];
};

/// @brief Item posto no chão na partida do servidor (InitItem.csv).
struct STRUCT_INITITEM
{
	short PosX;
	short PosY;
	short sIndex;
	short Rotate;
};

/// @brief Par efeito/valor fixo de um item do ItemList.
struct STRUCT_STATICEFFECT
{
	short sEffect;
	short sValue;
};

/// @brief Entrada do ItemList (140 bytes): nome, malha, requisitos, efeitos fixos e preço.
struct STRUCT_ITEMLIST
{
	char Name[64];
	short nIndexMesh;
	short nIndexTexture;
	short nIndexVisualEffect;
	short nReqLvl;
	short nReqStr;
	short nReqInt;
	short nReqDex;
	short nReqCon;
	STRUCT_STATICEFFECT stEffect[12];
	int nPrice;
	short nUnique;
	short nPos;
	short nExtra;
	short nGrade;
};

/// @brief Bônus das evocações e transformações (17 ints, em %): `Int_*` sobre a Int de quem invoca,
/// `Spe_*` sobre a maestria de evocação (pSummonBonus, pTransBonus no servidor).
struct STRUCT_BEASTBONUS
{
	int Int_Damage;
	int Spe_Damage;
	int Int_Ac;
	int Spe_Ac;
	int Int_Hp;
	int Spe_Hp;
	int Int_Run;
	int Spe_Run;
	int Int_Attack;
	int Spe_Attack;
	int Int_Critical;
	int Spe_Critical;
	int Int_Resist;
	int Spe_Regist;
	int Int_Delay;
	int Spe_Delay;
	int SancLevel;
};

/// @brief Ponto do mapa (x, y) em ints.
struct STRUCT_POSITION
{
	int X;
	int Y;
};

/// @brief Cidade com guerra de guilda (84 bytes), uma por reino (`g_pGuildZone`, com a mesma tabela nos
/// dois lados). Os nomes são os do cliente; entre parênteses, o nome que o servidor usava.
struct STRUCT_GUILDZONE
{
	int ChargeGuild;		///< Guilda dona da cidade.
	int ChallangeGuild;		///< Guilda desafiante.
	int GuildStartX;		///< Retorno dos membros da guilda dona (GuildSpawnX).
	int GuildStartY;		///< (GuildSpawnY)
	STRUCT_POSITION Start;	///< Retorno dos demais jogadores na cidade (CitySpawnX/CitySpawnY).
	int vx1;				///< Limites da cidade (CityLimitX1, CityLimitY1, CityLimitX2, CityLimitY2).
	int vy1;
	int vx2;
	int vy2;
	int ax1;				///< Área da guerra (WarAreaX1, WarAreaY1, WarAreaX2, WarAreaY2).
	int ay1;
	int ax2;
	int ay2;
	int hx;					///< Entrada da guilda dona na guerra (ChargeWarSpawnX/ChargeWarSpawnY).
	int hy;
	int gx;					///< Entrada da guilda desafiante (ChallangeWarSpawnX/ChallangeWarSpawnY).
	int gy;
	int Tax;				///< Imposto da cidade, em % (CityTax).
	int ChargeClan;			///< Reino da guilda dona (Clan).
	int ChargeCount;		///< Vitórias seguidas da guilda dona (Victory).
};
