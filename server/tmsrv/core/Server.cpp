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
#include <Windows.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <math.h>
#include <io.h>
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
#include "core/CReadFiles.h"
#include "systems/CWarTower.h"

#pragma region Defines

int FREEEXP = 35;
int PARTY_DIF = 150;

int ServerDown = -1000;

int g_EmptyMob = MAX_USER;

int ServerIndex = -1;
int Sapphire = 8;
int ServerGroup = -1;
int GuildCounter = 1;

int ValidGuild = MAX_GUILDZONE;
int WaterScrollPosition[3][10][2] = 
{
	{//N
		{ 1121, 3554 },//LV1
		{ 1085, 3554 },//2
		{ 1049, 3554 },//3
		{ 1049, 3518 },//4
		{ 1049, 3482 },//5
		{ 1085, 3482 },//6
		{ 1121, 3482 },//7
		{ 1121, 3518 },//8
		{ 1085, 3518 },
		{ 1085, 3518 }
	},
	{//M
		{ 1250, 3682 },//LV1
		{ 1214, 3682 },//2
		{ 1178, 3682 },//3
		{ 1178, 3646 },//4
		{ 1178, 3610 },//5
		{ 1214, 3610 },//6
		{ 1250, 3610 },//7
		{ 1250, 3646 },//8
		{ 1214, 3646 },
		{ 1214, 3646 }
	},
	{//A
		{ 1379, 3554 },//LV1
		{ 1340, 3554 },//2
		{ 1305, 3554 },//3
		{ 1305, 3518 },//4
		{ 1305, 3482 },//5
		{ 1341, 3482 },//6
		{ 1377, 3482 },//7
		{ 1377, 3518 },//8
		{ 1343, 3518 },
		{ 1343, 3518 }
	}
};
int PesaNPosStandard[13][2] = 
{
	{ 1304, 335 },
	{ 1304, 335 },
	{ 1304, 335 },
	{ 1304, 335 },
	{ 1304, 335 },
	{ 1304, 335 },
	{ 1304, 335 },
	{ 1304, 335 },
	{ 1304, 335 },
	{ 1304, 335 },
	{ 1304, 335 },
	{ 1304, 335 },
	{ 1304, 335 }
};

int PesaMPosStandard[13][2] =
{
	{ 1083, 308 },
	{ 1083, 308 },
	{ 1083, 308 },
	{ 1083, 308 },
	{ 1083, 308 },
	{ 1083, 308 },
	{ 1083, 308 },
	{ 1083, 308 },
	{ 1083, 308 },
	{ 1083, 308 },
	{ 1083, 308 },
	{ 1083, 308 },
	{ 1083, 308 }
};

int PesaAPosStandard[13][2] =
{
	{ 1204, 152 },
	{ 1217, 155 },
	{ 1195, 175 },
	{ 1182, 174 },
	{ 1171, 190 },
	{ 1189, 196 },
	{ 1209, 182 },
	{ 1226, 190 },
	{ 1230, 174 },
	{ 1247, 184 },
	{ 1224, 190 },
	{ 1211, 165 },
	{ 1209, 174 }
};

int CartaPos[4][2] = 
{
	{ 786, 3688 },
	{ 843, 3688 },
	{ 843, 3632 },
	{ 786, 3640 }
};

int PistaPos[7][3][2] = 
{
	{//Sala 1 Lich
		{ 3350, 1622 }, //Grupo 1
		{ 3431, 1634 }, //Grupo 2
		{ 0, 0 }  //Grupo 3
	},
	{//Sala 2 Torre
		{ 3362, 1574 }, //Grupo 1
		{ 3385, 1555 }, //Grupo 2
		{ 3414, 1575 }  //Grupo 3
	},
	{//Sala 3 Amon
		{ 3410, 1453 }, //Grupo 1
		{ 3419, 1426 }, //Grupo 2
		{ 3358, 1436 }  //Grupo 3
	},
	{//Sala 4 Sulrang
		{ 3376, 1096 }, //Grupo 1
		{ 3400, 1089 }, //Grupo 2
		{ 3392, 1073 }  //Grupo 3
	},
	{//Sala 5 Labirinto
		{ 3342, 1394 }, //Grupo 1
		{ 3444, 1394 }, //Grupo 2
		{ 3442, 1293 }  //Grupo 3
	},
	{//Sala 6 Balrog
		{ 3421, 1217 }, //Grupo 1
		{ 3426, 1211 }, //Grupo 2
		{ 3424, 1226 }  //Grupo 3
	},
	{//Sala 7 Coelho
		{ 3404, 1517 }, //Grupo 1
		{ 3392, 1479 }, //Grupo 2
		{ 3383, 1501 }  //Grupo 3
	}
};

int PistaBalrogPos[5][2] = 
{
	{3421, 1217}, //Sala1
	{3428, 1180}, //Sala2
	{3357, 1186}, //Sala3
	{3393, 1255}, //Sala4
	{3385, 1204} //Sala Boss
};

int PistaBalrogPortalPos[5][5][2] = 
{
	//Sala1
	{
		{3428, 1196},
		{0, 0},
		{0, 0},
		{0, 0}
	},

	//Sala2
	{
		{3380, 1176},
		{3380, 1164},
		{0, 0},
		{0, 0}
	},

	//Sala3
	{
		{3360, 1224},
		{3352, 1224},
		{3340, 1224},
		{0, 0}
	},

	//Sala4
	{
		{3400, 1236},
		{3392, 1236},
		{3384, 1236},
		{3376, 1236}
	},

	//Sala5
	{
		{0, 0},
		{0, 0},
		{0, 0},
		{0, 0}
	}
};

int CartaTime = 0;
int CartaSala = 0;

short weapon[32][9];

CPSock BillServerSocket;
char   BillServerAddress[32] = { 0, };
int	   BillServerPort = 0;

HWND  hWndMain;
HMENU hMainMenu;
HFONT hFont = NULL;

FILE *fLogFile = NULL;
FILE *fChatLogFile = NULL;
FILE *fItemLogFile = NULL;

CPSock DBServerSocket;

unsigned int pAdminIP[MAX_ADMIN] = {0,};

CNPCGenerator mNPCGen;
CNPCSummon mSummon;

CUser pUser[MAX_USER];
CMob pMob[MAX_MOB];
STRUCT_BLOCKMAC pMac[MAX_MAC];

STRUCT_TREASURE g_pTreasure[8];

STRUCT_ITEM LevelItem[4][4][400];

char temp[4096];

CItem pItem[MAX_ITEM];

unsigned int CurrentTime = 0;
unsigned int LastSendTime = 0;


int BillCounter = 0;

int	g_Week = 0;
int	g_Hour = 0;

int LastLogDay = 0;
int LastChatLogDay = 0;
int LastItemLogDay = 0;

int ConfigReady = 0;

///////////////////////////////////////
//Config for Event
//////////////////////////////////////
int evNotice = 1;
int evStartIndex = 0;
int evEndIndex = 0;
int evCurrentIndex = 0;
int evRate = 0;
int evItem = 0;
int evIndex = 0;
int evOn = 0;
int evDelete = 0;
////////////////////////////////////

int NewbieEventServer = 0;
int CastleServer = 0;

int DOUBLEMODE = 0;
int DUNGEONEVENT = 1;
int DEADPOINT = 1;
int StatSapphire = 30;
int BRItem = 0;
int BRHour = 19;

int BILLING = 3;
int CHARSELBILL = 0;
int POTIONCOUNT = 10;
int PARTYBONUS = 100;
int GUILDBOARD = 0;

int UserCount = 1;
int SaveCount = 1;
int ItemCount = 100;

int ForceWeather = -1;

int WeekMode = 4;
int ForceWeekMode = -1;
int GuildDay = 0;
int GuildHour = 20;

int NumGuildBattle = 26;
int PotionReady = 0;

int PotionDelay = 500;

int RvRState = 0;
int RvRHour = 22;
int RvRBonus = 0;

int GTorreState = 0;
int GTorreHour = 22;
int GTorreGuild = 0;

int NewbieHour = 20;
int ColoState = 8;

int NumServerInGroup = 0;

int LiveTower[3];

int pChallangerMoney[6];

unsigned short g_pGuildWar[65536];
unsigned short g_pGuildAlly[65536];
STRUCT_GUILDINFO GuildInfo[65536];

unsigned short pMobGrid[MAX_GRIDY][MAX_GRIDX];
unsigned short pItemGrid[MAX_GRIDY][MAX_GRIDX];
char pHeightGrid[MAX_GRIDY][MAX_GRIDX];

unsigned char LocalIP[4] = { 0, };

char FailAccount[16][16];

CPSock ListenSocket;

char DBServerAddress[32] = { 0, };
int DBServerPort = 0;

int isDropItem = 0;
int maxNightmare = 3;
int CurrentWeather = 0;
int TESTSERVER = 0;
int LOCALSERVER = 0;

int TransperCharacter = 0;

unsigned int SecCounter = 0;
unsigned int MinCounter = 0;

int GuildImpostoID[MAX_GUILDZONE];

int BrState = 0;
int BrMod = 0;
int BrGrid = 0;
int CastleState = 0;
int Colo150Limit = 0;
int KefraLive = 0;
char KefraKiller[32] = "LOCALSERVER";

int g_dLevel = 0;
int g_dLevel1 = 0;
int g_dLevel2 = 0;

int Kingdom1Clear = 0;
int Kingdom2Clear = 0;

int GuildNameInitialized = 0;

///////////////////////////////////////
//Config for Duel: 1:1 5:5 10:10 All:All
//////////////////////////////////////
char nRanking1[32] = { "unnamed" };
char nRanking2[32] = { "unnamed" };
char nTitle1[32] = { "unnamed" };
char nTitle2[32] = { "unnamed" };

int RankingProgress = 0;
int Ranking1 = 0;
int Ranking2 = 0;
int lTitle1 = 0;
int lTitle2 = 0;
int RankingTime = 0;

int Ranking1X = 147;
int Ranking1Y = 4044;
int Ranking2X = 189;
int Ranking2Y = 4044;

int cRanking1X = 2564;
int cRanking1Y = 1688;
int cRanking2X = 2579;
int cRanking2Y = 1714;

int kRanking1X = 142;
int kRanking1Y = 4007;
int kRanking2X = 195;
int kRanking2Y = 4082;

int TaxChanged[MAX_GUILDZONE] = { 0, 0, 0, 0, 0 };

HDC hDC;

//////////////////////////////////////

