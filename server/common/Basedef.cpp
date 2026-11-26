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


#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>
#include <stdio.h> 
#include <fcntl.h>
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/timeb.h>
#include <string.h>        
#include <time.h>
#include <mbstring.h>

#include "common/Basedef.h"
#include "common/ItemEffect.h"
#include "common/BasedefInternal.h"

#pragma region Defines

int BaseSIDCHM[4][6] = // Define the base Strength, Intelligence, Dexterity, Constance, HP and MP of the 4 classes
{
	{8, 4, 7, 6, 80, 45}, // TK
	{5, 8, 5, 5, 60, 65}, //FM
	{6, 6, 9, 5, 70, 55}, // BM
	{8, 9, 13, 6, 75, 60} // HT
}; 

STRUCT_GUILDZONE g_pGuildZone[MAX_GUILDZONE] = 
{
	{0, 0, 2088, 2148, 2086, 2093, 2052, 2052, 2171, 2163, 197, 213, 238, 230, 205, 220, 228, 220, 5, 0}, // Armia
	{0, 0, 2531, 1700, 2494, 1707, 2432, 1672, 2675, 1767, 197, 149, 238, 166, 205, 157, 228, 157, 5, 0}, // Azran
	{0, 0, 2460, 1976, 2453, 2000, 2448, 1966, 2476, 2024, 141, 213, 182, 230, 146, 220, 173, 220, 5, 0}, // Erion
	{0, 0, 3614, 3124, 3652, 3122, 3605, 3090, 3690, 3260, 141, 149, 182, 166, 146, 157, 173, 157, 5, 0}, // Nippleheim
	{0, 0, 1066, 1760, 1050, 1706, 1036, 1700, 1072, 1760, 4000, 4000, 4010, 4010, 4005, 4005, 4005, 4005, 5, 0} // Noatum
};

int g_pIncrementHp[4] = {3, 1, 1, 2}; // TK, FM, BM, HT. The ammount of HP that they get every level up
int g_pIncrementMp[4] = {1, 3, 2, 1}; // TK, FM, BM, HT. The ammount of MP that they get every level up

int CurrentClientGuild = -1;

int g_pSancRate[3][12] = 
{
	100, 100, 100, 85, 70, 40, 10, 00, 00, 00, 00, 00, // PO
	100, 100, 100, 100, 100, 80, 70, 70, 50, 60, 00, 00, // PL
	100, 80, 60, 40, 20, 10, 10, 10, 5, 5, 5, 5  // Amagos
};

int g_pSancGrade[2][5] =
{
	1, 1, 1, 1, 1,
	1, 1, 1, 1, 1
};


int g_pEhreRate[10] =
{
	0, 100, 100, 40, 10, 10, 100, 100, 100, 100
};

int g_pOdinRate[12] =
{
	0, 35, 100, 40, 10, 10, 100, 100, 100, 100, 100, 100
};

int g_pCelestialRate[15] =
{
	100, 100, 100, 100, 100, 100, 100, 100, 70, 50, 40, 40, 40, 10, 5 
};

int g_pTinyBase = 15;
int g_pShanyBase = 35;
int g_pAilynBase = 10;
int g_pAgathaBase = 15;

int g_pAnctChance[3] = { 2, 4, 10 };

int g_pItemSancRate12[11] = 
{
	1, 2, 3, 4, 5, 6, 7, 8, 9, 12, 15
};

int g_pItemSancRate12Minus[4] = { 2, 3, 4, 5 };

int g_pSuccessRate[11] = {5, 5, 5, 5, 4, 4, 3, 3, 2, 1, 0};

char g_pItemGrid[8][4][2] = 
{
// region ItemGrid
	{
		{01, 00},
		{00, 00},
		{00, 00},
		{00, 00}
	},
	{  
		{01, 00},
		{01, 00},
		{00, 00},
		{00, 00}

	},
	{
		{01, 00},
		{01, 00},
		{01, 00},
		{00, 00}
	},
	{   
		{01, 00},
		{01, 00},
		{01, 00},
		{01, 00}
	},
	{   
		{01, 01},
		{00, 00},
		{00, 00},
		{00, 00}
	},
	{  
		{01, 01},
		{01, 01},
		{00, 00},
		{00, 00}
	},
	{  
		{01, 01},
		{01, 01},
		{01, 01},
		{00, 00}
	},
	{  
		{01, 01},
		{01, 01},
		{01, 01},
		{01, 01}
	}

// endregion
};

int	g_pDistanceTable[7][7] =
{	
	0, 1, 2, 3, 4, 5, 6,
	1, 1, 2, 3, 4, 5, 6,
	2, 2, 3, 4, 4, 5, 6,
	3, 3, 4, 4, 5, 5, 6,
	4, 4, 4, 5, 5, 5, 6,
	5, 5, 5, 5, 5, 6, 6,
	6, 6, 6, 6, 6, 6, 6,
};

 int g_pGroundMask[MAX_GROUNDMASK][4][6][6];

 char g_pFormation[5][12][2] =
 {
	 1, 1, -1, 1, 1, -1, -1, -1,
	 1, 0, -1, 0, 0, 1, 0, -1, 2, 0, -2, 0,
	 0, 2, 0, -2, 1, 0, -1, 0, 2, 0, -2, 0,
	 3, 0, -3, 0, 4, 0, -4, 0, 5, 0, -5, 0,
	 0, 6, 0, -6, 1, 1, -1, 1, 1, -1, -1, -1,
	 1, 0, -1, 0, 0, 1, 0, -1, 2, 0, -2, 0,
	 0, 2, 0, -2, 1, 0, -1, 0, 2, 0, -2, 0,
	 3, 0, -3, 0, 4, 0, -4, 0, 5, 0, -5, 0,
	 0, 6, 0, -6, 2, 0, 0, 2, 1, 1, 0, 1,
	 1, 0, -1, 3, 3, -1, -1, 2, 2, -1, -1, 1,
	 1, -1, 1, 2
 };
 int g_pClanTable[9][9] =
 {
	{  1,  0,  1,  1,  1,  0,  1,  1,  1, },
	{  0,  1,  1,  0,  1,  1,  1,  0,  0, },
	{  1,  1,  1,  0,  1,  1,  1,  1,  1, },
	{  1,  0,  0,  1,  1,  0,  1,  1,  1, },
	{  1,  1,  1,  1,  1,  0,  1,  1,  1, },
	{  0,  1,  1,  0,  0,  1,  0,  0,  0, },
	{  1,  1,  1,  1,  1,  0,  1,  1,  1, },
	{  1,  0,  1,  1,  1,  0,  1,  1,  0, },
	{  1,  0,  1,  1,  1,  0,  1,  0,  1, }


 };

 int g_pDropBonus[64] = 
 {
	 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 
	 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 
	 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 
	 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100
 };

 int g_pDropRate[64] =
 {
	 900, 900, 900, 900, 900, 900, 900, 900, 4, 4, 4,  4, 900, 900, 900, 900, 20000, 20000, 20000, 20000,
	 20000, 20000, 20000, 20000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 
	 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 3000, 3000, 3000, 3000, 3000,
	 3000, 3000, 3000, 1, 35, 500, 2500, 5000, 5000, 10000, 20000
 };

// Regra do cliente (client/src/common/Basedef.h): ataque, magia, evasão, resistência, velocidade e
// a sexta coluna do cliente, que o servidor não usa.
int g_pMountBonus[30][6] =
{
  { 10, 1, 0, 0, 4, 75 },
  { 10, 1, 0, 0, 4, 75 },
  { 50, 10, 0, 0, 5, 75 },
  { 80, 15, 0, 0, 5, 75 },
  { 100, 20, 0, 0, 4, 75 },
  { 150, 25, 0, 0, 5, 75 },
  { 250, 50, 40, 0, 6, 75 },
  { 300, 60, 50, 0, 6, 75 },
  { 350, 65, 60, 0, 6, 75 },
  { 400, 70, 70, 0, 6, 75 },
  { 500, 85, 80, 0, 6, 73 },
  { 250, 50, 0, 16, 6, 75 },
  { 300, 60, 0, 20, 6, 75 },
  { 350, 65, 0, 24, 6, 75 },
  { 400, 70, 0, 28, 6, 75 },
  { 500, 85, 0, 32, 6, 73 },
  { 550, 90, 0, 0, 6, 73 },
  { 600, 90, 0, 0, 6, 66 },
  { 550, 90, 0, 20, 6, 73 },
  { 650, 100, 60, 28, 6, 65 },
  { 700, 110, 80, 32, 6, 65 },
  { 570, 90, 20, 16, 6, 71 },
  { 570, 90, 30, 8, 6, 71 },
  { 570, 90, 40, 12, 6, 69 },
  { 590, 95, 30, 20, 6, 69 },
  { 600, 95, 40, 16, 6, 65 },
  { 600, 95, 50, 16, 6, 65 },
  { 600, 40, 60, 28, 6, 65 },
  { 300, 95, 60, 28, 6, 65 },
  { 150, 25, 0, 20, 5, 75 }
};

