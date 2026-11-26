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

/// @file Server_Admin.cpp
/// @brief Protocolo administrativo do DBSrv: envio de sinais/mensagens ao cliente admin,
///        processamento de pacotes de administração (NP/Admin) e bloqueio/desbloqueio de contas.
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

int SendAdminSignal(int svr, unsigned short id, unsigned short signal)
{
	MSG_STANDARD sm;

	sm.Type = signal;
	sm.ID = id;
	sm.Size = sizeof(MSG_STANDARD);

	pAdmin[svr].cSock.SendOneMessage((char*)&sm, sizeof(sm));

	return TRUE;
}

int SendAdminMessage(int svr, unsigned short id, char *message)
{
	MSG_NPNotice sm;

	sm.Type = _MSG_NPNotice;
	sm.ID = id;
	sm.Size = sizeof(MSG_NPNotice);

	strncpy(sm.String, message, MESSAGE_LENGTH);

	pAdmin[svr].cSock.SendOneMessage((char*)&sm, sizeof(sm));

	return TRUE;
}

int SendAdminParm(int svr, unsigned short id, unsigned short signal, int parm)
{
	MSG_STANDARDPARM sm;

	sm.Type = signal;
	sm.ID = id;
	sm.Size = sizeof(MSG_STANDARDPARM);
	sm.Parm = parm;

	pAdmin[svr].cSock.SendOneMessage((char*)&sm, sizeof(sm));

	return TRUE;
}

int SendAdminState(int svr, unsigned short id, unsigned short signal, char *pass)
{
	MSG_STANDARDPARM sm;

	sm.Type = signal;
	sm.ID	= id;
	sm.Size = sizeof(MSG_STANDARDPARM);

	if(pass[0] == '@') sm.Parm = 1;
	if(pass[0] == '_') sm.Parm = 2;
	if(pass[0] == '#') sm.Parm = 3;

	pAdmin[svr].cSock.SendOneMessage((char*)&sm, sizeof(sm));

	return TRUE;
}

void ReadAdmin()
{
	char temp[256];

	sprintf(temp, "Admin.txt");

	FILE *fp = fopen(temp, "r");

	if(fp == NULL) 
		return;

	while(1)
	{
		int a, b, c, d;
		a = b = c = d = 0;

		char * ret = fgets((char*)temp, 127, fp);    

		if(ret == NULL) 
			break;

		int idx = -1;

		for(int i = 0; i < 255; i++) 
			if(temp[i] == '.')
				temp[i] = ' ';

		sscanf(temp, "%d %d %d %d %d", &idx, &a, &b, &c, &d);

		unsigned int ip = (d << 24) + (c << 16) + (b << 8) + a; 

		if (idx < 0 || idx >= MAX_ADMIN) 
			continue;

		pAdminIP[idx] = ip;
	};

	fclose(fp);
}

