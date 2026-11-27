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
*/
#include "net/ProcessClientMessage.h"

/// @brief Pedido de transferência/renomeação de personagem (0xFAA, cliente → TMSrv → DBSrv).
/// @details O cliente envia da seleção de personagem (TMSelectCharScene.cpp:694). O DBSrv já trata o
/// pedido (CFileDB_WorldPackets.cpp) e o TMSrv repassa a resposta ao cliente (ProcessDBMessage.cpp);
/// faltava este repasse de ida. Só vale com o modo de transferência ligado: sem ele o TMSrv também não
/// repassa a resposta, e o cliente ficaria esperando com o usuário preso em USER_WAITDB.
void Exec_MSG_ReqTransper(int conn, char *pMsg)
{
	MSG_ReqTransper *m = (MSG_ReqTransper*)pMsg;

	m->OldName[NAME_LENGTH - 1] = 0;
	m->NewName[NAME_LENGTH - 1] = 0;

	if (TransperCharacter == 0)
	{
		Log("err,transper request but not transper mode", pUser[conn].AccountName, pUser[conn].IP);
		return;
	}

	if (pUser[conn].Mode != USER_SELCHAR)
	{
		sprintf(temp, "err,transper not user_selchar %d %d", conn, pUser[conn].Mode);
		Log(temp, pUser[conn].AccountName, pUser[conn].IP);
		return;
	}

	m->ID = conn;

	pUser[conn].Mode = USER_WAITDB;

	DBServerSocket.SendOneMessage((char*)m, sizeof(MSG_ReqTransper));

	sprintf(temp, "etc,transper slot:%d old:%s new:%s", m->Slot, m->OldName, m->NewName);
	Log(temp, pUser[conn].AccountName, pUser[conn].IP);
}
