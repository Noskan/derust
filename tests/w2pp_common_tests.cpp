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

/// @file w2pp_common_tests.cpp
/// @brief Suíte de testes unitários (CTest) para funções puras e travas ABI da biblioteca w2pp_common.

#include "common/Basedef.h"
#include "common/ItemEffect.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace
{

int g_failures = 0;

void CheckImpl(bool cond, const char* expr, const char* file, int line)
{
	if (!cond)
	{
		std::fprintf(stderr, "FALHA [%s:%d]: %s\n", file, line, expr);
		++g_failures;
	}
}

#define CHECK(expr) CheckImpl(static_cast<bool>(expr), #expr, __FILE__, __LINE__)

/// @brief Valida tamanhos binários das estruturas de rede e persistência em Win32.
void TestProtocolSizes()
{
	CHECK(sizeof(MSG_STANDARD) == 12);
	CHECK(sizeof(STRUCT_ITEM) == 8);
	CHECK(sizeof(STRUCT_SCORE) == 48);
	CHECK(sizeof(STRUCT_MOB) == 1024);
	CHECK(sizeof(STRUCT_MOBEXTRA) == 536);
	CHECK(sizeof(STRUCT_RANKING) == 52);
	CHECK(sizeof(STRUCT_ACCOUNTFILE) == 8640);
}

/// @brief Testa leitura e gravação de refinação (EF_SANC, +10..+15, tinturas 116..125 e montarias).
void TestItemSanc()
{
	STRUCT_ITEM item{};
	CHECK(BASE_GetItemSanc(&item) == 0);

	// Montarias na faixa 2330..2389 sempre retornam 0 em BASE_GetItemSanc.
	item.sIndex = 2330;
	item.stEffect[0].cEffect = EF_SANC;
	item.stEffect[0].cValue = 9;
	CHECK(BASE_GetItemSanc(&item) == 0);

	item.sIndex = 2389;
	CHECK(BASE_GetItemSanc(&item) == 0);

	// Item comum com EF_SANC padrão +0..+9 e módulo 10 para valores comuns >= 10.
	item = {};
	item.sIndex = 1200;
	item.stEffect[0].cEffect = EF_SANC;
	item.stEffect[0].cValue = 9;
	CHECK(BASE_GetItemSanc(&item) == 9);
	item.stEffect[0].cValue = 10;
	CHECK(BASE_GetItemSanc(&item) == 0);

	// Faixas de refinação celestial +10..+15.
	item.stEffect[0].cValue = 230;
	CHECK(BASE_GetItemSanc(&item) == REF_10);
	item.stEffect[0].cValue = 233;
	CHECK(BASE_GetItemSanc(&item) == REF_10);
	item.stEffect[0].cValue = 234;
	CHECK(BASE_GetItemSanc(&item) == REF_11);
	item.stEffect[0].cValue = 238;
	CHECK(BASE_GetItemSanc(&item) == REF_12);
	item.stEffect[0].cValue = 242;
	CHECK(BASE_GetItemSanc(&item) == REF_13);
	item.stEffect[0].cValue = 246;
	CHECK(BASE_GetItemSanc(&item) == REF_14);
	item.stEffect[0].cValue = 250;
	CHECK(BASE_GetItemSanc(&item) == REF_15);
	item.stEffect[0].cValue = 253;
	CHECK(BASE_GetItemSanc(&item) == REF_15);

	// Tinturas (cEffect 116..125) em qualquer um dos 3 slots de efeito.
	item = {};
	item.sIndex = 1200;
	item.stEffect[1].cEffect = 120;
	item.stEffect[1].cValue = 235;
	CHECK(BASE_GetItemSanc(&item) == REF_11);

	item = {};
	item.sIndex = 1200;
	item.stEffect[2].cEffect = 125;
	item.stEffect[2].cValue = 7;
	CHECK(BASE_GetItemSanc(&item) == 7);

	// BASE_SetItemSanc deve falhar sem slot EF_SANC/tintura e atualizar corretamente quando presente.
	STRUCT_ITEM noSlot{};
	noSlot.sIndex = 1200;
	CHECK(BASE_SetItemSanc(&noSlot, 7, 2) == FALSE);

	STRUCT_ITEM withSlot{};
	withSlot.sIndex = 1200;
	withSlot.stEffect[0].cEffect = EF_SANC;
	CHECK(BASE_SetItemSanc(&withSlot, 7, 2) == TRUE);
	CHECK(withSlot.stEffect[0].cValue == 27);
	CHECK(BASE_GetItemSanc(&withSlot) == 7);

	CHECK(BASE_SetItemSanc(&withSlot, 12, 1) == TRUE);
	CHECK(withSlot.stEffect[0].cValue == 239);
	CHECK(BASE_GetItemSanc(&withSlot) == REF_12);
}

/// @brief Testa leitura/atualização de quantidade (EF_AMOUNT) e limpeza de STRUCT_ITEM/STRUCT_MOB.
void TestItemAmountAndClear()
{
	STRUCT_ITEM item{};
	item.sIndex = 412;
	CHECK(BASE_GetItemAmount(&item) == 0);

	BASE_SetItemAmount(&item, 15);
	CHECK(item.stEffect[0].cEffect == EF_AMOUNT);
	CHECK(item.stEffect[0].cValue == 15);
	CHECK(BASE_GetItemAmount(&item) == 15);

	// Nova atualização deve reutilizar o slot EF_AMOUNT existente sem ocupar o slot seguinte.
	BASE_SetItemAmount(&item, 42);
	CHECK(item.stEffect[0].cValue == 42);
	CHECK(item.stEffect[1].cEffect == 0);
	CHECK(BASE_GetItemAmount(&item) == 42);

	BASE_ClearItem(&item);
	CHECK(item.sIndex == 0);
	CHECK(item.stEffect[0].cEffect == 0);
	CHECK(item.stEffect[0].cValue == 0);

	STRUCT_MOB mob{};
	mob.CurrentScore.Level = 123;
	mob.Exp = 999999;
	BASE_ClearMob(&mob);
	CHECK(mob.CurrentScore.Level == 0);
	CHECK(mob.Exp == 0);
}

/// @brief Testa cálculo de código visual ancestral (BASE_VisualAnctCode).
void TestVisualCodes()
{
	STRUCT_ITEM item{};
	item.sIndex = 1200;
	CHECK(BASE_VisualAnctCode(&item) == 0);

	item.stEffect[0].cEffect = EF_SANC;
	item.stEffect[0].cValue = 9;
	CHECK(BASE_VisualAnctCode(&item) == 43);

	// Refinação celestial: o código visual depende de (valor % 4): 0->0x30, 1->0x40, 2->0x10, 3->0x20.
	item.stEffect[0].cValue = 230;
	CHECK(BASE_VisualAnctCode(&item) == 0x10);
	item.stEffect[0].cValue = 231;
	CHECK(BASE_VisualAnctCode(&item) == 0x20);
	item.stEffect[0].cValue = 232;
	CHECK(BASE_VisualAnctCode(&item) == 0x30);
	item.stEffect[0].cValue = 233;
	CHECK(BASE_VisualAnctCode(&item) == 0x40);

	// Tintura retorna (cEffect - 3).
	item = {};
	item.sIndex = 1200;
	item.stEffect[1].cEffect = 116;
	item.stEffect[1].cValue = 5;
	CHECK(BASE_VisualAnctCode(&item) == 113);
}

/// @brief Testa distância no grid, velocidade de movimento (AttackRun) e fórmula de dano base.
void TestCombatHelpers()
{
	CHECK(BASE_GetDistance(10, 10, 10, 10) == 0);
	CHECK(BASE_GetDistance(100, 200, 103, 204) == BASE_GetDistance(103, 204, 100, 200));
	CHECK(BASE_GetDistance(0, 0, 10, 3) == 11);

	STRUCT_SCORE score{};
	score.AttackRun = 0;
	CHECK(BASE_GetSpeed(&score) == 1);
	score.AttackRun = 3;
	CHECK(BASE_GetSpeed(&score) == 3);
	score.AttackRun = 15;
	CHECK(BASE_GetSpeed(&score) == 7); // regra do cliente: até 7 (o servidor limitava em 6)
	score.AttackRun = static_cast<unsigned char>(0xF4);
	CHECK(BASE_GetSpeed(&score) == 4);

	std::srand(12345);
	CHECK(BASE_GetDamage(10, 1000, 0) >= 1);
	CHECK(BASE_GetDamage(500, 100, 10) > BASE_GetDamage(100, 100, 0));
}

/// @brief Testa a conferência de espaço na grade do inventário e do baú.
/// @details Os chamadores percorrem os slots com DestX = i % 9 e DestY = i / 9 e gravam o item no
/// slot i quando a função devolve TRUE. Até a correção, a conferência usava a altura da grade
/// (CARRYGRIDY/CARGOGRIDY) como largura da linha e olhava outra célula: com o slot 9 ocupado,
/// olhava a célula 7 (inventário) ou 14 (baú), devolvia TRUE e o item do slot 9 era sobrescrito.
void TestCarryCargoGrid()
{
	STRUCT_ITEM item{};
	item.sIndex = 1; // g_pItemList zerado neste teste: EF_GRID 0 = item de uma célula

	STRUCT_ITEM cargo[MAX_CARGO]{};
	cargo[9].sIndex = 1; // linha 1, coluna 0
	CHECK(BASE_CanCargo(&item, cargo, 0, 1) == FALSE); // slot 9 ocupado
	CHECK(BASE_CanCargo(&item, cargo, 5, 1) == TRUE);  // slot 14 livre
	CHECK(BASE_CanCargo(&item, cargo, 0, 0) == TRUE);  // slot 0 livre
	CHECK(BASE_CanCargo(&item, cargo, 8, 13) == TRUE); // último slot (125), dentro da grade 9x14
	CHECK(BASE_CanCargo(&item, cargo, 0, 14) == FALSE); // fora da grade

	STRUCT_ITEM carry[MAX_CARRY]{};
	carry[9].sIndex = 1;
	int error = 0;
	CHECK(BASE_CanCarry(&item, carry, 0, 1, &error) == FALSE);
	CHECK(error == 10); // devolve slot + 1
	error = 0;
	CHECK(BASE_CanCarry(&item, carry, 7, 0, &error) == TRUE); // slot 7 livre
	CHECK(BASE_CanCarry(&item, carry, 9, 0, &error) == FALSE && error == -1); // fora da grade
}

/// @brief Testa entradas fora do domínio do jogo que antes liam além do fim de uma tabela.
/// @details Ver a seção "Corrigidos" de docs/BUGS_LATENTES.md. Os valores conferidos são os
/// definidos pela correção; no código antigo o resultado dependia do que havia depois da tabela.
void TestOutOfDomainInputs()
{
	// sIndex == MAX_ITEMLIST: a guarda era `idx > MAX_ITEMLIST` e lia g_pItemList[MAX_ITEMLIST].
	STRUCT_ITEM last{};
	last.sIndex = MAX_ITEMLIST;
	last.stEffect[0].cEffect = EF_DAMAGE;
	last.stEffect[0].cValue = 50;
	CHECK(BASE_GetItemAbility(&last, EF_DAMAGE) == 0);
	CHECK(BASE_GetItemAbilityNosanc(&last, EF_DAMAGE) == 0);
	CHECK(BASE_GetStaticItemAbility(&last, EF_DAMAGE) == 0);
	CHECK(BASE_GetBonusItemAbility(&last, EF_DAMAGE) == 0);
	CHECK(BASE_GetBonusItemAbilityNosanc(&last, EF_DAMAGE) == 0);

	// Montaria com nível >= 120: g_pSancRate[2] tem 12 colunas (níveis 0..119).
	STRUCT_ITEM mount{};
	mount.sIndex = 2365;
	mount.stEffect[1].cEffect = 115;
	CHECK(BASE_GetGrowthRate(&mount) == 5); // última coluna da tabela
	mount.stEffect[1].cEffect = 120;
	CHECK(BASE_GetGrowthRate(&mount) == 0);
	mount.stEffect[1].cEffect = 255;
	CHECK(BASE_GetGrowthRate(&mount) == 0);

	// EF_GRID fora de 0..7 (g_pItemGrid tem 8 formatos): vale como item de uma célula.
	STRUCT_ITEMLIST saved = g_pItemList[2];
	g_pItemList[2].stEffect[0].sEffect = EF_GRID;
	g_pItemList[2].stEffect[0].sValue = 200;
	STRUCT_ITEM bad{};
	bad.sIndex = 2;
	STRUCT_ITEM one{};
	one.sIndex = 1;

	STRUCT_ITEM cargo[MAX_CARGO]{};
	cargo[9] = bad;                                    // linha 1, coluna 0
	CHECK(BASE_CanCargo(&one, cargo, 0, 1) == FALSE);  // ocupa só a própria célula...
	CHECK(BASE_CanCargo(&one, cargo, 0, 2) == TRUE);   // ...e não a de baixo
	CHECK(BASE_CanCargo(&bad, cargo, 0, 0) == TRUE);
	CHECK(BASE_CanCargo(&bad, cargo, 0, 1) == FALSE);

	STRUCT_ITEM carry[MAX_CARRY]{};
	carry[9] = bad;
	int error = 0;
	CHECK(BASE_CanCarry(&one, carry, 0, 1, &error) == FALSE && error == 10);
	CHECK(BASE_CanCarry(&one, carry, 0, 2, &error) == TRUE);
	CHECK(BASE_CanCarry(&bad, carry, 1, 1, &error) == TRUE);
	g_pItemList[2] = saved;

	// Item 2390 no slot de montaria: g_pMountBonus tem 30 linhas (2360..2389), então 2390
	// não é montaria e não muda a velocidade (antes lia g_pMountBonus[30]).
	auto attackRun = [](short mountIndex) {
		STRUCT_MOB mob{};
		mob.Equip[14].sIndex = mountIndex;
		mob.Equip[14].stEffect[0].sValue = 100;
		STRUCT_AFFECT affect[MAX_AFFECT]{};
		STRUCT_MOBEXTRA extra{};
		int expBonus = 0, forceMob = 0, accuracy = 0, absHp = 0, forceDamage = 0;
		BASE_GetCurrentScore(mob, affect, &extra, &expBonus, &forceMob, 0, &accuracy, &absHp, &forceDamage);
		return (int)mob.CurrentScore.AttackRun;
	};
	CHECK(attackRun(2390) == attackRun(0));
}

/// @brief Testa utilitários de string, validação de nomes e checksum legado.
void TestStringAndChecksum()
{
	char text[64] = "Armia Field 1";
	BASE_SpaceToUnderBar(text);
	CHECK(std::strcmp(text, "Armia_Field_1") == 0);
	BASE_UnderBarToSpace(text);
	CHECK(std::strcmp(text, "Armia Field 1") == 0);

	char shortName[NAME_LENGTH] = "abc";
	// Regra do cliente: não há nomes reservados (o servidor recusava nomes de comandos como "king").
	char commandKing[NAME_LENGTH] = "king";
	char commandReino[NAME_LENGTH] = "Reino";
	char commandRelo[NAME_LENGTH] = "relo";
	char brokenWide[NAME_LENGTH] = "abc\xb0"; // primeiro byte de um caractere de dois bytes, sem o segundo
	char wide[NAME_LENGTH] = "abc\xb0\xa1";
	char invalidChar[NAME_LENGTH] = "Hero!1";
	char validHyphen[NAME_LENGTH] = "Guerreiro-1";
	char validAlphaNum[NAME_LENGTH] = "Test1234";

	CHECK(!BASE_CheckValidString(shortName));
	CHECK(BASE_CheckValidString(commandKing));
	CHECK(BASE_CheckValidString(commandReino));
	CHECK(BASE_CheckValidString(commandRelo));
	CHECK(!BASE_CheckValidString(brokenWide));
	CHECK(BASE_CheckValidString(wide));
	CHECK(!BASE_CheckValidString(invalidChar));
	CHECK(BASE_CheckValidString(validHyphen));
	CHECK(BASE_CheckValidString(validAlphaNum));

	char buf1[16] = "WYD_CHECK_SUM_1";
	char buf2[16] = "WYD_CHECK_SUM_2";
	CHECK(BASE_GetSum(buf1, 15) == BASE_GetSum(buf1, 15));
	CHECK(BASE_GetSum(buf1, 15) != BASE_GetSum(buf2, 15));
}

} // namespace

int main()
{
	TestProtocolSizes();
	TestItemSanc();
	TestItemAmountAndClear();
	TestVisualCodes();
	TestCombatHelpers();
	TestCarryCargoGrid();
	TestOutOfDomainInputs();
	TestStringAndChecksum();

	if (g_failures != 0)
	{
		std::fprintf(stderr, "w2pp_common_tests: %d falha(s).\n", g_failures);
		return 1;
	}

	std::printf("w2pp_common_tests: todos os testes passaram com sucesso.\n");
	return 0;
}