int ProcessAdminClientMessage(char *msg)
{
	MSG_STANDARD *std = (MSG_STANDARD *)msg;

	if(!(std->Type & FLAG_NP2DB) || (std->ID < 0) || (std->ID >= MAX_DBACCOUNT))
	{	
		char temp[128];
		sprintf(temp, "err,adminclient_packet:%d ID:%d Size:%d KeyWord:%d", std->Type, std->ID, std->Size, std->KeyWord);

		Log(temp,"-system",0);

		return FALSE;
	} 

    switch(std->Type)    
	{
		case _MSG_NPCreateCharacter_Reply:
		{	
			MSG_NPCreateCharacter_Reply *m = (MSG_NPCreateCharacter_Reply*)msg;

			char * account	= m->Account;
			int slot		= m->Slot;
			int result		= m->Result;
			int IdxName		= cFileDB.GetIndex(m->Account);
			int	svr			= IdxName / MAX_USER;
			int	id			= IdxName % MAX_USER;

			MSG_ReqTransper sm; 
			memset(&sm, 0, sizeof(sm));

			sm.Type = _MSG_ReqTransper; 
			sm.Size = sizeof(sm);
			sm.ID	= id;
			sm.Slot = slot;
			sm.Result = m->Result;

			EnableAccount(-1, account);

			if(m->Result == 0)//; //0: Success Delete Parm2
			{	
				STRUCT_ACCOUNTFILE file;
				memcpy(file.Info.AccountName, m->Account, ACCOUNTNAME_LENGTH);

				int ret = cFileDB.DBReadAccount(&file);

				if(ret == FALSE)
				{	
					char tt[256]; 
					sprintf(tt, "etc tranper_FAILED!!!!!(read) %s %s", m->Account, m->Name);

					Log(tt, file.Info.AccountName, 0);

					if(IdxName)	
					{
						sm.Result = 4;
						pUser[svr].cSock.SendOneMessage((char*)&sm, sm.Size);
					}

					return TRUE;
				}

				char tt[256]; 
				sprintf(tt, "etc tranper_success(deleted) %s %s", m->Account, m->Name);

				Log(tt, file.Info.AccountName, 0);

				memset(&file.Char[slot], 0, sizeof(STRUCT_MOB));

				_strupr(file.Info.AccountName);

				ret = cFileDB.DBWriteAccount(&file);

	            if(ret == FALSE)
				{	
					char tt[256]; 
					sprintf(tt, "etc tranper_FAILED!!!!!(write) %s %s", m->Account, m->Name);

					Log(tt, file.Info.AccountName, 0);

					if(IdxName)	
					{
						sm.Result = 4;	
						pUser[svr].cSock.SendOneMessage((char*)&sm, sm.Size);
					}

					return TRUE;
				}

				if(IdxName)
				{	
					pUser[svr].cSock.SendOneMessage((char*)&sm, sm.Size);

					memset(&(cFileDB.pAccountList[IdxName].File.Char[slot]), 0, sizeof(STRUCT_MOB));
				}
			}
			if(result == 1) //1: Threre's Same Name Already
			{	
				if(IdxName)	
					pUser[svr].cSock.SendOneMessage((char*)&sm, sm.Size);
			}	
			else if(result == 2) //2: No character space. delete first.
			{	
				if(IdxName)	
					pUser[svr].cSock.SendOneMessage((char*)&sm, sm.Size);
			}	
			else if(result == 3) //3: That account is playing
			{	
				if(IdxName)	
					pUser[svr].cSock.SendOneMessage((char*)&sm, sm.Size);
			}
		}
		break;	
	}
	
	return TRUE;
}

void DisableAccount(int conn, char *account, int Year, int YearDay)
{
	account[ACCOUNTNAME_LENGTH-1] = 0;
	account[ACCOUNTNAME_LENGTH-2] = 0;

	_strupr(account);

	STRUCT_ACCOUNTFILE file;
	memcpy(file.Info.AccountName, account, ACCOUNTNAME_LENGTH);

	int iret = cFileDB.DBReadAccount(&file); 

	if(iret == FALSE) 
	{	
		if(conn >= 0) 
			SendAdminSignal(conn, 0, _MSG_NPNotFound);

		if(conn >= 0) 
			SendAdminMessage(conn, 0, "There's no account with that account name");

		return;
	}
	
	if(file.Info.YearDay != 0 && file.Info.Year != 0) 
	{	
		if(conn >= 0) 
			SendAdminState(conn, 0, _MSG_NPState, "1");

		return;
	}

	file.Info.AccountPass[ACCOUNTPASS_LENGTH-1] = 0;
	file.Info.AccountPass[ACCOUNTPASS_LENGTH-2] = 0;

	file.Info.Year = Year;
	file.Info.YearDay = YearDay;

	cFileDB.DBWriteAccount(&file);

	if(conn >= 0)
	{	
		int idx = pAdmin[conn].DisableID;

		MSG_NPReqAccount sm; 
		memset(&sm, 0, sizeof(MSG_NPReqAccount));

		sm.Type = _MSG_NPReqAccount; 
		sm.Size = sizeof(MSG_NPReqAccount);
		sm.ID = idx;

		strncpy(sm.Account, account, ACCOUNTNAME_LENGTH);

		sm.Char[0] = 0;

		ProcessAdminMessage(conn, (char*)&sm);

		pAdmin[conn].DisableID = 0;
	}
}