int WaterClear1[3][10] = { 0, 0, 0, 0, 0, };
int PartyPesa[3] = { 0, 0, 0 };

int RvRBluePoint = 0;
int RvRRedPoint = 0;

STRUCT_PISTA Pista[7] = { 0, 0, 0, 0, 0, };
STRUCT_MERC pMobMerc[MAX_MOB_MERC];

int PistaRune[7][10] = 
{
	// Lich
	{
		5110, 5112, 5115, 5113, 5111, 0, 0, 0, 0, 0
	},

	// Torre
	{
		5114, 5113, 5117, 5111, 5115, 5112, 0, 0, 0, 0
	},

	// Amon
	{
		5118, 5121, 5122, 5116, 5130, 5119, 0, 0, 0, 0
	},

	// Sulrang
	{
		5122, 5126, 5121, 5116, 5119, 0, 0, 0, 0, 0
	},

	// Labirinto
	{
		5125, 5126, 5124, 5127, 0, 0, 0, 0, 0, 0
	},

	// Balrog
	{
		5120, 5131, 5118, 5119, 5123, 5132, 5121, 0, 0, 0
	},

	//Coelho
	{
		5130, 5131, 5119, 5133, 5120, 5123, 5132, 5129, 5128, 0
	}
};

//////////////////////////////////////
extern STRUCT_GUILDZONE g_pGuildZone[MAX_GUILDZONE];

#pragma endregion

void ClearCrown(int conn)
{
	if (pUser[conn].Mode != USER_PLAY)
		return;

	int GLevel = pMob[conn].MOB.GuildLevel;
	int Guild = pMob[conn].MOB.Guild;

	int IsCrownGuild = FALSE;

	int i = 0;

	for (i = 0; i < MAX_SERVER; i++)
	{
		int CrownGuild = ChargedGuildList[i][4];

		if ((Guild != 0) && (CrownGuild != 0) && (CrownGuild == Guild && GLevel == 9))
		{
			IsCrownGuild = TRUE;

			break;
		}
	}

	if (IsCrownGuild == 1)
	{
		int HaveCrow = 0;
		int HaveDrag = 0;

		for (i = 0; i < MAX_EQUIP; i++)
		{
			if(pMob[conn].MOB.Equip[i].sIndex == 747)
				HaveCrow = 1;

			if (pMob[conn].MOB.Equip[i].sIndex != 3993 && pMob[conn].MOB.Equip[i].sIndex != 3994) // 747 = Crown on itemlist
				continue;

			HaveDrag = 1;

			if(pMob[conn].MOB.Equip[i].sIndex == 3993 && pMob[conn].MOB.Clan == 7)
			{
				sprintf(temp, "etc,crown guild:%d level:%d charge:%d name:%s", Guild, GLevel, ChargedGuildList[ServerIndex][4], pMob[conn].MOB.MobName);
				Log(temp, pUser[conn].AccountName, pUser[conn].IP);

				pMob[conn].MOB.Equip[i].sIndex = 3994;

				SendItem(conn, ITEM_PLACE_EQUIP, i, &pMob[conn].MOB.Equip[i]);
			}
			else if(pMob[conn].MOB.Equip[i].sIndex == 3994 && pMob[conn].MOB.Clan == 8)
			{
				sprintf(temp, "etc,crown guild:%d level:%d charge:%d name:%s", Guild, GLevel, ChargedGuildList[ServerIndex][4], pMob[conn].MOB.MobName);
				Log(temp, pUser[conn].AccountName, pUser[conn].IP);

				pMob[conn].MOB.Equip[i].sIndex = 3993;

				SendItem(conn, ITEM_PLACE_EQUIP, i, &pMob[conn].MOB.Equip[i]);
			}
		}

		for (i = 0; i < MAX_CARRY; i++)
		{
			if (pMob[conn].MOB.Carry[i].sIndex == 747)
				HaveCrow = 1;

			if (pMob[conn].MOB.Carry[i].sIndex != 3993 && pMob[conn].MOB.Carry[i].sIndex != 3994) // 747 = Crown on itemlist
				continue;

			HaveDrag = 1;

			if(pMob[conn].MOB.Carry[i].sIndex == 3993 && pMob[conn].MOB.Clan == 7)
			{
				sprintf(temp, "etc,crown guild:%d level:%d charge:%d name:%s", Guild, GLevel, ChargedGuildList[ServerIndex][4], pMob[conn].MOB.MobName);
				Log(temp, pUser[conn].AccountName, pUser[conn].IP);

				pMob[conn].MOB.Carry[i].sIndex = 3994;

				SendItem(conn, ITEM_PLACE_CARRY, i, &pMob[conn].MOB.Carry[i]);
			}

			if(pMob[conn].MOB.Carry[i].sIndex == 3994 && pMob[conn].MOB.Clan == 8)
			{
				sprintf(temp, "etc,crown guild:%d level:%d charge:%d name:%s", Guild, GLevel, ChargedGuildList[ServerIndex][4], pMob[conn].MOB.MobName);
				Log(temp, pUser[conn].AccountName, pUser[conn].IP);

				pMob[conn].MOB.Carry[i].sIndex = 3993;

				SendItem(conn, ITEM_PLACE_CARRY, i, &pMob[conn].MOB.Carry[i]);
			}
		}

		for (i = 0; i < MAX_CARGO; i++)
		{
			if (pUser[conn].Cargo[i].sIndex == 747)
				HaveCrow = 1;

			if (pUser[conn].Cargo[i].sIndex != 3993 && pUser[conn].Cargo[i].sIndex != 3994) // 747 = Crown on itemlist
				continue;

			HaveDrag = 1;

			if(pUser[conn].Cargo[i].sIndex == 3993 && pMob[conn].MOB.Clan == 7)
			{
				sprintf(temp, "etc,crown guild:%d level:%d charge:%d name:%s", Guild, GLevel, ChargedGuildList[ServerIndex][4], pMob[conn].MOB.MobName);
				Log(temp, pUser[conn].AccountName, pUser[conn].IP);

				pUser[conn].Cargo[i].sIndex = 3994;

				SendItem(conn, ITEM_PLACE_CARGO, i, &pUser[conn].Cargo[i]);
			}
			else if(pUser[conn].Cargo[i].sIndex == 3994 && pMob[conn].MOB.Clan == 8)
			{
				sprintf(temp, "etc,crown guild:%d level:%d charge:%d name:%s", Guild, GLevel, ChargedGuildList[ServerIndex][4], pMob[conn].MOB.MobName);
				Log(temp, pUser[conn].AccountName, pUser[conn].IP);

				pUser[conn].Cargo[i].sIndex = 3993;

				SendItem(conn, ITEM_PLACE_CARGO, i, &pUser[conn].Cargo[i]);
			}
		}

		if(HaveCrow == 0)
		{
			STRUCT_ITEM Item;
			memset(&Item, 0, sizeof(STRUCT_ITEM));

			Item.sIndex = 747;
			Item.stEffect[0].cEffect = 43;
			Item.stEffect[0].cValue = 9;

			PutItem(conn, &Item);
		}

		if(HaveDrag == 0)
		{
			STRUCT_ITEM Item;
			memset(&Item, 0, sizeof(STRUCT_ITEM));

			if(pMob[conn].MOB.Clan == 8)
				Item.sIndex = 3993;

			else if(pMob[conn].MOB.Clan == 7)
				Item.sIndex = 3994;


			PutItem(conn, &Item);
		}

		return;
	}

	for (i = 0; i < MAX_EQUIP; i++)
	{
		if (pMob[conn].MOB.Equip[i].sIndex != 747 && pMob[conn].MOB.Equip[i].sIndex != 3993 && pMob[conn].MOB.Equip[i].sIndex != 3994) // 747 = Crown on itemlist
			continue;

		sprintf(temp, "etc,crown guild:%d level:%d charge:%d name:%s", Guild, GLevel, ChargedGuildList[ServerIndex][4], pMob[conn].MOB.MobName);
		Log(temp, pUser[conn].AccountName, pUser[conn].IP);

		memset(&pMob[conn].MOB.Equip[i], 0, sizeof(STRUCT_ITEM));

		SendItem(conn, ITEM_PLACE_EQUIP, i, &pMob[conn].MOB.Equip[i]);
	}

	for (i = 0; i < MAX_CARRY; i++)
	{
		if (pMob[conn].MOB.Carry[i].sIndex != 747 && pMob[conn].MOB.Carry[i].sIndex != 3993 && pMob[conn].MOB.Carry[i].sIndex != 3994) // 747 = Crown on itemlist
			continue;

		sprintf(temp, "etc,crown guild:%d level:%d charge:%d name:%s", Guild, GLevel, ChargedGuildList[ServerIndex][4], pMob[conn].MOB.MobName);
		Log(temp, pUser[conn].AccountName, pUser[conn].IP);

		memset(&pMob[conn].MOB.Carry[i], 0, sizeof(STRUCT_ITEM));

		SendItem(conn, ITEM_PLACE_CARRY, i, &pMob[conn].MOB.Carry[i]);
	}

	for (i = 0; i < MAX_CARGO; i++)
	{
		if (pUser[conn].Cargo[i].sIndex != 747 && pUser[conn].Cargo[i].sIndex != 3993 && pUser[conn].Cargo[i].sIndex != 3994) // 747 = Crown on itemlist
			continue;

		sprintf(temp, "etc,crown guild:%d level:%d charge:%d name:%s", Guild, GLevel, ChargedGuildList[ServerIndex][4], pMob[conn].MOB.MobName);
		Log(temp, pUser[conn].AccountName, pUser[conn].IP);

		memset(&pUser[conn].Cargo[i], 0, sizeof(STRUCT_ITEM));

		SendItem(conn, ITEM_PLACE_CARGO, i, &pUser[conn].Cargo[i]);
	}
}

BOOL AddCrackError(int conn, int val, int Type)
{
	if (Type != 3 && Type != 8 && Type != 15)
	{
		sprintf(temp, "cra point: %d  type: %d", val, Type);
		Log(temp, pUser[conn].AccountName, pUser[conn].IP);
	}

	pUser[conn].NumError = pUser[conn].NumError + val;

	if (pUser[conn].NumError >= 2000000000)
	{
		SendClientMessage(conn, g_pMessageStringTable[_NN_Bad_Network_Packets]);

		CharLogOut(conn);

		sprintf(temp, "cra char logout type: %d", Type);
		Log(temp, pUser[conn].AccountName, pUser[conn].IP);

		return TRUE;
	}

	return FALSE;
}

HFONT__ *GetAFont()
{
	return 0;
}

