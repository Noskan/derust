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

HWND		hWndMain;
HMENU		hMainMenu;
HINSTANCE   hInst;

int ServerIndex = -1;
int CurrentTransInfo = 0;
int Sapphire = 8;
int MinCounter = 0;
int SecCounter = 0;
int HourCounter = 0;
int TransperCharacter = 0;
unsigned int CurrentTime = 0;
unsigned int LastSendTime = 0;

int UserConnection[MAX_SERVER] = {0,};

unsigned char LocalIP[4] = {0,};
unsigned int pAdminIP[MAX_ADMIN] = {0,};

int TransGuildConn[MAX_TRANS];
int TransGuildIndex[MAX_TRANS];

extern unsigned short g_pGuildWar[65536];
extern unsigned short g_pGuildAlly[65536];

extern char g_pServerList[MAX_SERVERGROUP][MAX_SERVERNUMBER][64];

char adminclientid[256];
char adminclientpass[256];

FILE *fLogFile;
FILE *fDayLogFile;

int		LastLogDay	  =	-1;
int		LastDayLogDay = -1;

CUser TempUser;

CUser pUser[MAX_SERVER];
CUser pAdmin[MAX_ADMIN];

CPSock AdminClient;
CPSock AdminSocket;
CPSock ListenSocket;

CFileDB cFileDB;

STRUCT_MOB g_pBaseSet[MAX_CLASS];
STRUCT_ITEMLOG ItemDayLog[MAX_ITEMLIST];

HFONT hFont = NULL;
HFONT h;

int x = 0;
int y = 0;
HDC hDC = NULL;

void TextOutWind(char *str, int color)
{
	char String[1024];

	SetTextColor(hDC, color);

	sprintf(String, str);

	int len = strlen(String);


	TextOutA(hDC, x, y, String, len);

	y += 16;
}

void DrawConfig()
{
	x = 0;
	y = 0;

	h = 0;

	int VERMELHO = 255;
	int PRETO = 0;

	char String[1024];
	int len = 0;

	hDC = GetDC(hWndMain);

	if (hDC == NULL)
		return;

	if (hFont == 0)
		return;

	if (SelectObject(hDC, hFont) != 0)
		h = (HFONT)SelectObject(hDC, hFont);

	
	TextOutWind("Server Zone Status:", VERMELHO);

	for (int i = 0; i < MAX_SERVER; i++)
	{
		unsigned char *cIP = (unsigned char *)&(pUser[i].IP);

		sprintf(String, " %d - IP: %3d. %3d. %3d. %3d  Socket: %3d  Guild: %4d %4d %4d %4d %4d  User: %4d    ", i, cIP[0], cIP[1], cIP[2], cIP[3], pUser[i].cSock.Sock, ChargedGuildList[i][0], ChargedGuildList[i][1], ChargedGuildList[i][2], ChargedGuildList[i][3], ChargedGuildList[i][4], pUser[i].Count);
		TextOutWind(String, PRETO);
	}
	
	TextOutWind("Admin Status:", VERMELHO);
	for (int i = 0; i < MAX_ADMIN; i++)
	{
		unsigned char *cIP = (unsigned char *)&(pAdmin[i].IP);

		sprintf(String, "%2d - IP: %3d. %3d. %3d. %3d  Socket: %3d", i, cIP[0], cIP[1], cIP[2], cIP[3], pAdmin[i].cSock.Sock);
		TextOutWind(String, PRETO);
	}
	
	if (hFont && h)
		h = (HFONT)SelectObject(hDC, h);

	ReleaseDC(hWndMain, hDC);
}

int InitApplication(HANDLE hInstance) // Last updated 15/01/2013
{
	WNDCLASS  wc;  
	
	wc.style = CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS;    
	wc.lpfnWndProc = (WNDPROC)MainWndProc;  
	wc.cbClsExtra = 0;  
	wc.cbWndExtra = 0;                    
	wc.hIcon = (HICON)LoadIcon ((HINSTANCE)hInstance, "MAINICON");
	wc.hInstance = (HINSTANCE) hInstance;  
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);  
	wc.lpszMenuName =  NULL;           
	wc.lpszClassName = "MainClass"; 
	
	if(!RegisterClass(&wc)) 
		return RegisterClass(&wc);

	return TRUE;
}