// Montarias temporárias 3980..3999 (antes g_pMountTempBonus, com 15 linhas e outros valores).
int g_pMountBonus2[20][6] =
{
  { 35, 7, 0, 0, 6, 75 },
  { 350, 55, 10, 28, 6, 74 },
  { 450, 55, 0, 0, 6, 74 },
  { 35, 7, 0, 0, 6, 75 },
  { 450, 72, 10, 28, 6, 74 },
  { 450, 72, 0, 0, 6, 74 },
  { 120, 45, 0, 0, 6, 75 },
  { 450, 72, 10, 28, 6, 74 },
  { 450, 72, 0, 0, 6, 74 },
  { 325, 35, 16, 28, 6, 65 },
  { 350, 45, 10, 4, 6, 65 },
  { 250, 25, 0, 31, 6, 65 },
  { 80, 15, 0, 31, 6, 75 },
  { 950, 145, 60, 20, 6, 75 },
  { 950, 145, 60, 20, 6, 75 },
  { 300, 60, 50, 0, 6, 75 },
  { 350, 65, 60, 0, 6, 75 },
  { 400, 70, 70, 0, 6, 75 },
  { 500, 85, 80, 0, 6, 73 },
  { 0, 0, 0, 0, 0, 0 }
};
unsigned char g_pGuildSub[8] = {0xA5, 0xB0, 0xA5, 0xB1, 0xA5, 0xB2, 0x00, 0x00};

//char g_pGuildIndex[54] = {"????????????????"};
unsigned char g_pGuildIndex[36] = 
{
	0xA5, 0xB0, 0xA5, 0xB1, 0xA5, 0xB2, 0xA5, 0xB3, 0xA5, 0xB4, 0xA5, 0xB5, 0xA5, 0xB6, 0xA5, 0xB7, 
	0xA5, 0xB8, 0xA5, 0xB9, 0xA3, 0xC1, 0xA3, 0xC2, 0xA3, 0xC3, 0xA3, 0xC4, 0xA3, 0xC5, 0xA3, 0xC6, 
	0x00, 0x00, 0x00, 0x00
};
//char g_pGuildMark[168] = {"??????????????×?????¿???????†‡????????®?????????????????????????"};
unsigned char g_pGuildMark[132] = 
{
	0xA5, 0xB0, 0xA5, 0xB1, 0xA5, 0xB2, 0xA5, 0xB3, 0xA5, 0xB4, 0xA5, 0xB5, 0xA5, 0xB6, 0xA5, 0xB7, 
	0xA5, 0xB8, 0xA5, 0xB9, 0xA1, 0xE2, 0xA1, 0xE0, 0xA1, 0xDD, 0xA1, 0xD9, 0xA1, 0xBF, 0xA2, 0xB2, 
	0xA1, 0xDE, 0xA6, 0xB6, 0xA1, 0xCC, 0xA5, 0xC8, 0xA2, 0xAF, 0xA1, 0xDB, 0xA2, 0xB7, 0xA2, 0xBF, 
	0xA2, 0xC1, 0xA2, 0xC2, 0xA2, 0xC3, 0xA2, 0xC4, 0xA2, 0xD3, 0xA2, 0xD4, 0xA1, 0xC4, 0xA1, 0xD4, 
	0xA1, 0xF3, 0xA2, 0xB3, 0xA8, 0xCD, 0xA3, 0xC1, 0xA5, 0xC6, 0xA5, 0xCE, 0xA2, 0xE7, 0xA2, 0xB9, 
	0xA5, 0xE1, 0xA5, 0xE2, 0xA5, 0xE3, 0xA5, 0xE4, 0xA5, 0xE5, 0xA5, 0xE6, 0xA5, 0xE7, 0xA5, 0xE8, 
	0xA5, 0xE9, 0xA5, 0xEA, 0xA5, 0xEB, 0xA5, 0xEC, 0xA5, 0xED, 0xA5, 0xEE, 0xA5, 0xEF, 0xA5, 0xF0, 
	0xA5, 0xF1, 0xA5, 0xF2, 0xA5, 0xF3, 0xA5, 0xF4, 0xA5, 0xF5, 0xA5, 0xF6, 0xA5, 0xD7, 0xA5, 0xF8, 
	0x00, 0x00, 0x00, 0x00
};

char g_pAffectTable[32][24] = 
{
	"",
	"Slow",
	"Haste",
	"Drag Reduction",
	"Power-up",
	"Reduced avoidance",
	"Increase in avoidance",
	"Attack speed reduction",
	"SPEED",
	"Demijieop",
	"Reduction in attacks",
	"Magic Shield",
	"Reduction in defense",
	"Assault",
	"Gross",
	"Increase in skill",
	"Polymorph",
	"Tikrayipeu",
	"Magic Stamina",
	"Immunity",
	"Poison",
	"Melee attack",
	"Lightning",
	"Elemental",
	"Self-destruct",
	"Protectors",
	"Avoid falling",
	"Frost",
	"Hyde",
	"Drain",
	"Vision",
	"Coin Armor"
};

int g_pBonusValue5[30][4] = 
{
	{2, 30, 74, 18},
	{2, 30, 74, 15},
	{2, 30, 74, 12},

	{2, 24, 74, 18},
	{2, 24, 74, 15},
	{2, 24, 74, 12},

	{2, 18, 74, 18},
	{2, 18, 74, 15},
	{2, 18, 74, 12},

	{2, 12, 74, 18},
	{2, 12, 74, 15},
	{2, 12, 74, 12},

	{2, 6, 74, 18},
	{2, 6, 74, 15},
	{2, 6, 74, 12},

	{2, 30, 60, 10},
	{2, 30, 60, 8},
	{2, 30, 60, 6},

	{2, 24, 60, 10},
	{2, 24, 60, 8},
	{2, 24, 60, 6},

	{2, 18, 60, 10},
	{2, 18, 60, 8},
	{2, 18, 60, 6},

	{2, 12, 60, 10},
	{2, 12, 60, 8},
	{2, 12, 60, 6},

	{2, 6, 60, 10},
	{2, 6, 60, 8},
	{2, 6, 60, 6}
};

int g_pBonusValue4[30][4] = 
{
	{2, 30, 72, 30},
	{2, 30, 72, 25},
	{2, 30, 72, 20},
	{2, 30, 72, 15},
	{2, 30, 72, 10},

	{2, 24, 72, 30},
	{2, 24, 72, 25},
	{2, 24, 72, 20},
	{2, 24, 72, 15},
	{2, 24, 72, 10},

	{2, 18, 72, 30},
	{2, 18, 72, 25},
	{2, 18, 72, 20},
	{2, 18, 72, 15},
	{2, 18, 72, 10},

	{60, 10, 72, 30},
	{60, 10, 72, 25},
	{60, 10, 72, 20},
	{60, 10, 72, 15},

	{60, 8, 72, 30},
	{60, 8, 72, 25},
	{60, 8, 72, 20},
	{60, 8, 72, 15},

	{60, 6, 72, 30},
	{60, 6, 72, 25},
	{60, 6, 72, 20},
	{60, 6, 72, 15}
};

int g_pBonusValue3[25][4] = 
{
	{4, 60, 26, 18},
	{4, 60, 26, 15},
	{4, 60, 26, 12},

	{4, 50, 26, 18},
	{4, 50, 26, 15},
	{4, 50, 26, 12},

	{4, 40, 26, 18},
	{4, 40, 26, 15},
	{4, 40, 26, 12},

	{4, 30, 26, 18},
	{4, 30, 26, 15},
	{4, 30, 26, 12},

	{4, 60, 60, 12},
	{4, 60, 60, 10},
	{4, 60, 60, 8},
	{4, 60, 60, 6},

	{4, 50, 60, 12},
	{4, 50, 60, 10},
	{4, 50, 60, 8},
	{4, 50, 60, 6},

	{4, 40, 60, 12},
	{4, 40, 60, 10},
	{4, 40, 60, 8},
	{4, 40, 60, 6},
	{4, 40, 60, 4}
};

