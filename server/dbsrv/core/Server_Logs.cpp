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
/// @brief Logs do DBSrv: log corrente (`fLogFile`), log diário e relatórios diários de
///        experiência (ranking de EXP) e de itens (`ItemDayLog`).
///
/// Extraído de `Server.cpp` na Fase 2 (Onda 3) sem alteração de comportamento.


#include <winsock2.h>
#include <ws2tcpip.h>
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

#include "common/Basedef.h"
#include "common/CPSock.h"
#include "common/ItemEffect.h"
#include "net/CUser.h"
#include "persistence/CFileDB.h"
#include "core/Server.h"
#include "persistence/CRanking.h"
#include "core/CReadFiles.h"

void StartLog(char *cccc)
{
	 char Temp[256];

     if(fLogFile != NULL)
	 {  
		 int ret = fclose(fLogFile);

		 if(ret)
			 Log("Logfile close fail!!", "-system", 0);
	 }

	 struct tm when;
	 time_t now;
	 time(&now); 
	 when = *localtime(&now);

	 sprintf(Temp, "./Log/DB_%02d_%02d_%04d_%02d_%02d_%02d_%s.txt", when.tm_mday, when.tm_mon + 1, when.tm_year + 1900, when.tm_hour, when.tm_min, when.tm_sec, cccc);

   	 fLogFile = fopen(Temp, "a+");

	 LastLogDay = when.tm_mday;
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

void DayLog_ExpLog()
{
	FILE *fp = NULL;

	char Temp[1024];
	// char Temp2[1024];

	struct tm when;
	time_t now;
	time(&now);
	when = *localtime(&now);

	sprintf(Temp, "./DayLog/EXP_DB_%02d_%02d_%04d_%02d_%02d_%02d.txt", when.tm_mday, when.tm_mon + 1, when.tm_year + 1900, when.tm_hour, when.tm_min, when.tm_sec);

	fp = fopen(Temp, "a+");

	if (fp == NULL)
	{
		Log("err,daylog item open fail!!", "-system", 0);
		return;
	}

	DayLog_ReadAccountInDir("./account/*", fp);

	fclose(fp);
}

void DayLog_ReadAccountInDir(char *dir, FILE *log)
{
	HANDLE handle;
	WIN32_FIND_DATA win32_find_data;

	char tmp[1024];
	// char tmp2[1024];

	handle = FindFirstFile(dir, &win32_find_data);

	if (handle == INVALID_HANDLE_VALUE)
	{
		// TODO: Error: can't read initial file
	}
	else
	{
		while (FindNextFile(handle, &win32_find_data))
		{
			char *fileName = win32_find_data.cFileName;

			if (!strncmp(fileName, ".", 1))
				continue;

			if (win32_find_data.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
			{
				sprintf(tmp, "./account/%s/*", fileName);
				DayLog_ReadAccountInDir(tmp, log);
			}
			else
			{
				SYSTEMTIME fWriteTime;
				SYSTEMTIME sysTime;
				FileTimeToSystemTime(&win32_find_data.ftLastWriteTime, &fWriteTime);
				GetLocalTime(&sysTime);

				// Only use fresh active accounts 
				if (fWriteTime.wYear != sysTime.wYear || fWriteTime.wMonth != sysTime.wMonth)
					continue;

				strcpy(tmp, dir);
				tmp[strlen(dir) - 1] = '\0';
				sprintf(tmp, "%s%s", tmp, fileName);

				errno = 0;
				FILE* fs;
				fopen_s(&fs, tmp, "rb");

				if (fs != NULL)
				{
					STRUCT_ACCOUNTFILE accBuffer;
					memset(&accBuffer, 0, sizeof(STRUCT_ACCOUNTFILE));

					fread(&accBuffer, sizeof(STRUCT_ACCOUNTFILE), 1, fs);

					struct tm when;
					time_t now;
					time(&now);
					when = *localtime(&now);

					for (int i = 0; i < MOB_PER_ACCOUNT; i++)
					{
						if (accBuffer.Char[i].MobName[0] == '\0')
							continue;

						if (accBuffer.mobExtra[i].DayLog.YearDay == when.tm_yday-1)
							fprintf(log, "[%02d/%02d/%04d][%02d:%02d:%02d] %s Char:%s recebeu %llu exp hoje.\n", when.tm_mday, when.tm_mon + 1, when.tm_year + 1900, when.tm_hour, when.tm_min, when.tm_sec, accBuffer.Info.AccountName, accBuffer.Char[i].MobName, accBuffer.mobExtra[i].DayLog.Exp);
						
					}

					fclose(fs);
				}
				else
				{
					// TODO: Cant open file to read
				}
			}
		}
	}

	FindClose(handle);
}

void DayLog_ItemLog()
{
	FILE *fp = NULL;

	char Temp[1024];

	struct tm when;
	time_t now;
	time(&now);
	when = *localtime(&now);

	sprintf(Temp, "./DayLog/ITEM_DB_%02d_%02d_%04d_%02d_%02d_%02d.txt", when.tm_mday, when.tm_mon + 1, when.tm_year + 1900, when.tm_hour, when.tm_min, when.tm_sec);

	fp = fopen(Temp, "a+");

	if (fp == NULL)
	{
		Log("err,daylog item open fail!!", "-system", 0);
		return;
	}

	for (int i = 0; i < MAX_ITEMLIST; i++)
	{
		if (ItemDayLog[i].Num == 0)
			continue;

		sprintf(Temp, "[%02d/%02d/%04d][%02d:%02d:%02d] ", when.tm_mday, when.tm_mon + 1, when.tm_year + 1900, when.tm_hour, when.tm_min, when.tm_sec);

		sprintf(Temp, "%s ItemID:%d ItemName:%s ItemNum:%d \n", Temp, i, g_pItemList[i].Name, ItemDayLog[i].Num);

		if (fp)
			fprintf(fp, Temp);

		ItemDayLog[i].Num = 0;
	}

	fclose(fp);
}

BOOL file_exists(char *s, ...)
{
	char buf[1024];

	va_list pArgList;

	va_start(pArgList, s);
	vsprintf(buf, s, pArgList);
	va_end(pArgList);

	FILE *fp = fopen(buf, "r");

	if (fp)
	{
		fclose(fp);
		return TRUE;
	}

	return FALSE;
}