void ReadConfig(void) // TODO : EVERYTHING
{
	FILE *fp = NULL;
	fp = fopen("gameconfig.txt", "rt");

	if (fp == NULL)
	{
		MessageBox(hWndMain, "no gameconfig.txt, server will generate default setting.", "Error", MB_OK | MB_SYSTEMMODAL);

		ConfigReady = 1;

		DrawConfig(TRUE);

		return;
	}

	char tmp[512];
	char parm[16][32];
	int  value[26];

	fgets(tmp, 511, fp);

	if (strcmp(tmp, "Drop Item Event Settings:\n"))
	{
		MessageBox(hWndMain, "not game-server generated gameconfig.txt - Line1", "Error", MB_OK | MB_SYSTEMMODAL);

		fclose(fp);

		return;
	}

	for (int i = 0; i < 16; i++)
	{
		value[i] = -1;

		memset(parm[i], 0, 32);
	}

	fgets(tmp, 511, fp);

	sscanf(tmp, "%s %d %s %d %s %d %s %d %s %d %s %d %d %d %d", parm[0], &value[0], parm[1], &value[1], parm[2], &value[2], parm[3], &value[3], parm[4], &value[4], parm[5], &value[5], &value[6], &value[7], &value[8]);

	if (strcmp(parm[0], "evindex") || strcmp(parm[1], "evdelete") || strcmp(parm[2], "evon") || strcmp(parm[3], "evitem") || strcmp(parm[4], "evrate") || strcmp(parm[5], "evstart"))
	{
		MessageBox(hWndMain, "not game-server generated gameconfig.txt - Line2", "Error", MB_OK | MB_SYSTEMMODAL);

		fclose(fp);

		return;
	}

	if (value[0] == -1 || value[1] == -1 || value[2] == -1 || value[3] == -1 || value[4] == -1 || value[5] == -1 || value[6] == -1 || value[7] == -1)
	{
		MessageBox(hWndMain, "not game-server generated gameconfig.txt - Line2", "Error", MB_OK | MB_SYSTEMMODAL);

		fclose(fp);

		return;
	}

	evIndex = value[0];
	evDelete = value[1];
	evOn = value[2];
	evItem = value[3];
	evRate = value[4];
	evStartIndex = value[5];
	evCurrentIndex = value[6];
	evEndIndex = value[7];
	evNotice = value[8];

	fgets(tmp, 511, fp);

	if (strcmp(tmp, "Etc Event Settings:\n"))
	{
		MessageBox(hWndMain, "not game server generated gameconfig.txt - Line3", "Error", MB_OK | MB_SYSTEMMODAL);

		fclose(fp);

		return;
	}

	for (int i = 0; i < 16; i++)
	{
		value[i] = -1;

		memset(parm[i], 0, 32);
	}

	fgets(tmp, 511, fp);

	sscanf(tmp, "%s %d %s %d %s %d %s %d %s %d", parm[0], &value[0], parm[1], &value[1], parm[2], &value[2], parm[3], &value[3], parm[4], &value[4]);

	if (strcmp(parm[0], "double") || strcmp(parm[1], "deadpoint") || strcmp(parm[2], "dungeonevent") || strcmp(parm[3], "statsapphire"))
	{
		MessageBox(hWndMain, "not game-server generated gameconfig.txt - Line4", "Error", MB_OK | MB_SYSTEMMODAL);

		fclose(fp);

		return;
	}

	if (value[0] == -1 || value[1] == -1 || value[2] == -1 || value[3] == -1)
	{
		MessageBox(hWndMain, "not game-server generated gameconfig.txt - Line4", "Error", MB_OK | MB_SYSTEMMODAL);

		fclose(fp);

		return;
	}

	DOUBLEMODE = value[0];
	DUNGEONEVENT = value[1];
	DEADPOINT = value[2];
	StatSapphire = value[3];
	BRItem = value[4];

	fgets(tmp, 511, fp);

	if (strcmp(tmp, "Billing Settings:\n"))
	{
		MessageBox(hWndMain, "not game server generated gameconfig.txt - Line5", "Error", MB_OK | MB_SYSTEMMODAL);

		fclose(fp);

		return;
	}

	for (int i = 0; i < 16; i++)
	{
		value[i] = -1;

		memset(parm[i], 0, 32);
	}

	fgets(tmp, 511, fp);

	sscanf(tmp, "%s %d %s %d %s %d %s %d %s %d %s %d", parm[0], &value[0], parm[1], &value[1], parm[2], &value[2], parm[3], &value[3], parm[4], &value[4], parm[5], &value[5]);

	if (strcmp(parm[0], "billmode") || strcmp(parm[1], "freeexp"))
	{
		MessageBox(hWndMain, "not game-server generated gameconfig.txt - Line6", "Error", MB_OK | MB_SYSTEMMODAL);

		fclose(fp);

		return;
	}

	if (value[0] == -1 || value[1] == -1)
	{
		MessageBox(hWndMain, "not game-server generated gameconfig.txt - Line6", "Error", MB_OK | MB_SYSTEMMODAL);

		fclose(fp);

		return;
	}

	BILLING = value[0];
	FREEEXP = value[1];
	CHARSELBILL = value[2];
	POTIONCOUNT = value[3];
	PARTYBONUS = value[4];
	GUILDBOARD = value[5];

	if (PARTYBONUS < 50 || PARTYBONUS > 200)
		PARTYBONUS = 100;

	fgets(tmp, 511, fp);

	if (strcmp(tmp, "Item Drop Bonus Settings:\n"))
	{
		MessageBox(hWndMain, "not game server generated gameconfig.txt - Line7", "Error", MB_OK | MB_SYSTEMMODAL);

		return;
	}

	fgets(tmp, 511, fp);

	sscanf(tmp, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", &value[0], &value[1], &value[2], &value[3], &value[4], &value[5], &value[6], &value[7], &value[8], &value[9], &value[10], &value[11], &value[12], &value[13], &value[14], &value[15]);

	for (int k = 0; k < 16; k++)
	{
		if (value[k] < 0 || value[k] > 9999)
		{
			MessageBox(hWndMain, "not game-server generated gameconfig.txt - Line8", "Error", MB_OK | MB_SYSTEMMODAL);

			fclose(fp);

			return;
		}

		g_pDropBonus[k] = value[k];
		value[k] = -1;
	}

	fgets(tmp, 511, fp);

	sscanf(tmp, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", &value[0], &value[1], &value[2], &value[3], &value[4], &value[5], &value[6], &value[7], &value[8], &value[9], &value[10], &value[11], &value[12], &value[13], &value[14], &value[15]);

	for (int k = 0; k < 16; k++)
	{
		if (value[k] < 0 || value[k] > 9999)
		{
			MessageBox(hWndMain, "not game-server generated gameconfig.txt - Line9", "Error", MB_OK | MB_SYSTEMMODAL);

			fclose(fp);

			return;

		}

		g_pDropBonus[k + 16] = value[k];

		value[k] = -1;
	}

	fgets(tmp, 511, fp);

	sscanf(tmp, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", &value[0], &value[1], &value[2], &value[3], &value[4], &value[5], &value[6], &value[7], &value[8], &value[9], &value[10], &value[11], &value[12], &value[13], &value[14], &value[15]);

	for (int k = 0; k < 16; k++)
	{
		if (value[k] < 0 || value[k] > 9999)
		{
			MessageBox(hWndMain, "not game-server generated gameconfig.txt - Line10", "Error", MB_OK | MB_SYSTEMMODAL);

			fclose(fp);

			return;
		}

		g_pDropBonus[k + 32] = value[k];

		value[k] = -1;
	}

	fgets(tmp, 511, fp);

	sscanf(tmp, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", &value[0], &value[1], &value[2], &value[3], &value[4], &value[5], &value[6], &value[7], &value[8], &value[9], &value[10], &value[11], &value[12], &value[13], &value[14], &value[15]);

	for (int k = 0; k < 16; k++)
	{
		if (value[k] < 0 || value[k] > 9999)
		{
			MessageBox(hWndMain, "not game-server generated gameconfig.txt - Line11", "Error", MB_OK | MB_SYSTEMMODAL);

			fclose(fp);

			return;
		}

		g_pDropBonus[k + 48] = value[k];

		value[k] = -1;
	}

	fgets(tmp, 511, fp);

	if (strcmp(tmp, "Treasure Settings:\n"))
	{
		MessageBox(hWndMain, "not game server generated gameconfig.txt - Line12", "Error", MB_OK | MB_SYSTEMMODAL);

		fclose(fp);

		return;
	}

	for (int k = 0; k < 8; k++)
	{
		STRUCT_TREASURE *t = &g_pTreasure[k];

		memset(t, 0, sizeof(*t));

		for (int i = 0; i < 26; i++)
			value[i] = -1;

		fgets(tmp, 511, fp);

		sscanf(tmp, "%d  %d %d %d %d %d  %d %d %d %d %d  %d %d %d %d %d  %d %d %d %d %d  %d %d %d %d %d",
			&value[0], &value[1], &value[2], &value[3], &value[4], &value[5], &value[6], &value[7], &value[8], &value[9],
			&value[10], &value[11], &value[12], &value[13], &value[14], &value[15], &value[16], &value[17], &value[18], &value[19],
			&value[20], &value[21], &value[22], &value[23], &value[24], &value[25]);

		t->Source = value[0];

		for (int i = 0; i < 5; i++)
		{
			STRUCT_ITEM *it = &t->Target[i];

			it->sIndex = value[1 + i * 5];
			it->stEffect[0].cEffect = value[2 + i * 5];
			it->stEffect[0].cValue = value[3 + i * 5];
			it->stEffect[1].cEffect = value[4 + i * 5];
			it->stEffect[1].cValue = value[5 + i * 5];

		}

		fgets(tmp, 511, fp);

		sscanf(tmp, "%d %d %d %d %d", &value[0], &value[1], &value[2], &value[3], &value[4]);

		for (int i = 0; i < 5; i++)
			t->Rate[i] = value[i];
	}

	fgets(tmp, 511, fp);

	if (strcmp(tmp, "Etc Settings:\n"))
	{
		MessageBox(hWndMain, "not game server generated gameconfig.txt - Line13", "Error", MB_OK | MB_SYSTEMMODAL);

		fclose(fp);

		return;
	}

	for (int i = 0; i < 16; i++)
	{
		value[i] = -1;

		memset(parm[i], 0, 32);
	}

	fgets(tmp, 511, fp);

	sscanf(tmp, "%s %3d %s %3d %s %3d %s %3d %s %3d %s %3d %s %3d %s %3d", parm[0], &value[0], parm[1], &value[1], parm[2], &value[2], parm[3], &value[3], parm[4], &value[4], parm[5], &value[5], parm[6], &value[6], parm[7], &value[7]);

	if (strcmp(parm[0], "partydif") || strcmp(parm[1], "kefrastatus") || strcmp(parm[2], "GTorreHour") || strcmp(parm[3], "RVRHour") || strcmp(parm[4], "DropItem") || strcmp(parm[5], "BRHour") || strcmp(parm[6], "maxNightmare") || strcmp(parm[7], "PotionDelay"))
	{
		MessageBox(hWndMain, "not game-server generated gameconfig.txt - Line13", "Error", MB_OK | MB_SYSTEMMODAL);

		fclose(fp);

		return;
	}

	if (value[0] == -1 || value[1] == -1 || value[2] == -1 || value[3] == -1 || value[4] == -1 || value[5] == -1 || value[6] == -1 || value[7] == -1)
	{
		MessageBox(hWndMain, "not game-server generated gameconfig.txt - Line13", "Error", MB_OK | MB_SYSTEMMODAL);

		fclose(fp);

		return;
	}

	PARTY_DIF = value[0];
	KefraLive = value[1];
	GTorreHour = value[2];
	RvRHour = value[3];
	isDropItem = value[4];
	BRHour = value[5];
	maxNightmare = value[6];
	PotionDelay = value[7];

	fclose(fp);
}

void DrawConfig(int wb)
{
	if (ConfigReady == 0)
		return;

	int x = 0;
	int y = 0;
	HFONT h = 0;
	
	int color = 255;
	int backcolor = 0;

	char String[4096];
	int len = 0;

	hDC = GetDC(hWndMain);

	if (hDC == NULL)
		return;

	if (hFont == 0)
		return;

	if (SelectObject(hDC, hFont) != 0)
		h = (HFONT)SelectObject(hDC, hFont);

	FILE *fp = NULL;

	if (wb)
		fp = fopen("gameconfig.txt", "w+");

	SetTextColor(hDC, color);

	sprintf(String, "Drop Item Event Settings:");
	len = strlen(String);

	TextOutA(hDC, x, y, String, len);

	y += 16;

	if (wb)
		fprintf(fp, "%s\n", String);

	SetTextColor(hDC, backcolor);

	sprintf(String,"   evindex %d   evdelete %d   evon %d   evitem %d   evrate %d   evstart %d %d %d %d",
		evIndex, evDelete, evOn, evItem, evRate, evStartIndex, evCurrentIndex, evEndIndex, evNotice);
	
	len = strlen(String);

	TextOutA(hDC, x, y, String, len);

	y += 16;

	if (wb)
		fprintf(fp, "%s\n", String);

	SetTextColor(hDC, color);

	sprintf(String, "Etc Event Settings:");

	len = strlen(String);
	TextOutA(hDC, x, y, String, len);

	y += 16;

	if (wb)
		fprintf(fp, "%s\n", String);

	SetTextColor(hDC, backcolor);
	sprintf(String, "   double %d   deadpoint %d   dungeonevent %d   statsapphire %d    battleroyal %d",
		DOUBLEMODE, DEADPOINT, DUNGEONEVENT, StatSapphire, BRItem);
	
	len = strlen(String);
	
	TextOutA(hDC, x, y, String, len);

	y += 16;

	if (wb)
		fprintf(fp, "%s\n", String);

	SetTextColor(hDC, color);

	sprintf(String, "Billing Settings:");

	len = strlen(String);
	TextOutA(hDC, x, y, String, len);

	y += 16;

	if (wb)
		fprintf(fp, "%s\n", String);

	SetTextColor(hDC, backcolor);

	sprintf(String, "   billmode %d   freeexp %d   charselbill %d    potioncount %d   partybonus %d   guildboard %d",
		BILLING, FREEEXP, CHARSELBILL, POTIONCOUNT, PARTYBONUS, GUILDBOARD);
	
	len = strlen(String);
	TextOutA(hDC, x, y, String, len);

	y += 16;

	if (wb)
		fprintf(fp, "%s\n", String);

	SetTextColor(hDC, color);

	sprintf(String, "Item Drop Bonus Settings:");

	len = strlen(String);
	TextOutA(hDC, x, y, String, len);
	
	y += 16;

	if (wb)
		fprintf(fp, "%s\n", String);

	SetTextColor(hDC, backcolor);

	for (int i = 0; i < 4; i++)
	{
		int pos = 16 * i;
		sprintf(String, "   %3d %3d %3d %3d %3d %3d %3d %3d   %3d %3d %3d %3d %3d %3d %3d %3d",
			g_pDropBonus[0 + pos], g_pDropBonus[1 + pos], g_pDropBonus[2 + pos], g_pDropBonus[3 + pos],
			g_pDropBonus[4 + pos], g_pDropBonus[5 + pos], g_pDropBonus[6 + pos], g_pDropBonus[7 + pos],
			g_pDropBonus[8 + pos], g_pDropBonus[9 + pos], g_pDropBonus[10 + pos], g_pDropBonus[11 + pos],
			g_pDropBonus[12 + pos], g_pDropBonus[13 + pos], g_pDropBonus[14 + pos], g_pDropBonus[15 + pos]);

		len = strlen(String);
		TextOutA(hDC, x, y, String, len);

		y += 16;

		if (wb)
			fprintf(fp, "%s\n", String);
	}

	SetTextColor(hDC, color);

	sprintf(String, "Treasure Settings:");
	len = strlen(String);
	
	TextOutA(hDC, x, y, String, len);
	y += 16;

	if (wb)
		fprintf(fp, "%s\n", String);

	SetTextColor(hDC, backcolor);
	
	for (int j = 0; j < 8; j++)
	{
		STRUCT_TREASURE *tr = &g_pTreasure[j];

		sprintf(String, "%-4d ", g_pTreasure[j].Source);

		for (int k = 0; k < 5; ++k)
		{
			STRUCT_ITEM *item = &tr->Target[k];

			sprintf(String, "%s  %-4d %2d %2d %2d %2d   ",
				String, item->sIndex, item->stEffect[0].cEffect, item->stEffect[0].cValue, item->stEffect[1].cEffect, item->stEffect[1].cValue);
		}
		len = strlen(String);

		TextOutA(hDC, x, y, String, len);
		y += 16;

		if (wb)
			fprintf(fp, "%s\n", String);
		
		sprintf(String, "       %-5d                %-5d                %-5d                %-5d                %-5d",
			tr->Rate[0], tr->Rate[1], tr->Rate[2], tr->Rate[3], tr->Rate[4]);
		
		len = strlen(String);
		TextOutA(hDC, x, y, String, len);
		y += 16;
		if (wb)
			fprintf(fp, "%s\n", String);
	}

	SetTextColor(hDC, color);

	sprintf(String, "Etc Settings:");

	len = strlen(String);
	TextOutA(hDC, x, y, String, len);

	y += 16;

	if (wb)
		fprintf(fp, "%s\n", String);

	SetTextColor(hDC, backcolor);
	sprintf(String, "   partydif %d   kefrastatus %d   GTorreHour %d    RVRHour %d    DropItem %d   BRHour %d   maxNightmare %d   PotionDelay %d",
		PARTY_DIF, KefraLive, GTorreHour, RvRHour, isDropItem, BRHour, maxNightmare, PotionDelay);

	len = strlen(String);

	TextOutA(hDC, x, y, String, len);

	y += 16;

	if (wb)
		fprintf(fp, "%s\n", String);

	if (hFont && h)
		h = (HFONT)SelectObject(hDC, h);
	
	ReleaseDC(hWndMain, hDC);

	if (wb)
		fclose(fp);
}

void AddFailAccount(char *Account)
{
	for (int i = 0; i < 16; i++)
	{
		if (FailAccount[i][0] == 0)
		{
			strcpy(FailAccount[i], Account);

			return;
		}
	}
}

int  CheckFailAccount(char *Account)
{
	int Num = 0;

	for (int i = 0; i < 16; i++)
	{
		if (FailAccount[i][0] != 0)
		{
			if (strcmp(FailAccount[i], Account) == 0)
				Num++;
		}
	}

	return Num;
}

int  IsFree(STRUCT_SELCHAR *sel)
{
	if (FREEEXP <= 0)
		return 1;

	for (int i = 0; i < MOB_PER_ACCOUNT; i++)
	{
		if (sel->Name[i][0] != 0 && sel->Score[i].Level >= FREEEXP && sel->Score[i].Level < 999)
			return 1;
	}

	return 0;
}

int  SendBilling(int a1, char *a2, int a3, int a4)
{
	return TRUE;
}

int  SendBilling2(_AUTH_GAME *m, int hostlong)
{
	*(u_long*)&m->Unk = htonl(hostlong);

	sprintf(m->Unk, "%d", ServerIndex + 10 * ServerGroup);

	BillServerSocket.SendBillMessage((char*)m);

	return 0;
}

BOOL InitApplication(HANDLE hInstance)
{
	WNDCLASS  wc;

	wc.style = CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS;

	wc.lpfnWndProc = (WNDPROC)MainWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hIcon = (HICON)LoadIcon((HINSTANCE)hInstance, "MAINICON");
	wc.hInstance = (HINSTANCE)hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "MainClass";

	if (!RegisterClass(&wc))
		return RegisterClass(&wc);

	return TRUE;
}

BOOL InitInstance(HANDLE hInstance, int nCmdShow)
{
	hMainMenu = CreateMenu();
	hWndMain = CreateWindow("MainClass", "Game Server",
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,    // Window style. 
		CW_USEDEFAULT, CW_USEDEFAULT, 900, 600,            // W,H[MenuÇÕÄ¡¸é 480]
		NULL, hMainMenu, (HINSTANCE)hInstance, NULL);

	if (!hWndMain)
		return (FALSE);

	ShowWindow(hWndMain, nCmdShow);
	UpdateWindow(hWndMain);

	return TRUE;
}

BOOL WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	MSG msg;


	UNREFERENCED_PARAMETER(lpCmdLine);

	if (!hPrevInstance)
	if (!InitApplication(hInstance))
		return FALSE;

	if (!InitInstance(hInstance, nCmdShow))
		return FALSE;

	hFont = CreateFont(12, 0, 0, 0, FW_LIGHT, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEVICE_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Fixedsys"); // |FF_DECORATIVE

	BASE_InitModuleDir();
	BASE_InitializeHitRate();

	StartLog();
	StartChatLog();

	memset(&g_pTreasure, 0, sizeof(g_pTreasure));

	ReadConfig();
	ReadLevelItemConfig();

	ConfigReady = 1;

	DrawConfig(0);

	memset(g_pGuildWar, 0, sizeof(g_pGuildWar));
	memset(g_pGuildAlly, 0, sizeof(g_pGuildAlly));
	memset(GuildInfo, 0, sizeof(GuildInfo));
	memset(pMac, 0, sizeof(pMac));
	memset(pMobMerc, 0, sizeof(pMobMerc));

	int ret = ListenSocket.WSAInitialize();

	if (!ret)
	{
		Log("erro,não foi possivel iniciar a seguinte função : WSAInitialize", "-system", 0);

		return FALSE;
	}

	Reboot();

	SetCastleDoor(1);

	char name[255];

	FILE* fp = fopen("localip.txt", "rt");

	if (fp)
	{
		fscanf(fp, "%s", name);

		fclose(fp);
	}
	else
		MessageBox(hWndMain, "Não foi possivel encontrar o arquivo LocalIP.txt", "Erro ao iniciar", NULL);

	int r1 = 0, r2 = 0, r3 = 0, r4 = 0;

	sscanf(name, "%d.%d.%d.%d", &r1, &r2, &r3, &r4);

	LocalIP[0] = r1;
	LocalIP[1] = r2;
	LocalIP[2] = r3;
	LocalIP[3] = r4;
	DBServerAddress[0] = 0;

	for (int i = 0; i < MAX_SERVERGROUP; i++)
	{
		for (int j = 1; j < MAX_SERVERNUMBER; j++)
		{
			if (!strcmp(g_pServerList[i][j], name))
			{
				strcpy(DBServerAddress, g_pServerList[i][0]);

				ServerGroup = i;
				ServerIndex = j - 1;

				break;
			}
		}

		if (DBServerAddress[0] == 0)
			continue;
	}

	if (DBServerAddress[0] == 0)
	{
		MessageBox(hWndMain, "Não foi possivel pegar o ServerGroup. LocalIP.txt / ServerList.txt", "Erro ao iniciar", MB_OK | MB_SYSTEMMODAL);

		return TRUE;
	}

	DBServerPort = 7514;

	int *pip = (int*)LocalIP;

	ret = DBServerSocket.ConnectServer(DBServerAddress, 7514, *pip, WSA_READDB);

	if (ret == NULL)
	{
		Log("erro, Não foi possivel se conectar a DBServer", "-system", 0);
		MessageBox(hWndMain, "Não foi possivel se conectar a DBServer", "Erro ao iniciar", NULL);

		return FALSE;
	}

	for (int i = 1; i < MAX_SERVERNUMBER; i++)
	{
		if (g_pServerList[ServerGroup][i][0] != 0)
			NumServerInGroup++;
	}

	if (NumServerInGroup <= 0)
		NumServerInGroup = 1;

	if (NumServerInGroup > 10)
		NumServerInGroup = 10;
	
	if (!strcmp(DBServerAddress, "192.168.0.153"))
		TESTSERVER = 1;

	if (!strcmp(DBServerAddress, "192.168.0.50"))
		LOCALSERVER = 1;

	fp = fopen("biserver.txt", "rt");

	if (fp)
	{
		fscanf(fp, "%s %d", BillServerAddress, &BillServerPort);

		fclose(fp);
	}

	if (BILLING > 0)
	{
		int *pip2 = (int*)LocalIP;

		ret = BillServerSocket.ConnectBillServer(BillServerAddress, BillServerPort, *pip2, WSA_READBILL);

		if (ret == NULL)
		{
			Log("erro, Não foi possivel se conectar na BIServer", "-system", 0);
			BILLING = 0;
		}
		else
		{
			_AUTH_GAME sm;
			
			memset(&sm, 0, sizeof(sm));

			SendBilling2(&sm, 4); // MSG_BillLogin ?
		}
	}

	SetColoseumDoor(1);
	SetColoseumDoor2(3);

	memset(FailAccount, 0, sizeof(FailAccount));

	CurrentTime = timeGetTime();

	CReadFiles::CReadFiles();
	// CEncampment::ReadCamp();
	CCastleZakum::ReadCastleQuest();

	SetTimer(hWndMain, TIMER_SEC, 500, NULL);
	SetTimer(hWndMain, TIMER_MIN, 12000, NULL);

	ListenSocket.StartListen(hWndMain, *pip, GAME_PORT, WSA_ACCEPT);

	GuildZoneReport();

	if (KefraLive == 0)
	{
		for (int k = KEFRA_MOB_INITIAL; k < KEFRA_MOB_END; k++)
			GenerateMob(k, 0, 0);

		GenerateMob(KEFRA_BOSS, 0, 0);
	}

	//mNPCGen.ReadRegion();
	//mNPCGen.DropList();
	//mNPCGen.LevelList();

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return	msg.wParam;
}

void BuildList()
{

}

LONG APIENTRY MainWndProc(HWND hWnd, UINT message, UINT wParam, LONG lParam)
{
	switch (message)
	{

	case WM_TIMER:
	{
		if (wParam == TIMER_SEC)
			ProcessSecTimer();
		else if (wParam == TIMER_MIN)
			ProcessMinTimer();
	} break;

	case WSA_READBILL:
	{
		if (WSAGETSELECTEVENT(lParam) != FD_READ)
		{
			Log("err Billing Connection lost.", "-system", 0);

			BillServerSocket.CloseSocket();
			BillCounter = 360;

			Log("err,billing disconnected.", "-system", 0);

			return TRUE;
		}

		int ret = BillServerSocket.Receive();

		if (ret != TRUE)
		{
			Log("err bill receive fail", "-system", 0);

			BillServerSocket.CloseSocket();
			BillCounter = 360;

			return TRUE;
		}

		if (ret == FALSE)
		{
			ret = WSAGetLastError();

			sprintf(temp, "err,socket %d %d-%d-%d", ret, BillServerSocket.nRecvPosition, BillServerSocket.nProcPosition, BillServerSocket.nSendPosition);

			ret = BillServerSocket.Receive();

			if (ret != TRUE)
			{
				Log("err,bill receive retry_recv fail", "-system", 0);

				BillServerSocket.CloseSocket();
				BillCounter = 360;

				break;
			}
		}
		else if (ret == -1)
		{
			sprintf(temp, "clo,bill receive %d-%d-%d", BillServerSocket.nRecvPosition, BillServerSocket.nProcPosition, BillServerSocket.nSendPosition);
			Log(temp, "-system", 0);

			ret = recv(BillServerSocket.Sock, BillServerSocket.pRecvBuffer, RECV_BUFFER_SIZE, NULL);

			BillServerSocket.CloseSocket();
			BillCounter = 360;

			break;
		}

		int Error = 0;
		int ErrorCode = 0;

		while (1)
		{
			char *Msg = BillServerSocket.ReadBillMessage(&Error, &ErrorCode);

			if (Msg == NULL)
				break;

			if (Error == 1 || Error == 2)
			{
				sprintf(temp, "err readbill (%d),%d", Error, ErrorCode);
				Log(temp, "-system", 0);

				break;
			}

			ProcessBILLMessage(Msg);

		};

	}  break;

	case WSA_READDB:
	{
		int t = WSAGETSELECTEVENT(lParam);
		if (WSAGETSELECTEVENT(lParam) != FD_READ)
		{
			Log("err wsareaddb fail", "-system", 0);

			DBServerSocket.CloseSocket();

			int ret = 0;

			for (int i = 0; i < 2; i++)
			{
				int *pip = (int*)LocalIP;

				ret = DBServerSocket.ConnectServer(DBServerAddress, DBServerPort, *pip, WSA_READDB);

				if (ret != NULL)
					break;

				Sleep(200);
			}

			if (ret == 0)
			{
				Log("reconnect DB fail.", "-system", 0);

				PostQuitMessage(NULL);

				return TRUE;

			}

			Sleep(200);
			PostQuitMessage(NULL);
			return TRUE;
		}

		if (DBServerSocket.Receive() == FALSE)
		{
			Log("err wsareaddb fail", "-system", 0);

			DBServerSocket.CloseSocket();

			int ret = 0;

			for (int i = 0; i < 2; i++)
			{
				int *pip = (int*)LocalIP;

				ret = DBServerSocket.ConnectServer(DBServerAddress, DBServerPort, *pip, WSA_READDB);

				if (ret != NULL)
					break;

				Sleep(200);
			}

			if (ret == 0)
			{
				Log("reconnect DB fail.", "-system", 0);

				PostQuitMessage(NULL);

				return TRUE;

			}
			PostQuitMessage(NULL);
			return TRUE;

		}

		int Error = 0;
		int ErrorCode = 0;

		while (1)
		{
			char *Msg = DBServerSocket.ReadMessage(&Error, &ErrorCode);

			if (Msg == NULL)
				break;


#ifdef  _PACKET_DEBUG
			MSG_STANDARD *debug = (MSG_STANDARD*)Msg;

			if (BASE_CheckPacket(debug))
			{
				sprintf(temp, "**PACKET_DEBUG** Type:%d Size:%d", debug->Type, debug->Size);
				Log(temp, "-system", 0);

				break;
			}
#endif

			if (Error == 1 || Error == 2)
			{
				sprintf(temp, "err readdb (%d),%d", Error, ErrorCode);
				Log(temp, "-system", 0);
				break;
			}

			ProcessDBMessage(Msg);
		};

	} break;

	case  WSA_READ:
	{
		int User = GetUserFromSocket(wParam);
		if (User == 0)
		{
			closesocket(wParam);

			break;
		}

		if (WSAGETSELECTEVENT(lParam) != FD_READ)
		{
			sprintf(temp, "clo,fd %d-%d", pUser[User].Mode, pMob[User].Mode);
			Log(temp, pUser[User].AccountName, pUser[User].IP);

			CloseUser(User);
			break;
		}

		int ret = pUser[User].cSock.Receive();

		if (ret == FALSE)
		{
			ret = WSAGetLastError();

			sprintf(temp, "err,socket %d %d-%d-%d %d-%d", ret, pUser[User].cSock.nRecvPosition, pUser[User].cSock.nProcPosition, pUser[User].cSock.nSendPosition, pUser[User].Mode, pMob[User].Mode);

			ret = pUser[User].cSock.Receive();

			if (ret == FALSE)
			{
				Log("err,retry_recv fail", pUser[User].AccountName, pUser[User].IP);

				CloseUser(User);

				break;
			}
		}
		else if (ret == -1)
		{
			sprintf(temp, "clo,receive %d-%d-%d %d-%d", pUser[User].cSock.nRecvPosition, pUser[User].cSock.nProcPosition, pUser[User].cSock.nSendPosition, pUser[User].Mode, pMob[User].Mode);

			Log("clo,receive %d-%d-%d", pUser[User].AccountName, pUser[User].IP);

			ret = recv(pUser[User].cSock.Sock, pUser[User].cSock.pRecvBuffer, RECV_BUFFER_SIZE, NULL);

			CloseUser(User);

			break;
		}

		int Error = 0;
		int ErrorCode = 0;

		while (1)
		{
			char *Msg = pUser[User].cSock.ReadMessage(&Error, &ErrorCode);

			if (Msg == NULL)
				break;

#ifdef  _PACKET_DEBUG
			MSG_STANDARD *debug = (MSG_STANDARD*)Msg;

			if (Msg != NULL && BASE_CheckPacket(debug))
			{
				sprintf(temp, "**PACKET_DEBUG** Type:%d Size:%d", debug->Type, debug->Size);
				Log(temp, "-system", 0);
			}
#endif

			if (Error == 1 || Error == 2)
			{
				pUser[User].AccountName[ACCOUNTNAME_LENGTH - 1] = 0;
				pUser[User].AccountName[ACCOUNTNAME_LENGTH - 2] = 0;

				sprintf(temp, "err,read (%d),%d", Error, ErrorCode);
				Log(temp, pUser[User].AccountName, pUser[User].IP);

				break;
			}

			ProcessClientMessage(User, Msg, FALSE);
		};

	} break;

	case WSA_ACCEPT:
	{
		if (WSAGETSELECTERROR(lParam) == 0)
		{

			int User = GetEmptyUser();

			if (User == 0)
			{
				Log("err,accept fail - no empty", "-system", 0);

				break;
			}

			int ret = pUser[User].AcceptUser(ListenSocket.Sock);

			if (User >= MAX_USER - ADMIN_RESERV)
			{
				SendClientMessage(User, g_pMessageStringTable[_NN_Reconnect]);
				pUser[User].cSock.SendMessageA();

				CloseUser(User);

				return TRUE;
			}

			if (ServerDown != -1000)
			{
				SendClientMessage(User, g_pMessageStringTable[_NN_ServerReboot_Cant_Connect]);

				CloseUser(User);

				return TRUE;
			}

		}
		else
			Log("err,- accept - wsagetselecterror", "-system", 0);

	} break;

	case WM_PAINT:
		PAINTSTRUCT ps;
		BeginPaint(hWnd, &ps);
		DrawConfig(0);

		EndPaint(hWnd, &ps);
		break;

	case WM_CREATE:
	{
		HMENU hMenu, hSubMenu;
		// HICON hIcon, hIconSm;

		hMenu = CreateMenu();

		hSubMenu = CreatePopupMenu();
		AppendMenu(hSubMenu, MF_STRING, IDC_EXIT, "&Exit");
		AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT)hSubMenu, "&File");

		hSubMenu = CreatePopupMenu();
		AppendMenu(hSubMenu, MF_STRING, IDC_MOBRELOAD, "&ReloadMob");
		AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT)hSubMenu, "&Mob");

		hSubMenu = CreatePopupMenu();
		AppendMenu(hSubMenu, MF_STRING, IDC_REBOOT, "&Reboot");
		AppendMenu(hSubMenu, MF_STRING, IDC_SAVEALL, "&SaveAll");
		AppendMenu(hSubMenu, MF_STRING, IDC_READGUILD, "&ReadGuild");
		AppendMenu(hSubMenu, MF_STRING, IDC_READGUILDNAME, "&ReadGuildName");
		AppendMenu(hSubMenu, MF_STRING, IDC_READGAMECONFIG, "&ReadGameConfig");
		AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT)hSubMenu, "&Sistema");

		SetMenu(hWnd, hMenu);
	} break;

	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case IDC_EXIT:
			PostMessage(hWnd, WM_CLOSE, 0, 0);
			break;

		case IDC_MOBRELOAD:
			mNPCGen.ReadNPCGenerator();
			break;

		case IDC_REBOOT:
			ServerDown = 1;
			break;

		case IDC_SAVEALL:
			SaveAll();
			break;

		case IDC_READGUILD:
			CReadFiles::ReadGuild();
			break;

		case IDC_READGUILDNAME:
			BASE_InitializeGuildName();
			break;

		case IDC_READGAMECONFIG:
		{
			ReadConfig();
			DrawConfig(0);
		} break;
		}
	} break;
	case WM_CLOSE:
	{
		if (MessageBox(hWndMain, "Você realmente deseja desligar o server?", "Aviso!!!", MB_ICONQUESTION | MB_YESNO) == IDNO)
			break;

		if (BILLING != 0)
		{
			_AUTH_GAME sm;
			memset(&sm, 0, sizeof(sm));
			SendBilling2(&sm, 4);

			BILLING = 0;
		}

		CReadFiles::WriteGuild();

		if (fLogFile)
			fclose(fLogFile);

		if (hFont)
		{
			DeleteObject(hFont);
			hFont = NULL;
		}

		DefWindowProc(hWnd, message, wParam, lParam);

	} break;

	case WM_DESTROY:
		WSACleanup();
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

