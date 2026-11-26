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

/// @file QuestInternal.h
/// @brief Declarações internas dos sub-handlers de NPCs de quest e serviços do TMSrv.

#include "net/ProcessClientMessage.h"

/// @brief Trata NPCs de progressão, treino inicial, quests clássicas e composições básicas
///        (MOUNT_MASTER, ZAKUM, AMU_MISTICO, Coveiro..Elfos, JEFFI, SHAMA, TREINADORNEWBIE1..4, etc.).
/// @return true se o npcMode foi tratado; false caso contrário.
bool HandleQuest_Progression(int conn, int npcIndex, int confirm, int npcMode, int npcMerc, int npcGrade);

/// @brief Trata NPCs de reino, guerra, contribuição e dragões
///        (KING, KINGDOM, URNAMMU, UXMAL, ARZAN_DRAGON, GOLD_DRAGON, EXPLOIT_LEADER, GODGOVERNMENT).
/// @return true se o npcMode foi tratado; false caso contrário.
bool HandleQuest_Kingdom(int conn, int npcIndex, int confirm, int npcMode, int npcMerc, int npcGrade);

/// @brief Trata NPCs especiais, reset/habilidades, eventos e trocas avançadas
///        (MESTREHAB, COMP_SEPHI, BLACKORACLE, CARBUNCLE_WIND, PERZEN, KIBITA, SOBREVIVENTE, GUARDA_REAL_EVT1).
/// @return true se o npcMode foi tratado; false caso contrário.
bool HandleQuest_Special(int conn, int npcIndex, int confirm, int npcMode, int npcMerc, int npcGrade);
