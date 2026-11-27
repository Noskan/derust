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

/// @file ScheduledEventSystem.cpp
/// @brief Eventos de mundo disparados pelo relógio do servidor dentro de ProcessSecTimer():
///        respawn do Kefra, ciclos da Pesa (Pesadelo) e da Pista de Runas, e os portais
///        das salas do Balrog na Pista +5.
///
/// Extraído de `core/ProcessSecMinTimer.cpp` na Fase 2 (Onda 3) sem alteração de comportamento.

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
#include "systems/CWarTower.h"
#include "systems/SecTimerInternal.h"

/// Teleporta o grupo do líder `bb` quando ele pisa em um portal das salas da Pista de Runas +5 (Balrog).
///
/// As coordenadas do jogador são alinhadas à grade de 4x4 (`& 0xFFFC`) e comparadas com
/// `PistaBalrogPortalPos`; o destino é sorteado entre as salas de `PistaBalrogPos`
/// (`W2Rand() % 3`) e todo o grupo (`Pista[5].Party[x]`) é levado junto via DoTeleport().
/// Chamado uma vez por segundo para cada usuário (`bb` em 1..MAX_USER-1).
void ProcessRuneQuestPortals(int bb)
{
#pragma region Pista de Runas Balrog Portais
			int xv = (pMob[bb].TargetX) & 0xFFFC;
			int yv = (pMob[bb].TargetY) & 0xFFFC;

			for(int x = 0; x < 3; x++)
			{
				if(pUser[bb].Mode != USER_PLAY || pMob[bb].MOB.CurrentScore.Hp <= 0 || Pista[5].Party[x].LeaderID == 0 || Pista[5].Party[x].LeaderID != bb || strcmp(Pista[5].Party[x].LeaderName, pMob[bb].MOB.MobName))
					continue;

				if (pMob[Pista[5].Party[x].LeaderID].Leader != -1 && pMob[Pista[5].Party[x].LeaderID].Leader)
					continue;

				int _rd = W2Rand()%3;

				int tx = 2100;
				int ty = 2100;

				//Sala 1 Portal 1
				if(xv == PistaBalrogPortalPos[0][0][0] && yv == PistaBalrogPortalPos[0][0][1])
				{
					if(_rd <= 1)
					{
						tx = PistaBalrogPos[1][0];
						ty = PistaBalrogPos[1][1];
					}

					else
					{
						tx = PistaBalrogPos[0][0];
						ty = PistaBalrogPos[0][1];
					}
				}

				//Sala 2 Portal 1
				else if(xv == PistaBalrogPortalPos[1][0][0] && yv == PistaBalrogPortalPos[1][0][1])
				{
					if(_rd <= 1)
					{
						tx = PistaBalrogPos[1][0];
						ty = PistaBalrogPos[1][1];
					}
					else if(_rd == 2)
					{
						tx = PistaBalrogPos[2][0];
						ty = PistaBalrogPos[2][1];
					}

					else
					{
						tx = PistaBalrogPos[0][0];
						ty = PistaBalrogPos[0][1];
					}
				}

				//Sala 2 Portal 2
				else if(xv == PistaBalrogPortalPos[1][1][0] && yv == PistaBalrogPortalPos[1][1][1])
				{
					if(_rd >= 2)
					{
						tx = PistaBalrogPos[1][0];
						ty = PistaBalrogPos[1][1];
					}
					else if(_rd == 0)
					{
						tx = PistaBalrogPos[2][0];
						ty = PistaBalrogPos[2][1];
					}

					else
					{
						tx = PistaBalrogPos[0][0];
						ty = PistaBalrogPos[0][1];
					}
				}

				//Sala 3 Portal 1
				else if(xv == PistaBalrogPortalPos[2][0][0] && yv == PistaBalrogPortalPos[2][0][1])
				{
					if(_rd <= 1)
					{
						tx = PistaBalrogPos[2][0];
						ty = PistaBalrogPos[2][1];
					}
					else if(_rd == 2)
					{
						tx = PistaBalrogPos[3][0];
						ty = PistaBalrogPos[3][1];
					}

					else
					{
						tx = PistaBalrogPos[1][0];
						ty = PistaBalrogPos[1][1];
					}
				}

				//Sala 3 Portal 2
				else if(xv == PistaBalrogPortalPos[2][1][0] && yv == PistaBalrogPortalPos[2][1][1])
				{
					if(_rd >= 2)
					{
						tx = PistaBalrogPos[2][0];
						ty = PistaBalrogPos[2][1];
					}
					else if(_rd == 0)
					{
						tx = PistaBalrogPos[3][0];
						ty = PistaBalrogPos[3][1];
					}

					else
					{
						tx = PistaBalrogPos[1][0];
						ty = PistaBalrogPos[1][1];
					}
				}

				//Sala 3 Portal 3
				else if(xv == PistaBalrogPortalPos[2][2][0] && yv == PistaBalrogPortalPos[2][2][1])
				{
					if(_rd <= 1)
					{
						tx = PistaBalrogPos[2][0];
						ty = PistaBalrogPos[2][1];
					}
					else if(_rd == 2)
					{
						tx = PistaBalrogPos[3][0];
						ty = PistaBalrogPos[3][1];
					}

					else
					{
						tx = PistaBalrogPos[1][0];
						ty = PistaBalrogPos[1][1];
					}
				}

				//Sala 4 Portal 1
				else if(xv == PistaBalrogPortalPos[3][0][0] && yv == PistaBalrogPortalPos[3][0][1])
				{
					if(_rd <= 1)
					{
						tx = PistaBalrogPos[3][0];
						ty = PistaBalrogPos[3][1];
					}
					else if(_rd == 2)
					{
						tx = PistaBalrogPos[4][0];
						ty = PistaBalrogPos[4][1];

						if(Pista[5].Party[0].MobCount == 0)
							GenerateMob(RUNEQUEST_LV5_MOB_BOSS, 0, 0);
					}

					else
					{
						tx = PistaBalrogPos[2][0];
						ty = PistaBalrogPos[2][1];
					}
				}

				//Sala 4 Portal 2
				else if(xv == PistaBalrogPortalPos[3][1][0] && yv == PistaBalrogPortalPos[3][1][1])
				{
					if(_rd >= 2)
					{
						tx = PistaBalrogPos[3][0];
						ty = PistaBalrogPos[3][1];
					}
					else if(_rd == 0)
					{
						tx = PistaBalrogPos[4][0];
						ty = PistaBalrogPos[4][1];

						if(Pista[5].Party[0].MobCount == 0)
							GenerateMob(RUNEQUEST_LV5_MOB_BOSS, 0, 0);
					}

					else
					{
						tx = PistaBalrogPos[2][0];
						ty = PistaBalrogPos[2][1];
					}
				}

				//Sala 4 Portal 3
				else if(xv == PistaBalrogPortalPos[3][2][0] && yv == PistaBalrogPortalPos[3][2][1])
				{
					if(_rd <= 1)
					{
						tx = PistaBalrogPos[3][0];
						ty = PistaBalrogPos[3][1];
					}
					else if(_rd == 2)
					{
						tx = PistaBalrogPos[4][0];
						ty = PistaBalrogPos[4][1];

						if(Pista[5].Party[0].MobCount == 0)
							GenerateMob(RUNEQUEST_LV5_MOB_BOSS, 0, 0);
					}

					else
					{
						tx = PistaBalrogPos[2][0];
						ty = PistaBalrogPos[2][1];
					}
				}

				//Sala 4 Portal 4
				else if(xv == PistaBalrogPortalPos[3][3][0] && yv == PistaBalrogPortalPos[3][3][1])
				{
					if(_rd <= 1)
					{
						tx = PistaBalrogPos[3][0];
						ty = PistaBalrogPos[3][1];
					}
					else if(_rd == 2)
					{
						tx = PistaBalrogPos[4][0];
						ty = PistaBalrogPos[4][1];

						if(Pista[5].Party[0].MobCount == 0)
							GenerateMob(RUNEQUEST_LV5_MOB_BOSS, 0, 0);
					}

					else
					{
						tx = PistaBalrogPos[2][0];
						ty = PistaBalrogPos[2][1];
					}
				}
				else
					continue;

				int inv = 0;
				for (inv = 0; inv < MAX_CARRY - 4; inv = BASE_NextCarry(pMob[bb].MOB.Carry, inv))
				{
					if(pMob[bb].MOB.Carry[inv].sIndex == 4032)
					{
						BASE_ClearItem(&pMob[bb].MOB.Carry[inv]);
						SendItem(bb, ITEM_PLACE_CARRY, inv, &pMob[bb].MOB.Carry[inv]);
						break;
					}
				}

				if(inv == MAX_CARRY - 4)
					continue;

				DoTeleport(Pista[5].Party[x].LeaderID, tx, ty);

				for (int c = 0;c < MAX_PARTY; c++)
				{
					int partyconn = pMob[Pista[5].Party[x].LeaderID].PartyList[c];

					if (partyconn > 0 && partyconn < MAX_USER && partyconn != Pista[5].Party[x].LeaderID && pUser[partyconn].Mode == USER_PLAY)
							DoTeleport(partyconn, tx, ty);		
				}
			}

#pragma endregion
}

