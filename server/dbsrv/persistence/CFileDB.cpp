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

#include <windows.h>
#include <ERRNO.H>
#include <stdlib.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h> 

#include "persistence/CFileDB.h"
#include "net/CUser.h"
#include "core/Server.h"
#include "persistence/CRanking.h"
#include "core/CReadFiles.h"
#include "common/Basedef.h"


extern int ServerIndex; //Server.cpp
extern int Sapphire;
extern int TransperCharacter;

extern CUser pUser[MAX_SERVER]; // Server.cpp
extern CUser pAdmin[MAX_ADMIN]; // Server.cpp

extern CPSock AdminClient;

extern STRUCT_MOB g_pBaseSet[MAX_CLASS]; // Server.cpp

extern int  ChargedGuildList[MAX_SERVER][MAX_GUILDZONE];
extern STRUCT_ITEMLOG ItemDayLog[MAX_ITEMLIST];

unsigned short	g_pGuildWar[65536] = {0,};
unsigned short	g_pGuildAlly[65536] = {0,};
STRUCT_GUILDINFO GuildInfo[65536];
unsigned short  LastCapsule = 0; 


CFileDB::CFileDB()
{
	for(int i = 0; i < MAX_DBACCOUNT; i++)
	{
		pAccountList[i].Login =  0;
        pAccountList[i].Slot  = -1;
		pAccountList[i].SecurePass = -1;

		pAccountList[i].Mac[0] = 0;
		pAccountList[i].Mac[1] = 0;
		pAccountList[i].Mac[2] = 0;
		pAccountList[i].Mac[3] = 0;

		memset(&pAccountList[i].File, 0, sizeof(pAccountList[i].File));
	}
}

CFileDB::~CFileDB()
{
}

int CFileDB::AddAccount(char *id, char *pass, char *name, int ssn1, int ssn2, char *email, char *tel, char *addr, int bonus)
{
	char check[ACCOUNTNAME_LENGTH];

	strncpy(check, id, ACCOUNTNAME_LENGTH);

	_strupr(check);

	if(check[0] == 'C' && check[1] == 'O' && check[2] == 'M' && check[3] >= '0' && check[3] <= '9' && check[4] == 0)
		return FALSE;
	if(check[0] == 'L' && check[1] == 'P' && check[2] == 'T' && check[3] >= '0' && check[3] <= '9' && check[4] == 0)
		return FALSE;

	char First[128];

	BASE_GetFirstKey(id, First);

	char temp[128];

	sprintf(temp, "./account/%s/%s", First, check);

	int Handle = _open(temp, O_RDONLY | O_BINARY);

	if(Handle != -1)
	{
		_close(Handle);

		return FALSE;
	}

	STRUCT_ACCOUNTFILE file;

	memset(&file, 0, sizeof(STRUCT_ACCOUNTFILE));

	strncpy(file.Info.AccountName, id, ACCOUNTNAME_LENGTH);
	strncpy(file.Info.AccountPass, pass, ACCOUNTPASS_LENGTH);
	strncpy(file.Info.RealName, name, REALNAME_LENGTH);
	strncpy(file.Info.Email, email, EMAIL_LENGTH);
	strncpy(file.Info.Telephone, tel, TELEPHONE_LENGTH);
	strncpy(file.Info.Address, addr, ADDRESS_LENGTH);

	file.Info.NumericToken[0] = -1;
	file.Donate = 0;
	file.Info.SSN1 = ssn1;
	file.Info.SSN2 = ssn2;

	memset(file.ShortSkill, -1, 8);

	if(bonus != 0)
	{
		for(int i = 0; i < 100; i++)
		{
			if(i < 50)
				file.Cargo[i].sIndex = 401;
			else
				file.Cargo[i].sIndex = 406;
		}
	}

	int ret = DBWriteAccount(&file);

	if(ret == 0)
		return FALSE;

	return TRUE;
}

