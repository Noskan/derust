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

/// @file BasedefInternal.h
/// @brief Declarações internas compartilhadas entre Basedef.cpp, BaseItem.cpp e BaseCombat.cpp em w2pp_common.

#ifndef _BASEDEF_INTERNAL_H_
#define _BASEDEF_INTERNAL_H_

extern int CurrentClientGuild;
extern int g_pSuccessRate[11];
extern char g_pItemGrid[8][4][2];
extern int g_pDistanceTable[7][7];
// Nomes do cliente para códigos que o servidor já tem com outro nome, usados nas regras portadas do
// cliente. Ficam fora de ItemEffect.h porque o TMSrv lê aquele arquivo em tempo de execução e recusa
// dois nomes com o mesmo valor.
#define EF_REFLEVEL				EF_ITEMLEVEL	// 87
#define EF_TRANS				EF_MOBTYPE		// 112
#define EF_FAME					EF_ITEMTYPE		// 113: fama no cliente

extern int g_pMountBonus[30][6];
extern int g_pMountBonus2[20][6];
extern unsigned char g_pGuildSub[8];
extern unsigned char g_pGuildIndex[36];
extern unsigned char g_pGuildMark[132];
extern char g_pAffectTable[32][24];
extern unsigned char KorFirst[36];
extern int KorIndex[19];
extern char g_pClientGuildName[16][MAX_GUILD][4][GUILDNAME_LENGTH];
extern char g_pEnglish[MAX_ENGLISH][3][NAME_LENGTH];
extern int g_pHitRate[1024];

#endif
