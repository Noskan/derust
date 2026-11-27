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
/// @brief Ponto de entrada e janela de console do TMSrv: `WinMain`, registro da janela,
///        laço de mensagens (`MainWndProc`: timers, sockets, menu) e painel de estado (`DrawConfig`).
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

HFONT__ *GetAFont()
{
	return 0;
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

			sprintf(String + strlen(String), "  %-4d %2d %2d %2d %2d   ",
				item->sIndex, item->stEffect[0].cEffect, item->stEffect[0].cValue, item->stEffect[1].cEffect, item->stEffect[1].cValue);
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

	ret = DBServerSocket.ConnectServer(DBServerAddress, 7514, *pip, WSA_READDB);

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

	CurrentTime = W2TickCount();

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

				W2SleepMs(200);
			}

			if (ret == 0)
			{
				Log("reconnect DB fail.", "-system", 0);

				PostQuitMessage(NULL);

				return TRUE;

			}

			W2SleepMs(200);
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

				W2SleepMs(200);
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