int g_pBonusValue2[48][4] = 
{
	{2, 30, 3, 30},
	{2, 30, 3, 25},
	{2, 30, 3, 20},
	{2, 30, 3, 10},

	{2, 24, 3, 30},
	{2, 24, 3, 25},
	{2, 24, 3, 20},
	{2, 24, 3, 15},

	{2, 18, 3, 30},
	{2, 18, 3, 25},
	{2, 18, 3, 20},
	{2, 18, 3, 15},

	{2, 30, 71, 50},
	{2, 30, 71, 60},
	{2, 30, 71, 70},

	{2, 24, 71, 50},
	{2, 24, 71, 60},
	{2, 24, 71, 70},

	{2, 18, 71, 50},
	{2, 18, 71, 60},
	{2, 18, 71, 70},

	{60, 10, 3, 30},
	{60, 10, 3, 25},
	{60, 10, 3, 20},
	{60, 10, 3, 15},
	{60, 10, 3, 10},

	{60, 8, 3, 30},
	{60, 8, 3, 25},
	{60, 8, 3, 20},
	{60, 8, 3, 15},
	{60, 8, 3, 10},

	{60, 6, 3, 30},
	{60, 6, 3, 25},
	{60, 6, 3, 20},
	{60, 6, 3, 15},
	{60, 6, 3, 10},

	{60, 10, 71, 50},
	{60, 10, 71, 60},
	{60, 10, 71, 70},

	{60, 8, 71, 50},
	{60, 8, 71, 60},
	{60, 8, 71, 70},

	{60, 6, 71, 50},
	{60, 6, 71, 60},
	{60, 6, 71, 70},

	{60, 4, 71, 50},
	{60, 4, 71, 60},
	{60, 4, 71, 70}
};


int g_pBonusValue[10][2][2] =
{
	{
		{10 , 30},
		{2 , 5}
	},

	{
		{3 , 10},
		{2 , 8}
	},

	{
		{2 , 8},
		{2 , 8}
	},

	{
		{2 , 6},
		{2 , 6}
	},

	{
		{2 , 6},
		{2 , 6}
	},

	{
		{20 , 45},
		{4 , 8}
	},

	{
		{7 , 20},
		{5 , 14}
	},

	{
		{5 , 14},
		{5 , 14}
	},

	{
		{4 , 12},
		{4 , 12}
	},

	{
		{4 , 12},
		{4 , 12}
	}
};

int g_pBonusType[10] = {10, 30, 2, 5, 3, 10, 2, 8, 2, 8 };

int DungeonPos[30][2] = 
{
// region DungeonPos
	{244, 3822}, 
	{380, 3810},
	{352, 3780},
	{349, 3748},
	{456, 3736},
	{562, 3802},
	{592, 3770}, 
	{612, 3795},
	{748, 3732}, 
	{740, 3776}, 
	{424, 3892}, 
	{478, 3928},
	{432, 3944}, 
	{402, 3876}, 
	{492, 4076}, 
	{448, 4078},
	{996, 3988}, 
	{922, 4062}, 
	{802, 4000}, 
	{774, 4048},
	{710, 3988}, 
	{660, 4076}, 
	{788, 3860}, 
	{876, 3892},
	{964, 3821}, 
	{956, 3732}, 
	{1032, 3820}, 
	{1104, 3737},
	{1124, 3804}, 
	{1132, 3756}
// endregion
};

int DungeonItem[10] = {	421, 422, 423, 419, 420, 685, 691, 696, 696, 696 };

long long g_pNextLevel[MAX_LEVEL+2] = 
{
		0, 500 ,1124 ,1826 ,2610 ,3480 ,4440 ,5494 ,6646 ,7900 ,
		9260 ,10893 ,12817 ,15050 ,17610 ,20515 ,23783 ,27432 ,31480 ,
		35945 ,40845 ,46251 ,52187 ,58677 ,65745 ,73415 ,81711 ,90657 ,
		100277 ,110595 ,121635 ,133647 ,146671 ,160747 ,175915 ,192215 ,209687 ,
		228371 ,248307 ,269535 ,292095 ,316151 ,341751 ,368943 ,397775 ,428295 ,
		460551 ,494591 ,530463 ,568215 ,607895 ,649715 ,693731 ,739999 ,788575 ,
		839515 ,892875 ,948711 ,1007079 ,1068035 ,1131635 ,1198670 ,1269230 ,1343405 ,
		1421285 ,1502960 ,1588520 ,1678055 ,1771655 ,1869410 ,1971410 ,2078255 ,2190055 ,
		2306920 ,2428960 ,2556285 ,2689005 ,2827230 ,2971070 ,3120635 ,3276035 ,3438521 ,
		3608249 ,3785375 ,3970055 ,4162445 ,4362701 ,4570979 ,4787435 ,5012225 ,5245505 ,
		5488163 ,5740379 ,6002333 ,6274205 ,6556175 ,6848423 ,7151129 ,7464473 ,7788635 ,
		8123795 ,8460174 ,8797774 ,9136597 ,9476645 ,9817920 ,10160424 ,10504159 ,10849127 ,
		11195330 ,11542770 ,11892311 ,12243959 ,12597720 ,12953600 ,13311605 ,13671741 ,14034014 ,
		14398430 ,14764995 ,15133715 ,15508850 ,15890450 ,16278565 ,16673245 ,17074540 ,17482500 ,
		17897175 ,18318615 ,18746870 ,19181990 ,19625811 ,20078403 ,20539836 ,21010180 ,21489505 ,
		21977881 ,22475378 ,22982066 ,23498015 ,24023295 ,24559110 ,25105558 ,25662737 ,26230745 ,
		26809680 ,27399640 ,28000723 ,28613027 ,29236650 ,29871690 ,30517485 ,31174125 ,31841700 ,
		32520300 ,33210015 ,33910935 ,34623150 ,35346750 ,36081825 ,36828465 ,37587867 ,38360139 ,
		39145389 ,39943725 ,40755255 ,41580087 ,42418329 ,43270089 ,44135475 ,45014595 ,45904870 ,
		46806370 ,47719165 ,48643325 ,49578920 ,50526020 ,51484695 ,52455015 ,53437050 ,54430870 ,
		55439542 ,56463162 ,57501826 ,58555630 ,59624670 ,60709042 ,61808842 ,62924166 ,64055110 ,
		65201770 ,66366010 ,67547930 ,68747630 ,69965210 ,71200770 ,72454410 ,73726230 ,75016330 ,
		76324810 ,77651770 ,78985354 ,80325578 ,81672458 ,83026010 ,84386250 ,85753194 ,87126858 ,
		88507258 ,89894410 ,91288330 ,92693002 ,94108458 ,95534730 ,96971850 ,98419850 ,99878762 ,
		101348618 ,102829450 ,104321290 ,105824170 ,107352234 ,108905674 ,110484682 ,112089450 ,113720170 ,
		115377034 ,117060234 ,118769962 ,120506410 ,122269770 ,124065890 ,125895058 ,127757562 ,129653690 ,
		131583730 ,133547970 ,135546698 ,137580202 ,139648770 ,141752690 ,143928178 ,146176386 ,148498466 ,
		150895570 ,153368850 ,155919458 ,158548546 ,161257266 ,164046770 ,166918210 ,169956978 ,173167682 ,
		176554930 ,180123330 ,205345890 ,209100050 ,212902550 ,216753470 ,220652890 ,224600890 ,228597550 ,
		232642950 ,236737170 ,240880290 ,245072390 ,249313550 ,253603850 ,257943370 ,262332190 ,266770390 ,
		271258050 ,275795250 ,280382070 ,285018590 ,289904810 ,295042730 ,300434350 ,306081670 ,311986690 ,
		318151410 ,324577830 ,331267950 ,338223770 ,345447290 ,354039310 ,364049830 ,375528850 ,388526370 ,
		403092390 ,419276910 ,437129930 ,456701450 ,476272970 ,495844490 ,515416010 ,534987530 ,554559050 ,
		574130570 ,593702090 ,613273610 ,632845130 ,652416650 ,671988170 ,691559690 ,711131210 ,730702730 ,
		750274250 ,769845770 ,789417290 ,808988810 ,828560330 ,848131850 ,867703370 ,887274890 ,906846410 ,
		926417930 ,945989450 ,965560970 ,985132490 ,1004704010 ,1024275530 ,1043847050 ,1063418570 ,1082990090 ,
		1102561610 ,1122133130 ,1141704650 ,1161276170 ,1180847690 ,1200419210 ,1222705731 ,1244995262 ,1267288477 ,
		1289622601 ,1311966887 ,1334333102 ,1356724650 ,1379151914 ,1401651370 ,1424151231 ,1448674779 ,1473220997 ,
		1497782544 ,1522364697 ,1546957043 ,1571581919 ,1596243411 ,1620925875 ,1645647464 ,1670373305 ,1710373305 ,
		1770373305 ,1870373305 ,2000000000 ,2039000000 ,2078000000 ,2117000000 ,2156000000 ,2195000000 ,2234000000 ,
		2273000000 ,2312000000 ,2351000000 ,2390000000 ,2429000000 ,2468000000 ,2507000000 ,2546000000 ,2585000000 ,
		2624000000 ,2663000000 ,2702000000 ,2741000000 ,2780000000 ,2819000000 ,2858000000 ,2897000000 ,2936000000 ,
		3000000000 ,3043000000 ,3086000000 ,3129000000 ,3172000000 ,3215000000 ,3258000000 ,3301000000 ,3344000000 ,
		3387000000 ,3430000000 ,3473000000 ,3516000000 ,3559000000 ,3602000000 ,3645000000 ,3688000000 ,3731000000 ,
		3774000000 ,3817000000 ,4000000000, 4100000000
};

