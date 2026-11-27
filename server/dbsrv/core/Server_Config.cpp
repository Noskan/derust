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
/// @brief Leitura/gravação da configuração do DBSrv e das informações de transferência de guildas
///        entre servidores do grupo (`TransGuildConn`/`TransGuildIndex`).
///
/// Extraído de `Server.cpp` na Fase 2 (Onda 3) sem alteração de comportamento.


#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h> 
#include <fcntl.h>
#include <fcntl.h>
#include <unistd.h>
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

int ReadTrandGuildInfo()
{
	memset(TransGuildConn, 0, MAX_TRANS * sizeof(int));
	memset(TransGuildIndex, 0, MAX_TRANS * sizeof(int));

	FILE *fp = fopen("Trans.txt", "rt");

	if(fp == NULL)
		return FALSE;

	int ret = fscanf(fp, "%d", &CurrentTransInfo);

	if(!ret)
		return FALSE;

	int i = 0;

	for(; i < MAX_TRANS; i++)
	{
		int target = 0, conn = 0, index = 0;

		int ret = fscanf(fp, "%d %d %d", &target, &conn, &index);

		if(ret < 0)
			break;

		if(target != i + MAX_TRANS*2)
			break;

		TransGuildConn[i] = conn;
		TransGuildIndex[i] = index;
	}

	fclose(fp);

	CurrentTransInfo = i;

	return TRUE;
}

int WriteTransGuildInfo()
{
	FILE *fp = fopen("Trans.txt", "wt");

	int ret = fprintf(fp, "%d\n", CurrentTransInfo);

	if(!ret)
		return FALSE;

	for(int i = 0; i < CurrentTransInfo; i++)
	{
		int ret = fprintf(fp, "%d %d %d\n", i + MAX_TRANS*2, TransGuildConn[i], TransGuildIndex[i]);

		if(ret < 0)
			break;
	}

	fclose(fp);

	return TRUE;
}

void ConvertGuildNumber(int group, struct STRUCT_ITEM *sour)
{
	unsigned short guildnumber = BASE_GetGuild(sour);

	int i = 0;

	for(; i < CurrentTransInfo && (TransGuildConn[i] != group || TransGuildIndex[i] != guildnumber); i++);

	if(i == CurrentTransInfo)
	{
		TransGuildConn[i]  = group;
		TransGuildIndex[i] = guildnumber;

		CurrentTransInfo++;

		WriteTransGuildInfo();
	}

	unsigned short targetguild = (i + MAX_TRANS*2);

	sour->stEffect[0].cEffect = EF_HWORDGUILD;
	sour->stEffect[0].cValue = (unsigned char)(targetguild >> 8);
	sour->stEffect[1].cEffect = EF_LWORDGUILD;
	sour->stEffect[1].cValue = targetguild % 256;
}

int ReadConfig()
{
	FILE *fp = fopen("Config.txt", "rt");
	
	if(fp == NULL) 
	{ 
		ServerAlert("cant find Config.txt", "BOOTING ERROR");

		return FALSE; 
	}

	fscanf(fp, "Sapphire %d\n", &Sapphire);
	fscanf(fp, "LastCapsule %hu\n", &LastCapsule);

	fclose(fp);

	return TRUE;
}

int WriteConfig()
{
	FILE * fp= fopen("Config.txt", "wt");

	if(fp == NULL)
		return FALSE;

	fprintf(fp, "Sapphire %d\n", Sapphire);
	fprintf(fp, "LastCapsule %d\n", LastCapsule);

	fclose(fp);

	return TRUE;
}
