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

/// @file MobKilledInternal.h
/// @brief Declarações internas dos subsistemas extraídos de MobKilled().

#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "common/Basedef.h"
#include "common/CPSock.h"
#include "common/ItemEffect.h"
#include "core/Language.h"

#include "entities/CItem.h"
#include "core/Server.h"
#include "net/GetFunc.h"
#include "net/SendFunc.h"
#include "net/ProcessClientMessage.h"
#include "net/ProcessDBMessage.h"
#include "systems/CCastleZakum.h"
#include "systems/CWarTower.h"

/// @brief Calcula e distribui a experiência PvE ao jogador e ao grupo (incluindo bônus de Pesadelo e ranking).
/// @return true se a classe (ClassMaster/UNK_3) entrou no fluxo principal de recompensa e drops; false para abate sem drops.
bool ProcessMobKilledExp(int target, int conn, int Leader);

/// @brief Processa mortes de chefes/estruturas (Kefra, Noatum), drops especiais/comuns, eventos e CWarTower.
void ProcessMobKilledDrops(int target, int conn, int PosX, int PosY, int GenerateIndex);

/// @brief Processa penalidades/recompensas de abate PvP (perda de EXP, CP, drops de PK e fada 10x/20x).
void ProcessMobKilledPvP(int target, int conn, MSG_CNFMobKill& sm);