int CFileDB::UpdateAccount(char *id, char *pass)
{
	char check[ACCOUNTNAME_LENGTH];

	strncpy(check, id, ACCOUNTNAME_LENGTH);

	_strupr(check);

	if(check[0] == 'C' && check[1] == 'O' && check[2] == 'M' && check[3] >= '0' && check[3] <= '9' && check[4] == 0)
		return FALSE;
	if(check[0] == 'L' && check[1] == 'P' && check[2] == 'T' && check[3] >= '0' && check[3] <= '9' && check[4] == 0)
		return FALSE;

	char First[128];

	BASE_GetFirstKey(id, First);

	char temp[128];

	sprintf(temp, "./account/%s/%s", First, check);

	int Handle = _open(temp, O_RDONLY | O_BINARY);

	if(Handle == -1)
		return FALSE;

	_close(Handle);

	int Idx = GetIndex(id);

	if(Idx != 0)
		strncpy(pAccountList[Idx].File.Info.AccountPass, pass, ACCOUNTPASS_LENGTH);

	STRUCT_ACCOUNTFILE file;

	memcpy(file.Info.AccountName, id, ACCOUNTNAME_LENGTH);

	int ret = DBReadAccount(&file);

	if(ret == FALSE)
	{
		Log("err, updateaccount read fail", id, 0);

		return FALSE;
	}

	strncpy(file.Info.AccountPass, pass, ACCOUNTPASS_LENGTH);

	ret = DBWriteAccount(&file);

	if(ret == FALSE)
	{
		Log("err, updateAccount write fail", id, 0);

		return FALSE;
	}

	int IdxName = GetIndex(file.Info.AccountName);

	if(IdxName > 0 && IdxName < MAX_DBACCOUNT)
		strncpy(file.Info.AccountPass, pass, ACCOUNTPASS_LENGTH);

	return TRUE;
}

int ProcessRecord(int conn, char * str)
{
	char file[256];

	struct tm when;
	time_t now;
	time(&now);
	when = *localtime(&now);

    sprintf(file,"RC%4.4d_%2.2d_%2.2d_%2.2d_%2.2d_%d2.2d_%2d.txt", when.tm_year-100, when.tm_mon+1, when.tm_mday, when.tm_hour, when.tm_min, when.tm_sec, conn);
	
	char filetemp[236];
   
	sprintf(filetemp, "../../Common/record%d/%s", ServerIndex, file);
	
	FILE *fp = fopen(filetemp, "wt");

	if(fp == NULL)
		return FALSE;
	
	fprintf(fp, "%s", str);
	fclose(fp);

	return TRUE;
}

int CFileDB::ProcessMessage(char *Msg, int conn)
{
	bool handled = false;

	int ret = ProcessAccountMessage(Msg, conn, handled);
	if (handled)
		return ret;

	ret = ProcessWorldMessage(Msg, conn, handled);
	if (handled)
		return ret;

	return TRUE;
}

int CFileDB::SendDBSignal(int svr, unsigned short id, unsigned short signal)
{
	MSG_STANDARD sm;

	sm.Type = signal;
    sm.ID = id;
	sm.Size = sizeof(sm);

	if(pUser[svr].cSock.Sock)
		pUser[svr].cSock.SendOneMessage((char*)&sm, sizeof(sm));

	return TRUE;
}

int CFileDB::SendDBSignalParm(int svr, unsigned short id, unsigned short signal, int parm)
{
	 MSG_STANDARDPARM sm;

	 sm.Type = signal;
     sm.ID = id;
	 sm.Size = sizeof(sm);
	 sm.Parm = parm;

	 if(pUser[svr].cSock.Sock)
		 pUser[svr].cSock.SendOneMessage((char*)&sm, sizeof(sm));

	 return TRUE;
}

int CFileDB::SendDBSignalParm2(int svr, unsigned short id, unsigned short signal, int parm1, int parm2)
{
	MSG_STANDARDPARM2 sm;

	sm.Type = signal;
    sm.ID = id;
	sm.Size = sizeof(sm);
	sm.Parm1 = parm1;
	sm.Parm2 = parm2;

	if(pUser[svr].cSock.Sock)
		pUser[svr].cSock.SendOneMessage((char*)&sm, sizeof(sm));

	return TRUE;
}

int CFileDB::SendDBSignalParm3(int svr, unsigned short id, unsigned short signal, int parm1, int parm2, int parm3)
{
	MSG_STANDARDPARM3 sm;
	
	sm.Type = signal;
    sm.ID = id;
	sm.Size = sizeof(sm);
	sm.Parm1 = parm1;
	sm.Parm2 = parm2;
	sm.Parm3 = parm3;

	if(pUser[svr].cSock.Sock)
		pUser[svr].cSock.SendOneMessage((char*)&sm, sizeof(sm));

	return TRUE;
}