long long g_pNextLevel_2[MAX_CLEVEL + 202] = {
	0, 20000000, 40000000, 60000000, 80000000, 100000000, 120000000, 140000000, 160000000, 180000000, 
	200000000, 220000000, 240000000, 260000000, 280000000, 300000000, 320000000, 340000000, 360000000, 
	380000000, 400000000, 420000000, 440000000, 460000000, 480000000, 500000000, 520000000, 540000000, 
	560000000, 580000000, 600000000, 620000000, 640000000, 660000000, 680000000, 700000000, 720000000, 
	740000000, 760000000, 780000000, 800000000, 820000000, 840000000, 860000000, 880000000, 900000000, 
	920000000, 940000000, 960000000, 980000000, 1000000000, 1020000000, 1040000000, 1060000000, 1080000000, 
	1100000000, 1120000000, 1140000000, 1160000000, 1180000000, 1200000000, 1220000000, 1240000000, 1260000000, 
	1280000000, 1300000000, 1320000000, 1340000000, 1360000000, 1380000000, 1400000000, 1420000000, 1440000000, 
	1460000000, 1480000000, 1500000000, 1520000000, 1540000000, 1560000000, 1580000000, 1600000000, 1620000000, 
	1640000000, 1660000000, 1680000000, 1700000000, 1720000000, 1740000000, 1760000000, 1780000000, 1800000000, 
	1820000000, 1840000000, 1860000000, 1880000000, 1900000000, 1920000000, 1940000000, 1960000000, 1980000000, 
	2000000000, 2020000000, 2040000000, 2060000000, 2080000000, 2100000000, 2120000000, 2140000000, 2160000000, 
	2180000000, 2200000000, 2220000000, 2240000000, 2260000000, 2280000000, 2300000000, 2320000000, 2340000000, 
	2360000000, 2380000000, 2400000000, 2420000000, 2440000000, 2460000000, 2480000000, 2500000000, 2520000000, 
	2540000000, 2560000000, 2580000000, 2600000000, 2620000000, 2640000000, 2660000000, 2680000000, 2700000000, 
	2720000000, 2740000000, 2760000000, 2780000000, 2800000000, 2820000000, 2840000000, 2860000000, 2880000000, 
	2900000000, 2920000000, 2940000000, 2960000000, 2980000000, 3000000000, 3020000000, 3040000000, 3060000000, 
	3080000000, 3100000000, 3120000000, 3140000000, 3160000000, 3180000000, 3200000000, 3220000000, 3240000000, 
	3260000000, 3280000000, 3300000000, 3320000000, 3340000000, 3360000000, 3380000000, 3400000000, 3420000000, 
	3440000000, 3460000000, 3480000000, 3500000000, 3520000000, 3540000000, 3560000000, 3580000000, 3600000000, 
	3620000000, 3640000000, 3660000000, 3680000000, 3700000000, 3720000000, 3740000000, 3760000000, 3780000000, 
	3800000000, 3820000000, 3840000000, 3860000000, 3880000000, 3900000000, 3920000000, 3940000000, 3960000000, 
	4000000000, 4020000000, 4040000000, 4060000000, 4080000000, 4100000000, 4120000000, 4140000000, 4160000000, 
	4180000000, 4200000000, 4220000000, 4240000000, 4260000000, 4280000000, 4300000000, 4320000000, 4340000000, 
	4360000000, 4380000000, 4400000000, 4420000000, 4440000000, 4460000000, 4480000000, 4500000000, 4520000000, 
	4540000000, 4560000000, 4580000000, 4600000000, 4620000000, 4640000000, 4660000000, 4680000000, 4700000000, 
	4720000000, 4740000000, 4760000000, 4780000000, 4800000000, 4820000000, 4840000000, 4860000000, 4880000000, 
	4900000000, 4920000000, 4940000000, 4960000000, 4980000000, 5000000000, 5020000000, 5040000000, 5060000000, 
	5080000000, 5100000000, 5120000000, 5140000000, 5160000000, 5180000000, 5200000000, 5220000000, 5240000000, 
	5260000000, 5280000000, 5300000000, 5320000000, 5340000000, 5360000000, 5380000000, 5400000000, 5420000000, 
	5440000000, 5460000000, 5480000000, 5500000000, 5520000000, 5540000000, 5560000000, 5580000000, 5600000000, 
	5620000000, 5640000000, 5660000000, 5680000000, 5700000000, 5720000000, 5740000000, 5760000000, 5780000000, 
	5800000000, 5820000000, 5840000000, 5860000000, 5880000000, 5900000000, 5920000000, 5940000000, 5960000000, 
	5980000000, 6000000000, 6020000000, 6040000000, 6060000000, 6080000000, 6100000000, 6120000000, 6140000000, 
	6160000000, 6180000000, 6200000000, 6220000000, 6240000000, 6260000000, 6280000000, 6300000000, 6320000000, 
	6340000000, 6360000000, 6380000000, 6400000000, 6420000000, 6440000000, 6460000000, 6480000000, 6500000000, 
	6520000000, 6540000000, 6560000000, 6580000000, 6600000000, 6620000000, 6640000000, 6660000000, 6680000000, 
	6700000000, 6720000000, 6740000000, 6760000000, 6780000000, 6800000000, 6820000000, 6840000000, 6860000000, 
	6880000000, 6900000000, 6920000000, 6940000000, 6960000000, 6980000000, 7000000000, 7020000000, 7040000000, 
	7060000000, 7080000000, 7100000000, 7120000000, 7140000000, 7160000000, 7180000000, 7200000000, 7220000000, 
	7240000000, 7260000000, 7280000000, 7300000000, 7320000000, 7340000000, 7360000000, 7380000000, 7400000000, 
	7420000000, 7440000000, 7460000000, 7480000000, 7500000000, 7520000000, 7540000000, 7560000000, 7580000000, 
	7600000000, 7620000000, 7640000000, 7660000000, 7680000000, 7700000000, 7720000000, 7740000000, 7760000000, 
	7780000000, 7800000000, 7820000000, 7840000000, 7860000000, 7880000000, 7900000000, 7920000000, 7940000000, 
	7960000000, 7980000000, 8000000000, 8200000000
};


STRUCT_BEASTBONUS pSummonBonus[MAX_SUMMONLIST] = 
{
	{ 80, 300, 50,  75, 100, 400, 0, 0, 0, 0, 0, 7, 0, 0, 100, 100,   5},
	{ 80, 250, 50, 150, 125, 400, 0, 0, 0, 0, 0, 7, 0, 0, 100, 100,  20},
	{ 80, 400, 50, 125, 125, 400, 0, 0, 0, 0, 0, 6, 0, 0, 100, 100,  50},
	{ 80, 350, 50, 200, 150, 400, 0, 0, 0, 0, 0, 6, 0, 0, 100, 100,  70},
	{ 80, 500, 50, 175, 150, 400, 0, 0, 0, 0, 0, 6, 0, 0, 100, 100,  90},
	{ 80, 450, 50, 250, 175, 400, 0, 0, 0, 0, 0, 6, 0, 0, 100, 100, 110},
	{100, 500, 50, 250, 174, 400, 0, 0, 0, 0, 0, 6, 0, 0, 100, 100, 140},
	{130, 250, 60, 200, 180, 250, 0, 0, 0, 0, 0, 6, 0, 0, 100, 100, 160},
	{0,}
};


STRUCT_BEASTBONUS pTransBonus[5] = 
{
	//MinDam, MaxDam, MinAc, MaxAc, MinHp, MaxHp, RunSpeed, UNK, AttackSpeed
	{110, 130,  95, 105,  95, 105, 1, 0, 20, 0, 100, 100, 100, 100, 100, 100,  15},
	{ 80, 100, 100, 110, 110, 140, 0, 0,  0, 0, 100, 100, 100, 100, 100, 100,  60},
	{100, 120, 105, 115, 100, 120, 1, 0, 20, 0, 100, 100, 100, 100, 100, 100, 115},
	{ 90, 110, 110, 125, 105, 110, 0, 0, 20, 0, 100, 100, 100, 100, 100, 100, 155 },
	{ 105, 120, 110, 120, 105, 115, 3, 0, 20, 0, 100, 100, 100, 100, 100, 100, 155 }
};

