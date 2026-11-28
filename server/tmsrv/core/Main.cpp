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

/// @file Main.cpp
/// @brief Ponto de entrada do TMSrv (`main`) e os comandos do operador.
///
/// Separado de Server_Window.cpp para o resto do servidor virar a biblioteca `tmsrv_core`, que
/// os alvos de fuzzing (tests/fuzz) ligam sem o `main`.
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
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
#include "systems/CCastleZakum.h"

/// Comandos do operador: os itens do menu da janela do Windows, pelo FIFO de controle.
static void RegisterCommands()
{
	W2LoopAddCommand(IDC_EXIT, "Exit");
	W2LoopAddCommand(IDC_MOBRELOAD, "ReloadMob");
	W2LoopAddCommand(IDC_REBOOT, "Reboot");
	W2LoopAddCommand(IDC_SAVEALL, "SaveAll");
	W2LoopAddCommand(IDC_READGUILD, "ReadGuild");
	W2LoopAddCommand(IDC_READGUILDNAME, "ReadGuildName");
	W2LoopAddCommand(IDC_READGAMECONFIG, "ReadGameConfig");
}

int main()
{
	W2LoopInit();
	RegisterCommands();

	BASE_InitModuleDir();
	BASE_InitializeHitRate();

	StartLog();
	StartChatLog();

	memset(&g_pTreasure, 0, sizeof(g_pTreasure));

	ReadConfig();
	ReadLevelItemConfig();

	ConfigReady = 1;

	memset(g_pGuildWar, 0, sizeof(g_pGuildWar));
	memset(g_pGuildAlly, 0, sizeof(g_pGuildAlly));
	memset(GuildInfo, 0, sizeof(GuildInfo));
	memset(pMac, 0, sizeof(pMac));
	memset(pMobMerc, 0, sizeof(pMobMerc));

	int ret = 0;

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
		ServerAlert("Não foi possivel encontrar o arquivo LocalIP.txt", "Erro ao iniciar");

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
		ServerAlert("Não foi possivel pegar o ServerGroup. LocalIP.txt / ServerList.txt", "Erro ao iniciar");

		return TRUE;
	}

	DBServerPort = 7514;

	int *pip = (int*)LocalIP;

	ret = DBServerSocket.ConnectServer(DBServerAddress, 7514, *pip, NET_READDB);

	if (ret == NULL)
	{
		Log("erro, Não foi possivel se conectar a DBServer", "-system", 0);
		ServerAlert("Não foi possivel se conectar a DBServer", "Erro ao iniciar");

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

		ret = BillServerSocket.ConnectBillServer(BillServerAddress, BillServerPort, *pip2, NET_READBILL);

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

	CurrentTime = W2TickCount();

	CReadFiles();  // o construtor lê SancRate, QuestsRate, CompRate, Admin, Mac, Chall e Guild
	// CEncampment::ReadCamp();
	CCastleZakum::ReadCastleQuest();

	W2LoopSetTimer(TIMER_SEC, 500);
	W2LoopSetTimer(TIMER_MIN, 12000);

	ListenSocket.StartListen(*pip, GAME_PORT, NET_ACCEPT);

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

	return W2LoopRun(OnServerEvent);
}