void ProcessBILLMessage(char *pMsg)
{
}

void CheckIdle(int conn)
{
	int ser = SecCounter;
	int lst = pUser[conn].LastReceiveTime;

	if (lst > ser)
		pUser[conn].LastReceiveTime = SecCounter;

	if (lst < ser - 1440)
		pUser[conn].LastReceiveTime = SecCounter;

	if (lst < ser - 720)
	{
		sprintf(temp, "sys,disconnect last:%d server:%d mode:%d conn:%d", pUser[conn].LastReceiveTime, SecCounter, pUser[conn].Mode, conn);
		Log(temp, pUser[conn].AccountName, 0);

		CloseUser(conn);
	}
}

void StartLog()
{
	if (fLogFile != NULL)
		fclose(fLogFile);

	struct tm when;
	time_t now;
	time(&now);
	when = *localtime(&now);

	sprintf(temp, ".\\Log\\TM_%02d_%02d_%04d_%02d_%02d_%02d.txt", when.tm_mday, when.tm_mon + 1, when.tm_year + 1900, when.tm_hour, when.tm_min, when.tm_sec);

	fLogFile = fopen(temp, "wt");

	LastLogDay = when.tm_mday;
}

void CrackLog(int conn, char *str)
{
	char AccountName[16];

	if (conn <= 0 || conn >= MAX_USER)
	{
		if (conn < 0 || conn >= MAX_MOB)
			sprintf(temp, "index:%d", conn);

		else
			sprintf(temp, "mob:%d name:%s", conn, pMob[conn].MOB.MobName);
	}

	else
	{
		memcpy(AccountName, pUser[conn].AccountName, sizeof(pUser[conn].AccountName));

		pUser[conn].AccountName[15] = 0;
		pUser[conn].AccountName[14] = 0;

		sprintf(temp, "conn:%d mode:%d name:%s", conn, pUser[conn].Mode, AccountName);
	}

	strcat(temp, str);

	Log(temp, "-system", 0);

	SetWindowTextA(hWndMain, temp);
}

