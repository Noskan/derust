#pragma once

/// @file GameStructs.h
/// @brief Structs de jogo com a mesma definição no servidor (W2PP) e no cliente (TMProject).
/// @details Os dois lados incluem este arquivo por `common/Basedef.h`. Uma struct só entra aqui quando
/// nome, layout (Win32, sem `#pragma pack`) e tipo de cada campo são iguais nos dois lados, com os
/// nomes do cliente. Onde só o sinal de um campo difere (como em STRUCT_AFFECT), a struct fica em cada
/// lado até alguém decidir qual sinal vale, porque o sinal muda contas que um dos lados faz.
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