int CFileDB::SendDBMessage(int svr, unsigned short id, char *msg)
{
	MSG_MessagePanel sm;

	sm.Type = _MSG_DBMessagePanel;  
    sm.ID = id;
	sm.Size = sizeof(MSG_MessagePanel);

    strncpy(sm.String, msg, MESSAGE_LENGTH);

    pUser[svr].cSock.SendOneMessage((char*)&sm, sizeof(MSG_MessagePanel));
	
	return TRUE;
}

void CFileDB::SendGuildInfo(int srv, int Guild)
{
	MSG_GuildInfo sm;

	sm.Type = _MSG_GuildInfo;  
    sm.ID = 0;
	sm.Size = sizeof(MSG_GuildInfo);

	sm.Guild = Guild;
//	sm.GuildInfo = GuildInfo[Guild];

	memcpy(&sm.GuildInfo, &GuildInfo[Guild], sizeof(STRUCT_GUILDINFO));

	if(pUser[srv].cSock.Sock)
		pUser[srv].cSock.SendOneMessage((char*)&sm, sizeof(MSG_GuildInfo));
}

void CFileDB::AddAccountList(int Idx)
{ 
	if(pAccountList[Idx].Login == TRUE)
	{
		Log("err,addAccountlist - already added", pAccountList[Idx].File.Info.AccountName, 0);

		return;
	}

    int conn = Idx / MAX_USER;

	pUser[conn].Count++;

	pAccountList[Idx].Login = TRUE;
	pAccountList[Idx].Slot  = -1;

	return;
}

void CFileDB::RemoveAccountList(int Idx)
{
    if(pAccountList[Idx].Login == FALSE)
		return;
	
	int conn = Idx / MAX_USER;

	pUser[conn].Count--;

	pAccountList[Idx].Login = FALSE;
	pAccountList[Idx].Slot  = -1;

	memset(&pAccountList[Idx].File, 0, sizeof(pAccountList[Idx].File));
}

int CFileDB::CreateCharacter(char *ac, char *ch)
{
	char check[ACCOUNTNAME_LENGTH];

	strncpy(check, ac, ACCOUNTNAME_LENGTH);

	_strupr(check);

	if(check[0] == 'C' && check[1] == 'O' && check[2] == 'M' && check[3] >= '0' && check[3] <= '9' && check[4] == 0)
		return FALSE;
	if(check[0] == 'L' && check[1] == 'P' && check[2] == 'T' && check[3] >= '0' && check[3] <= '9' && check[4] == 0)
		return FALSE;

	strncpy(check, ch, NAME_LENGTH);

	_strupr(check);

	if(check[0] == 'C' && check[1] == 'O' && check[2] == 'M' && check[3] >= '0' && check[3] <= '9' && check[4] == 0)
		return FALSE;
	if(check[0] == 'L' && check[1] == 'P' && check[2] == 'T' && check[3] >= '0' && check[3] <= '9' && check[4] == 0)
		return FALSE;

	char First[128];
	char temp[128];

	BASE_GetFirstKey(check, First);

	sprintf(temp, "./char/%s/%s", First, check);

	FILE *fp = NULL;

	fp = fopen(temp, "r");

	if(fp != NULL)
	{
		fclose(fp);
		Log("err createchar EEXIST", ch, 0);

        return FALSE;
	}

	int Handle = _open(temp, O_RDWR | O_CREAT | O_BINARY, _S_IREAD | _S_IWRITE);

	if(Handle == -1)
	{
		 if (errno == EEXIST)
			 Log("err createchar EEXIST", ch, 0);
         if (errno == EACCES)
			 Log("err createchar EACCES", ch, 0);
         if (errno == EINVAL)
			 Log("err createchar EINVAL", ch, 0);
	     if (errno == EMFILE)
			 Log("err createchar EMFILE", ch, 0);
	     if (errno == ENOENT)
			 Log("err createchar ENOENT", ch, 0);
		 else
			Log("err createchar UNKNOWN", ch, 0);

         return FALSE;
	}

	_write(Handle, ac, ACCOUNTNAME_LENGTH);
	_close(Handle);

	return TRUE;
}

