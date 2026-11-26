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
#include "handlers/world/QuestInternal.h"

/// @brief Processa a interação de quest/diálogo com NPCs (_MSG_Quest),
///        resolvendo npcMode via npcMerc/npcGrade e despachando para o sub-handler.
void Exec_MSG_Quest(int conn, char *pMsg)
{
	MSG_STANDARDPARM2 *m = (MSG_STANDARDPARM2*)pMsg;

	int npcIndex = m->Parm1;
	int confirm = m->Parm2;

	if (npcIndex < MAX_USER || npcIndex >= MAX_MOB)
		return;

	int npcMode = -1;

	int npcMerc = pMob[npcIndex].MOB.Merchant;
	int npcGrade = BASE_GetItemAbilityNosanc(&pMob[npcIndex].MOB.Equip[0], EF_GRADE0);

	if (pUser[conn].Trade.OpponentID)
	{
		RemoveTrade(pUser[conn].Trade.OpponentID);
		RemoveTrade(conn);
		return;
	}

	if (pUser[conn].TradeMode)
	{
		SendClientMessage(conn, g_pMessageStringTable[_NN_CantWhenAutoTrade]);
		return;
	}


#pragma region Set npcMode Base

	if (npcMerc == 100 && npcGrade == 0)
		npcMode = QUEST_COVEIRO;

	if (npcMerc == 100 && npcGrade == 1)
		npcMode = QUEST_JARDINEIRO;

	if (npcMerc == 100 && npcGrade == 2)
		npcMode = QUEST_KAIZEN;

	if (npcMerc == 100 && npcGrade == 3)
		npcMode = QUEST_HIDRA;

	if (npcMerc == 100 && npcGrade == 4)
		npcMode = QUEST_ELFOS;

	if (npcMerc == 100 && npcGrade == 5)
		npcMode = LIDER_APRENDIZ;

	if (npcMerc == 100 && npcGrade == 9 || npcMerc == 100 && npcGrade == 8 || npcMerc == 100 && npcGrade == 7)
		npcMode = PERZEN;

	if (npcMerc == 100 && npcGrade == 13)
		npcMode = QUEST_CAPAREAL;

	if (npcMerc == 100 && npcGrade == 14)
		npcMode = CAPAVERDE_TELEPORT;

	if (npcMerc == 100 && npcGrade == 15)
		npcMode = MOLARGARGULA;

	if (npcMerc == 100 && npcGrade == 16)
		npcMode = TREINADORNEWBIE4;

	if (npcMerc == 100 && npcGrade == 22)
		npcMode = SOBREVIVENTE;

	if (npcMerc == 100 && npcGrade == 30)
		npcMode = GUARDA_REAL_EVT1;

	if (npcMerc == 72)
		npcMode = UXMAL;

	if (npcMerc == 36)
		npcMode = TREINADORNEWBIE1;

	if (npcMerc == 40)
		npcMode = TREINADORNEWBIE2;

	if (npcMerc == 41)
		npcMode = TREINADORNEWBIE3;

	if (npcMerc == 8)
		npcMode = CAPAVERDE_TRADE;

	if(npcMerc == 78)
		npcMode = BLACKORACLE;

	if(npcMerc == 120)
		npcMode = CARBUNCLE_WIND;

	if(npcMerc == 4)
		npcMode = GOLD_DRAGON;

	if (npcMerc == 10)
		npcMode = AMU_MISTICO;

	if (npcMerc == 11)
		npcMode = EXPLOIT_LEADER;

	if (npcMerc == 12)
		npcMode = JEFFI;

	if (npcMerc == 13)
		npcMode = SHAMA;

	if (npcMerc == 14)
		npcMode = KING;

	if (npcMerc == 15)
		npcMode = KING;

	if(npcMerc == 19)
		npcMode = COMP_SEPHI;

	if (npcMerc == 26)
		npcMode = KINGDOM;

	if (npcMerc == 30)
		npcMode = ZAKUM;

	if (npcMerc == 31)
		npcMode = MESTREHAB;

	if(npcMerc == 68)
		npcMode = GODGOVERNMENT;

	if (npcMerc == 58)
		npcMode = MOUNT_MASTER;

	if (npcMerc == 62)
		npcMode = ARZAN_DRAGON;

	if (npcMerc == 76)
		npcMode = URNAMMU;

	if (npcMerc == 74)
		npcMode = KIBITA;

	if (npcMode == -1)
		return;

#pragma endregion


	if (HandleQuest_Progression(conn, npcIndex, confirm, npcMode, npcMerc, npcGrade, m))
		return;
	if (HandleQuest_Kingdom(conn, npcIndex, confirm, npcMode, npcMerc, npcGrade, m))
		return;
	if (HandleQuest_Special(conn, npcIndex, confirm, npcMode, npcMerc, npcGrade, m))
		return;

	switch (npcMode)
	{
	default:
		sprintf(temp, "etc,quest %d %d", npcMerc, npcGrade);
		Log(temp, pUser[conn].AccountName, pUser[conn].IP);
		break;
	}
}
