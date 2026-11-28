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

/// @file Server_Window.cpp
/// @brief Tratador do laço de eventos do TMSrv (`OnServerEvent`: timers, sockets, comandos,
///        encerramento) e a gravação do gameconfig.txt (`WriteGameConfig`). O `main` está em
///        core/Main.cpp.
///
/// Extraído de `Server.cpp` na Fase 2 (Onda 4). Era a janela do servidor no Windows: o laço de
/// mensagens virou o de common/EventLoop.h, e o painel de estado, que só desenhava o que o
/// gameconfig.txt grava, saiu.

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
#include "net/NetGuard.h"
#include "core/CReadFiles.h"
#include "systems/CWarTower.h"

void WriteGameConfig()
{
	if (ConfigReady == 0)
		return;

	char String[4096];

	FILE *fp = fopen("gameconfig.txt", "w+");

	if (fp == NULL)
		return;

	sprintf(String, "Drop Item Event Settings:");
	fprintf(fp, "%s\n", String);

	sprintf(String,"   evindex %d   evdelete %d   evon %d   evitem %d   evrate %d   evstart %d %d %d %d",
		evIndex, evDelete, evOn, evItem, evRate, evStartIndex, evCurrentIndex, evEndIndex, evNotice);
	fprintf(fp, "%s\n", String);

	sprintf(String, "Etc Event Settings:");
	fprintf(fp, "%s\n", String);

	sprintf(String, "   double %d   deadpoint %d   dungeonevent %d   statsapphire %d    battleroyal %d",
		DOUBLEMODE, DEADPOINT, DUNGEONEVENT, StatSapphire, BRItem);
	fprintf(fp, "%s\n", String);

	sprintf(String, "Billing Settings:");
	fprintf(fp, "%s\n", String);

	sprintf(String, "   billmode %d   freeexp %d   charselbill %d    potioncount %d   partybonus %d   guildboard %d",
		BILLING, FREEEXP, CHARSELBILL, POTIONCOUNT, PARTYBONUS, GUILDBOARD);
	fprintf(fp, "%s\n", String);

	sprintf(String, "Item Drop Bonus Settings:");
	fprintf(fp, "%s\n", String);

	for (int i = 0; i < 4; i++)
	{
		int pos = 16 * i;
		sprintf(String, "   %3d %3d %3d %3d %3d %3d %3d %3d   %3d %3d %3d %3d %3d %3d %3d %3d",
			g_pDropBonus[0 + pos], g_pDropBonus[1 + pos], g_pDropBonus[2 + pos], g_pDropBonus[3 + pos],
			g_pDropBonus[4 + pos], g_pDropBonus[5 + pos], g_pDropBonus[6 + pos], g_pDropBonus[7 + pos],
			g_pDropBonus[8 + pos], g_pDropBonus[9 + pos], g_pDropBonus[10 + pos], g_pDropBonus[11 + pos],
			g_pDropBonus[12 + pos], g_pDropBonus[13 + pos], g_pDropBonus[14 + pos], g_pDropBonus[15 + pos]);
		fprintf(fp, "%s\n", String);
	}

	sprintf(String, "Treasure Settings:");
	fprintf(fp, "%s\n", String);

	for (int j = 0; j < 8; j++)
	{
		STRUCT_TREASURE *tr = &g_pTreasure[j];

		sprintf(String, "%-4d ", g_pTreasure[j].Source);

		for (int k = 0; k < 5; ++k)
		{
			STRUCT_ITEM *item = &tr->Target[k];

			sprintf(String + strlen(String), "  %-4d %2d %2d %2d %2d   ",
				item->sIndex, item->stEffect[0].cEffect, item->stEffect[0].cValue, item->stEffect[1].cEffect, item->stEffect[1].cValue);
		}
		fprintf(fp, "%s\n", String);

		sprintf(String, "       %-5d                %-5d                %-5d                %-5d                %-5d",
			tr->Rate[0], tr->Rate[1], tr->Rate[2], tr->Rate[3], tr->Rate[4]);
		fprintf(fp, "%s\n", String);
	}

	sprintf(String, "Etc Settings:");
	fprintf(fp, "%s\n", String);

	sprintf(String, "   partydif %d   kefrastatus %d   GTorreHour %d    RVRHour %d    DropItem %d   BRHour %d   maxNightmare %d   PotionDelay %d",
		PARTY_DIF, KefraLive, GTorreHour, RvRHour, isDropItem, BRHour, maxNightmare, PotionDelay);
	fprintf(fp, "%s\n", String);

	fclose(fp);
}