int CFileDB::DeleteCharacter(char *ch, char *account)
{
	char check[ACCOUNTNAME_LENGTH];

	strncpy(check, ch, ACCOUNTNAME_LENGTH);

	_strupr(check);

	if(check[0] == 'C' && check[1] == 'O' && check[2] == 'M' && check[3] >= '0' && check[3] <= '9' && check[4] == 0)
		return FALSE;
	if(check[0] == 'L' && check[1] == 'P' && check[2] == 'T' && check[3] >= '0' && check[3] <= '9' && check[4] == 0)
		return FALSE;

	char First[128];
	char temp[128];

	BASE_GetFirstKey(check, First);

	sprintf(temp, "./char/%s/%s", First, check);

	int ret = DeleteFileA(temp);

	return ret;
}

int CFileDB::GetIndex(int server, int id)
{
    int ret = server * MAX_USER + id;

	return ret;
}

int CFileDB::GetIndex(char *account)
{
	int i;

	for(i = 0; i < MAX_DBACCOUNT; i++)
	{
		if(pAccountList[i].Login == 0) 
			continue;
		
		if(strcmp(pAccountList[i].File.Info.AccountName, account) == 0)
			return i;
	}

	return 0;
}

int CFileDB::GetAccountsByMac(int *Mac)
{
	int i;

	int num = 0;

	for (i = 0; i < MAX_DBACCOUNT; i++)
	{
		if (pAccountList[i].Login == 0)
			continue;

		if (memcmp(pAccountList[i].Mac, Mac, sizeof(pAccountList[i].Mac)) == 0)
			num++;
	}

	return num;
}

void CFileDB::SendDBSavingQuit(int Idx, int mode)
{
	int conn = Idx / MAX_USER;
	int id   = Idx % MAX_USER;

    MSG_DBSavingQuit sm;
	memset(&sm, 0, sizeof(MSG_DBSavingQuit));

	sm.Type = _MSG_DBSavingQuit;
	sm.ID = id;
	sm.Size = sizeof(MSG_DBSavingQuit);
	sm.Mode = mode;

    memcpy(sm.AccountName, pAccountList[Idx].File.Info.AccountName, ACCOUNTNAME_LENGTH);

	if(pUser[conn].cSock.Sock && pUser[conn].Mode != USER_EMPTY)
		pUser[conn].cSock.SendOneMessage((char*)&sm, sizeof(MSG_DBSavingQuit));

    return;
}

int CFileDB::DBWriteAccount(STRUCT_ACCOUNTFILE *account)
{
	char *accname = account->Info.AccountName;

	char check[ACCOUNTNAME_LENGTH];

	strncpy(check, accname, ACCOUNTNAME_LENGTH);

	 _strupr(check);

	if(check[0] == 'C' && check[1] == 'O' && check[2] == 'M' && check[3] >= '0' && check[3] <= '9' && check[4] == 0)
		return FALSE;
	if(check[0] == 'L' && check[1] == 'P' && check[2] == 'T' && check[3] >= '0' && check[3] <= '9' && check[4] == 0)
		return FALSE;

	char First[128];

	BASE_GetFirstKey(check, First);

	char temp[128];

	sprintf(temp, "./account/%s/%s", First, check);	

	int Handle = _open(temp, O_RDWR|O_CREAT|O_BINARY, _S_IREAD | _S_IWRITE);

	if(Handle == -1)
	{
		if (errno == EEXIST)
			Log("err writeaccount EEXIST", accname, 0);
		if (errno == EACCES)
			Log("err writeaccount EACCES", accname, 0);
		if (errno == EINVAL)
			Log("err writeaccount EINVAL", accname, 0);
		if (errno == EMFILE)
			Log("err writeaccount EMFILE", accname, 0);
		if (errno == ENOENT)
			Log("err writeaccount ENOENT", accname, 0);
		else
			Log("err writeaccount UNKNOWN", accname, 0);

		return FALSE;
	}

	int ret = _lseek(Handle, 0, SEEK_SET);

	if(ret == -1)
	{
		Log("err,writeaccount lseek fail", accname, 0);

		_close(Handle); 

		return FALSE;
	}

	ret = _write(Handle, account, sizeof(STRUCT_ACCOUNTFILE));

	if(ret == -1)
	{
		_close(Handle);

		Log("CreateAccount write fail", accname, 0);

		if (errno == EEXIST)
			Log("CreateAccount EEXIST", accname, 0);
		if (errno == EACCES)
			Log("CreateAccount EACCES", accname, 0);
		if (errno == EINVAL)
			Log("CreateAccount EINVAL", accname, 0);
		if (errno == EMFILE)
			Log("CreateAccount EMFILE", accname, 0);
		if (errno == ENOENT)
			Log("CreateAccount ENOENT", accname, 0);

		return FALSE;
	}

	_close(Handle);

	return TRUE;
}