//char KorFirst[36] = {0,};//"¤¡¤¢¤¤¤§¤¨¤©¤±¤²¤³¤µ¤¶¤·¤¸¤¹¤º¤»¤¼¤½¤¾";
unsigned char KorFirst[36] = 
{
	0xA4, 0xA1, 0xA4, 0xA2, 0xA4, 0xA4, 0xA4, 0xA7, 0xA4, 0xA8, 0xA4, 0xA9, 0xA4, 0xB1, 0xA4, 0xB2, 
	0xA4, 0xB3, 0xA4, 0xB5, 0xA4, 0xB6, 0xA4, 0xB7, 0xA4, 0xB8, 0xA4, 0xB9, 0xA4, 0xBA, 0xA4, 0xBB, 
	0xA4, 0xBC, 0xA4, 0xBD
};
int KorIndex[19] = {171, 291, 432, 560, 646, 773, 902, 1031, 1104, 1237, 1353, 1561, 1696, 1770, 1890, 1997, 2103, 2208, 2350};

int  ChargedGuildList[MAX_SERVER][MAX_GUILDZONE] = {0,};
char g_pGuildName[10][16][MAX_GUILD][GUILDNAME_LENGTH];
char g_pClientGuildName[16][MAX_GUILD][4][GUILDNAME_LENGTH];
char EffectNameTable[MAX_EFFECTINDEX][24];
char g_pMessageStringTable[MAX_STRING][128];

STRUCT_SPELL g_pSpell[MAX_SKILLINDEX];
STRUCT_INITITEM g_pInitItem[MAX_INITITEM];

char g_pEnglish[MAX_ENGLISH][3][NAME_LENGTH];

int g_pHitRate[1024] = {0,};

unsigned char g_pAttribute[1024][1024];

STRUCT_ITEMLIST g_pItemList[MAX_ITEMLIST];

char g_pServerList[MAX_SERVERGROUP][MAX_SERVERNUMBER][64];

int g_dwInitItem = 0;

int g_HeightWidth = 4096;
int g_HeightHeight = 4096;
int g_HeightPosX = 0;
int g_HeightPosY = 0;

#pragma endregion

void BASE_GetLanguage(char *str, int idx)
{
  if(idx >= 0 && idx < MAX_STRING)
	  sprintf(str, g_pMessageStringTable[idx]);
  else
	  strcpy(str, "[UNHANDLED]");
}

void BASE_GetLanguage(char *str, int idx, char *parm1)
{
	if(idx >= 0 && idx < MAX_STRING)
		sprintf(str, g_pMessageStringTable[idx], parm1);
	else
		strcpy(str, "[UNHANDLED]");
}

void BASE_GetLanguage(char *str, int idx, char *parm1, char *parm2)
{
	if (idx >= 0 && idx < MAX_STRING)
		sprintf(str, g_pMessageStringTable[idx], parm1, parm2);
	else
		strcpy(str, "[UNHANDLED]");
}

void BASE_GetLanguage(char *str, int idx, int parm1)
{
	if (idx >= 0 && idx < MAX_STRING)
		sprintf(str, g_pMessageStringTable[idx], parm1);
	else
		strcpy(str, "[UNHANDLED]");
}

void BASE_GetLanguage(char *str, int idx, int parm1, int parm2)
{
	if(idx >= 0 && idx < MAX_STRING)
		sprintf(str, g_pMessageStringTable[idx], parm1, parm2);
	else
		strcpy(str, "[UNHANDLED]");
}

void BASE_GetLanguage(char *str, int idx, char * parm1, int parm2)
{
	if(idx >= 0 && idx < MAX_STRING)
		sprintf(str, g_pMessageStringTable[idx], parm1, parm2);
	else
		strcpy(str, "[UNHANDLED]");
}

void BASE_GetLanguage(char *str, int idx, int parm1, char *parm2)
{
	if(idx >= 0 && idx < MAX_STRING)
		sprintf(str, g_pMessageStringTable[idx], parm1, parm2);
	else
		strcpy(str, "[UNHANDLED]");
}

void BASE_GetGuildName(int ServerGroup, unsigned short usGuild, char *szGuildName)
{
	int Group = ServerGroup;
	int Server = (usGuild / MAX_GUILD); //& 15;
	int Guild = usGuild - (Server * MAX_GUILD);

	if(szGuildName == NULL)
		return;

	if((Group < 0 || Group >= MAX_SERVERGROUP) || (Server < 0 || Server >= 16))
	{
		szGuildName[0] = 0;

		return;
	}

	if(Guild >= 0 && Guild < MAX_GUILD &&  g_pGuildName[Group][Server][Guild][0] != 0)
	{
		memcpy(szGuildName, g_pGuildName[Group][Server][Guild], GUILDNAME_LENGTH);

		BASE_UnderBarToSpace(szGuildName);

		return;
	}
	else
	{
		char szGuildNum[3];
		memset(szGuildNum, 0, 3);

		int nGuildIndex = Server * 2;
		memcpy(szGuildNum, &g_pGuildIndex[nGuildIndex], 2);
		sprintf(szGuildName, "%s-", szGuildNum);

		nGuildIndex = 2 * ((usGuild / 64) & 63);
		memcpy(szGuildNum, &g_pGuildMark[nGuildIndex], 2);
		strcat(szGuildName, szGuildNum);

		nGuildIndex = 2 * (usGuild & 63);
		memcpy(szGuildNum, &g_pGuildMark[nGuildIndex], 2);
		strcat(szGuildName, szGuildNum);
	}
}

void BASE_GetClientGuildName(int ServerGroup, unsigned short usGuild, char *szGuildName, int Sub)
{
	char szGuildNum[4];
	memset(szGuildNum, 0, 4);

	int Group = ServerGroup;
	int Server = usGuild / MAX_GUILD;
	int Guild = usGuild & MAX_GUILD-1;

	if(CurrentClientGuild != Group)
		BASE_InitializeClientGuildName(Group);

	if(szGuildName == NULL)
		return;

	if(Group < 0 || Group >= MAX_SERVERGROUP || Server < 0 || Server >= 16)
	{
		szGuildName[0] = 0;

		return;
	}

	if(Guild >= 0 && Guild < MAX_GUILD &&  g_pGuildName[Server][Guild][0][0] != 0)
	{
		memcpy(szGuildName, g_pGuildName[Server][Guild], GUILDNAME_LENGTH);

		BASE_UnderBarToSpace(szGuildName);

		if(Sub >= 1 && Sub <= 3)
		{
			if(g_pClientGuildName[Server][Guild][Sub][0] != 0)
			{
				strcat(szGuildName, "-");
				strcat(szGuildName, g_pClientGuildName[Server][Guild][Sub]);
			}
			else
			{
				strcat(szGuildName, "-");
				memcpy(szGuildNum, (void*)(((int)&g_pGuildSub - 2) + 2 * Sub), 2);
				strcat(&szGuildName[0], szGuildNum);
			}
		}

		return;
	}

	int nGuildIndex = (usGuild / MAX_GUILD) * 2;

	if(nGuildIndex < 0 || nGuildIndex > 20)
	{
		sprintf(szGuildName, "");

		return;
	}

	memcpy(szGuildNum, &g_pGuildIndex[nGuildIndex], 2);
	sprintf(szGuildName, "%s-", szGuildNum);

	nGuildIndex = 2 * ((usGuild / 64) & 63);

	if(nGuildIndex < 0 || nGuildIndex > 128)
	{
		 sprintf(szGuildName, "");

		 return;
	}

	memcpy(&szGuildNum, &g_pGuildMark[nGuildIndex], 2);
	strcat(szGuildName, szGuildNum);

	nGuildIndex = 2 * (usGuild & 0x3F);

	if(nGuildIndex < 0 || nGuildIndex > 128)
	{
		sprintf(szGuildName, "");

		return;
	}

	memcpy(&szGuildNum, &g_pGuildMark[nGuildIndex], 2);
	strcat(szGuildName, szGuildNum);

	if(Sub >= 1 && Sub <= 3)
	{
		memcpy(&szGuildNum, (void*)(((int)&g_pGuildSub - 2) + 2 * Sub), 2);
		strcat(szGuildName, szGuildNum);
	}
}

#define ISALPHA(x) (((x) >= 'a' && (x) <= 'z') || ((x) >= 'A' && (x) <= 'Z'))
#define ISDIGIT(x) ((x) >= '0' && (x) <= '9')

