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

/// @brief Log de erro crítico que o cliente envia (TMScene::LogMsgCriticalError): falha ao carregar o
/// mapa de tiles ou dado inválido de malha, com a posição do personagem. Só registra no log do servidor.
void Exec_MSG_MessageLog(int conn, char *pMsg)
{
	MSG_MessageLog *m = (MSG_MessageLog*)pMsg;

	// Texto do cliente: fecha a string e troca o que não for imprimível, para não sujar o log.
	m->String[sizeof(m->String) - 1] = 0;

	for (int i = 0; m->String[i]; i++)
	{
		if ((unsigned char)m->String[i] < 0x20)
			m->String[i] = ' ';
	}

	sprintf(temp, "clienterr,%s", m->String);
	Log(temp, pUser[conn].AccountName, pUser[conn].IP);
}