void Log(char *str1, char *str2, unsigned int ip)
{
	struct tm when;
	time_t now;
	time(&now);
	when = *localtime(&now);

	char LogTemp[1024];

	unsigned char *cIP = (unsigned char*)&ip;

	//sprintf(LogTemp, "%2.2d%2.2d%2.2d,%2.2d%2.2d%2.2d,%8.8x", when.tm_year - 100, when.tm_mon + 1, when.tm_mday, when.tm_hour, when.tm_min, when.tm_sec, ip);
	//sprintf(LogTemp, "%s,%s,%s \n", LogTemp, str2, str1);

	if (ip != 0)
		sprintf(LogTemp, "[%02d/%02d/%04d][%02d:%02d:%02d] IP: %d.%d.%d.%d", when.tm_mday, when.tm_mon + 1, when.tm_year + 1900, when.tm_hour, when.tm_min, when.tm_sec, cIP[0], cIP[1], cIP[2], cIP[3]);
	else
		sprintf(LogTemp, "[%02d/%02d/%04d][%02d:%02d:%02d] ", when.tm_mday, when.tm_mon + 1, when.tm_year + 1900, when.tm_hour, when.tm_min, when.tm_sec);
	
	sprintf(LogTemp, "%s %s %s \n", LogTemp, str2, str1);

	if (fLogFile)
		fprintf(fLogFile, LogTemp);

	SetWindowText(hWndMain, LogTemp);
	
}

