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

#ifndef _CFILEDB_
#define _CFILEDB_

#include "common/Basedef.h"

/// @brief Estado em memória de uma conta conectada no DBSrv.
typedef struct
{
     int Login;
     int Slot;
     STRUCT_ACCOUNTFILE File;
	 int SecurePass;
	 
	 int Mac[4];
} STRUCT_ACCOUNTLIST;

/// @brief Registra uma entrada no arquivo de log/registro textual do DBSrv.
int ProcessRecord(int conn, char * str);

/// @brief Camada de persistência de contas, personagens, guildas e rankings do DBSrv.
/// @details Centraliza cache em memória (`pAccountList`), leitura/escrita em disco e despacho de pacotes DB.
class CFileDB
{
public:
	STRUCT_ACCOUNTLIST pAccountList[MAX_DBACCOUNT];

	CFileDB();
	~CFileDB();

	int  AddAccount				(char *id, char *pass, char *name, int ssn1, int ssn2, char *email, char *tel, char *addr, int bonus);
	int  UpdateAccount			(char *id, char *pass);

	/// @brief Roteia pacotes recebidos dos TMSrvs entre blocos de conta/personagem e de mundo/guilda.
	int  ProcessMessage			(char *Msg, int conn);

	/// @brief Sub-handler de pacotes DB de contas, personagens, cápsulas, PIN e troca de canal.
	int  ProcessAccountMessage  (char *Msg, int conn, bool& handled);

	/// @brief Sub-handler de pacotes DB de guildas, guerras, safira, logs, ranking e anúncios.
	int  ProcessWorldMessage    (char *Msg, int conn, bool& handled);

	int  SendDBSignal			(int svr, unsigned short id, unsigned short signal);
	int  SendDBSignalParm		(int svr, unsigned short id, unsigned short signal, int parm);
	int  SendDBSignalParm2		(int svr, unsigned short id, unsigned short signal, int parm1, int parm2);
	int  SendDBSignalParm3		(int svr, unsigned short id, unsigned short signal, int parm1, int parm2, int parm3);

	int  SendDBMessage			(int svr, unsigned short id, char * Msg);

	void AddAccountList			(int Idx);
	void RemoveAccountList	    (int Idx);

	int  CreateCharacter		(char *ac, char *ch);
	int  DeleteCharacter		(char *ch, char *account);

	int  GetIndex               (int server ,int id);
	int	 GetIndex				(char *account);
	int  GetAccountsByMac		(int *Mac);

	void SendDBSavingQuit		(int Idx, int mode);

	int  DBWriteAccount         (STRUCT_ACCOUNTFILE *account);
	int  DBExportAccount		(STRUCT_ACCOUNTFILE *file);
	int  DBReadAccount          (STRUCT_ACCOUNTFILE *file);

	int  DBCheckImpleName		(char **ilchat, char * Str);

	void DBGetSelChar			(STRUCT_SELCHAR *sell, STRUCT_ACCOUNTFILE *file);	

	void GetAccountByChar		(char *acc, char *cha);
	
	void InitAccountList		(int idx);
	void SendGuildInfo			(int srv, int Guild);
	int  GetEncPassword			(int idx, int *Enc);
	void SetEncPassword			(int idx, int newsrv, int *Enc, int Unk);
};

extern STRUCT_GUILDINFO GuildInfo[65536];
extern unsigned short	g_pGuildWar[65536];
extern unsigned short	g_pGuildAlly[65536];
extern unsigned short   LastCapsule;
#endif