bool BASE_CheckValidString(char *name)
{
	int l = strlen(name);

	if (l < 4 || l >= NAME_LENGTH)  
		return FALSE;

	if(strcmp(name, "Reino") == 0 || strcmp(name, "subcreate") == 0 || strcmp(name, "create") == 0 || strcmp(name, "gritar") == 0 || strcmp(name, "king") == 0 || strcmp(name, "kingdom") == 0 ||
	   strcmp(name, "getout") == 0 || strcmp(name, "gfame") == 0 || strcmp(name, "expulsar") == 0 || strcmp(name, "summonguild") == 0 || strcmp(name, "summon") == 0 || strcmp(name, "time") == 0 ||
	   strcmp(name, "relo") == 0 || strcmp(name, "stopally") == 0 || strcmp(name, "stopwar") == 0)
	   return FALSE;

	for(int j = 0; j < l; j++)
	{	 
		char x = name[j];

		if(x < 0) 
		{
			j++;
			continue;
		}

		if(ISALPHA(x) || ISDIGIT(x) || x == '-')  
			continue;

		return false;
	}

	return true;
}

bool BASE_CheckHangul(unsigned short word)
{   
	 unsigned char x = (unsigned char)word;
	 unsigned char y = (unsigned char)(word >> 8);

	 if(x >= 0xb0 && x <= 0xc9 && y >= 0xa1 && y<= 0xff)	
		 return true;
	 else 
		 return false;
}

int BASE_InitializeBaseDef()
{
	memset(ChargedGuildList, 0, sizeof(ChargedGuildList));

	BASE_InitializeServerList();

	int ret	= 1;
	int tret = 0;

	BASE_InitializeSkill();
	BASE_InitializeEffectName();
	BASE_InitializeItemList();
	BASE_InitializeInitItem();
	BASE_InitializeGuildName();
	BASE_InitializeAttribute();

	return ret;
}

void BASE_ApplyAttribute(char *pHeight, int size)
{
	int endx = g_HeightPosX + size;
	int endy = g_HeightPosY + size;

	int xx;
	int yy;

	for(int y = g_HeightPosY; y < endy; y++)
	{
		for(int x = g_HeightPosX; x < endx; x++)
		{
			xx = (x >> 2) & 0x3FF;

			yy = (y >> 2) & 0x3FF;

			unsigned char att = g_pAttribute[yy][xx];

			if(att & 2)
				pHeight[x + g_HeightWidth * (y - g_HeightPosY) - g_HeightPosX] = 127;
		}

	}
}

void BASE_InitModuleDir()
{
	char String[256];
	
	GetModuleFileName(NULL, String, 256);

	int len = strlen(String);

	len--;

	for(int i = len; i > 0; i--)
	{
		if(String[i] == '\\')
		{
			String[i] = 0;

			break;
		}
	}

	SetCurrentDirectory(String);	
}

int BASE_GetSum(char * p ,int size)
{
	int sum = 0;

    for(int i = 0; i < size; i++)
	{    
		int mod = i%7;

	    if(mod == 0)
			sum = sum + p[i] / 2;
	    
		if(mod == 1)
			sum = sum + (p[i] ^ 0xFF);
		
		if(mod == 2)
			sum = sum + p[i] * 3;
	 	
		if(mod == 3)
			sum = sum + p[i] * 2;
		
		if(mod == 4)
			sum = sum - (p[i] / 7);
		
		if(mod == 5)
			sum =sum - p[i];
		else
			sum = sum + (p[i] / 3);
	}

	return sum;
}

int BASE_GetSum2(char *p, int size)
{
	int sum = 0;

    for(int i = 0; i < size; i++)
	{
		 int mod = i%9;
	    
		 if (mod == 0)
			 sum = sum + p[i] * 2;
	     
		 if (mod == 1)
			 sum = sum + (p[i] ^ 0xFF);
		 
		 if (mod == 2)
			 sum = sum + p[i] / 3;
	 	 
		 if (mod == 3)
			 sum = sum + p[i] * 2;
		 
		 if (mod == 4)
			 sum = sum - (p[i] ^ 0x5A);
		 
		 if (mod == 5)
			 sum = sum - p[i];
		 
		 else
			 sum = sum + (p[i] / 5);
	}

	return sum;
}

void BASE_SpaceToUnderBar(char *szStr)
{
	for(unsigned int i = 0; i < strlen(szStr); i++)
	{
		if(szStr[i] == ' ')
			szStr[i] = '_';
	}
}

void BASE_UnderBarToSpace(char *szStr)
{ 	
	int nStrLen = strlen(szStr);
	/*
	unsigned char *pStr = (unsigned char*)szStr;
	unsigned char *pNext = _mbschr(pStr, '_');

	while(1)
	{
		*pNext = ' ';

		pNext = _mbschr(pNext, '_');

		nStrLen--;

		if(nStrLen < 0)
			break;
	}*/

	for (int i = 0; i < nStrLen; i++)
	{
		if (szStr[i] == '_')
			szStr[i] = ' ';
	}
}

void BASE_GetFirstKey(char * source, char * dest)
{
	 if((source[0] >= 'A' && source[0] <= 'Z' ) || (source[0] >= 'a' && source[0] <= 'z' ))
	 {
		 dest[0] = source[0];
	     dest[1] = 0;

	 	 return;
	 }

	 else if(source[0] < 0)
	 {
		 unsigned char *unsource = (unsigned char*)source;
		 int Point = (unsource[0] - 176) * 94  + unsource[1] - 161; 

         if (Point < 0 || Point >= 2350)
		 {
			strcpy(dest, "etc");

		    return;
		 }

		 int a;

		 for(a = 0; a < 18; a++)
		 {
			 if (Point < KorIndex[a])
				 break;
		 }

		 if  (a >= 0 && a <= 17)
		 {
			 dest[0] = KorFirst[a * 2];
		     dest[1] = KorFirst[a * 2 + 1];
		     dest[2] = 0;
		 }
		 else
		 {
			 strcpy(dest, "etc");

		     return;
		 }

	 } 
	 else
		 strcpy(dest, "etc");

     return;
}

int BASE_GetGuild(int x, int y)
{
	for(int i = 0; i < MAX_GUILDZONE; ++i)
	{
		if (x >= g_pGuildZone[i].CityLimitX1 && x <= g_pGuildZone[i].CityLimitX2 && y >= g_pGuildZone[i].CityLimitY1 && y <= g_pGuildZone[i].CityLimitY2)
			return i;
	}

	return 5;
}

int BASE_GetIndex(STRUCT_ITEM * item)
{
	int hword = 0;
	int lword = 0;

	if(item->sIndex >= 2330 && item->sIndex < 2390) 
		return 0;

	if(item->stEffect[0].cEffect == EF_HWORDINDEX || item->stEffect[1].cEffect == EF_HWORDINDEX || item->stEffect[2].cEffect == EF_HWORDINDEX)
	{ 
		if(item->stEffect[0].cEffect == EF_HWORDINDEX) 
			hword = (unsigned char)item->stEffect[0].cValue; 
		else if(item->stEffect[1].cEffect == EF_HWORDINDEX) 
			hword = (unsigned char)item->stEffect[1].cValue; 
		else
			hword = (unsigned char)item->stEffect[2].cValue;
	}
	if(item->stEffect[0].cEffect == EF_LWORDINDEX || item->stEffect[1].cEffect == EF_LWORDINDEX || item->stEffect[2].cEffect == EF_LWORDINDEX)
	{  
		if(item->stEffect[0].cEffect == EF_LWORDINDEX) 
			lword = (unsigned char)item->stEffect[0].cValue; 
		else if (item->stEffect[1].cEffect == EF_LWORDINDEX) 
			lword = (unsigned char)item->stEffect[1].cValue; 
		else
			lword = (unsigned char)item->stEffect[2].cValue;
	}

	int value = hword * 256 + lword;

	return value;
}

int BASE_GetGuild(STRUCT_ITEM *item)
{
	int hword = 0;
	int lword = 0;
	int value = 0;

	if(item->stEffect[0].cEffect == EF_HWORDGUILD || item->stEffect[1].cEffect == EF_HWORDGUILD || item->stEffect[2].cEffect == EF_HWORDGUILD)
	{
		if(item->stEffect[0].cEffect == EF_HWORDGUILD)
			hword = item->stEffect[0].cValue;
		else if(item->stEffect[1].cEffect == EF_HWORDGUILD)
			hword = item->stEffect[1].cValue;
		else if(item->stEffect[2].cEffect == EF_HWORDGUILD)
			hword = item->stEffect[2].cValue;
	}
	if(item->stEffect[0].cEffect == EF_LWORDGUILD || item->stEffect[1].cEffect == EF_LWORDGUILD || item->stEffect[2].cEffect == EF_LWORDGUILD)
	{
		if(item->stEffect[0].cEffect == EF_LWORDGUILD)
			lword = item->stEffect[0].cValue;
		else if(item->stEffect[1].cEffect == EF_LWORDGUILD)
			lword = item->stEffect[1].cValue;
		else if(item->stEffect[2].cEffect == EF_LWORDGUILD)
			lword = item->stEffect[2].cValue;
	}

	value = (hword * 256) + lword;

	return value;
}