int  GetUserFromSocket(int Sock)
{
	if (Sock == 0)
		return 0;

	for (int i = 1; i < MAX_USER; i++)
	{
		if (pUser[i].cSock.Sock == (unsigned)Sock)
			return i;
	}

	return 0;
}

int  GetEmptyUser(void)
{
	for (int i = 1; i < MAX_USER; i++)
	{
		if (pUser[i].Mode == USER_EMPTY)
			return i;
	}

	return 0;
}

int  GetEmptyItem()
{
	for (int i = 1; i < MAX_ITEM; i++)
	{
		if (pItem[i].Mode == 0)
			return i;
	}

	return 0;
}

int  GetEmptyNPCMob()
{
	if (g_EmptyMob >= MAX_MOB)
		g_EmptyMob = MAX_USER;

	int MaxCount = MAX_MOB - MAX_USER;
	int EmptyMob = g_EmptyMob;

	for (int i = 0; i < MaxCount; i++, EmptyMob++)
	{
		if (EmptyMob >= MAX_MOB)
		{
			EmptyMob = MAX_USER;

			continue;
		}

		if (pMob[EmptyMob].Mode == MOB_EMPTY)
		{
			g_EmptyMob = EmptyMob;

			return EmptyMob;
		}
	}

	return 0;
}

int  GetUserByName(char *name)
{
	if (name[0] == '+')
	{
		char *tname = name + 1;

		_strupr(tname);

		for (int i = 1; i < MAX_USER; i++)
		{
			if (pMob[i].Mode == MOB_EMPTY)
				continue;

			if (pUser[i].Mode != USER_PLAY)
				continue;

			if (!strcmp(tname, pUser[i].AccountName))
				return i;
		}
		return 0;
	}

	for (int i = 1; i < MAX_USER; i++)
	{
		if (pMob[i].Mode == MOB_EMPTY)
			continue;

		if (pUser[i].Mode != USER_PLAY)
			continue;

		if (!strcmp(name, pMob[i].MOB.MobName))
			return i;
	}
	return 0;
}