/// Processa os eventos agendados por horário local (`localtime`). Chamado a cada 2 segundos.
///
/// - Kefra: renasce às terças-feiras, 12:00, se `KefraLive` estiver ativo.
/// - Pesa (Pesadelo): limpeza das áreas nos minutos 09/29/49, 04/24/44 e 19/39/59; início das
///   salas (se houver jogadores na área) nos minutos 14/34/54, 09/29/49 e 04/24/44
///   (regiões "Pesa Reset" e "Pesa Start").
/// - Pista de Runas: entrada nos minutos 00/20/40 e saída/premiação nos minutos 15/35/55.
///
/// Como a checagem usa `tm_sec == 0` e roda em segundos pares, a janela de disparo é de 1 tick.
void ProcessScheduledWorldEvents()
{
	struct tm when;
	time_t now;
	time(&now);
	when = *localtime(&now);

	if ((when.tm_wday == 2 && when.tm_hour == 12 && when.tm_min == 0) && when.tm_sec >= 0 && when.tm_sec <= 2 && KefraLive)
	{
		KefraLive = 0;

		//Kefra
		GenerateMob(KEFRA_BOSS, 0, 0);

		for(int xx = KEFRA_MOB_INITIAL; xx <= KEFRA_MOB_END; xx++)
			GenerateMob(xx, 0, 0);
	}
#pragma region Pesa Reset
	if ((when.tm_min == 9 || when.tm_min == 29 || when.tm_min == 49) && when.tm_sec == 0)
	{
		DeleteMobMapa(9, 1);
		ClearMapa(9, 1);//Pesa A
		PartyPesa[2] = 0;

		sprintf(temp, "etc,nightmare_arcan clear -%d:%d", when.tm_hour, when.tm_min);
		Log(temp, "-system", 0);
	}
	if ((when.tm_min == 4 || when.tm_min == 24 || when.tm_min == 44) && when.tm_sec == 0)
	{
		DeleteMobMapa(8, 2);
		ClearMapa(8, 2);//Pesa M
		PartyPesa[1] = 0;

		sprintf(temp, "etc,nightmare_mystic clear -%d:%d", when.tm_hour, when.tm_min);
		Log(temp, "-system", 0);
	}

	if ((when.tm_min == 19 || when.tm_min == 39 || when.tm_min == 59) && when.tm_sec == 0)
	{
		DeleteMobMapa(10, 2);
		ClearMapa(10, 2);//Pesa N
		PartyPesa[0] = 0;

		sprintf(temp, "etc,nightmare_normal clear -%d:%d", when.tm_hour, when.tm_min);
		Log(temp, "-system", 0);
	}
#pragma endregion
#pragma region Pista de runas entrada
	if ((when.tm_min == 20 || when.tm_min == 40 || when.tm_min == 00) && when.tm_sec == 0)
	{//Pista de Runas entrada

		Pista[4].Party[0].MobCount = 1;
		Pista[4].Party[1].MobCount = 1;
		Pista[4].Party[2].MobCount = 1;

		Pista[6].Party[0].MobCount = 100;

		for(int s = 0; s < 7; s++)
		{
			for(int t = 0; t < 3; t++)
			{
				if(Pista[s].Party[t].LeaderID == 0 || strcmp(pMob[Pista[s].Party[t].LeaderID].MOB.MobName, Pista[s].Party[t].LeaderName) != 0)
					continue;

				if((pMob[Pista[s].Party[t].LeaderID].TargetX/128) != 25 || (pMob[Pista[s].Party[t].LeaderID].TargetY/128) != 13)
					continue;

				if (pMob[Pista[s].Party[t].LeaderID].Leader != -1 && pMob[Pista[s].Party[t].LeaderID].Leader)
					continue;

				DoTeleport(Pista[s].Party[t].LeaderID, PistaPos[Pista[s].Party[t].Sala][t][0], PistaPos[Pista[s].Party[t].Sala][t][1]);

				for (int i = 0; i < MAX_PARTY; i++)
				{
					int partyconn = pMob[Pista[s].Party[t].LeaderID].PartyList[i];

					if (partyconn > 0 && partyconn < MAX_USER && partyconn != Pista[s].Party[t].LeaderID && pUser[partyconn].Mode == USER_PLAY)
						if((pMob[partyconn].TargetX/128) == 25 && (pMob[partyconn].TargetY/128) == 13)
							DoTeleport(partyconn, PistaPos[Pista[s].Party[t].Sala][t][0], PistaPos[Pista[s].Party[t].Sala][t][1]);		
				}

				//Pista+0 Lich
				if(s == 0 && t == 0)
					GenerateMob(RUNEQUEST_LV0_LICH2, 0, 0);

				if(s == 0 && t == 1)
					GenerateMob(RUNEQUEST_LV0_LICH1, 0, 0);

				//Pista +1 Torre
				if(s == 1)
				{
					GenerateMob(RUNEQUEST_LV1_TORRE1, 3358, 1582);
					GenerateMob(RUNEQUEST_LV1_TORRE2, 3386, 1548);
					GenerateMob(RUNEQUEST_LV1_TORRE3, 3418, 1582);

					for(int generateindex = RUNEQUEST_LV1_MOB_INITIAL; generateindex <= RUNEQUEST_LV1_MOB_END; generateindex++)
						GenerateMob(generateindex, 0, 0);
				}
			

				if(s == 2)
					GenerateMob(RUNEQUEST_LV2_MOB_BOSS, 0, 0);

				if(s == 4)
					Pista[s].Party[t].MobCount = 8 + W2Rand()%8;

				if(s == 4 && t == 0)
				{
					for(int generateindex = RUNEQUEST_LV4_MOB_INITIAL; generateindex <= RUNEQUEST_LV4_MOB_END; generateindex++)
						GenerateMob(generateindex, 0, 0);
				}
			}
		}
	}
#pragma endregion
#pragma region Pista de runas saída
	if ((when.tm_min == 15 || when.tm_min == 35 || when.tm_min == 55) && when.tm_sec == 0)
	{//Pista de Runas saida

		for (int c = MAX_USER; c < MAX_MOB; c++)
		{
			if (pMob[c].Mode == USER_EMPTY)
				continue;

			if (pMob[c].TargetX < 3310 || pMob[c].TargetX > 3588 || pMob[c].TargetY < 1005 || pMob[c].TargetY > 1663)
				continue;

			DeleteMob(c, 3);
		}

		for (int x = 0; x < MAX_USER; x++)
		{
			if (pUser[x].Mode != USER_PLAY)
				continue;

			if (pMob[x].Mode == USER_EMPTY)
				continue;

			if (pMob[x].TargetX < 3310 || pMob[x].TargetX > 3588 || pMob[x].TargetY < 1005 || pMob[x].TargetY > 1663)
				continue;

			if (pMob[x].MOB.CurrentScore.Hp <= 0)
			{
				pMob[x].MOB.CurrentScore.Hp = 1;

				SendScore(x);
			}

			DoTeleport(x, 3294, W2Rand()%1 == 0 ? 1701 : 1686);
		}

		//Pista +1 - Torre

		//Grupo 1 ganhou
		if(Pista[1].Party[0].MobCount > Pista[1].Party[1].MobCount && Pista[1].Party[0].MobCount > Pista[1].Party[2].MobCount)
		{
			if(Pista[1].Party[0].LeaderID != 0 && strcmp(pMob[Pista[1].Party[0].LeaderID].MOB.MobName, Pista[1].Party[0].LeaderName) == 0)
			{
				STRUCT_ITEM Runa;
				memset(&Runa, 0, sizeof(STRUCT_ITEM));

				Runa.sIndex = PistaRune[1][W2Rand()%5];

				if (Pista[1].Party[0].LeaderID > 0 && Pista[1].Party[0].LeaderID < MAX_USER)
					PutItem(Pista[1].Party[0].LeaderID, &Runa);
				
				for (int i = 0; i < MAX_PARTY; i++)
				{
					int partymember = pMob[Pista[1].Party[0].LeaderID].PartyList[i];

					if (pUser[partymember].Mode != USER_PLAY)
						continue;

					Runa.sIndex = PistaRune[1][W2Rand()%5];
					PutItem(partymember, &Runa);
				}

				STRUCT_ITEM Prize;
				memset(&Prize, 0, sizeof(STRUCT_ITEM));

				Prize.sIndex = 5134;
				Prize.stEffect[0].cEffect = 43;
				Prize.stEffect[0].cValue = 2;

				PutItem(Pista[1].Party[0].LeaderID, &Prize);

				sprintf(temp, "etc,questRune complete +1 party1");
				Log(temp, "-system", 0);
			}
		}

		//Grupo 2 ganhou
		else if(Pista[1].Party[1].MobCount > Pista[1].Party[0].MobCount && Pista[1].Party[1].MobCount > Pista[1].Party[2].MobCount)
		{
			if(Pista[1].Party[1].LeaderID != 0 && strcmp(pMob[Pista[1].Party[1].LeaderID].MOB.MobName, Pista[1].Party[1].LeaderName) == 0)
			{
				STRUCT_ITEM Runa;
				memset(&Runa, 0, sizeof(STRUCT_ITEM));

				Runa.sIndex = PistaRune[1][W2Rand()%5];

				if (Pista[1].Party[1].LeaderID > 0 && Pista[1].Party[1].LeaderID < MAX_USER)
					PutItem(Pista[1].Party[1].LeaderID, &Runa);
				
				for (int i = 0; i < MAX_PARTY; i++)
				{
					int partymember = pMob[Pista[1].Party[1].LeaderID].PartyList[i];

					if (pUser[partymember].Mode != USER_PLAY)
						continue;

					Runa.sIndex = PistaRune[1][W2Rand()%5];
					PutItem(partymember, &Runa);
				}

				STRUCT_ITEM Prize;
				memset(&Prize, 0, sizeof(STRUCT_ITEM));

				Prize.sIndex = 5134;
				Prize.stEffect[0].cEffect = 43;
				Prize.stEffect[0].cValue = 2;

				PutItem(Pista[1].Party[1].LeaderID, &Prize);

				sprintf(temp, "etc,questRune complete +1 party2");
				Log(temp, "-system", 0);
			}
		}

		//Grupo 3 ganhou
		else if(Pista[1].Party[2].MobCount > Pista[1].Party[0].MobCount && Pista[1].Party[2].MobCount > Pista[1].Party[1].MobCount)
		{
			if(Pista[1].Party[2].LeaderID != 0 && strcmp(pMob[Pista[1].Party[2].LeaderID].MOB.MobName, Pista[1].Party[2].LeaderName) == 0)
			{
				STRUCT_ITEM Runa;
				memset(&Runa, 0, sizeof(STRUCT_ITEM));

				Runa.sIndex = PistaRune[1][W2Rand()%5];

				if (Pista[1].Party[2].LeaderID > 0 && Pista[1].Party[2].LeaderID < MAX_USER)
					PutItem(Pista[1].Party[0].LeaderID, &Runa);
				
				for (int i = 0; i < MAX_PARTY; i++)
				{
					int partymember = pMob[Pista[1].Party[2].LeaderID].PartyList[i];

					if (pUser[partymember].Mode != USER_PLAY)
						continue;

					Runa.sIndex = PistaRune[1][W2Rand()%5];
					PutItem(partymember, &Runa);
				}

				STRUCT_ITEM Prize;
				memset(&Prize, 0, sizeof(STRUCT_ITEM));

				Prize.sIndex = 5134;
				Prize.stEffect[0].cEffect = 43;
				Prize.stEffect[0].cValue = 2;

				PutItem(Pista[1].Party[2].LeaderID, &Prize);

				sprintf(temp, "etc,questRune complete +1 party3");
				Log(temp, "-system", 0);
			}
		}

		//Pista +3 - Sulrang

		//Grupo 1 ganhou
		if(Pista[3].Party[0].MobCount > Pista[3].Party[1].MobCount && Pista[3].Party[0].MobCount > Pista[3].Party[2].MobCount)
		{
			if(Pista[3].Party[0].LeaderID != 0 && strcmp(pMob[Pista[3].Party[0].LeaderID].MOB.MobName, Pista[3].Party[0].LeaderName) == 0)
			{
				STRUCT_ITEM Runa;
				memset(&Runa, 0, sizeof(STRUCT_ITEM));

				Runa.sIndex = PistaRune[3][W2Rand()%5];

				if (Pista[3].Party[0].LeaderID > 0 && Pista[3].Party[0].LeaderID < MAX_USER)
					PutItem(Pista[3].Party[0].LeaderID, &Runa);
				
				for (int i = 0; i < MAX_PARTY; i++)
				{
					int partymember = pMob[Pista[3].Party[0].LeaderID].PartyList[i];

					if (pUser[partymember].Mode != USER_PLAY)
						continue;

					Runa.sIndex = PistaRune[3][W2Rand()%5];
					PutItem(partymember, &Runa);
				}

				STRUCT_ITEM Prize;
				memset(&Prize, 0, sizeof(STRUCT_ITEM));

				Prize.sIndex = 5134;
				Prize.stEffect[0].cEffect = 43;
				Prize.stEffect[0].cValue = 4;

				PutItem(Pista[3].Party[0].LeaderID, &Prize);

				sprintf(temp, "etc,questRune complete +3 party1");
				Log(temp, "-system", 0);
			}
		}

		//Grupo 2 ganhou
		else if(Pista[3].Party[1].MobCount > Pista[3].Party[0].MobCount && Pista[3].Party[1].MobCount > Pista[3].Party[2].MobCount)
		{
			if(Pista[3].Party[1].LeaderID != 0 && strcmp(pMob[Pista[3].Party[1].LeaderID].MOB.MobName, Pista[3].Party[1].LeaderName) == 0)
			{
				STRUCT_ITEM Runa;
				memset(&Runa, 0, sizeof(STRUCT_ITEM));

				Runa.sIndex = PistaRune[3][W2Rand()%5];

				if (Pista[3].Party[1].LeaderID > 0 && Pista[3].Party[1].LeaderID < MAX_USER)
					PutItem(Pista[3].Party[1].LeaderID, &Runa);
				
				for (int i = 0; i < MAX_PARTY; i++)
				{
					int partymember = pMob[Pista[3].Party[1].LeaderID].PartyList[i];

					if (pUser[partymember].Mode != USER_PLAY)
						continue;

					Runa.sIndex = PistaRune[3][W2Rand()%5];
					PutItem(partymember, &Runa);
				}

				STRUCT_ITEM Prize;
				memset(&Prize, 0, sizeof(STRUCT_ITEM));

				Prize.sIndex = 5134;
				Prize.stEffect[0].cEffect = 43;
				Prize.stEffect[0].cValue = 4;

				// Party[1] é o grupo vencedor deste bloco. Estava Party[3], além do fim de Party[3]
				// (caía em Pista[4].Party[0], o líder de outra pista).
				PutItem(Pista[3].Party[1].LeaderID, &Prize);

				sprintf(temp, "etc,questRune complete +3 party2");
				Log(temp, "-system", 0);
			}
		}

		//Grupo 3 ganhou
		else if(Pista[3].Party[2].MobCount > Pista[3].Party[0].MobCount && Pista[3].Party[2].MobCount > Pista[3].Party[1].MobCount)
		{
			if(Pista[3].Party[2].LeaderID != 0 && strcmp(pMob[Pista[3].Party[2].LeaderID].MOB.MobName, Pista[3].Party[2].LeaderName) == 0)
			{
				STRUCT_ITEM Runa;
				memset(&Runa, 0, sizeof(STRUCT_ITEM));

				Runa.sIndex = PistaRune[3][W2Rand()%5];

				if (Pista[3].Party[2].LeaderID > 0 && Pista[3].Party[2].LeaderID < MAX_USER)
					PutItem(Pista[3].Party[2].LeaderID, &Runa);
				
				for (int i = 0; i < MAX_PARTY; i++)
				{
					int partymember = pMob[Pista[3].Party[2].LeaderID].PartyList[i];

					if (pUser[partymember].Mode != USER_PLAY)
						continue;

					Runa.sIndex = PistaRune[3][W2Rand()%5];
					PutItem(partymember, &Runa);
				}

				STRUCT_ITEM Prize;
				memset(&Prize, 0, sizeof(STRUCT_ITEM));

				Prize.sIndex = 5134;
				Prize.stEffect[0].cEffect = 43;
				Prize.stEffect[0].cValue = 4;

				PutItem(Pista[3].Party[2].LeaderID, &Prize);

				sprintf(temp, "etc,questRune complete +3 party3");
				Log(temp, "-system", 0);
			}
		}

		for(int x = 0;x < 7; x++)
		{
			for(int z = 0; z < 3;z ++)
			{
				Pista[x].Party[z].LeaderID = 0;
				strncpy(Pista[x].Party[z].LeaderName, " ", 16);
				Pista[x].Party[z].MobCount = 0;
				Pista[x].Party[z].Sala = 0;
			}
		}

	}
#pragma endregion

#pragma region Pesa Start
	if (GetUserInArea(1152, 128, 1280, 256, temp) && (when.tm_min == 14 && when.tm_sec == 0 || when.tm_min == 34 && when.tm_sec == 0 || when.tm_min == 54 && when.tm_sec == 0))
	{
		MSG_STANDARDPARM sm;
		memset(&sm, 0, sizeof(MSG_STANDARDPARM));

		sm.Type = _MSG_StartTime;
		sm.ID = ESCENE_FIELD;
		sm.Size = sizeof(MSG_STANDARDPARM);
		sm.Parm = 900;

		MapaMulticast(9, 1, (MSG_STANDARD*)&sm, 0);
		
		//Pesa A
		for (int i = NIGHTMARE_A_INITIAL; i <= NIGHTMARE_A_END; i++)
			GenerateMob(i, 0, 0);

		sprintf(temp, "etc,nigthmare_arcan started");
		Log(temp, "-system", 0);
	}

	if (GetUserInArea(1024, 256, 1152, 384, temp) > 0 && (when.tm_min == 9 && when.tm_sec == 0 || when.tm_min == 29 && when.tm_sec == 0 || when.tm_min == 49 && when.tm_sec == 0))
	{
		MSG_STANDARDPARM sm;
		memset(&sm, 0, sizeof(MSG_STANDARDPARM));

		sm.Type = _MSG_StartTime;
		sm.ID = ESCENE_FIELD;
		sm.Size = sizeof(MSG_STANDARDPARM);
		sm.Parm = 900;

		MapaMulticast(8, 2, (MSG_STANDARD*)&sm, 0);//Pesa M

		for (int i = NIGHTMARE_M_INITIAL; i <= NIGHTMARE_M_END; i++)
			GenerateMob(i, 0, 0);

		sprintf(temp, "etc,nigthmare_mistycal started");
		Log(temp, "-system", 0);
	}

	if (GetUserInArea(1280, 256, 1408, 384, temp) > 0 && (when.tm_min == 24 && when.tm_sec == 0 || when.tm_min == 44 && when.tm_sec == 0 || when.tm_min == 4 && when.tm_sec == 0))
	{
		MSG_STANDARDPARM sm;
		memset(&sm, 0, sizeof(MSG_STANDARDPARM));

		sm.Type = _MSG_StartTime;
		sm.ID = ESCENE_FIELD;
		sm.Size = sizeof(MSG_STANDARDPARM);
		sm.Parm = 900;

		MapaMulticast(10, 2, (MSG_STANDARD*)&sm, 0);

		//Pesa N
		for (int i = NIGHTMARE_N_INITIAL; i <= NIGHTMARE_N_END; i++)
			GenerateMob(i, 0, 0);

		sprintf(temp, "etc,nigthmare_normal started");
		Log(temp, "-system", 0);
	}
#pragma endregion
}