int BASE_GetSubGuild(int item)
{
	int ret = 0;

	if(item >= 526 && item <= 537)
		ret = (item - 526) % 3 + 1;

	return ret;
}

int BASE_GetArena(int x, int y)
{
	for(int i = 0; i < MAX_GUILDZONE; i++)
	{
		if(x >= g_pGuildZone[i].WarAreaX1 && x <= g_pGuildZone[i].WarAreaX2 && y >= g_pGuildZone[i].WarAreaY1 && y <= g_pGuildZone[i].WarAreaY2)
			return i;
	}

	return 5;
}

int BASE_GetVillage(int x, int y)
{
	for(int i = 0; i < 5; ++i )
	{
		if (x >= g_pGuildZone[i].CityLimitX1 && x <= g_pGuildZone[i].CityLimitX2 && y >= g_pGuildZone[i].CityLimitY1 && y <= g_pGuildZone[i].CityLimitY2)
			return i;
	}

	return 5;
}

void BASE_GetKorFirst(int temp, int *a)
{
     int word1;
     int temp1;

     word1 = (temp >> 10) & 31;
     word1 = word1-1;
     temp1 = word1;

     *a = temp1;
}

int IsClearString3(char *str, int nTarget)
{
	if(str == NULL)
		return TRUE;

	char *pNextRightChar = CharNext(&str[nTarget]);

	if	(str+nTarget+1 != pNextRightChar)
		return FALSE;

	return TRUE;
}

int BASE_GetHttpRequest(char *httpname, char *Request, int MaxBuffer)
{
	return 1;
}

int BASE_GetEnglish(char *name)
{
	// Havia uma cópia para um `temp` nunca usado, com sizeof(name) (o ponteiro) como tamanho.
	for(int i = 0; i < MAX_ENGLISH; i++)
	{
		if(!strcmp(name, g_pEnglish[i][0]))
		{ 
			strcpy(name, g_pEnglish[i][2]);

		   return TRUE;
		}
	}

    return FALSE;
}

int BASE_GetWeekNumber()
{
	time_t now;

	time(&now);

	unsigned int week = 86400;

	int ret = (int)(now / week - 3);

	return ret;
}

