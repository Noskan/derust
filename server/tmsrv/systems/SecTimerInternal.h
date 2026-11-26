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
*   Contact at:
*/

/// @file SecTimerInternal.h
/// @brief Sub-rotinas de ProcessSecTimer() extraídas para `systems/` na Fase 2 (Onda 3).
///
/// Uso interno do TMSrv: chamadas apenas a partir de `core/ProcessSecMinTimer.cpp`.

#ifndef __SECTIMER_INTERNAL_H__
#define __SECTIMER_INTERNAL_H__

/// Portais das salas da Pista de Runas +5 (Balrog) para o líder `bb` (ScheduledEventSystem.cpp).
void ProcessRuneQuestPortals(int bb);

/// Eventos agendados por horário: Kefra, Pesa e Pista de Runas (ScheduledEventSystem.cpp).
void ProcessScheduledWorldEvents();

/// Movimento/fala de 1/6 dos NPCs a partir de `initial` (MobAISystem.cpp).
void ProcessNpcMovementTick(int initial);

/// Batalha de 1/4 dos mobs em combate; `Sec16` seleciona quem recebe ProcessAffect() (MobAISystem.cpp).
void ProcessMobCombatTick(int Sec4, int Sec16);

#endif