int InitInstance(HANDLE hInstance, int nCmdShow) // Last updated 15/01/2013
{    
	hMainMenu = CreateMenu();

	hWndMain = CreateWindow("MainClass",  "DB Server", 
		WS_OVERLAPPED | WS_CLIPCHILDREN | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,    // Window style. 
		CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,            // W,H[MenuÇÕÄ¡¸é 480]
		NULL, hMainMenu, (HINSTANCE)hInstance, NULL);

	if (!hWndMain) 
		return FALSE;

	ShowWindow(hWndMain, nCmdShow);  
	UpdateWindow(hWndMain);      

	return TRUE;     
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) // Last Updated 19/01/2013
{
	MSG msg;

	UNREFERENCED_PARAMETER(lpCmdLine);

	hInst = hInstance;

	if(!hPrevInstance)
		if(!InitApplication(hInstance)) 
			return FALSE;      

	if(!InitInstance(hInstance, nCmdShow)) 
		return FALSE;


	BASE_InitModuleDir();
    BASE_InitializeBaseDef();

#pragma region Load Base Characters

	int handle = -1;

	handle = _open("./BaseMob/TK", _O_RDONLY | _O_BINARY);

	if(handle == -1)
	{
		MessageBoxA(hWndMain, "no TransKnight file", "BOOTING ERROR", MB_OK);

		return FALSE;
	}

	_read(handle, &g_pBaseSet[0], sizeof(STRUCT_MOB));
	_close(handle);


	handle = -1;

	handle = _open("./BaseMob/FM", _O_RDONLY | _O_BINARY);

	if(handle == -1)
	{
		MessageBoxA(hWndMain, "no Foema file", "BOOTING ERROR", MB_OK);

		return FALSE;
	}

	_read(handle, &g_pBaseSet[1], sizeof(STRUCT_MOB));
	_close(handle);

	handle = -1;

	handle = _open("./BaseMob/BM", _O_RDONLY | _O_BINARY);

	if(handle == -1)
	{
		MessageBoxA(hWndMain, "no BeastMaster file", "BOOTING ERROR", MB_OK);

		return FALSE;
	}

	_read(handle, &g_pBaseSet[2], sizeof(STRUCT_MOB));
	_close(handle);

	handle = -1;

	handle = _open("./BaseMob/HT", _O_RDONLY | _O_BINARY);

	if(handle == -1)
	{
		MessageBoxA(hWndMain, "no Huntress file", "BOOTING ERROR", MB_OK);

		return FALSE;
	}

	_read(handle, &g_pBaseSet[3], sizeof(STRUCT_MOB));
	_close(handle);

	g_pBaseSet[0].BaseScore = g_pBaseSet[0].CurrentScore;
	g_pBaseSet[1].BaseScore = g_pBaseSet[1].CurrentScore;
	g_pBaseSet[2].BaseScore = g_pBaseSet[2].CurrentScore;
	g_pBaseSet[3].BaseScore = g_pBaseSet[3].CurrentScore;

#pragma endregion

	StartLog("A");
	Log("start log", "-system", 0);

	memset(g_pGuildWar, 0, sizeof(g_pGuildWar));
	memset(g_pGuildAlly, 0, sizeof(g_pGuildAlly));

	ReadConfig();

	int ret = ListenSocket.WSAInitialize();

	if(!ret)
	{
		Log("err,wsainitialize fail", "-system", 0);

		return FALSE;
	}

	BASE_InitializeServerList();

	char name[256];
	int i = 0;

	struct addrinfo hints, *res, *p;

	memset(&hints, 0, sizeof(addrinfo));
	
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	gethostname(name, sizeof(name));

	getaddrinfo(name, NULL, &hints, &res);

	for (p = res; p != NULL; p = p->ai_next)
	{
		if (p->ai_family != AF_INET)
			continue;

		struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;

		inet_ntop(p->ai_family, &(ipv4->sin_addr), name, sizeof(name));

		for (i = 0; i < MAX_SERVERGROUP; i++)
		{
			if (!strcmp(g_pServerList[i][0], name))
			{
				ServerIndex = i;

				sscanf(name, "%hhu.%hhu.%hhu.%hhu", &LocalIP[0], &LocalIP[1], &LocalIP[2], &LocalIP[3]);

				break;
			}
		}

		if (ServerIndex != -1)
			break;
	}

	/*freeaddrinfo(res);
	freeaddrinfo(p);
	freeaddrinfo(&hints);
	*/
    if(ServerIndex == -1)
    {	
		MessageBox(hWndMain, "Can't get Server Group Index LocalIP:", "", MB_OK|MB_SYSTEMMODAL);
		MessageBox(hWndMain, "Can't get Server Group Index TestServerIP:", g_pServerList[i][0], MB_OK | MB_SYSTEMMODAL);

	   return TRUE;
    }

	CReadFiles::ImportItem();
	ReadTrandGuildInfo();

	ListenSocket.StartListen(hWndMain, 0, DB_PORT, WSA_ACCEPT);
	AdminSocket.StartListen(hWndMain, 0, ADMIN_PORT, WSA_ACCEPTADMIN);

	FILE *fp = fopen("redirect.txt", "rt");

	if(fp)	
	{	
		char sip[256];
		int  port;

		fscanf(fp, "%s %d %s %s", sip, &port, adminclientid, adminclientpass);
		fclose(fp);

		ret = AdminClient.ConnectServer(sip, port, INADDR_ANY, WSA_READADMINCLIENT);

		if(ret == NULL)
		{	
			MessageBox(hWndMain, "there's redirect.txt but can't connect", "BOOTING ERROR", NULL);
			return FALSE;		
		}	
		else
		{	
			Log("sys redirection server connected","-system",0);
			TransperCharacter = 1;
		}
	}

	SetTimer(hWndMain, TIMER_SEC, 1000, NULL); 

	CReadFiles::ReadGuildInfo();

	while(GetMessage(&msg, NULL, 0, 0))
	{    
		TranslateMessage(&msg);
		DispatchMessage(&msg); 
	}

	return msg.wParam;  
}