BOOL BASE_CheckPacket(MSG_STANDARD *m)
{/*
	if (m == NULL)
		return 0;

	int code = 0;
	
	if	(m->Type==  _MSG_MessagePanel				&& m->Size != sizeof( MSG_MessagePanel			))code = 1;
	if	(m->Type==  _MSG_MessageBoxOk 				&& m->Size != sizeof( MSG_MessageBoxOk 			))code = 1;
	if	(m->Type==  _MSG_DBNewAccount   				&& m->Size != sizeof( MSG_NewAccount   		))code = 1;
	if	(m->Type==  _MSG_AccountLogin   			&& m->Size != sizeof( MSG_AccountLogin   		))code = 1;
	if	(m->Type==  _MSG_CNFAccountLogin 			&& m->Size != sizeof( MSG_DBCNFAccountLogin 	))code = 1;
	if  (m->Type ==  _MSG_CreateCharacter				&& m->Size != sizeof(MSG_CreateCharacter	))code = 1;
	if	(m->Type==  _MSG_CNFNewCharacter			&& m->Size != sizeof( MSG_CNFNewCharacter		))code = 1;
	if	(m->Type==  _MSG_DeleteCharacter			&& m->Size != sizeof( MSG_DeleteCharacter		))code = 1;
	if	(m->Type==  _MSG_CNFDeleteCharacter 		&& m->Size != sizeof( MSG_CNFDeleteCharacter 	))code = 1;
	if	(m->Type==  _MSG_CharacterLogin  			&& m->Size != sizeof( MSG_CharacterLogin  		))code = 1;
	if	(m->Type==  _MSG_CNFCharacterLogin 			&& m->Size != sizeof( MSG_CNFCharacterLogin 	))code = 1;
	if	(m->Type==  _MSG_CNFCharacterLogout 		&& m->Size != sizeof( MSG_STANDARD 				))code = 1;
	if	(m->Type==  _MSG_NewAccountFail    			&& m->Size != sizeof( MSG_STANDARD    			))code = 1;
	if  (m->Type == _MSG_DBAccountLoginFail_Account	&& m->Size != sizeof(MSG_STANDARD				))code = 1;
	if	(m->Type==  _MSG_CharacterLoginFail			&& m->Size != sizeof( MSG_STANDARD  			))code = 1;
	if	(m->Type==  _MSG_NewCharacterFail   		&& m->Size != sizeof( MSG_STANDARD   			))code = 1;
	if	(m->Type==  _MSG_DeleteCharacterFail		&& m->Size != sizeof( MSG_STANDARD 				))code = 1;
	if	(m->Type==  _MSG_AlreadyPlaying   			&& m->Size != sizeof( MSG_STANDARD   			))code = 1;
	if	(m->Type==  _MSG_StillPlaying   			&& m->Size != sizeof( MSG_STANDARD   			))code = 1;
	if	(m->Type==  _MSG_MessageChat   				&& m->Size != sizeof( MSG_MessageChat   		))code = 1;
	if	(m->Type==  _MSG_MessageWhisper				&& m->Size != sizeof( MSG_MessageWhisper		))code = 1;
	if	(m->Type==  _MSG_UpdateScore  				&& m->Size != sizeof( MSG_UpdateScore  			))code = 1;
	if	(m->Type==  _MSG_UpdateEtc					&& m->Size != sizeof( MSG_UpdateEtc				))code = 1;
	if	(m->Type==  _MSG_CNFMobKill   				&& m->Size != sizeof( MSG_CNFMobKill   			))code = 1;
	if	(m->Type==  _MSG_CreateMob  				&& m->Size != sizeof( MSG_CreateMob  			))code = 1;
	if	(m->Type==  _MSG_RemoveMob					&& m->Size != sizeof( MSG_RemoveMob				))code = 1;
	if	(m->Type==  _MSG_Action						&& m->Size != sizeof( MSG_Action				))code = 1;
//	if	(m->Type==  _MSG_REQMobByID					&& m->Size != sizeof( MSG_REQMobByID			))code = 1;
	if	(m->Type==  _MSG_Motion    					&& m->Size != sizeof( MSG_Motion    			))code = 1;
	if	(m->Type==  _MSG_UpdateEquip				&& m->Size != sizeof( MSG_UpdateEquip			))code = 1;
	if	(m->Type==  _MSG_Attack						&& m->Size != sizeof( MSG_Attack				))code = 1;
//	if	(m->Type==  _MSG_Stop          				&& m->Size != sizeof( MSG_Action          		))code = 1;
	if	(m->Type==  _MSG_CreateItem   				&& m->Size != sizeof( MSG_CreateItem   			))code = 1;
//	if	(m->Type==  _MSG_RemoveItem					&& m->Size != sizeof( MSG_RemoveItem			))code = 1;
	if	(m->Type==  _MSG_GetItem					&& m->Size != sizeof( MSG_GetItem				))code = 1;
	if	(m->Type==  _MSG_CNFGetItem					&& m->Size != sizeof( MSG_CNFGetItem			))code = 1;
	if	(m->Type==  _MSG_DropItem 					&& m->Size != sizeof( MSG_DropItem 				))code = 1;
	if	(m->Type==  _MSG_UseItem					&& m->Size != sizeof( MSG_UseItem				))code = 1;
	if	(m->Type==  _MSG_UpdateItem 				&& m->Size != sizeof( MSG_UpdateItem 			))code = 1;
	if	(m->Type==  _MSG_CNFDropItem 				&& m->Size != sizeof( MSG_CNFDropItem 			))code = 1;
//	if	(m->Type==  _MSG_SwapItem					&& m->Size != sizeof( MSG_SwapItem				))code = 1;
	if	(m->Type==  _MSG_ApplyBonus					&& m->Size != sizeof( MSG_ApplyBonus			))code = 1;
	if	(m->Type==  _MSG_SetShortSkill				&& m->Size != sizeof( MSG_SetShortSkill			))code = 1;
	if	(m->Type==  _MSG_Buy						&& m->Size != sizeof( MSG_Buy					))code = 1;
	if	(m->Type==  _MSG_Sell						&& m->Size != sizeof( MSG_Sell					))code = 1;
	if	(m->Type==  _MSG_REQShopList				&& m->Size != sizeof( MSG_REQShopList			))code = 1;
	if	(m->Type==  _MSG_ShopList					&& m->Size != sizeof( MSG_ShopList				))code = 1;
//	if	(m->Type==  _MSG_AddParty					&& m->Size != sizeof( MSG_AddParty				))code = 1;
	if	(m->Type==  _MSG_RemoveParty				&& m->Size != sizeof( MSG_RemoveParty			))code = 1;
	if (m->Type == _MSG_SendReqParty				&& m->Size != sizeof(MSG_SendReqParty))code = 1;
	if	(m->Type==  _MSG_CNFAddParty				&& m->Size != sizeof( MSG_CNFAddParty				))code = 1;
	if	(m->Type==  _MSG_SetHpMp					&& m->Size != sizeof( MSG_SetHpMp				))code = 1;
	if	(m->Type==  _MSG_SendItem					&& m->Size != sizeof( MSG_SendItem				))code = 1;
	if	(m->Type==  _MSG_Trade						&& m->Size != sizeof( MSG_Trade					))code = 1;
	if	(m->Type==  _MSG_QuitTrade					&& m->Size != sizeof( MSG_STANDARD				))code = 1;
	if (m->Type == _MSG_TradingItem						&& m->Size != sizeof(MSG_TradingItem		))code = 1;
	if	(m->Type==  _MSG_CNFCheck					&& m->Size != sizeof( MSG_STANDARD				))code = 1;
	if	(m->Type==  _MSG_Withdraw        			&& m->Size != sizeof( MSG_STANDARDPARM        	))code = 1;
	if	(m->Type==  _MSG_Deposit					&& m->Size != sizeof( MSG_STANDARDPARM			))code = 1;
	if	(m->Type==  _MSG_Restart					&& m->Size != sizeof( MSG_STANDARD 				))code = 1;
	if	(m->Type==  _MSG_SetHpDam					&& m->Size != sizeof( MSG_SetHpDam				))code = 1;
	if (m->Type == _MSG_UpdateWeather          			&& m->Size != sizeof(MSG_UpdateWeather))code = 1;
	if	(m->Type==  _MSG_Quest             			&& m->Size != sizeof( MSG_STANDARDPARM2         ))code = 1;
	if	(m->Type==  _MSG_Deprivate        			&& m->Size != sizeof( MSG_STANDARDPARM       	))code = 1;
	if	(m->Type==  _MSG_ReqChallange       		&& m->Size != sizeof( MSG_STANDARD       		))code = 1;
	if	(m->Type==  _MSG_Challange        			&& m->Size != sizeof( MSG_STANDARDPARM        	))code = 1;
	if	(m->Type==  _MSG_ChallangeConfirm  			&& m->Size != sizeof( MSG_STANDARDPARM2  		))code = 1;
	if	(m->Type==  _MSG_ReqTeleport       			&& m->Size != sizeof( MSG_STANDARDPARM       	))code = 1;
	if (m->Type ==  _MSG_ChangeCity      			&& m->Size != sizeof(MSG_STANDARDPARM))code = 1;
	if	(m->Type==  _MSG_SetHpMode           		&& m->Size != sizeof( MSG_STANDARDPARM          ))code = 1;
	if	(m->Type==  _MSG_SetClan             		&& m->Size != sizeof( MSG_STANDARDPARM          ))code = 1;
//	if	(m->Type==  _MSG_BillingPage				&& m->Size != sizeof( MSG_STANDARDPARM          ))code = 1;
	if	(m->Type==  _MSG_DBMessagePanel				&& m->Size != sizeof( MSG_MessagePanel			))code = 1;
	if	(m->Type==  _MSG_DBMessageBoxOk				&& m->Size != sizeof( MSG_MessageBoxOk       	))code = 1;
//	if	(m->Type==  _MSG_DBCNFNewAccount			&& m->Size != sizeof( MSG_DBCNFNewAccount		))code = 1; // À¥¿¡¼­¸¸Áö¿ø
	if	(m->Type==  _MSG_DBCNFAccountLogin			&& m->Size != sizeof( MSG_DBCNFAccountLogin    	))code = 1;
	if	(m->Type==  _MSG_DBCNFCharacterLogin		&& m->Size != sizeof( MSG_CNFCharacterLogin  	))code = 1;
	if	(m->Type==  _MSG_DBCNFNewCharacter			&& m->Size != sizeof( MSG_CNFNewCharacter    	))code = 1;
	if	(m->Type==  _MSG_DBCNFDeleteCharacter		&& m->Size != sizeof( MSG_CNFDeleteCharacter 	))code = 1;
	if	(m->Type==  _MSG_DBNewAccountFail			&& m->Size != sizeof( MSG_STANDARD     	        ))code = 1;
//	if	(m->Type==  _MSG_DBAccountLoginFail			&& m->Size != sizeof( MSG_STANDARD   	        ))code = 1;
	if	(m->Type==  _MSG_DBCharacterLoginFail		&& m->Size != sizeof( MSG_STANDARD 	            ))code = 1;
	if	(m->Type==  _MSG_DBNewCharacterFail			&& m->Size != sizeof( MSG_STANDARD   	        ))code = 1;
	if	(m->Type==  _MSG_DBDeleteCharacterFail		&& m->Size != sizeof( MSG_STANDARD			    ))code = 1;
	if	(m->Type==  _MSG_DBAlreadyPlaying			&& m->Size != sizeof( MSG_STANDARD 	            ))code = 1;
	if	(m->Type==  _MSG_DBStillPlaying				&& m->Size != sizeof( MSG_STANDARD   	        ))code = 1;
	if	(m->Type==  _MSG_DBAccountLoginFail_Account && m->Size != sizeof( MSG_STANDARD				))code = 1;
	if	(m->Type==  _MSG_DBAccountLoginFail_Pass    && m->Size != sizeof( MSG_STANDARD				))code = 1;
	if	(m->Type==  _MSG_DBSetIndex					&& m->Size != sizeof( MSG_STANDARDPARM3			))code = 1;
	if	(m->Type==  _MSG_DBAccountLoginFail_Block	&& m->Size != sizeof( MSG_STANDARD				))code = 1;
	if	(m->Type==  _MSG_DBAccountLoginFail_Disable	&& m->Size != sizeof( MSG_STANDARD				))code = 1;
	if	(m->Type==  _MSG_DBNewAccount				&& m->Size != sizeof(MSG_NewAccount				))code = 1;
	if	(m->Type==  _MSG_DBCreateCharacter			&& m->Size != sizeof(MSG_CreateCharacter     		))code = 1;
	if	(m->Type==  _MSG_DBAccountLogin				&& m->Size != sizeof(MSG_AccountLogin     		))code = 1;
	if	(m->Type==  _MSG_DBCharacterLogin			&& m->Size != sizeof(MSG_CharacterLogin			))code = 1;
	if	(m->Type==  _MSG_DBNoNeedSave          		&& m->Size != sizeof(MSG_STANDARD       		))code = 1;
	if	(m->Type==  _MSG_DBSaveMob             		&& m->Size != sizeof(MSG_DBSaveMob				))code = 1;
	if	(m->Type==  _MSG_DBDeleteCharacter     		&& m->Size != sizeof(MSG_DeleteCharacter		))code = 1;
	if	(m->Type==  _MSG_NPReqIDPASS           		&& m->Size != sizeof(MSG_NPIDPASS        		))code = 1;
	if	(m->Type==  _MSG_NPIDPASS              		&& m->Size != sizeof(MSG_NPIDPASS           	))code = 1;
	if	(m->Type==  _MSG_NPReqAccount          		&& m->Size != sizeof(MSG_NPReqAccount       	))code = 1;
	if	(m->Type==  _MSG_NPNotFound            		&& m->Size != sizeof(MSG_STANDARD         		))code = 1;
	if	(m->Type==  _MSG_NPAccountInfo         		&& m->Size != sizeof(MSG_NPAccountInfo      	))code = 1;
	if	(m->Type==  _MSG_NPReqSaveAccount      		&& m->Size != sizeof(MSG_NPAccountInfo   		))code = 1;
	if	(m->Type==  _MSG_NPDisable             		&& m->Size != sizeof(MSG_NPEnable          		))code = 1;
	if	(m->Type==  _MSG_NPEnable              		&& m->Size != sizeof(MSG_NPEnable           	))code = 1;
	if	(m->Type==  _MSG_NPNotice              		&& m->Size != sizeof(MSG_NPNotice				))code = 1;
	if	(m->Type==  _MSG_NPState               		&& m->Size != sizeof(MSG_STANDARDPARM			))code = 1;
	if	(m->Type==  _MSG_War						&& m->Size != sizeof(MSG_STANDARDPARM2			))code = 1;
	if	(m->Type==	_MSG_ReqTransper				&& m->Size != sizeof(MSG_ReqTransper			))code = 1;
	
	return code;
	*/
	return FALSE;
}