int  IsImple(int user)
{
	return 0;
}

void CloseUser(int conn)
{
	int unk = 0;
	int unk2 = 0;

	if (pUser[conn].Mode == USER_PLAY && pMob[conn].TargetX >= 0 && pMob[conn].TargetX < 4096
		&& pMob[conn].TargetY >= 0 && pMob[conn].TargetY < 4096)
		pMobGrid[pMob[conn].TargetY][pMob[conn].TargetX] = 0;

	pUser[conn].Admin = 0;

	if (pUser[conn].IsBillConnect)
		SendBilling(conn, pUser[conn].AccountName, 2, 0);

	pUser[conn].cSock.CloseSocket();

	int Mode = pUser[conn].Mode;

	if (Mode && Mode != USER_ACCEPT)
	{
		if (Mode != USER_PLAY && Mode != USER_SAVING4QUIT)
		{
			MSG_STANDARD sm;
			memset(&sm, 0, sizeof(MSG_STANDARD));

			sm.Type = _MSG_DBNoNeedSave;
			sm.ID = conn;

			DBServerSocket.SendOneMessage((char*)&sm, sizeof(MSG_STANDARD));

			pMob[conn].Mode = USER_EMPTY;

			pUser[conn].CloseUser();
		}

		else
		{
			RemoveParty(conn);

			int tradeOpponentID = pUser[conn].Trade.OpponentID;

			if (tradeOpponentID > 0 && tradeOpponentID < MAX_USER && pUser[tradeOpponentID].Mode == USER_PLAY && pUser[tradeOpponentID].Trade.OpponentID == conn)
				RemoveTrade(tradeOpponentID);

			pUser[conn].Trade.OpponentID = 0;

			MSG_SavingQuit sm;
			memset(&sm, 0, sizeof(MSG_SavingQuit));

			sm.Type = _MSG_SavingQuit;

			if (conn < 0 || conn > MAX_MOB)
			{
				CrackLog(conn, " SaveAndQuitMobidx");
				return;
			}

			int Slot = pUser[conn].Slot;

			if (Slot < 0 || Slot >= MAX_CLASS)
			{
				CrackLog(conn, " SaveAndQuitWrongSlot");
				return;
			}

			memcpy(&sm.MOB, &pMob[conn], sizeof(STRUCT_MOB));
			memcpy(sm.Cargo, &pUser[conn].Cargo, sizeof(pUser[conn].Cargo));
			memcpy(sm.ShortSkill, &pUser[conn].CharShortSkill, 16);

			memcpy(sm.affect, pMob[conn].Affect, sizeof(sm.affect));
			sm.extra = pMob[conn].extra;

			strncpy((char*)&sm.AccountName, pUser[conn].AccountName, ACCOUNTNAME_LENGTH);

			sm.Coin = pUser[conn].Coin;
			sm.Donate = pUser[conn].Donate;

			sm.Slot = pUser[conn].Slot;

			sm.ID = conn;

			DBServerSocket.SendOneMessage((char*)&sm, sizeof(MSG_SavingQuit));

			pUser[conn].Mode = USER_SAVING4QUIT;

			DeleteMob(conn, 2);
		}

	}

	else
		pUser[conn].CloseUser();
}

CMob *GetMobFromIndex(unsigned int index)
{
	if (index < 0 || index >= MAX_MOB)
		return NULL;

	return &pMob[index];
}

int  GetAngle(int x, int y)
{
	if (x == 0 || y == 0)
		return 0;

	double tangle = atan2((FLOAT)y, (FLOAT)x);

	int angle = (int)((tangle * 3600) / 6.283184);

	if (angle < 0)
		angle += 3600;

	if (angle > 3600)
		angle -= 3600;

	return angle;
}

int  GetLength(int x, int y)
{
	FLOAT valx = (FLOAT)(x * x);
	FLOAT valy = (FLOAT)(y * y);

	FLOAT valxy = valx + valy;

	if (valxy > 100)
		return 100;

	if (valxy <= 0)
		return 0;

	double fout = sqrt(valxy);

	int out = (int)fout;

	return out;
}

void SaveUser(int conn, int Export)
{
	MSG_DBSaveMob sm;
	memset(&sm, 0, sizeof(MSG_DBSaveMob));

	sm.Type = _MSG_DBSaveMob;

	memcpy(&sm.MOB, &pMob[conn].MOB, sizeof(STRUCT_MOB));

	memcpy(&sm.Cargo, pUser[conn].Cargo, sizeof(STRUCT_ITEM)* MAX_CARGO);

	sm.Coin = pUser[conn].Coin;

	sm.ID = conn;
	sm.Export = Export;

	sm.Slot = pUser[conn].Slot;
	sm.Donate = pUser[conn].Donate;

	strncpy(sm.AccountName, pUser[conn].AccountName, ACCOUNTNAME_LENGTH);

	memcpy(sm.ShortSkill, pUser[conn].CharShortSkill, 16);

	memcpy(sm.affect, pMob[conn].Affect, sizeof(sm.affect));
	sm.extra = pMob[conn].extra;

	DBServerSocket.SendOneMessage((char*)&sm, sizeof(MSG_DBSaveMob));
}

void CharLogOut(int conn)
{
	if(conn < 0 || conn >= MAX_USER)
	{
		Log("err,char logout - not conn valid", "-system", 0);
		return;
	}
	if (pUser[conn].Mode != USER_PLAY)
	{
		Log("err,char logout - not user_play", pUser[conn].AccountName, pUser[conn].IP);

		SendClientSignal(conn, conn, _MSG_CNFCharacterLogout);

		return;
	}

	if (pUser[conn].IsBillConnect && CHARSELBILL == 0)
		SendBilling(conn, pUser[conn].AccountName, 2, 0);

	int trade = pUser[conn].Trade.OpponentID;

	if (trade > 0 && trade < MAX_USER)
	{
		if (pUser[trade].Mode == USER_PLAY && pUser[trade].Trade.OpponentID == conn)
			RemoveTrade(trade);
	}


	pUser[conn].Mode = USER_SELCHAR;

	pUser[conn].SelChar.Score[pUser[conn].Slot].Level = pMob[conn].MOB.BaseScore.Level;

	SaveUser(conn, 1);
	DeleteMob(conn, 2);

	pMob[conn].TargetX = pMob[conn].MOB.SPX;
	pMob[conn].TargetY = pMob[conn].MOB.SPY;
	pMob[conn].Mode = 0;

	SendClientSignal(conn, conn, _MSG_CNFCharacterLogout);
}

void Reboot()
{
	for (int y = 0; y < MAX_GRIDY; y++)
	{
		for (int x = 0; x < MAX_GRIDX; x++)
		{
			pMobGrid[y][x] = 0;
			pItemGrid[y][x] = 0;
			pHeightGrid[y][x] = 0;
		}
	}

	for (int i = 0; i < 3; i++)
		LiveTower[i] = 0;

	BASE_InitializeMobname("MobName.txt", 0);

	mNPCGen.ReadNPCGenerator();

	int Handle = _open("./heightmap.dat", _O_RDONLY | _O_BINARY, NULL);

	if (Handle == -1)
	{
		Log("err Load height grid fail", "-system", 0);

		return;
	}

	_read(Handle, (void*)pHeightGrid, sizeof(pHeightGrid));
	_close(Handle);

	BASE_InitializeBaseDef();
	BASE_ApplyAttribute((char*)pHeightGrid, MAX_GRIDX);
	BASE_InitializeMessage("Language.txt");

	mSummon.Initialize();

	Log("sys,start server", "-system", 0);

	for (int i = 0; i < g_dwInitItem; i++)
	{
		STRUCT_ITEM Item;
		memset(&Item, 0, sizeof(STRUCT_ITEM));

		Item.sIndex = g_pInitItem[i].ItemIndex;

		int ID = CreateItem(g_pInitItem[i].PosX, g_pInitItem[i].PosY, &Item, g_pInitItem[i].Rotate, 1);

		if (ID >= MAX_ITEM || ID <= 0)
			continue;

		int ret = BASE_GetItemAbility(&Item, EF_KEYID);

		if (ret != 0 && ret < 15)
		{
			int Height;

			UpdateItem(ID, STATE_LOCKED, &Height);
		}
	}

	WeekMode = 4;

	struct tm when;
	time_t now;
	time(&now);
	when = *localtime(&now);

	int GDay = GuildDay - 1;

	if (GDay < 0)
		GDay += 7;

	if (when.tm_wday == GDay)
		WeekMode = 5;

	if (when.tm_wday == GuildDay && when.tm_hour < GuildHour)
		WeekMode = 0;

	SetColoseumDoor(1);
}

void RemoveTrade(int conn)
{
	if (conn <= 0 || conn >= MAX_USER)
		return;

	memset(&pUser[conn].Trade, 0, sizeof(MSG_Trade));

	for (int i = 0; i < MAX_TRADE; i++)
		pUser[conn].Trade.InvenPos[i] = -1;

	memset(&pUser[conn].AutoTrade, 0, sizeof(MSG_SendAutoTrade));

	for (int i = 0; i < MAX_AUTOTRADE; i++)
		pUser[conn].AutoTrade.CarryPos[i] = -1;

	if (pUser[conn].Mode != USER_PLAY)
		return;

	SendClientSignal(conn, conn, 900);

	if (pUser[conn].TradeMode)
	{
		int posX = pMob[conn].TargetX;
		int posY = pMob[conn].TargetY;

		MSG_CreateMob sm;
		memset(&sm, 0, sizeof(MSG_CreateMob));

		GetCreateMob(conn, &sm);
		GridMulticast(posX, posY, (MSG_STANDARD*)&sm, 0);

		pUser[conn].TradeMode = 0;
	}
}

void DoRemoveHide(int conn)
{
	if (conn <= 0 || conn >= MAX_USER)
		return;

	for (int i = 0; i < MAX_AFFECT; i++)
	{
		if (pMob[conn].Affect[i].Type != 28)
			continue;

		pMob[conn].Affect[i].Type = 0;
		pMob[conn].Affect[i].Value = 0;
		pMob[conn].Affect[i].Level = 0;
		pMob[conn].Affect[i].Time = 0;

		pMob[conn].GetCurrentScore(conn);
		SendScore(conn);

		break;
	}
}

