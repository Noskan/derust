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

/// @file Server_Logs.cpp
/// @brief Logs do TMSrv: log geral, de chat, de itens e de suspeita de crack (`CrackLog`).
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

void StartLog()
{
	if (fLogFile != NULL)
		fclose(fLogFile);

	struct tm when;
	time_t now;
	time(&now);
	when = *localtime(&now);

	sprintf(temp, "./Log/TM_%02d_%02d_%04d_%02d_%02d_%02d.txt", when.tm_mday, when.tm_mon + 1, when.tm_year + 1900, when.tm_hour, when.tm_min, when.tm_sec);

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

void StartChatLog()
{
	if (fChatLogFile != NULL)
		fclose(fChatLogFile);

	struct tm when;
	time_t now;
	time(&now);
	when = *localtime(&now);

	sprintf(temp, "./chat_log/TM_%02d_%02d_%04d_%02d_%02d_%02d.txt", when.tm_mday, when.tm_mon + 1, when.tm_year + 1900, when.tm_hour, when.tm_min, when.tm_sec);

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

	sprintf(temp, "./item_log/TM_%02d_%02d_%04d_%02d_%02d_%02d.txt", when.tm_mday, when.tm_mon + 1, when.tm_year + 1900, when.tm_hour, when.tm_min, when.tm_sec);

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
