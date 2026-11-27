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

/// @file CFileDB_WorldPackets.cpp
/// @brief Tratamento de pacotes DB relacionados a guildas, guerras, safira, logs, ranking e anúncios.

#include <errno.h>
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

int CFileDB::ProcessWorldMessage(char *Msg, int conn, bool& handled)
{
	MSG_STANDARD *std = (MSG_STANDARD*)Msg;
	handled = true;

	switch (std->Type)
	{
	case _MSG_ReqTransper:
		{
			int IdxName = conn;

			int svr = IdxName / MAX_USER;
			int id = IdxName % MAX_USER;

			MSG_ReqTransper *m = (MSG_ReqTransper*)Msg;

			m->OldName[NAME_LENGTH-1] = 0;
			m->OldName[NAME_LENGTH-2] = 0;
			m->NewName[NAME_LENGTH-1] = 0;
			m->NewName[NAME_LENGTH-2] = 0;

			int slot = m->Slot;

			if((m->ID <= 0 || m->ID >= MAX_USER) || (slot < 0 || slot >= MOB_PER_ACCOUNT))
			{
				char tt[256];
				sprintf(tt, "err,transper id:%d slot:%d old:%s new:%s", m->ID, slot, m->OldName, m->NewName);
				Log(tt, "system", 0);

				MSG_ReqTransper sm;

				sm.Type = _MSG_ReqTransper;
				sm.Size = sizeof(MSG_ReqTransper);
				sm.ID = id;

				sm.Result = 4;

				if(IdxName)
					pUser[svr].cSock.SendOneMessage((char*)&sm, sm.Size);

				return TRUE;
			}

			int idx = GetIndex(conn, m->ID);

			if(TransperCharacter == 0 || AdminClient.Sock == 0)
			{
				Log("err,transper but not transper_server", pAccountList[idx].File.Info.AccountName, 0);

				MSG_ReqTransper sm;

				sm.Type = _MSG_ReqTransper;
				sm.Size = sizeof(MSG_ReqTransper);
				sm.ID = id;

				sm.Result = 4;

				if(IdxName != 0)
					pUser[svr].cSock.SendOneMessage((char*)&sm, sm.Size);

				return TRUE;
			}

			STRUCT_MOB *mob;

			mob = &pAccountList[idx].File.Char[slot];

			mob->MobName[NAME_LENGTH-1] = 0;
			mob->MobName[NAME_LENGTH-2] = 0;

			char slotname[NAME_LENGTH];

			strncpy(slotname, mob->MobName, NAME_LENGTH);
			strncpy(mob->MobName, m->NewName, NAME_LENGTH);

			mob->MobName[NAME_LENGTH-1] = 0;
			mob->MobName[NAME_LENGTH-2] = 0;

			_strupr(slotname);
			_strupr(m->OldName);

			if(strcmp(slotname, m->OldName))
			{
				char tt[256];

				sprintf(tt, "err,transper id:%d slot:%d old:%s current:%s", m->ID, slot, m->OldName, slotname);
				Log(tt, pAccountList[idx].File.Info.AccountName, 0);

				MSG_ReqTransper sm;

				sm.Type = _MSG_ReqTransper;
				sm.Size = sizeof(MSG_ReqTransper);
				sm.ID = id;

				sm.Result = 4;

				if(IdxName != 0)
					pUser[svr].cSock.SendOneMessage((char*)&sm, sm.Size);

				return TRUE;
			}

			DisableAccount(-1, pAccountList[idx].File.Info.AccountName, 0, 0);

			MSG_NPCreateCharacter sm;

			sm.Type = _MSG_NPCreateCharacter;
			sm.Size = sizeof(MSG_NPCreateCharacter);
			sm.ID = id;

			sm.Slot = m->Slot;

			strncpy(sm.Account, pAccountList[idx].File.Info.AccountName, ACCOUNTNAME_LENGTH);

			sm.Mob = *mob;

			AdminClient.SendOneMessage((char*)&sm, sm.Size);

			return TRUE;
		}

	case _MSG_GuildZoneReport:
		{
			MSG_GuildZoneReport *m = (MSG_GuildZoneReport*)Msg;

			int srv = m->ID;

			if(srv < 0 || srv >= MAX_SERVER)
				break;

			for(int i = 0; i < 5; i++)
				ChargedGuildList[srv][i] = m->Guild[i];

			MSG_GuildReport sm;

			sm.Type = _MSG_GuildReport;
			sm.Size = sizeof(MSG_GuildReport);
			sm.ID = 0;

			memcpy(sm.ChargedGuildList, ChargedGuildList, sizeof(ChargedGuildList));

			for(int i = 0; i < MAX_SERVER; i++)
			{
				if(pUser[i].Mode == USER_EMPTY)
					continue;

				if(pUser[i].cSock.Sock != 0)
					pUser[i].cSock.SendOneMessage((char*)&sm, sizeof(MSG_GuildReport));
			}

		} break;

	case _MSG_War:
		{
			MSG_STANDARDPARM2 *m = (MSG_STANDARDPARM2*)Msg;

			struct tm when;
			time_t now;
			time(&now);
			when = *localtime(&now);

			int myguild = m->Parm1;

			char temp[128];
			sprintf(temp, "guild war request %d %d", m->Parm1, m->Parm2);
			Log(temp, "system", 0);

			if(myguild <= 0 || myguild >= 65536)
				break;

			g_pGuildWar[myguild] = m->Parm2;
			//g_pGuildWar[myguild] = when.tm_mday;

			for(int i = 0; i < MAX_SERVER; i++)
			{
				if(pUser[i].Mode == USER_EMPTY)
					continue;

				if(pUser[i].cSock.Sock)
					SendDBSignalParm2(i, 0, _MSG_War, m->Parm1, m->Parm2);
			}
			
		} break;

	case _MSG_GuildAlly:
		{
			MSG_GuildAlly *m = (MSG_GuildAlly*)Msg;

			struct tm when;
			time_t now;
			time(&now);
			when = *localtime(&now);

			char temp[256];

			int myguild = m->Guild;

			sprintf(temp, "guild ally request %d %d",  m->Guild, m->Ally);

			Log(temp, "-system", 0);

			if(myguild <= 0 || myguild >= 65536)
				break;

			g_pGuildAlly[myguild] = m->Ally;
			//g_pGuildAlly[myguild] = when.tm_mday;

			for(int i = 0; i < MAX_SERVER; i++)
			{
				if(pUser[i].Mode == USER_EMPTY)
					continue;

				if(pUser[i].cSock.Sock != 0)
					SendDBSignalParm2(i, 0, _MSG_GuildAlly, m->Guild, m->Ally);
			}

		} break;

	case _MSG_GuildInfo:
		{
			MSG_GuildInfo *m = (MSG_GuildInfo*)Msg;

			int myguild = m->Guild;

			// char temp[256];


			if(myguild <= 0 || myguild >= 65536)
				break;

			GuildInfo[myguild] = m->GuildInfo;

			for(int i = 0; i < MAX_SERVER; i++)
			{
				if(pUser[i].Mode == USER_EMPTY)
					continue;

				if(pUser[i].cSock.Sock == 0)
					continue;

				if(pUser[i].cSock.Sock != 0)
					SendGuildInfo(i, myguild);
			}

			CReadFiles::WriteGuildInfo();
		} break;

	case _MSG_DBUpdateSapphire:
		{
			MSG_STANDARDPARM *m = (MSG_STANDARDPARM*)Msg;

			if(m->Parm == 1)
				Sapphire *= 2;
			else
				Sapphire /= 2;

			if(Sapphire < 1)
				Sapphire = 1;
			else if(Sapphire > 64)
				Sapphire = 64;

			for(int i = 0; i < MAX_SERVER; i++)
			{
				if(pUser[i].Mode == USER_EMPTY)
					continue;

				if(pUser[i].cSock.Sock != 0)
					SendDBSignalParm3(i, 0, _MSG_DBSetIndex, -1, Sapphire, i);
			}

			WriteConfig();

		} break;

	case  _MSG_MessageDBRecord:
		{
			MSG_MessageDBRecord *m = (MSG_MessageDBRecord*)Msg;

			m->Record[255] = 0;
			m->Record[254] = 0;

			int ret = ProcessRecord(conn, m->Record);

			Log(m->Record, "system", 0);

			if(ret == 0)
				Log("DBRecord Fail", "system", 0);

		} break;

	case _MSG_NPAppeal:
		{
			for(int i = 0; i < MAX_ADMIN; i++)
			{
				if(pAdmin[i].Mode == USER_EMPTY)
					continue;

				if(pAdmin[i].cSock.Sock != 0)
					pAdmin[i].cSock.SendOneMessage((char*)std, std->Size);
			}

		} break;

	case _MSG_MessageDBImple:
		{
			MSG_MessageDBImple *m = (MSG_MessageDBImple*)Msg;

			m->ID = 0;

			m->String[MESSAGE_LENGTH-1] = 0;
			m->String[MESSAGE_LENGTH-2] = 0;

			for(int i = 0; i < MAX_SERVER; i++)
			{
				if(pUser[i].Mode == USER_EMPTY)
					continue;

				if(pUser[i].cSock.Sock)
					pUser[i].cSock.SendOneMessage((char*)m, sizeof(MSG_MessageDBImple));
			}
		} break;

	case _MSG_MagicTrumpet:
	{
			for(int i = 0; i < MAX_SERVER; i++)
			{
				if(pUser[i].Mode == USER_EMPTY)
					continue;

				if(pUser[i].cSock.Sock != 0)
					pUser[i].cSock.SendOneMessage(Msg, sizeof(MSG_MagicTrumpet));
			}

			for(int i = 0; i < MAX_ADMIN; i++)
			{
				if(pAdmin[i].Mode == USER_EMPTY)
					continue;

				if(pAdmin[i].cSock.Sock != 0)
					pAdmin[i].cSock.SendOneMessage(Msg, sizeof(MSG_MagicTrumpet));
			}
	}break;

	case _MSG_DBNotice:
	{
			for(int i = 0; i < MAX_SERVER; i++)
			{
				if(pUser[i].Mode == USER_EMPTY)
					continue;

				if(pUser[i].cSock.Sock != 0)
					pUser[i].cSock.SendOneMessage(Msg, sizeof(MSG_DBNotice));
			}
	}break;

	case _MSG_UpdateExpRanking:
	{
		/*
			Process the update exp entries from tmsrv and update the ranking
		*/

		MSG_UpdateExpRanking* m = (MSG_UpdateExpRanking*)Msg;

		int rankId = rankingSystem.grindRanking.getElementIndex(m->RankInfo.Name);
		int newRankId = rankId;

		if (rankId == OUT_OF_RANK)
		{
			/* The player is not in the ranking, so we can try to insert him. */

			STRUCT_RANKING olderLastPlayer = STRUCT_RANKING();
			GrindRankingConnId olderLastConnId = GrindRankingConnId();
			GrindRankingConnId rankConnId = GrindRankingConnId(conn, m->ID);

			newRankId = rankingSystem.tryInsertInRanking(rankId, m->RankInfo.Name, m->RankInfo.Value, m->RankInfo.ClassMaster, m->RankInfo.Level, m->RankInfo.Class, &rankConnId, &olderLastPlayer, &olderLastConnId);

			if (newRankId >= LAST && newRankId <= FIRST)
			{
				/* The player have just entered the ranking. Now we have to notify the appropriate online players of these changes */

				rankId = LAST;

				/* Update the older [LAST] player that was kicked at the ranking */
				if (olderLastConnId.TMSrvId >= 0 && olderLastConnId.TMSrvId <= MAX_SERVER && olderLastConnId.PlayerId > 0 && olderLastConnId.PlayerId < MAX_USER)
					rankingSystem.sendUpdateIndividualRank(OUT_OF_RANK, olderLastConnId.TMSrvId, olderLastConnId.PlayerId, &olderLastPlayer);
				// ---

				/* Update all the other players that were affected by this change */
				rankingSystem.sendUpdateRangeRank(rankId, newRankId);
				// ---
			}
		}
		else if (rankId >= LAST && rankId <= FIRST)
		{
			/* The player is already in the ranking. Here we have to check if he can rise in the ranking and notify the appropriate online players */

			STRUCT_RANKING* thisRank = rankingSystem.grindRanking.getElement(rankId);

			if (m->RankInfo.Value > thisRank->Value && m->RankInfo.ClassMaster <= thisRank->ClassMaster || m->RankInfo.ClassMaster > thisRank->ClassMaster)
			{
				newRankId = rankingSystem.increaseRankingElementValue(rankId, (int)m->RankInfo.Value, m->RankInfo.ClassMaster);

				/* Update all the other players that were affected by this change */
				rankingSystem.sendUpdateRangeRank(rankId, newRankId);
				// ---
			}
		}
	} break;

	case _MSG_DBItemDayLog:
	{
		MSG_STANDARDPARM *m = (MSG_STANDARDPARM*)Msg;

		int ItemID = m->Parm;

		if (ItemID <= 0 || ItemID >= MAX_ITEMLIST)
		{
			Log("err,itemid invalid", "-system", 0);

			return TRUE;
		}

		ItemDayLog[ItemID].Num++;
	} break;

	default:
		handled = false;
		break;
	}

	return TRUE;
}