int CFileDB::DBExportAccount(STRUCT_ACCOUNTFILE *file)
{
	char check[ACCOUNTNAME_LENGTH];

	char *accname = file->Info.AccountName;

	strncpy(check, accname, 16);

	_strupr(check);

	if(check[0] == 'C' && check[1] == 'O' && check[2] == 'M' && check[3] >= '0' && check[3] <= '9' && check[4] == 0)
		return FALSE;
	if(check[0] == 'L' && check[1] == 'P' && check[2] == 'T' && check[3] >= '0' && check[3] <= '9' && check[4] == 0)
		return FALSE;

	char temp[128];

	sprintf(temp, "S:/export/account%d/%s", ServerIndex, check);

	int Handle = _open(temp, O_RDWR | O_CREAT | O_BINARY, _S_IREAD | _S_IWRITE);

	if(Handle == -1)
		return FALSE;

	int ret = _lseek(Handle, 0, SEEK_SET);

	if(ret == -1)
	{
		_close(Handle);

		return FALSE;
	}

	ret = _write(Handle, file, sizeof(STRUCT_ACCOUNTFILE));

	if(ret == -1)
	{
		_close(Handle);

		return FALSE;
	}

	_close(Handle);

	return TRUE;
}

int CFileDB::DBReadAccount(STRUCT_ACCOUNTFILE *file)
{
	file->Info.AccountName[ACCOUNTNAME_LENGTH-1] = 0;
	file->Info.AccountName[ACCOUNTNAME_LENGTH-2] = 0;
	file->Info.AccountPass[ACCOUNTPASS_LENGTH-1] = 0;
	file->Info.AccountPass[ACCOUNTPASS_LENGTH-2] = 0;

	char check[ACCOUNTNAME_LENGTH];

	strncpy(check, file->Info.AccountName, ACCOUNTNAME_LENGTH);

	_strupr(check);

	if(check[0] == 'C' && check[1] == 'O' && check[2] == 'M' && check[3] >= '0' && check[3] <= '9' && check[4] == 0)
		return FALSE;
	if(check[0] == 'L' && check[1] == 'P' && check[2] == 'T' && check[3] >= '0' && check[3] <= '9' && check[4] == 0)
		return FALSE;

	char First[128];

	BASE_GetFirstKey(check, First);

	char temp[128];

	sprintf(temp, "./account/%s/%s", First, check);

	int Handle = _open(temp, O_RDONLY | O_BINARY);

	if(Handle == -1)
	{
		if(errno == EINVAL)
			Log("err readaccount EINVAL", file->Info.AccountName, 0);
		else if(errno == EMFILE)
			Log("err readaccount EEMFILE", file->Info.AccountName, 0);
		/*else
			Log("err readaccount UNKNOW", file->Info.AccountName, 0);*/

		return FALSE;
	}

	int length = _filelength(Handle);

	int reqsz = sizeof(STRUCT_ACCOUNTFILE);

	int ret = _read(Handle, file, reqsz);

	if(ret == -1)
	{
		_close(Handle);

		return FALSE;
	}

	_close(Handle);

	if(length < reqsz)
		memset(file->ShortSkill, 0, 8);

	return TRUE;
}