void EnableAccount(int conn, char *account)
{
	account[ACCOUNTNAME_LENGTH-1] = 0;
	account[ACCOUNTNAME_LENGTH-2] = 0;

	_strupr(account);

	STRUCT_ACCOUNTFILE file;
  	memcpy(file.Info.AccountName, account, ACCOUNTNAME_LENGTH);

	int iret = cFileDB.DBReadAccount(&file);  

	if(iret == FALSE) 
	{	
		if(conn >= 0) 
			SendAdminSignal(conn, 0, _MSG_NPNotFound);

		if(conn >= 0) 
			SendAdminMessage(conn, 0, "There's no account with that account name");

		return;
	}

	if(file.Info.Year == 0 && file.Info.YearDay == 0) 
	{	
		if(conn >= 0) 
			SendAdminState(conn, 0, _MSG_NPState, file.Info.AccountPass);

		return;
	}

	file.Info.AccountPass[ACCOUNTPASS_LENGTH-1] = 0;
	file.Info.AccountPass[ACCOUNTPASS_LENGTH-2] = 0;
	file.Info.Year = 0;
	file.Info.YearDay = 0;

	cFileDB.DBWriteAccount(&file);

	if(conn >= 0)
		SendAdminState(conn, 0, _MSG_NPState, file.Info.AccountPass);

	return ;
}