LONG APIENTRY MainWndProc(HWND hWnd, UINT message, UINT wParam, LONG lParam)
{
	switch(message)    
	{

	case WM_TIMER:
		{ 
			if(wParam == TIMER_SEC) 
				ProcessSecTimer(); 
			else if(wParam == TIMER_MIN) 
				ProcessMinTimer(); 
		} break;

	case WSA_READADMINCLIENT:
		{
			if(WSAGETSELECTEVENT(lParam) != FD_READ)
			{	
				Log("adminclient close fd read","-system",0);

				AdminClient.CloseSocket();

				break;
			}

			if(AdminClient.Receive() != 1)
			{	
				Log("adminclient %d close receive", "-system", 0);

				break;
			}

			char temp[256];	
			int Error;	
			int ErrorCode;

			while(1)
			{	
				char *Msg = AdminClient.ReadMessage(&Error, &ErrorCode); 

				if(Msg == NULL) 
					break;

				if(Error == 1 || Error == 2)
				{	
					sprintf(temp, "err wsa_ReadadminClient (%d),%d", Error, ErrorCode);
					Log(temp, "-system", 0);

					break;
				}

				int ret = ProcessAdminClientMessage(Msg);
			};
		}	break;

	case WSA_READADMIN:
		{
			int User = GetAdminFromSocket(wParam);  

			if(User == -1)
			{	
				closesocket(wParam);

				Log("err, wsa_read unregistered sever socket", "-system", 0);

				break;
			} 

			if(WSAGETSELECTEVENT(lParam) != FD_READ)
			{	
				char temp[256];
				sprintf(temp, "admin close fd %d", User);
				Log(temp, "-system", 0);

				pAdmin[User].cSock.CloseSocket();
				pAdmin[User].Mode = USER_EMPTY;
				pAdmin[User].Name[0] = 0;

				break;
			} 

		 	CurrentTime = timeGetTime();

			if(pAdmin[User].cSock.Receive() != 1)
			{	
				char temp[256];
				sprintf(temp, "admin %d close receive", User);
				Log(temp, "-system", 0);

				break;
			}

			char temp[256];
			int Error;
			int ErrorCode;

            while(1)  
			{	
				char *Msg = pAdmin[User].cSock.ReadMessage(&Error, &ErrorCode);

				if(Msg == NULL) 
					break;

	            #ifdef  _PACKET_DEBUG
				MSG_STANDARD * debug = (MSG_STANDARD*) Msg;

				if(BASE_CheckPacket(debug))
				{	
					sprintf(temp, "err packet Type:%d Size:%d", debug->Type, debug->Size);
					Log(temp, "-system", 0);
				}
				#endif

				if(Error == 1 || Error == 2)
				{	
					sprintf(temp, "err,wsa_Readadmin (%d),%d", Error, ErrorCode);
					Log(temp, "-system", 0);

					break;
				}

				int ret = ProcessAdminMessage(User, Msg);

				if(ret == FALSE) 
				{	
					pAdmin[User].cSock.CloseSocket();
					pAdmin[User].Mode = USER_EMPTY;
					pAdmin[User].Name[0] = 0;

					break;
				}
			}; 

		} break;

	case WSA_READ:
		{
			int User = GetUserFromSocket(wParam);  

			if(User == -1)
			{	
				closesocket(wParam);

				Log("err wsa_read unregistered game server socket", "-system", 0);

				break;
			}

			if(WSAGETSELECTEVENT(lParam) != FD_READ)
			{	
				char temp[256];
				sprintf(temp, "clo server fd %d", User);
				Log(temp, "-system", 0);

				pUser[User].cSock.CloseSocket();
				pUser[User].Mode = USER_EMPTY;

				break;
			}

			CurrentTime = timeGetTime();

			if(pUser[User].cSock.Receive() != 1)
			{	
				char temp[256];
				sprintf(temp, "clo, server receive %d", User);
				Log(temp, "-system", 0);
				Log(temp, "-system", 0);
				Log(temp, "-system", 0);

				pUser[User].cSock.nRecvPosition = 0;
				pUser[User].cSock.nProcPosition = 0;

				break;
			}

			char temp[256];
			int Error;
			int ErrorCode;

			do 
			{	
				char *Msg = pUser[User].cSock.ReadMessage(&Error, &ErrorCode); 

				if (Msg == NULL) 
					break;

				#ifdef  _PACKET_DEBUG
				MSG_STANDARD * debug = (MSG_STANDARD*) Msg;
				if(BASE_CheckPacket(debug))
				{	
					sprintf(temp, "**PACKET_DEBUG** Type:%d Size:%d", debug->Type, debug->Size);
					Log(temp, "-system", 0);
				}
				#endif

				if(Error == 1 || Error == 2)
				{	
					sprintf(temp, "err wsa_read (%d),%d", Error, ErrorCode);
					Log(temp, "-system", 0);

					break;
				} 

				ProcessClientMessage(User, Msg);
			} while(1);

		} break;

	case WSA_ACCEPTADMIN:
		{
			 ReadAdmin();

			 if(WSAGETSELECTERROR(lParam))
			 {
				 Log("err,accept_admin accepting fails", "-system", 0);

				 break;
			 }

			 TempUser.AcceptUser(AdminSocket.Sock, WSA_READADMIN);

			 unsigned char *cIP = (unsigned char *)&(TempUser.IP);
			 int User = -1;
			 int i = 0;

			 // Era i <= MAX_ADMIN: lia pAdmin[MAX_ADMIN], além do fim. Com -O2 o GCC usa esse
			 // comportamento indefinido para tirar o limite do laço, que varria a memória até cair.
			 for(i = 0; i < MAX_ADMIN; i++)
			 {   
				 if(pAdmin[i].IP == TempUser.IP) 
				 {
					 User = i;
					 break;
				 }
			 }

			 if(User == -1)
			 {
				 int i = 0;

				 for(i = 0; i < MAX_ADMIN; i++)
				 {	
					 int a, b, c, d; a = b = c = d = 0;

					 if(pAdminIP[i] == 0) 
						 continue;

					 if(pAdminIP[i] == TempUser.IP) 
					 {
						 User = i;
						 break;
					 }
				 }

				 if(i == MAX_ADMIN)
				 {	
					 TempUser.cSock.CloseSocket();

					 char temp[256];   
					 sprintf(temp, "err, wsa_acceptadmin request accept from  %d.%d.%d.%d", cIP[0], cIP[1], cIP[2], cIP[3]);
					 Log(temp, "-system", 0);
				 }
			 }

			 if(User >= 0 && User < MAX_ADMIN)
			 {
				 if(pAdmin[User].Mode != USER_EMPTY)
				 {	
					 char temp[256];
					 sprintf(temp, "err, wsa_acceptadmin not empty previous slot %d", i);
					 Log(temp, "-system", 0);

					 TempUser.cSock.CloseSocket();

					 pAdmin[User].cSock.CloseSocket();
					 pAdmin[User].Mode = USER_EMPTY;

					 break;
				 } 
			 }   
			 else
			 {   
				 Log("err,accept_admin unknown attempt", "-system", 0);

				 TempUser.cSock.CloseSocket();

				 break;
			 }
			 
			 if(User == -1)
			 {	
				 Log("err,accept_admin no empty", "-system", 0);
				 break;
			 } 

			 pAdmin[User].Level               = -1;
			 pAdmin[User].DisableID           = 0;
			 pAdmin[User].IP                  = TempUser.IP;
			 pAdmin[User].Mode                = TempUser.Mode;
			 pAdmin[User].Name[0]			 = 0;
			 pAdmin[User].cSock.Sock          = TempUser.cSock.Sock;
			 pAdmin[User].cSock.nRecvPosition = TempUser.cSock.nRecvPosition;
			 pAdmin[User].cSock.nProcPosition = TempUser.cSock.nProcPosition;
			 pAdmin[User].cSock.nSendPosition = TempUser.cSock.nSendPosition;

			 cIP = (unsigned char *)&(pAdmin[User].IP);

			 char tmp[100];
			 sprintf(tmp, "sys,wsa_acceptadmin %d.%d.%d.%d <%d>", cIP[0], cIP[1], cIP[2], cIP[3], User);
			 Log(tmp, "-system", 0);

			 pAdmin[User].Encode1 = rand() % 10000;
			 pAdmin[User].Encode2 = rand() % 10000;
			 
			 MSG_NPIDPASS sm;
			 memset(&sm, 0, sizeof(MSG_NPIDPASS));

			 sm.Type = _MSG_NPReqIDPASS;
			 sm.Size = sizeof(MSG_NPIDPASS);
			 sm.ID = 0;

			 sm.Encode1 = pAdmin[User].Encode1;
			 sm.Encode2 = pAdmin[User].Encode2;

			 pAdmin[User].cSock.SendOneMessage((char*)&sm, sizeof(MSG_NPIDPASS));
		} break;

	case WSA_ACCEPT:
		{
			if(WSAGETSELECTERROR(lParam)) 
			{	
				Log("err WSAGETSELECTERROR - accepting new client.", "-system", 0);
				break;
			}

			TempUser.AcceptUser(ListenSocket.Sock, WSA_READ);

			unsigned char *cIP = (unsigned char *)&(TempUser.IP);

			char sIP[256]; 
			
			sprintf(sIP, "%d.%d.%d.%d", cIP[0], cIP[1], cIP[2], cIP[3]);

			int User = -1;

			for(int i = 0; i < MAX_SERVER; i++)
			{
				if(pUser[i].IP == TempUser.IP) 
				{
					User = i;
					break;
				}
			}

			if(User == -1)
			{ 
				int i = 1;

				for(; i < MAX_SERVERNUMBER; i++)
				{	
					if(!strcmp(g_pServerList[ServerIndex][i], sIP)) 
						break;
				}

				if(i == MAX_SERVERNUMBER)
				{	
					TempUser.cSock.CloseSocket();

					char temp[256];   
					sprintf(temp, "err wsa_accept request from  %s", sIP );

					Log(temp, "-system", 0);

					break;
				}

				User = i - 1;
			}
			else if(User >= 0 && User < MAX_SERVERNUMBER)
			{   
				if(pUser[User].Mode != USER_EMPTY)
				{	
					char temp[256];
					sprintf(temp, "err wsa_accept no previous slot %d", User );

					Log(temp, "-system", 0);

					TempUser.cSock.CloseSocket();
					TempUser.Mode = 0;

					pUser[User].cSock.CloseSocket();
					pUser[User].Mode = 0;

					break;
				}
			}
			else
			{
				Log("err wsa_accept unknown attempt", "-system", 0);
				TempUser.cSock.CloseSocket();
			}

			if(User == -1)
			{	
				Log("err wsa_accept no empty", "-system", 0);

				break;
			}

			if(cIP[0] == LocalIP[0] && cIP[1] == LocalIP[1] && cIP[2] == LocalIP[2])
			{
				pUser[User].IP                  = TempUser.IP;
				pUser[User].Mode                = TempUser.Mode;
				pUser[User].cSock.Sock          = TempUser.cSock.Sock;
				pUser[User].cSock.nRecvPosition = TempUser.cSock.nRecvPosition;
				pUser[User].cSock.nProcPosition = TempUser.cSock.nProcPosition;
				pUser[User].cSock.nSendPosition = TempUser.cSock.nSendPosition;

				cFileDB.SendDBSignalParm3(User, 0, _MSG_DBSetIndex, ServerIndex, Sapphire, User);

				for(int i = 0; i < 65536; i++)
				{
					if(GuildInfo[i].Citizen)
						cFileDB.SendGuildInfo(User, i);

					if(g_pGuildWar[i])
						cFileDB.SendDBSignalParm2(User, 0, _MSG_War, i, g_pGuildWar[i]);

					if(g_pGuildAlly[i])
						cFileDB.SendDBSignalParm2(User, 0, _MSG_GuildAlly, i, g_pGuildAlly[i]);
				}

				if(TransperCharacter)
					cFileDB.SendDBSignalParm2(User, 0, _MSG_TransperCharacter, 0, 0);

				break;
			}
			else
			{
				TempUser.cSock.CloseSocket();

				char temp[256];
				sprintf(temp, "err,wsa_accept outer ethernet IP: %d.%d.%d.%d", cIP[0], cIP[1], cIP[2], cIP[3]);

				Log(temp, "-system", 0);

				break;
			}
		  		

		} break;

	case WM_CREATE:
	{
			HMENU hMenu, hSubMenu;
			// HICON hIcon, hIconSm;

			hMenu = CreateMenu();

			hSubMenu = CreatePopupMenu();
			AppendMenu(hSubMenu, MF_STRING, IDC_EXIT, "&Exit");
			AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT)hSubMenu, "&File");

			hSubMenu = CreatePopupMenu();
			AppendMenu(hSubMenu, MF_STRING, IDC_SHUTDOWNNP, "&Reload MobBase");
			AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT)hSubMenu, "&Sistema");

			SetMenu(hWnd, hMenu);
	} break;

	case WM_COMMAND:
	{
		switch(LOWORD(wParam))
		{
			case IDC_EXIT:
				PostMessage(hWnd, WM_CLOSE, 0, 0);
			break;

			case IDC_SHUTDOWNNP:
			{
#pragma region Load Base Characters

				int handle = -1;

				handle = _open("./BaseMob/TK", _O_RDONLY | _O_BINARY);

				if (handle == -1)
				{
					MessageBoxA(hWndMain, "no TransKnight file", "BOOTING ERROR", MB_OK);

					return FALSE;
				}

				_read(handle, &g_pBaseSet[0], sizeof(STRUCT_MOB));
				_close(handle);


				handle = -1;

				handle = _open("./BaseMob/FM", _O_RDONLY | _O_BINARY);

				if (handle == -1)
				{
					MessageBoxA(hWndMain, "no Foema file", "BOOTING ERROR", MB_OK);

					return FALSE;
				}

				_read(handle, &g_pBaseSet[1], sizeof(STRUCT_MOB));
				_close(handle);

				handle = -1;

				handle = _open("./BaseMob/BM", _O_RDONLY | _O_BINARY);

				if (handle == -1)
				{
					MessageBoxA(hWndMain, "no BeastMaster file", "BOOTING ERROR", MB_OK);

					return FALSE;
				}

				_read(handle, &g_pBaseSet[2], sizeof(STRUCT_MOB));
				_close(handle);

				handle = -1;

				handle = _open("./BaseMob/HT", _O_RDONLY | _O_BINARY);

				if (handle == -1)
				{
					MessageBoxA(hWndMain, "no Huntress file", "BOOTING ERROR", MB_OK);

					return FALSE;
				}

				_read(handle, &g_pBaseSet[3], sizeof(STRUCT_MOB));
				_close(handle);

				g_pBaseSet[0].BaseScore = g_pBaseSet[0].CurrentScore;
				g_pBaseSet[1].BaseScore = g_pBaseSet[1].CurrentScore;
				g_pBaseSet[2].BaseScore = g_pBaseSet[2].CurrentScore;
				g_pBaseSet[3].BaseScore = g_pBaseSet[3].CurrentScore;

#pragma endregion
			} break;
		}
	} break;

	case WM_PAINT:
		PAINTSTRUCT ps;
		memset(&ps, 0, sizeof(PAINTSTRUCT));

		BeginPaint(hWnd, &ps);

		hFont = CreateFont(12, 0, 0, 0, FW_LIGHT, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEVICE_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Fixedsys"); // |FF_DECORATIVE

		hDC = ps.hdc;

		DrawConfig();

		EndPaint(hWnd, &ps);
		break; 

	case WM_CLOSE:
		{	
			int rret = MessageBox(hWndMain, "Desligando servidor, não se esqueça de salvar as contas !", "Aviso !! Desligando Servidor !!", MB_OKCANCEL | MB_ICONWARNING);

			if(rret == 1)
			{
				WriteConfig();

				if (fLogFile)
					fclose(fLogFile);

				DayLog_ExpLog();
				DayLog_ItemLog();

				DefWindowProc(hWnd,message,wParam,lParam);
			}	

			return true;			

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

void ProcessClientMessage(int conn, char *msg)
{
	MSG_STANDARD *std = (MSG_STANDARD *)msg;

	if (!(std->Type & FLAG_GAME2DB) || (std->ID < 0) || (std->ID > MAX_USER)) 
	{	
		char temp[256];

		MSG_STANDARD *m = (MSG_STANDARD *)msg;

		sprintf(temp, "err,packet Type:%d ID:%d Size:%d KeyWord:%d", m->Type, m->ID, m->Size, m->KeyWord);

		Log(temp,"-system",0);

		return;
	}

	cFileDB.ProcessMessage(msg, conn);
}

void ProcessSecTimer()
{
	CReadFiles::ImportUser();
	CReadFiles::UpdateUser();

	SecCounter++;

	if(SecCounter % 30 == 0) 
	{
		CReadFiles::ImportItem(); // Once every two minutes
		CReadFiles::ImportDonate();
	}

	DrawConfig();

	if(SecCounter % 30 == 0)
	{
		CReadFiles::UpdateConnection();

		MinCounter++;

		if(MinCounter % 30 == 0)
		{
			CReadFiles::UpdateConnectionData();
			HourCounter++;
		}
	}

	if (SecCounter % 600 == 0)
	{
#pragma region Gerador de Ranking
		CReadFiles::WriteRanking();
#pragma endregion
	}
	struct tm when;
	time_t now;
	time(&now);
	when = *localtime(&now);

	if (when.tm_mday != LastLogDay)
		StartLog("A");

	if (when.tm_mday != LastDayLogDay)
	{
		DayLog_ItemLog();
		DayLog_ExpLog();
		LastDayLogDay = when.tm_mday;
	}
	if(when.tm_hour == 0 && when.tm_wday == 0 && when.tm_min == 0 && when.tm_sec == 0)
	{
		for(int i = 0; i < 65536;i++)
		{
			if(GuildInfo[i].Fame)
			{
				GuildInfo[i].Fame = 0;

				for(int j = 0; j < MAX_SERVER; j++)
				{
					if(pUser[j].cSock.Sock == 0)
						continue;

					if(pUser[j].Mode == 0)
						continue;

					cFileDB.SendGuildInfo(j, i);
				}
			}
		}

		CReadFiles::WriteGuildInfo();
	}
}

void ProcessMinTimer()
{

}


int GetUserFromSocket(int Sock)
{
	for(int i = 0; i < MAX_SERVER; i++)
	{   
		if(pUser[i].cSock.Sock == (unsigned)Sock) 
			return i;
	}

	return -1;
}

int GetAdminFromSocket(int Sock)
{
	for(int i = 0; i < MAX_ADMIN; i++)
	{   
		if(pAdmin[i].cSock.Sock == (unsigned)Sock) 
			return i;
	}

	return -1;
}

int GetEmptyUser()
{
	for(int i = 0; i < MAX_SERVER; i++)
	{   
		if(pUser[i].Mode == USER_EMPTY) 
			return i;
	}

	return -1;
}
