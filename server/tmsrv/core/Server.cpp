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
		pMob[conn].Affect[i].Level = 0;
		pMob[conn].Affect[i].Value = 0;
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
		pMob[conn].Affect[i].Level = 0;
		pMob[conn].Affect[i].Value = 0;
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