int ProcessAdminMessage(int conn, char *msg)
{
	MSG_STANDARD *std = (MSG_STANDARD *)msg;

	if(!(std->Type & FLAG_NP2DB) || (std->ID < 0) || (std->ID >= MAX_DBACCOUNT))
	{	
		MSG_STANDARD *m = (MSG_STANDARD *)msg;

		char temp[128];
		sprintf(temp, "err,packet:%d ID:%d Size:%d KeyWord:%d", m->Type, m->ID, m->Size, m->KeyWord);

		Log(temp,"-system",0);

		return FALSE;
	} 

	switch(std->Type)
	{
	case _MSG_NPCreateCharacter:
		{
			MSG_NPCreateCharacter *m = (MSG_NPCreateCharacter*)msg;

			MSG_NPCreateCharacter_Reply	sm; 
			memset(&sm, 0, sizeof(MSG_NPCreateCharacter_Reply));
			sm.Type = _MSG_NPCreateCharacter_Reply;
			sm.Size		= sizeof(MSG_NPCreateCharacter_Reply);
			sm.Slot		= m->Slot;
			sm.ID		= m->ID;
			sm.Result	= 0;

			strncpy(sm.Account, m->Account, ACCOUNTNAME_LENGTH);
			strncpy(sm.Name, m->Mob.MobName, NAME_LENGTH);

			char *account	= m->Account;
			int slot		= m->Slot;
			int IdxName		= cFileDB.GetIndex(m->Account);

			if(IdxName != 0)	
			{
				sm.Result = 3;

				pAdmin[conn].cSock.SendOneMessage((char*)&sm, sm.Size);

				return TRUE;
			}

			STRUCT_ACCOUNTFILE file;

			memcpy(file.Info.AccountName, m->Account, ACCOUNTNAME_LENGTH);

			int ret = cFileDB.DBReadAccount(&file);

			if(ret == FALSE)
			{	
				char tt[256]; 
				sprintf(tt, "etc,tranper_FAILED!!!!!(read) %s %s", m->Account, m->Mob.MobName);
				Log(tt, file.Info.AccountName, 0);

				sm.Result = 4;

				pAdmin[conn].cSock.SendOneMessage((char*)&sm, sm.Size);

				return TRUE;
			}

			int empty;

			for(empty = 0; empty < MOB_PER_ACCOUNT; empty++) 
			{	
				if(file.Char[empty].MobName[0] == 0) 
					break;
			}

			if(empty == MOB_PER_ACCOUNT)	
			{
				sm.Result = 2;
				
				pAdmin[conn].cSock.SendOneMessage((char*)&sm, sm.Size);

				return TRUE;
			}

			ret = cFileDB.CreateCharacter(m->Account, m->Mob.MobName);

			if(ret == FALSE)
			{	
				sm.Result = 1;

				pAdmin[conn].cSock.SendOneMessage((char*)&sm, sm.Size);

				return TRUE;
			}

			for(int i = 0; i < MAX_EQUIP + MAX_CARRY -1 ; i++)
			{
				STRUCT_ITEM *sour;

				if(i < 16)
					sour = &m->Mob.Equip[i];
				else
					sour = &m->Mob.Carry[i];

				int sidx = sour->sIndex;

				if(sidx == 446 || (sidx >= 508 && sidx <= 509) || (sidx >= 526 && sidx <= 537) || sidx == 522)
					ConvertGuildNumber(conn, sour);
			}
	
			char tt[256]; 
			sprintf(tt, "etc,tranper_success(deleted) %s %s", m->Account, m->Mob.MobName);
			Log(tt, file.Info.AccountName, 0);

			memcpy(&file.Char[empty], &(m->Mob), sizeof(STRUCT_MOB));

			_strupr(file.Info.AccountName);

			ret = cFileDB.DBWriteAccount(&file);

			if(ret == FALSE)
			{	
				char tt[256]; 
				sprintf(tt, "etc,tranper_FAILED!!!!!(write) %s %s", m->Account, m->Mob.MobName);
				Log(tt, file.Info.AccountName, 0);

				sm.Result = 4;
				
				pAdmin[conn].cSock.SendOneMessage((char*)&sm, sm.Size);

				return TRUE;
			}

			sm.Result = 0;
			
			pAdmin[conn].cSock.SendOneMessage((char*)&sm, sm.Size);

		} break;

	case _MSG_NPNotice:
		{
			MSG_NPNotice *m = (MSG_NPNotice*)msg;

			m->AccountName[ACCOUNTNAME_LENGTH-1] = 0;
			m->AccountName[ACCOUNTNAME_LENGTH-2] = 0;
			m->String[MAX_NOTIFY_LENGTH-1] = 0;
			m->String[MAX_NOTIFY_LENGTH-2] = 0;

			if(m->AccountName[0] == 0 && m->Parm1 == 1)
			{
				if(pAdmin[conn].Level < 2)
					return TRUE;

				for (int i = 0; i < MAX_SERVER; i++)
				{	
					if(pUser[i].Mode == USER_EMPTY)
						continue;

					if(pUser[i].cSock.Sock == NULL)	
						continue;

					MSG_NPNotice sm;
					memset(&sm, 0, sizeof(MSG_NPNotice));

					sm.Type = _MSG_NPNotice;
					sm.ID = 0;
					sm.Size = sizeof(MSG_NPNotice);

					sm.Parm1 = 1;
					sm.Parm2 = 1;

					strncpy(sm.String, m->String, MAX_NOTIFY_LENGTH);

					pUser[i].cSock.SendOneMessage((char*)&sm, sizeof(sm));
				}

				return TRUE;
			}

			if(pAdmin[conn].Level <= 0)
				return TRUE;

			int IdxName = cFileDB.GetIndex(m->AccountName);

			if(IdxName == 0)
			{   
				SendAdminMessage(conn, 0, "Specified user not connected. can't send notice.");

				return TRUE;
			}

			int svr = IdxName / MAX_USER;
			int id = IdxName % MAX_USER;

			if(svr < 0|| svr >= MAX_SERVER || id <= 0 || id >= MAX_USER)
			{	
				SendAdminMessage(conn, 0, "Wrong SVR and ID");

				return TRUE;
			}

			MSG_NPNotice sm;
			memset(&sm, 0, sizeof(MSG_NPNotice));

			sm.Type = _MSG_NPNotice;
			sm.ID = id;
			sm.Size = sizeof(MSG_NPNotice);

			sm.Parm1 = 0;
			sm.Parm2 = 0;

			strncpy(sm.String, m->String, MAX_NOTIFY_LENGTH);

			pUser[svr].cSock.SendOneMessage((char*)&sm, sizeof(sm));

			return TRUE;

		} break;

	case _MSG_NPIDPASS:
		{
			MSG_NPIDPASS *m = (MSG_NPIDPASS *) msg;

			_strupr(m->Account);

			if(pAdmin[conn].Encode1 != m->Encode1 || pAdmin[conn].Encode2 != m->Encode2) 
				return FALSE;

			if(pAdmin[conn].Level != -1) 
				return TRUE;

			STRUCT_ACCOUNTFILE file;

			memcpy(file.Info.AccountName, m->Account, ACCOUNTNAME_LENGTH);

			int iret = cFileDB.DBReadAccount(&file);  

			if(iret == FALSE) 
				return TRUE;

			if(file.Info.AccountPass[0] == '_') 
				return FALSE;
			if(file.Info.AccountPass[0] == '@') 
				return FALSE; 

			char *p = (char*)file.Info.AccountPass;

			iret = strncmp(m->Pass, p, ACCOUNTPASS_LENGTH);

			if(iret) 
				return FALSE;

			int maxlevel = 0;

			for (int i = 0; i < MOB_PER_ACCOUNT; i++)
			{  
				if (file.Char[i].MobName[0] != 0 && file.Char[i].BaseScore.Level > maxlevel) 
					maxlevel = file.Char[i].BaseScore.Level;
			}

			if(maxlevel < 1000) 
				return FALSE;

			int admin = maxlevel -  1000;

			pAdmin[conn].Level = admin;
			pAdmin[conn].DisableID = 0;

			strncpy(pAdmin[conn].Name, m->Account, ACCOUNTNAME_LENGTH);

			char temp[256];
			sprintf(temp, "sys,Admin Login Success - Level: %d", admin);
			Log(temp, pAdmin[conn].Name, pAdmin[conn].IP);  
			SendAdminMessage(conn, 0, temp);

		} break;

	case _MSG_NPReqAccount: 
		{
			MSG_NPReqAccount *m = (MSG_NPReqAccount*)msg;

			if(pAdmin[conn].Level <= 0)
				return TRUE;

			_strupr(m->Account);
			_strupr(m->Char);

			if(m->Char[0] != 0)		
				cFileDB.GetAccountByChar(m->Account, m->Char); 


			if(m->Account[0] == 0)
			{
				SendAdminSignal(conn, 0, _MSG_NPNotFound);
				SendAdminMessage(conn, 0, "There's no account with that character name");

				return TRUE;
			}  

			STRUCT_ACCOUNTFILE file;

			memcpy(file.Info.AccountName, m->Account, ACCOUNTNAME_LENGTH);

			int iret = cFileDB.DBReadAccount(&file);

			if(iret == FALSE)
			{	
				SendAdminSignal(conn, 0, _MSG_NPNotFound);
				SendAdminMessage(conn, 0, "There's no account with that account name");

				return TRUE;
			}

			MSG_NPAccountInfo sm; 
			memset(&sm, 0, sizeof(MSG_NPAccountInfo));

			sm.Type = _MSG_NPAccountInfo; 
			sm.Size = sizeof(MSG_NPAccountInfo);
			sm.ID = 0;
			sm.account = file; 

			int IdxName = cFileDB.GetIndex(m->Account);

			sm.Session = IdxName;

			file.Char[0].MobName[NAME_LENGTH-1] = 0;
			file.Char[0].MobName[NAME_LENGTH-2] = 0;
			file.Char[1].MobName[NAME_LENGTH-1] = 0;
			file.Char[1].MobName[NAME_LENGTH-2] = 0;
			file.Char[2].MobName[NAME_LENGTH-1] = 0;
			file.Char[2].MobName[NAME_LENGTH-2] = 0;
			file.Char[3].MobName[NAME_LENGTH-1] = 0;
			file.Char[3].MobName[NAME_LENGTH-2] = 0;
			file.Info.AccountName[ACCOUNTNAME_LENGTH-1] = 0;
			file.Info.AccountName[ACCOUNTNAME_LENGTH-2] = 0;

			if(pAdmin[conn].Level <= 0)
				return TRUE;

			int maxlevel = 0;

			for(int i = 0; i < MOB_PER_ACCOUNT; i++)
			{
				if (file.Char[i].MobName[0] != 0 && file.Char[i].BaseScore.Level > maxlevel) 
					maxlevel = file.Char[i].BaseScore.Level;

			}

			int admin = maxlevel - 1000;

			sm.State = 0;

			char accountstate[256];
			memset(accountstate, 0, 255);

			if (sm.account.Info.AccountPass[0] == '@' || sm.account.Info.Year != 0) 
			{
				sprintf(accountstate, "Conta bloqueada.");
				sm.State = 1;
			}
			if (sm.account.Info.AccountPass[0] == '_') 
			{
				sprintf(accountstate, "Conta Defeituosa.");
				sm.State = 2;
			}
			if (sm.account.Info.AccountPass[0] == '#') 
			{
				sprintf(accountstate, "Conta Desativada.");
				sm.State = 3;
			}

			char temp[256]; 			
			sprintf(temp, "%s < %s | %s | %s | %s> - %s", file.Info.AccountName, file.Char[0].MobName, file.Char[1].MobName, file.Char[2].MobName, file.Char[3].MobName, accountstate);

			temp[MESSAGE_LENGTH-1] = 0;
			temp[MESSAGE_LENGTH-2] = 0;

			SendAdminMessage(conn, 0, temp);
			/*
			sm.account.Info.AccountPass[0] = rand() % 24 + 'A';
			sm.account.Info.AccountPass[1] = rand() % 24 + 'A';
			sm.account.Info.AccountPass[2] = rand() % 24 + 'A';
			sm.account.Info.AccountPass[3] = rand() % 24 + 'A';
			sm.account.Info.AccountPass[4] = 0;
			*/
			pAdmin[conn].cSock.SendOneMessage((char*)&sm, sizeof(MSG_NPAccountInfo));

		} break;

	case _MSG_NPReqSaveAccount:
		{
			if(pAdmin[conn].Level <= 2)
			{
				SendAdminMessage(conn, 0, "Not allowed");

				return TRUE;
			}

			MSG_NPAccountInfo *m = (MSG_NPAccountInfo*)msg;

			int IdxName = cFileDB.GetIndex(m->account.Info.AccountName);

			if(IdxName != 0)
			{	
				SendAdminMessage(conn, 0, "For saving, account should be disabled.");

				return TRUE;
			}

			int maxlevel = 0;

			for(int i = 0; i < MOB_PER_ACCOUNT; i++)
			{

				if (m->account.Char[i].MobName[0] != 0 && m->account.Char[i].BaseScore.Level > maxlevel) 
					maxlevel = m->account.Char[i].BaseScore.Level;

			}

			int admin = maxlevel - 1000;

			if(maxlevel >= 2000)
			{
				if(admin > pAdmin[conn].Level)
				{
					SendAdminMessage(conn, 0, "Set admin level error.");

					return TRUE;
				}

				if(pAdmin[conn].Level == admin && strcmp(pAdmin[conn].Name, m->account.Info.AccountName))
				{
					SendAdminMessage(conn, 0, "Set admin level error.");

					return TRUE;
				}

			}

			STRUCT_ACCOUNTFILE tmpact;

			memcpy(tmpact.Info.AccountName, m->account.Info.AccountName, ACCOUNTNAME_LENGTH);

			int iret = cFileDB.DBReadAccount(&tmpact);

			if(!iret)
			{
				SendAdminMessage(conn, 0, "There's no account with that account name");

				return TRUE;
			}

			memcpy(m->account.Info.AccountPass, tmpact.Info.AccountPass, ACCOUNTPASS_LENGTH);

			cFileDB.DBWriteAccount(&m->account);

			char temp[128]; 
			sprintf(temp, "Saving account [%s] success", m->account.Info.AccountName);
			SendAdminMessage(conn, 0, temp);

			sprintf(temp, "sys,%s written", m->account.Info.AccountName);

			Log(temp, pAdmin[conn].Name, pAdmin[conn].IP);

		} break;

	case _MSG_NPDisable:
		{
			if(pAdmin[conn].Level <= 2) 
				return TRUE;

			MSG_NPEnable *m = (MSG_NPEnable*)msg;

			m->AccountName[ACCOUNTNAME_LENGTH-1] = 0;
			m->AccountName[ACCOUNTNAME_LENGTH-2] = 0;

			_strupr(m->AccountName);

			int IdxName = cFileDB.GetIndex(m->AccountName);

			if(IdxName == 0)
			{	
				DisableAccount(conn, m->AccountName, m->Year, m->YearDay);

				return TRUE;
			}	
			else
			{	
				cFileDB.SendDBSavingQuit(IdxName, 1);
				pAdmin[conn].DisableID = IdxName;
				pAdmin[conn].Year = m->Year;
				pAdmin[conn].YearDay = m->YearDay;
			}

		} break;

	case _MSG_NPEnable:
		{
			if(pAdmin[conn].Level <= 2) 
				return TRUE;

			MSG_NPEnable *m = (MSG_NPEnable*)msg;

			m->AccountName[ACCOUNTNAME_LENGTH-1] = 0;
			m->AccountName[ACCOUNTNAME_LENGTH-2] = 0;

			_strupr(m->AccountName);

			int IdxName = cFileDB.GetIndex(m->AccountName);

			if (IdxName != 0)
			{	
				SendAdminMessage(conn, 0, "Check session. already playing.");

				return TRUE;
			}

			EnableAccount(conn, m->AccountName);	

		} break;

	case _MSG_NPDonate:
		{
			MSG_NPDonate *m = (MSG_NPDonate*)msg;

			char filename[256];
			char name[256];


			struct tm when;
			time_t now;
			time(&now);
			when = *localtime(&now);

			sprintf(name, "%02d%02d%04d%02d%02d%02d%02d", when.tm_mday, when.tm_mon, when.tm_year+1900, when.tm_hour, when.tm_min, when.tm_sec, rand()%255);
			sprintf(filename, "../../Common/ImportDonate/%s", name);

			FILE *fp = fopen(filename, "wt");

			if(fp == NULL)
				break;

			fprintf(fp, "%s %d", m->AccountName, m->Donate);

			fclose(fp);
		} break;
	}

	return TRUE;

}