void DoRemoveSamaritano(int conn)
{
	if (conn <= 0 || conn >= MAX_USER)
		return;

	for (int i = 0; i < MAX_AFFECT; i++)
	{
		if (pMob[conn].Affect[i].Type != 24)
			continue;

		pMob[conn].Affect[i].Type = 0;
		pMob[conn].Affect[i].Value = 0;
		pMob[conn].Affect[i].Level = 0;
		pMob[conn].Affect[i].Time = 0;

		pMob[conn].GetCurrentScore(conn);
		SendScore(conn);

		break;
	}
}

void DoTeleport(int mob, int x, int y)
{
	MSG_Action sm;
	memset(&sm, 0, sizeof(MSG_Action));

	int ret = GetEmptyMobGrid(mob, &x, &y);

	if (ret)
	{
		GetAction(mob, x, y, &sm);

		sm.Effect = 1; // teleport effect

		if (mob < MAX_USER)
			pUser[mob].cSock.AddMessage((char*)&sm, sizeof(MSG_Action));

		GridMulticast(mob, x, y, (MSG_STANDARD*)&sm);
	}
}

void DoSummon(int target, int x, int y)
{
	MSG_Action sm;

	memset(&sm, 0, sizeof(MSG_Action));

	int tret = GetEmptyMobGrid(target, &x, &y);

	GetAction(target, x, y, &sm);

	sm.Effect = 1; // teleport effect

	if (target < MAX_USER)
		pUser[target].cSock.SendOneMessage((char*)&sm, sizeof(sm));

	GridMulticast(target, x, y, (MSG_STANDARD*)&sm);
}

void DoGreatTeleport(int mob, int x, int y)
{
	MSG_Action sm;
	memset(&sm, 0, sizeof(MSG_Action));

	int ret = GetEmptyMobGridGreat(mob, &x, &y);

	if (ret)
	{
		GetAction(mob, x, y, &sm);

		sm.Effect = 1; // teleport effect

		if (mob < MAX_USER)
			pUser[mob].cSock.SendOneMessage((char*)&sm, sizeof(sm));

		GridMulticast(mob, x, y, (MSG_STANDARD*)&sm);
	}
}

int ApplyHp(int conn)
{
	int retnValue = FALSE;

	if(pUser[conn].ReqHp > pMob[conn].MOB.CurrentScore.MaxHp)
		pUser[conn].ReqHp = pMob[conn].MOB.CurrentScore.MaxHp;

	int hp = pMob[conn].MOB.CurrentScore.Hp;
	int reqhp = pUser[conn].ReqHp;

	if(reqhp > hp)
	{
		int hpdif = reqhp - hp;
		if (reqhp - hp > 2000)
			hpdif = 2000;

		hp += hpdif;

		if (hp > 2000000000)
			hp = 2000000000;

		pMob[conn].MOB.CurrentScore.Hp = hp;

		if(pMob[conn].MOB.CurrentScore.Hp > reqhp)
			pMob[conn].MOB.CurrentScore.Hp = reqhp;

		retnValue = TRUE;
	}
	else
		retnValue = FALSE;

	return retnValue;
}

int ApplyMp(int conn)
{
	int retnValue = FALSE;

	if (pUser[conn].ReqMp > pMob[conn].MOB.CurrentScore.MaxMp)
		pUser[conn].ReqMp = pMob[conn].MOB.CurrentScore.MaxMp;

	int mp = pMob[conn].MOB.CurrentScore.Mp;
	int reqmp = pUser[conn].ReqMp;

	if (reqmp > mp)
	{
		int mpdif = reqmp - mp;
		if (reqmp - mp > 2000)
			mpdif = 2000;

		mp += mpdif;

		if (mp > 2000000000)
			mp = 2000000000;

		pMob[conn].MOB.CurrentScore.Mp = mp;

		if (pMob[conn].MOB.CurrentScore.Mp > reqmp)
			pMob[conn].MOB.CurrentScore.Mp = reqmp;

		retnValue = TRUE;
	}
	else
		retnValue = FALSE;

	return retnValue;
}

void SetReqHp(int conn)
{
	if (conn <= 0 || conn >= MAX_USER)
		return;

	if (pUser[conn].ReqHp < 0)
		pUser[conn].ReqHp = 0;

	if (pMob[conn].MOB.CurrentScore.Hp > pMob[conn].MOB.CurrentScore.MaxHp)
		pMob[conn].MOB.CurrentScore.Hp = pMob[conn].MOB.CurrentScore.MaxHp;

	if (pUser[conn].ReqHp < pMob[conn].MOB.CurrentScore.Hp)
		pUser[conn].ReqHp = pMob[conn].MOB.CurrentScore.Hp;
}

void SetReqMp(int conn)
{
	if (conn <= 0 || conn >= MAX_USER)
		return;

	if (pUser[conn].ReqMp < 0)
		pUser[conn].ReqMp = 0;

	if (pMob[conn].MOB.CurrentScore.Mp > pMob[conn].MOB.CurrentScore.MaxMp)
		pMob[conn].MOB.CurrentScore.Mp = pMob[conn].MOB.CurrentScore.MaxMp;

	if (pUser[conn].ReqMp < pMob[conn].MOB.CurrentScore.Mp)
		pUser[conn].ReqMp = pMob[conn].MOB.CurrentScore.Mp;
}

void ReadLevelItemConfig(void)
{
	FILE *fp = NULL;

	fp = fopen("LevelItem.txt", "rt");

	if (fp == NULL)
	{
		MessageBox(hWndMain, "no LevelItem.txt", "Error", MB_OK | MB_SYSTEMMODAL);

		return;
	}

	memset(&LevelItem, 0, sizeof(LevelItem));

	while(1)
	{
		char tmp[256];
		char *ret = fgets(tmp, 256, fp);

		if(ret == NULL)
			break;

		int cls = 0;
		int type = 0;
		int level = 0;
		STRUCT_ITEM Item;
		memset(&Item, 0, sizeof(STRUCT_ITEM));

		int ival1 = 0, ival2 = 0, ival3 = 0;
		int ival4 = 0, ival5 = 0, ival6 = 0, ival7 = 0;

		sscanf(tmp, "%d %d %d %d %d %d %d %d %d %d", &cls, &type, &level, &ival1, &ival2, &ival3, &ival4, &ival5, &ival6, &ival7);

		Item.sIndex = ival1;
		Item.stEffect[0].cEffect = ival2;
		Item.stEffect[0].cValue = ival3;
		Item.stEffect[1].cEffect = ival4;
		Item.stEffect[1].cValue = ival5;
		Item.stEffect[2].cEffect = ival6;
		Item.stEffect[2].cValue = ival7;

		if(cls == 4 && type != 4)//Item para todas as classes mais não para todas build
		{
			LevelItem[0][type][level] = Item;
			LevelItem[1][type][level] = Item;
			LevelItem[2][type][level] = Item;
			LevelItem[3][type][level] = Item;
		}

		else if(cls == 4 && type == 4)//Item para todas as classes e todas as build
		{
			for(int j = 0; j < 4; j++)
			{
				for(int k = 0; k < 4; k++)
					LevelItem[j][k][level] = Item;
			}
			
		}

		else if(type == 4)//Item para todas as build
		{
			LevelItem[cls][0][level] = Item;
			LevelItem[cls][1][level] = Item;
			LevelItem[cls][2][level] = Item;
			LevelItem[cls][3][level] = Item;
		}

		else
			LevelItem[cls][type][level] = Item;
	}

	fclose(fp);
}

void StartChatLog()
{
	if (fChatLogFile != NULL)
		fclose(fChatLogFile);

	struct tm when;
	time_t now;
	time(&now);
	when = *localtime(&now);

	sprintf(temp, ".\\chat_log\\TM_%02d_%02d_%04d_%02d_%02d_%02d.txt", when.tm_mday, when.tm_mon + 1, when.tm_year + 1900, when.tm_hour, when.tm_min, when.tm_sec);

	fChatLogFile = fopen(temp, "wt");

	LastChatLogDay = when.tm_mday;
}

void ChatLog(char *str1, char *str2, unsigned int ip)
{
	struct tm when;
	time_t now;
	time(&now);
	when = *localtime(&now);

	char LogTemp[1024];

	unsigned char *cIP = (unsigned char*)&ip;

	if (ip != 0)
		sprintf(LogTemp, "%02d.%02d.%04d.%02d.%02d.%02d IP: %d.%d.%d.%d", when.tm_mday, when.tm_mon + 1, when.tm_year + 1900, when.tm_hour, when.tm_min, when.tm_sec, cIP[0], cIP[1], cIP[2], cIP[3]);
	else
		sprintf(LogTemp, "%02d.%02d.%04d.%02d.%02d.%02d ", when.tm_mday, when.tm_mon + 1, when.tm_year + 1900, when.tm_hour, when.tm_min, when.tm_sec);
	
	sprintf(LogTemp, "%s %s %s \n", LogTemp, str2, str1);

	if (fChatLogFile)
		fprintf(fChatLogFile, LogTemp);
}

void StartItemLog()
{
	if (fItemLogFile != NULL)
		fclose(fItemLogFile);

	struct tm when;
	time_t now;
	time(&now);
	when = *localtime(&now);

	sprintf(temp, ".\\item_log\\TM_%02d_%02d_%04d_%02d_%02d_%02d.txt", when.tm_mday, when.tm_mon + 1, when.tm_year + 1900, when.tm_hour, when.tm_min, when.tm_sec);

	fItemLogFile = fopen(temp, "wt");

	LastItemLogDay = when.tm_mday;
}

void ItemLog(char *str1, char *str2, unsigned int ip)
{
	struct tm when;
	time_t now;
	time(&now);
	when = *localtime(&now);

	char LogTemp[1024];

	unsigned char *cIP = (unsigned char*)&ip;

	if (ip != 0)
		sprintf(LogTemp, "%02d.%02d.%04d.%02d.%02d.%02d IP: %d.%d.%d.%d", when.tm_mday, when.tm_mon + 1, when.tm_year + 1900, when.tm_hour, when.tm_min, when.tm_sec, cIP[0], cIP[1], cIP[2], cIP[3]);
	else
		sprintf(LogTemp, "%02d.%02d.%04d.%02d.%02d.%02d ", when.tm_mday, when.tm_mon + 1, when.tm_year + 1900, when.tm_hour, when.tm_min, when.tm_sec);
	
	sprintf(LogTemp, "%s %s %s \n", LogTemp, str2, str1);

	if (fItemLogFile)
		fprintf(fItemLogFile, LogTemp);
}