int CFileDB::DBCheckImpleName(char **ilchat, char *Str)
{
     for (int j = 0; j < 7; j++)
	 {
		 for (int i = 0; i < ACCOUNTNAME_LENGTH; i++)
	     {
			 if (Str[i] == 0 || Str[i+1] == 0|| Str[i+2] == 0|| Str[i+3]== 0)
				 break;

             if (Str[i] == ' ' || Str[i+1] == ' ' || Str[i+2] == ' ' || Str[i+3] == ' ')
				 continue;

	         if (Str[i] == ilchat[j][0] && Str[i+1] == ilchat[j][1] && Str[i+2] == ilchat[j][2] && Str[i+3] == ilchat[j][3] && ilchat[j][4] == 0)
				 return TRUE;

			 if (ilchat[j][i+4] == 0)
				 continue;

	         if (Str[i] == ilchat[j][0] && Str[i+1] == ilchat[j][1] && Str[i+2] == ilchat[j][2] && Str[i+3] == ilchat[j][3] && Str[i+4] == ilchat[j][4] && Str[i+5] == ilchat[j][5] && ilchat[j][6] == 0)
				 return TRUE;

			 if (ilchat[j][i+6] == 0)
				 continue;

	         if (Str[i] == ilchat[j][0] && Str[i+1] == ilchat[j][1] && Str[i+2] == ilchat[j][2] && Str[i+3] == ilchat[j][3] && Str[i+4] == ilchat[j][4] && Str[i+5] == ilchat[j][5] && Str[i+6] == ilchat[j][6] && Str[i+7] == ilchat[j][7] && ilchat[j][8] == 0)
				 return TRUE;
		 }
	 }

	 return FALSE;
}

void CFileDB::DBGetSelChar(STRUCT_SELCHAR *sel, STRUCT_ACCOUNTFILE *file)
{
	for(int i = 0; i < MOB_PER_ACCOUNT; i++)
	{
		memcpy(sel->MobName[i], file->Char[i].MobName, NAME_LENGTH);
		memcpy(sel->Equip[i], file->Char[i].Equip, sizeof(file->Char[i].Equip));

		if (sel->Equip[i][0].sIndex == 22 || sel->Equip[i][0].sIndex == 23 || sel->Equip[i][0].sIndex == 24 || sel->Equip[i][0].sIndex == 25 || sel->Equip[i][0].sIndex == 32)
			sel->Equip[i][0].sIndex = file->mobExtra[i].ClassMaster == MORTAL ? 21 : file->mobExtra[i].MortalFace + 7;

		sel->Guild[i] = file->Char[i].Guild;

		sel->HomeTownX[i] = file->Char[i].HomeTownX;
		sel->HomeTownX[i] = file->Char[i].HomeTownY;

		sel->Score[i] = file->Char[i].CurrentScore;		

		sel->Coin[i] = file->Char[i].Coin;
		sel->Exp[i]  = file->Char[i].Exp;
	}
}

void CFileDB::GetAccountByChar(char *acc, char *cha)
{
	char check[NAME_LENGTH];

	strncpy(check, cha, NAME_LENGTH);

	_strupr(check);

	if(check[0] == 'C' && check[1] == 'O' && check[2] == 'M' && check[3] >= '0' && check[3] <= '9' && check[4] == 0)
		return;
	if(check[0] == 'L' && check[1] == 'P' && check[2] == 'T' && check[3] >= '0' && check[3] <= '9' && check[4] == 0)
		return;

	char first[16];
	memset(first, 0, 16);

	BASE_GetFirstKey(check, first);

	char temp[128];

	sprintf(temp, "./char/%s/%s", first, check);

	int Handle = _open(temp, O_RDONLY | O_BINARY);

	if(Handle == -1)
	{
		return;
	}

	_read(Handle, acc, ACCOUNTNAME_LENGTH);
	_close(Handle);
}

void CFileDB::InitAccountList(int idx)
{	 
	 memset(&pAccountList[idx].File, 0, sizeof(pAccountList[idx].File));  
}

int  CFileDB::GetEncPassword(int idx, int *Enc)
{
	Enc[0] = W2Rand() % 900 + 100;
	Enc[1] = W2Rand() % 900 + 100;
	Enc[2] = W2Rand() % 900 + 100;
	Enc[3] = W2Rand() % 900 + 100;
	Enc[4] = W2Rand() % 900 + 100;
	Enc[5] = W2Rand() % 900 + 100;
	Enc[6] = W2Rand() % 900 + 100;
	Enc[7] = W2Rand() % 900 + 100;

	return FALSE;
}

void CFileDB::SetEncPassword(int idx, int newsrv, int *Enc, int Unk)
{

}