void BuildList()
{

}

void OnServerEvent(const W2Event& ev)
{
	switch (ev.code)
	{

	case W2_EVENT_TIMER:
	{
		if (ev.id == TIMER_SEC)
			ProcessSecTimer();
		else if (ev.id == TIMER_MIN)
			ProcessMinTimer();
	} break;

	case NET_READBILL:
	{
		if (ev.net != W2_NET_READ)
		{
			Log("err Billing Connection lost.", "-system", 0);

			BillServerSocket.CloseSocket();
			BillCounter = 360;

			Log("err,billing disconnected.", "-system", 0);

			return;
		}

		int ret = BillServerSocket.Receive();

		if (ret != TRUE)
		{
			Log("err bill receive fail", "-system", 0);

			BillServerSocket.CloseSocket();
			BillCounter = 360;

			return;
		}

		if (ret == FALSE)
		{
			ret = errno;

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

	case NET_READDB:
	{
		int t = ev.net;
		if (ev.net != W2_NET_READ)
		{
			Log("err wsareaddb fail", "-system", 0);

			DBServerSocket.CloseSocket();

			int ret = 0;

			for (int i = 0; i < 2; i++)
			{
				int *pip = (int*)LocalIP;

				ret = DBServerSocket.ConnectServer(DBServerAddress, DBServerPort, *pip, NET_READDB);

				if (ret != NULL)
					break;

				W2SleepMs(200);
			}

			if (ret == 0)
			{
				Log("reconnect DB fail.", "-system", 0);

				W2LoopQuit(0);

				return;

			}

			W2SleepMs(200);
			W2LoopQuit(0);
			return;
		}

		if (DBServerSocket.Receive() == FALSE)
		{
			Log("err wsareaddb fail", "-system", 0);

			DBServerSocket.CloseSocket();

			int ret = 0;

			for (int i = 0; i < 2; i++)
			{
				int *pip = (int*)LocalIP;

				ret = DBServerSocket.ConnectServer(DBServerAddress, DBServerPort, *pip, NET_READDB);

				if (ret != NULL)
					break;

				W2SleepMs(200);
			}

			if (ret == 0)
			{
				Log("reconnect DB fail.", "-system", 0);

				W2LoopQuit(0);

				return;

			}
			W2LoopQuit(0);
			return;

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

	case  NET_READ:
	{
		int User = GetUserFromSocket(ev.fd);
		if (User == 0)
		{
			W2CloseSocket(ev.fd);

			break;
		}

		if (ev.net != W2_NET_READ)
		{
			sprintf(temp, "clo,fd %d-%d", pUser[User].Mode, pMob[User].Mode);
			Log(temp, pUser[User].AccountName, pUser[User].IP);

			CloseUser(User);
			break;
		}

		int ret = pUser[User].cSock.Receive();

		if (ret == FALSE)
		{
			ret = errno;

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

	case NET_ACCEPT:
	{
		if (ev.error == 0)
		{

			int User = GetEmptyUser();

			if (User == 0)
			{
				Log("err,accept fail - no empty", "-system", 0);

				break;
			}

			int ret = pUser[User].AcceptUser(ListenSocket.Sock);

			if (ret == FALSE)
				break;

			if (!NetGuardAccept(User))
			{
				CloseUser(User);

				return;
			}

			if (User >= MAX_USER - ADMIN_RESERV)
			{
				SendClientMessage(User, g_pMessageStringTable[_NN_Reconnect]);
				pUser[User].cSock.SendMessage();

				CloseUser(User);

				return;
			}

			if (ServerDown != -1000)
			{
				SendClientMessage(User, g_pMessageStringTable[_NN_ServerReboot_Cant_Connect]);

				CloseUser(User);

				return;
			}

		}
		else
			Log("err,- accept - wsagetselecterror", "-system", 0);

	} break;

	case W2_EVENT_COMMAND:
	{
		switch (ev.id)
		{
		case IDC_EXIT:
			W2LoopPostClose();
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
		} break;
		}
	} break;
	case W2_EVENT_CLOSE:
	{
		// No Windows, uma pergunta de confirmação. Aqui o pedido (SIGTERM/SIGINT ou o comando Exit)
		// já é a decisão do operador: o texto fica como aviso no stderr.
		ServerAlert("Você realmente deseja desligar o server?", "Aviso!!!");

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

		W2LoopQuit(0);

	} break;
	}
}
