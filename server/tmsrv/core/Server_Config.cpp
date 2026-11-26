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

/// @file Server_Config.cpp
/// @brief Leitura das configurações do TMSrv (`ReadConfig`) e da tabela de itens por nível
///        (`ReadLevelItemConfig`).
///
/// Extraído de `Server.cpp` na Fase 2 (Onda 4) sem alteração de comportamento.

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
