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
#pragma once

/// @file UseItemInternal.h
/// @brief Declarações internas dos sub-handlers de uso de itens/voláteis do TMSrv.
/// @details Cada grupo processa uma faixa coesa de valores de EF_VOLATILE / sIndex
///          a partir de Exec_MSG_UseItem(), mantendo a ordem original de avaliação.

#include "net/ProcessClientMessage.h"

/// @brief Processa poções de HP/MP, Poeira de Oriharucon/Lactolerium (PO/PL), Pílula Orc,
///        Poeira de Fada, Olho Crescente, Adamantita/Beril (LE) e Poção Kappa.
/// @return true se o item foi tratado por alguma regra deste grupo; false caso contrário.
bool UseItem_PotionsAndRefine(int conn, MSG_UseItem* m, STRUCT_ITEM* item, int Vol, int amount);

/// @brief Processa retornos, Gema Estelar, portais, DBRecord, rações/âmagos de montaria,
///        quadro de guilda, fogos, pergaminhos da água (N/M/A), livros Sephira, contratos,
///        Divina, Vigor, Baú de XP e Frango Assado.
/// @return true se o item foi tratado por alguma regra deste grupo; false caso contrário.
bool UseItem_MountsAndScrolls(int conn, MSG_UseItem* m, STRUCT_ITEM* item, int Vol, int amount);

/// @brief Processa itens de quests diárias, Molar do Gárgula, Pesadelos (N/M/A), Carta de Duelo,
///        Pedidos de Caça, Pedra Ideal, Pedra Misteriosa, Escritura, Selo do Guerreiro,
///        cristais elementais, Pedra da Fúria e elixires/remédios da coragem.
/// @return true se o item foi tratado por alguma regra deste grupo; false caso contrário.
bool UseItem_QuestsAndDungeons(int conn, MSG_UseItem* m, STRUCT_ITEM* item, int Vol, int amount);

/// @brief Processa feijões mágicos, Removedor, Pergaminho do Perdão, gemas (diamante/esmeralda/
///        coral/garnet), Entrada do Território, Barra de Prata, Jóias PvP, armazenagem e Barras de Mythril.
/// @return true se o item foi tratado por alguma regra deste grupo; false caso contrário.
bool UseItem_GemsAndMithril(int conn, MSG_UseItem* m, STRUCT_ITEM* item, int Vol, int amount);

/// @brief Processa troca de classe, catalizadores, restauradores, extrações de equipamentos
///        (elmo, peito, calça, luva, bota, arma), Selo das Almas, Bolsa do Andarilho e War Pack.
/// @return true se o item foi tratado por alguma regra deste grupo; false caso contrário.
bool UseItem_ExtractionsAndSpecials(int conn, MSG_UseItem* m, STRUCT_ITEM* item, int Vol, int amount);
