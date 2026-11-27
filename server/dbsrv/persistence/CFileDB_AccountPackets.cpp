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

/// @file CFileDB_AccountPackets.cpp
/// @brief Tratamento de pacotes DB relacionados a contas, personagens, cápsulas e troca de canal.

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

int CFileDB::ProcessAccountMessage(char *Msg, int conn, bool& handled)
{
	MSG_STANDARD *std = (MSG_STANDARD*)Msg;
	handled = true;

	switch (std->Type)
	{
	case _MSG_DBNewAccount:
		{
			MSG_NewAccount *m = (MSG_NewAccount*)Msg;

			int Idx = GetIndex(conn, m->ID);

			_strupr(m->AccountInfo.AccountName);

			char *accname = m->AccountInfo.AccountName;

			if((accname[0] == 'C' && accname[1] == 'O' && accname[2] == 'M' && accname[3] >= '0' && accname[3] <= '9' && accname[4] == 0) || 
				(accname[0] == 'L' && accname[1] == 'P' && accname[2] == 'T' && accname[3] >= '0' && accname[3] <= '9' && accname[4] == 0))
				return FALSE;

			STRUCT_ACCOUNTFILE *file;

			file = &pAccountList[Idx].File;

			file->Info = m->AccountInfo;

			memset(file->Char, 0, sizeof(STRUCT_ACCOUNTFILE)-sizeof(STRUCT_ACCOUNTINFO));
			memset(file->Cargo, 0, sizeof(STRUCT_ITEM)*MAX_CARGO);

			file->Coin = 0;

			int ret = DBWriteAccount(file);

			if(ret == 0)
			{
				SendDBSignal(conn, m->ID, _MSG_DBNewAccountFail);

				return TRUE;				
			}

			_strupr(file->Info.AccountName);

			AddAccountList(Idx);

			MSG_DBCNFAccountLogin sm;

			sm.Type = _MSG_DBCNFAccountLogin;
			sm.ID = m->ID;
			sm.Size = sizeof(MSG_DBCNFAccountLogin);

			memcpy(sm.AccountName, file->Info.AccountName, ACCOUNTNAME_LENGTH);
			memset(sm.Cargo, 0, sizeof(STRUCT_ITEM)*MAX_CARGO);

			DBGetSelChar(&sm.sel, file);

			pUser[conn].cSock.SendOneMessage((char*)&sm, sizeof(MSG_DBCNFAccountLogin));

		} break;

	case _MSG_DBAccountLogin:
		{
			MSG_AccountLogin *m = (MSG_AccountLogin*)Msg;

			_strupr(m->AccountName);

			char *ac = m->AccountName;

			if((ac[0] == 'C' && ac[1] == 'O' && ac[2] == 'M' && ac[3] >= '0' && ac[3] <= '9' && ac[4] == 0) || 
				(ac[0] == 'L' && ac[1] == 'P' && ac[2] == 'T' && ac[3] >= '0' && ac[3] <= '9' && ac[4] == 0))
			{
				SendDBSignal(conn, m->ID, _MSG_DBAccountLoginFail_Account);

				return FALSE;
			}

			int Idx = GetIndex(conn, m->ID);
			int IdxName = GetIndex(m->AccountName);

			STRUCT_ACCOUNTFILE file;

			memset(&file, 0, sizeof(file));

			memcpy(&file.Info.AccountName, m->AccountName, ACCOUNTNAME_LENGTH);

			int ret = DBReadAccount(&file);

			if(ret == 0)
			{
				SendDBSignal(conn, m->ID, _MSG_DBAccountLoginFail_Account);

				return TRUE;
			}

			if(file.Coin < 0)
				file.Coin = 0;

			if(file.Info.Year != 0 && file.Info.YearDay != 0)
			{
				struct tm when;
				time_t now;
				time(&now);
				when = *localtime(&now);

				if(file.Info.Year >= when.tm_year || file.Info.Year >= when.tm_year && file.Info.YearDay >= when.tm_yday)
				{
					SendDBSignalParm(conn, m->ID, _MSG_DBAccountLoginFail_Block, 0);
					return TRUE;
				}
			}

			int ChangeServer = 0;

			if (file.TempKey[0] != 0 && m->TID[0] != 0)
			{
				if (memcmp(file.TempKey, m->TID, sizeof(file.TempKey)) == 0)
				{
					memset(file.TempKey, 0, sizeof(file.TempKey));
					ChangeServer = 1;
					goto lb_sucess;
				}
				memset(file.TempKey, 0, sizeof(file.TempKey));
				DBWriteAccount(&file);
				return TRUE;
			}

			if(strcmp(file.Info.AccountPass, m->AccountPass) != 0)
			{
				SendDBSignal(conn, m->ID, _MSG_DBAccountLoginFail_Pass);

				return TRUE;
			}
		lb_sucess:

			if(IdxName == Idx)
				return TRUE;

			if(IdxName != 0)
			{
				Log("err, desconectado. conexão anterior finalizada.", m->AccountName, 0);

				if(m->Force == 0)
				{
					SendDBSignal(conn, m->ID, _MSG_DBAlreadyPlaying);

					return TRUE;
				}

				SendDBSignal(conn, m->ID, _MSG_DBStillPlaying);
				SendDBSavingQuit(IdxName, 0);

				break;
			}

			_strupr(file.Info.AccountName);

			int right = -1;
			int left = -1;

			for(int q = 0; q < 4; q++)
			{
				if(file.Char[q].MobName[0] != 0)
				{
					if(file.Char[q].Equip[13].sIndex == 774)
						left = q;
				}

				if(file.Char[q].MobName[0] != 0)
				{
					if(file.Char[q].Equip[13].sIndex == 775)
						right  = q;
				}
			}

			if(left != -1 && right != -1)
			{
				char temp[NAME_LENGTH];

				memcpy(temp, file.Char[left].MobName, NAME_LENGTH);
				memcpy(file.Char[left].MobName, file.Char[right].MobName, NAME_LENGTH);
				memcpy(file.Char[right].MobName, temp, NAME_LENGTH);

				file.Char[left].Equip[13].sIndex = 0;
				file.Char[right].Equip[13].sIndex = 0;

				char tt[256];

				sprintf(tt, "etc,name swap %s %s", file.Char[left].MobName, file.Char[right].MobName);

				Log(tt, m->AccountName, 0);

				DBWriteAccount(&file);
			}

			STRUCT_ACCOUNTFILE *pFile;

			pFile = &pAccountList[Idx].File;

			memcpy(pFile, &file, sizeof(STRUCT_ACCOUNTFILE));

			pAccountList[Idx].Mac[0] = m->Mac[0];
			pAccountList[Idx].Mac[1] = m->Mac[1];
			pAccountList[Idx].Mac[2] = m->Mac[2];
			pAccountList[Idx].Mac[3] = m->Mac[3];

			AddAccountList(Idx);

			STRUCT_SELCHAR selchar;

			DBGetSelChar(&selchar, &pAccountList[Idx].File);

			pAccountList[Idx].SecurePass = -1;

			MSG_DBCNFAccountLogin sm;

			memset(&sm, 0, sizeof(MSG_DBCNFAccountLogin));

			sm.Type = _MSG_DBCNFAccountLogin;
			sm.ID = m->ID;

			sm.Unknow_28 = 0xCCCCCCCC;

			strncpy(sm.AccountName, file.Info.AccountName, ACCOUNTNAME_LENGTH);
			memcpy(sm.Cargo, pAccountList[Idx].File.Cargo, sizeof(pAccountList[Idx].File.Cargo));

			sm.Coin = pAccountList[Idx].File.Coin;

			sm.sel = selchar;

			pUser[conn].cSock.SendOneMessage((char *)&sm, sizeof(MSG_DBCNFAccountLogin));
			
			if (GetAccountsByMac(m->Mac) <= 1)
			{
				MSG_DBCheckPrimaryAccount sm_pa;
				memset(&sm_pa, 0, sizeof(MSG_DBCheckPrimaryAccount));

				sm_pa.Size = sizeof(MSG_DBCheckPrimaryAccount);
				sm_pa.Type = _MSG_DBCheckPrimaryAccount;

				memcpy(&sm_pa.Mac, m->Mac, sizeof(sm_pa.Mac));

				strncpy(sm_pa.AccountName, pAccountList[Idx].File.Info.AccountName, ACCOUNTNAME_LENGTH);

				for (int i = 0; i < MAX_SERVER; i++)
				{
					if (pUser[i].cSock.Sock == 0 || pUser[i].Mode == USER_EMPTY)
						continue;

					pUser[i].cSock.SendOneMessage((char*)&sm_pa, sizeof(MSG_DBCheckPrimaryAccount));
				}
			}
			if (ChangeServer == 1)
			{
				DBWriteAccount(&file);

				pAccountList[Idx].SecurePass = 1;

				int Slot;
				int Server;
				int Enc[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

				sscanf(m->TID, "*%d %d %d %d %d %d %d %d %d %d", &Server, &Enc[0], &Enc[1], &Enc[2], &Enc[3], &Enc[4], &Enc[5], &Enc[6], &Enc[7], &Slot);

				pAccountList[Idx].Slot = Slot;

				MSG_CNFCharacterLogin sm;

				sm.Type = _MSG_DBCNFCharacterLogin;
				sm.ID = m->ID;
				sm.Size = sizeof(MSG_CNFCharacterLogin);

				sm.Slot = Slot;

				sm.mob = pAccountList[Idx].File.Char[Slot];
				sm.Donate = pAccountList[Idx].File.Donate;

				memcpy(sm.ShortSkill, pAccountList[Idx].File.ShortSkill[Slot], 16);
				memcpy(sm.affect, pAccountList[Idx].File.affect[Slot], sizeof(pAccountList[Idx].File.affect[Slot]));
				memcpy(&sm.mobExtra, &pAccountList[Idx].File.mobExtra[Slot], sizeof(STRUCT_MOBEXTRA));

				char *mob;

				mob = pAccountList[Idx].File.Char[Slot].MobName;

				if (mob[0] == 0)
				{
					Log("err,charlogin mobname empty", pAccountList[Idx].File.Info.AccountName, 0);

					return TRUE;
				}
				pUser[conn].cSock.SendOneMessage((char*)&sm, sizeof(MSG_CNFCharacterLogin));
#pragma region Ranking
				int rankId = rankingSystem.grindRanking.getElementIndex(pAccountList[Idx].File.Char[Slot].MobName);

				if (rankId >= LAST && rankId <= FIRST)
				{
					rankingSystem.grindRanking.setElementConnId(rankId, GrindRankingConnId(conn, sm.ID));
				}

				// Send ranking info to tm->client
				rankingSystem.sendUpdateIndividualRank(rankId, conn, sm.ID, new STRUCT_RANKING(pAccountList[Idx].File.Char[Slot].MobName, pAccountList[Idx].File.Char[Slot].Exp, pAccountList[Idx].File.mobExtra[Slot].ClassMaster, pAccountList[Idx].File.Char[Slot].CurrentScore.Level, pAccountList[Idx].File.Char[Slot].Class));
#pragma endregion
			}
		} break;
		
	case _MSG_DBCreateCharacter:
		{
			MSG_CreateCharacter *m = (MSG_CreateCharacter*)Msg;

			int Slot = m->Slot;
			int cls = m->MobClass;

			int Idx = GetIndex(conn, m->ID);

			if(Slot < 0 || Slot >= MOB_PER_ACCOUNT)
			{
				Log("err,newchar  slot out of range", pAccountList[Idx].File.Info.AccountName, 0);

				SendDBSignal(conn, m->ID, _MSG_DBNewCharacterFail);

				return TRUE;
			}
			if(cls < 0 || cls >= 4)
			{
				Log("err,newchar - class out of range", pAccountList[Idx].File.Info.AccountName, 0);

				SendDBSignal(conn, m->ID, _MSG_DBNewCharacterFail);

				return TRUE;
			}

			int Secure = pAccountList[Idx].SecurePass;

			if (Secure == -1)
			{
				Log("err,newchar secure illegal", pAccountList[Idx].File.Info.AccountName, 0);

				break;
			}

			char check[NAME_LENGTH];

			strncpy(check, m->MobName, NAME_LENGTH);

			_strupr(check);

			if(strcmp(check, "KING") == 0 || strcmp(check, "KINGDOM") == 0 || strcmp(check, "GRITAR") == 0 || strcmp(check, "RELO") == 0)
			{
				Log("err,newchar - cmd name", pAccountList[Idx].File.Info.AccountName, 0);

				SendDBSignal(conn, m->ID, _MSG_DBNewCharacterFail);

				return FALSE;
			}

			if((check[0] == 'C' && check[1] == 'O' && check[2] == 'M' && check[3] >= '0' && check[3] <= '9' && check[4] == 0) || 
				(check[0] == 'L' && check[1] == 'P' && check[2] == 'T' && check[3] >= '0' && check[3] <= '9' && check[4] == 0))
			{
				Log("err,newchar - com", pAccountList[Idx].File.Info.AccountName, 0);

				SendDBSignal(conn, m->ID, _MSG_DBNewCharacterFail);

				return FALSE;
			}

			STRUCT_MOB *mob;

			mob = &pAccountList[Idx].File.Char[Slot];


			mob->MobName[NAME_LENGTH-1] = 0;
			mob->MobName[NAME_LENGTH-2] = 0;

			m->MobName[NAME_LENGTH-1] = 0;
			m->MobName[NAME_LENGTH-2] = 0;

			if(mob->MobName[0] != 0)
			{
				SendDBSignal(conn, m->ID, _MSG_DBNewCharacterFail);

				Log("err,newchar already charged", pAccountList[Idx].File.Info.AccountName, 0);
				Log(mob->MobName, m->MobName, 0);

				return TRUE;
			}

			m->MobName[NAME_LENGTH-1] = 0;
			m->MobName[NAME_LENGTH-2] = 0;

			int len = strlen(m->MobName);

			for(int i = 0; i < len; i++)
			{
				if(m->MobName[i] == 'í' && m->MobName[i + 1] == 'í')
				{
					SendDBSignal(conn, m->ID, _MSG_DBNewCharacterFail);

					return TRUE;
				}
			}

			int ret = CreateCharacter(pAccountList[Idx].File.Info.AccountName, m->MobName);

			if(ret == 0)
			{
				SendDBSignal(conn, m->ID, _MSG_DBNewCharacterFail);

				return TRUE;
			}
			STRUCT_MOBEXTRA *extra;

			extra = &pAccountList[Idx].File.mobExtra[Slot];

			BASE_ClearMob(mob);
			BASE_ClearMobExtra(extra);

			memset(&pAccountList[Idx].File.affect[Slot], 0, sizeof(pAccountList[Idx].File.affect[Slot]));

			memset(&pAccountList[Idx].File.ShortSkill[Slot], -1, 16);

			extra->ClassMaster = MORTAL;

			if(cls == 0)
				memcpy(mob, &g_pBaseSet[0], sizeof(STRUCT_MOB));
			
			else if(cls == 1)
				memcpy(mob, &g_pBaseSet[1], sizeof(STRUCT_MOB));
			
			else if(cls == 2)
				memcpy(mob, &g_pBaseSet[2], sizeof(STRUCT_MOB));
			
			else if(cls == 3)
				memcpy(mob, &g_pBaseSet[3], sizeof(STRUCT_MOB));
			
			else
			{
				SendDBSignal(conn, m->ID, _MSG_DBNewCharacterFail);

				Log("err,newchar fail - undefined class", m->MobName, 0);

				return TRUE;
			}


			extra->MortalFace = mob->Equip[0].sIndex;

			memcpy(mob->MobName, m->MobName, NAME_LENGTH);

			ret = DBWriteAccount(&pAccountList[Idx].File);

			if(ret == 0)
			{
				SendDBSignal(conn, m->ID, _MSG_DBNewCharacterFail);

				Log("err,newchar fail - create file", m->MobName, 0);

				return TRUE;
			}

			char temp[256];

			sprintf(temp, "create character [%s]", mob->MobName);

			Log(temp, pAccountList[Idx].File.Info.AccountName, 0);

			MSG_CNFNewCharacter sm;

			sm.Type = _MSG_DBCNFNewCharacter;

			DBGetSelChar(&sm.SelChar, &pAccountList[Idx].File);

			sm.ID = m->ID;

			pUser[conn].cSock.SendOneMessage((char*)&sm, sizeof(MSG_CNFNewCharacter));

		}	break;

	case _MSG_DBCharacterLogin:
		{
			MSG_CharacterLogin *m = (MSG_CharacterLogin*)Msg;

			int Slot = m->Slot;

			int Idx = GetIndex(conn, m->ID);

			if(Slot < 0 || Slot >= MOB_PER_ACCOUNT)
			{
				Log("err,charlogin slot illegal", pAccountList[Idx].File.Info.AccountName, 0);

				break;
			}

			int Secure = pAccountList[Idx].SecurePass;

			if (Secure == -1)
			{
				Log("err,charlogin secure illegal", pAccountList[Idx].File.Info.AccountName, 0);

				break;
			}

			pAccountList[Idx].Slot = Slot;

			MSG_CNFCharacterLogin sm;

			sm.Type = _MSG_DBCNFCharacterLogin;
			sm.ID = m->ID;
			sm.Size = sizeof(MSG_CNFCharacterLogin);

			sm.Slot = Slot;

			sm.mob = pAccountList[Idx].File.Char[Slot];
			sm.Donate = pAccountList[Idx].File.Donate;

			memcpy(sm.ShortSkill, pAccountList[Idx].File.ShortSkill[Slot], 16);
			memcpy(sm.affect, pAccountList[Idx].File.affect[Slot], sizeof(pAccountList[Idx].File.affect[Slot]));
			memcpy(&sm.mobExtra, &pAccountList[Idx].File.mobExtra[Slot], sizeof(STRUCT_MOBEXTRA));

			char *mob;

			mob = pAccountList[Idx].File.Char[Slot].MobName;

			if(mob[0] == 0)
			{
				Log("err,charlogin mobname empty", pAccountList[Idx].File.Info.AccountName, 0);

				return TRUE;
			}

			pUser[conn].cSock.SendOneMessage((char*)&sm, sizeof(MSG_CNFCharacterLogin));

#pragma region Ranking
			int rankId = rankingSystem.grindRanking.getElementIndex(pAccountList[Idx].File.Char[Slot].MobName);

			if (rankId >= LAST && rankId <= FIRST)
			{
				rankingSystem.grindRanking.setElementConnId(rankId, GrindRankingConnId(conn, sm.ID));
			}

			// Send ranking info to tm->client
			rankingSystem.sendUpdateIndividualRank(rankId, conn, sm.ID, new STRUCT_RANKING(pAccountList[Idx].File.Char[Slot].MobName, pAccountList[Idx].File.Char[Slot].Exp, pAccountList[Idx].File.mobExtra[Slot].ClassMaster, pAccountList[Idx].File.Char[Slot].CurrentScore.Level, pAccountList[Idx].File.Char[Slot].Class));
#pragma endregion
			return TRUE;
		}

	case _MSG_DBNoNeedSave:
		{
			MSG_STANDARD *m = (MSG_STANDARD*)Msg;

			int Idx = GetIndex(conn, m->ID);

			char acc[ACCOUNTNAME_LENGTH];

			strncpy(acc, pAccountList[Idx].File.Info.AccountName, ACCOUNTNAME_LENGTH);

			RemoveAccountList(Idx);

			for(int i = 0; i < MAX_ADMIN; i++)
			{
				if(pAdmin[i].Level <= 2)
					continue;

				if(pAdmin[i].DisableID == 0)
					continue;

				if(pAdmin[i].DisableID != Idx)
					continue;

				DisableAccount(i, acc, pAdmin[i].Year, pAdmin[i].YearDay);
			}

		}	break;

	case _MSG_DBSaveMob:
		{
			MSG_DBSaveMob *m = (MSG_DBSaveMob*)Msg;

			int Idx = GetIndex(conn, m->ID);

			pAccountList[Idx].File.Info.AccountName[ACCOUNTNAME_LENGTH-1] = 0;
			pAccountList[Idx].File.Info.AccountName[ACCOUNTNAME_LENGTH-2] = 0;

			char acc[ACCOUNTNAME_LENGTH];

			strncpy(acc, pAccountList[Idx].File.Info.AccountName, ACCOUNTNAME_LENGTH);

			m->AccountName[ACCOUNTNAME_LENGTH-1] = 0;
			m->AccountName[ACCOUNTNAME_LENGTH-2] = 0;

			int Slot = pAccountList[Idx].Slot;

			if(Slot < 0 || Slot >= MOB_PER_ACCOUNT)
			{
				char temp[1024];

				sprintf(temp, "err,savemob1 %d %d %d %s %s",  m->ID, Slot, m->Slot, acc, m->AccountName);

				Log(temp, "-system", 0);

				return TRUE;
			}

			if(Slot != m->Slot)
			{
				char temp[1024];

				sprintf(temp, "err,savemob2 %d %d %d %s %s",  m->ID, Slot, m->Slot, acc, m->AccountName);

				Log(temp, "-system", 0);
			}

			if(pAccountList[Idx].Login == 0)
			{
				char temp[1024];

				sprintf(temp, "err,savemob3 %d %d %d %s %s",  m->ID, Slot, m->Slot, acc, m->AccountName);

				Log(temp, "-system", 0);

				return TRUE;
			}

			pAccountList[Idx].File.Donate = m->Donate;
			pAccountList[Idx].File.Char[Slot] = m->MOB;
			pAccountList[Idx].File.mobExtra[Slot] = m->extra;
			
			memcpy(&pAccountList[Idx].File.affect[Slot], m->affect, sizeof(pAccountList[Idx].File.affect[Slot]));
			memcpy(&pAccountList[Idx].File.Cargo, m->Cargo, sizeof(pAccountList[Idx].File.Cargo));

			memcpy(&pAccountList[Idx].File.ShortSkill[Slot], m->ShortSkill, 16);

			pAccountList[Idx].File.Coin = m->Coin;

			DBWriteAccount(&pAccountList[Idx].File);

			if(m->Export != 0)
				DBExportAccount(&pAccountList[Idx].File);

		} break;

	case _MSG_SavingQuit:
		{
			MSG_SavingQuit *m = (MSG_SavingQuit*)Msg;

			int Idx = GetIndex(conn, m->ID);

			int Slot = pAccountList[Idx].Slot;

			pAccountList[Idx].File.Char[Slot].MobName[NAME_LENGTH-1] = 0;
			pAccountList[Idx].File.Char[Slot].MobName[NAME_LENGTH-2] = 0;

			char acc[ACCOUNTNAME_LENGTH];

			strncpy(acc, pAccountList[Idx].File.Info.AccountName, ACCOUNTNAME_LENGTH);

			m->AccountName[ACCOUNTNAME_LENGTH-1] = 0;
			m->AccountName[ACCOUNTNAME_LENGTH-2] = 0;

			if(strncmp(m->AccountName, acc, ACCOUNTNAME_LENGTH))
			{
				char temp[256];

				sprintf(temp, "err,savenquit1 %d %d %d %s %s", m->ID, Slot, m->Slot, acc, m->AccountName);
				Log(temp, "-system", 0);

				RemoveAccountList(Idx);

				return TRUE;
			}

			if((Slot < 0 || Slot >= MOB_PER_ACCOUNT) || (Slot != m->Slot))
			{
				char temp[256];

				sprintf(temp, "err,savenquit2 %d %d %d %s %s", m->ID, Slot, m->Slot, acc, m->AccountName);
				Log(temp, "-system", 0);

				RemoveAccountList(Idx);

				return TRUE;
			}

			if(pAccountList[Idx].Login == 0)
			{
				char temp[256];

				sprintf(temp, "err,savenquit3 %d %d %d %s %s", m->ID, Slot, m->Slot, acc, m->AccountName);
				Log(temp, "-system", 0);

				return TRUE;
			}

			pAccountList[Idx].File.Char[Slot] = m->MOB;

			memcpy(pAccountList[Idx].File.Cargo, m->Cargo, sizeof(STRUCT_ITEM)*MAX_CARGO);
			memcpy(pAccountList[Idx].File.ShortSkill[Slot], m->ShortSkill, 16);

			pAccountList[Idx].File.mobExtra[Slot] = m->extra;

			memcpy(&pAccountList[Idx].File.affect[Slot], m->affect, sizeof(pAccountList[Idx].File.affect[Slot]));

			pAccountList[Idx].File.Coin = m->Coin;
			pAccountList[Idx].File.Donate = m->Donate;

			DBWriteAccount(&pAccountList[Idx].File);
			DBExportAccount(&pAccountList[Idx].File);
			RemoveAccountList(Idx);

			SendDBSignal(conn, m->ID, _MSG_DBCNFAccountLogOut);

			for(int i = 0; i < MAX_ADMIN; i++)
			{
				if(pAdmin[i].Level <= 2 || pAdmin[i].DisableID == 0)
					continue;

				if(pAdmin[i].DisableID == Idx)
				{
					DisableAccount(i, acc, pAdmin[i].Year, pAdmin[i].YearDay);

					break;
				}
			}

		} break;

	case _MSG_DBDeleteCharacter:
		{
			MSG_DeleteCharacter *m = (MSG_DeleteCharacter*)Msg;

			int Idx = GetIndex(conn, m->ID);

			int Slot = m->Slot;

			if (Slot < 0 || Slot >= MOB_PER_ACCOUNT)
			{
				SendDBSignal(conn, m->ID, _MSG_DBDeleteCharacterFail);

				Log("err,deletechar slot", pAccountList[Idx].File.Info.AccountName, 0);

				return TRUE;
			}

			int Secure = pAccountList[Idx].SecurePass;

			if (Secure == -1)
			{
				Log("err,deletechar secure illegal", pAccountList[Idx].File.Info.AccountName, 0);

				break;
			}

			if (strncmp(m->Password, pAccountList[Idx].File.Info.AccountPass, ACCOUNTPASS_LENGTH) != 0)
			{
				SendDBSignal(conn, m->ID, _MSG_DBDeleteCharacterFail);

				Log("err,deletechar password", pAccountList[Idx].File.Info.AccountName, 0);

				return TRUE;
			}

			STRUCT_MOB *mob;

			mob = &pAccountList[Idx].File.Char[Slot];

			if (pAccountList[Idx].File.mobExtra[Slot].ClassMaster != MORTAL && pAccountList[Idx].File.mobExtra[Slot].ClassMaster != ARCH)
			{
				SendDBSignal(conn, m->ID, _MSG_DBDeleteCharacterFail);
				return TRUE;
			}

			if(mob->BaseScore.Level >= 219)
			{
				SendDBSignal(conn, m->ID, _MSG_DBDeleteCharacterFail);

				Log("err,deletechar level 219", pAccountList[Idx].File.Info.AccountName, 0);

				return TRUE;
			}

	
			memset(pAccountList[Idx].File.ShortSkill[Slot], 0, 16);

			DeleteCharacter(mob->MobName, pAccountList[Idx].File.Info.AccountName);

			char Temp[128];
			sprintf(Temp, "delete char [%s]", mob->MobName);		

			Log(Temp, pAccountList[Idx].File.Info.AccountName, 0);

			BASE_ClearMob(mob);
			BASE_ClearMobExtra(&pAccountList[Idx].File.mobExtra[Slot]);

			DBWriteAccount(&pAccountList[Idx].File);

			MSG_CNFDeleteCharacter sm;

			sm.Type = _MSG_DBCNFDeleteCharacter;

			DBGetSelChar(&sm.SelChar, &pAccountList[Idx].File);

			sm.ID = m->ID;

			pUser[conn].cSock.SendOneMessage((char*)&sm, sizeof(MSG_CNFDeleteCharacter));

		} break;

	case _MSG_AccountSecure:
	{
		MSG_AccountSecure *m = (MSG_AccountSecure*)Msg;

		int Idx = GetIndex(conn, m->ID);
		int Secure = pAccountList[Idx].SecurePass;

		int Change = m->ChangeNumeric;

		if (Change && Secure == -1)
		{
			Log("err,secureverify change error 1", pAccountList[Idx].File.Info.AccountName, 0);

			break;
		}

		if (pAccountList[Idx].File.Info.NumericToken[0] == -1)
		{
			strncpy(pAccountList[Idx].File.Info.NumericToken, m->NumericToken, 6);

			int ret = DBWriteAccount(&pAccountList[Idx].File);

			if (ret == 0)
			{
				Log("err,save secure - create file", pAccountList[Idx].File.Info.AccountName, 0);

				return TRUE;
			}

			pAccountList[Idx].SecurePass = 1;
			SendDBSignal(conn, m->ID, _MSG_AccountSecure);

			break;
		}

		if (Change == 0 && Secure == -1 && strncmp(pAccountList[Idx].File.Info.NumericToken, m->NumericToken, 6) == 0)
		{
			pAccountList[Idx].SecurePass = 1;
			SendDBSignal(conn, m->ID, _MSG_AccountSecure);

			break;
		}

		else if (Change == 1 && Secure == 1)
		{
			strncpy(pAccountList[Idx].File.Info.NumericToken, m->NumericToken, 6);

			int ret = DBWriteAccount(&pAccountList[Idx].File);

			if (ret == 0)
			{
				Log("err,save secure - create file", pAccountList[Idx].File.Info.AccountName, 0);

				return TRUE;
			}

			pAccountList[Idx].SecurePass = 1;
			SendDBSignal(conn, m->ID, _MSG_AccountSecure);

			break;
		}

		pAccountList[Idx].SecurePass = -1;
		SendDBSignal(conn, m->ID, _MSG_AccountSecureFail);
	} break;

	case _MSG_DBCreateArchCharacter:
	{
		MSG_DBCreateArchCharacter *m = (MSG_DBCreateArchCharacter*)Msg;

		int cls = m->MobClass;
		int MortalFace = m->MortalFace;

		int Idx = GetIndex(conn, m->ID);

		int Slot = 0;

		for (Slot = 0; Slot < MOB_PER_ACCOUNT; Slot++)
		if (pAccountList[Idx].File.Char[Slot].MobName[0] == 0)
			break;			   

		if (Slot < 0 || Slot >= MOB_PER_ACCOUNT)
		{
			Log("err,newchar  slot out of range", pAccountList[Idx].File.Info.AccountName, 0);

			SendDBSignal(conn, m->ID, _MSG_DBCNFArchCharacterFail);

			return TRUE;
		}
		if (cls < 0 || cls >= MAX_CLASS)
		{
			Log("err,newchar - class out of range", pAccountList[Idx].File.Info.AccountName, 0);

			SendDBSignal(conn, m->ID, _MSG_DBNewCharacterFail);

			return TRUE;
		}

		char check[NAME_LENGTH];

		strncpy(check, m->MobName, NAME_LENGTH);

		_strupr(check);

		if(strcmp(check, "KING") == 0 || strcmp(check, "KINGDOM") == 0 || strcmp(check, "GRITAR") == 0 || strcmp(check, "RELO") == 0)
		{
			Log("err,newchar - cmd name", pAccountList[Idx].File.Info.AccountName, 0);

			SendDBSignal(conn, m->ID, _MSG_DBNewCharacterFail);

			return FALSE;
		}

		if ((check[0] == 'C' && check[1] == 'O' && check[2] == 'M' && check[3] >= '0' && check[3] <= '9' && check[4] == 0) ||
			(check[0] == 'L' && check[1] == 'P' && check[2] == 'T' && check[3] >= '0' && check[3] <= '9' && check[4] == 0))
		{
			Log("err,newchar - com", pAccountList[Idx].File.Info.AccountName, 0);

			SendDBSignal(conn, m->ID, _MSG_DBNewCharacterFail);

			return FALSE;
		}

		STRUCT_MOB *mob;

		mob = &pAccountList[Idx].File.Char[Slot];


		mob->MobName[NAME_LENGTH - 1] = 0;
		mob->MobName[NAME_LENGTH - 2] = 0;

		m->MobName[NAME_LENGTH - 1] = 0;
		m->MobName[NAME_LENGTH - 2] = 0;

		if (mob->MobName[0] != 0)
		{
			SendDBSignal(conn, m->ID, _MSG_DBNewCharacterFail);

			Log("err,newchar already charged", pAccountList[Idx].File.Info.AccountName, 0);
			Log(mob->MobName, m->MobName, 0);

			return TRUE;
		}

		m->MobName[NAME_LENGTH - 1] = 0;
		m->MobName[NAME_LENGTH - 2] = 0;

		int len = strlen(m->MobName);

		for (int i = 0; i < len; i++)
		{
			if (m->MobName[i] == 'í' && m->MobName[i + 1] == 'í')
			{
				SendDBSignal(conn, m->ID, _MSG_DBNewCharacterFail);

				return TRUE;
			}
		}

								  
		STRUCT_MOBEXTRA *extra;

		extra = &pAccountList[Idx].File.mobExtra[Slot];

		BASE_ClearMob(mob);
		BASE_ClearMobExtra(extra);

		memset(&pAccountList[Idx].File.affect[Slot], 0, sizeof(pAccountList[Idx].File.affect[Slot]));
		memset(&pAccountList[Idx].File.ShortSkill[Slot], -1, 16);

		extra->ClassMaster = ARCH;
		extra->QuestInfo.Arch.MortalSlot = m->MortalSlot;

		if (cls == 0)
			memcpy(mob, &g_pBaseSet[0], sizeof(STRUCT_MOB));

		else if (cls == 1)
			memcpy(mob, &g_pBaseSet[1], sizeof(STRUCT_MOB));

		else if (cls == 2)
			memcpy(mob, &g_pBaseSet[2], sizeof(STRUCT_MOB));

		else if (cls == 3)
			memcpy(mob, &g_pBaseSet[3], sizeof(STRUCT_MOB));

		else
		{
			SendDBSignal(conn, m->ID, _MSG_DBNewCharacterFail);

			Log("err,newchar fail - undefined class", m->MobName, 0);

			return TRUE;
		}

		mob->Equip[0].sIndex = MortalFace + 5 + cls;
		mob->BaseScore.Ac = 230;

		memset(&mob->Equip[1], 0, sizeof(STRUCT_ITEM)*(MAX_EQUIP - 1));
		memset(&mob->Carry[0], 0, sizeof(STRUCT_ITEM)*(MAX_CARRY - 4));

		extra->MortalFace = MortalFace;

		memcpy(mob->MobName, m->MobName, NAME_LENGTH);

		int ret = DBWriteAccount(&pAccountList[Idx].File);

		if (ret == 0)
		{
			SendDBSignal(conn, m->ID, _MSG_DBNewCharacterFail);

			Log("err,newchar fail - create file", m->MobName, 0);

			return TRUE;
		}

		char temp[256];

		sprintf(temp, "create character [%s]", mob->MobName);

		Log(temp, pAccountList[Idx].File.Info.AccountName, 0);

		MSG_CNFNewCharacter sm;

		sm.Type = _MSG_DBCNFNewCharacter;

		DBGetSelChar(&sm.SelChar, &pAccountList[Idx].File);

		sm.ID = m->ID;

		SendDBSignalParm(conn, m->ID, _MSG_DBCNFArchCharacterSucess, Slot);
		pUser[conn].cSock.SendOneMessage((char*)&sm, sizeof(MSG_CNFNewCharacter));

	}	break;

	case _MSG_DBCapsuleInfo:
	{
		MSG_STANDARDPARM *m = (MSG_STANDARDPARM*)Msg;

		int Index = m->Parm;

		char temp[256];

		sprintf(temp, "./capsule/%d", Index);

		int Handle = _open(temp, O_RDONLY | O_BINARY);

		if(Handle == -1)
		{
			/*if(errno == EINVAL)
				Log("err readcapsule EINVAL", temp, 0);
			else if(errno == EMFILE)
				Log("err readcapsule EEMFILE", temp, 0);
			else
				Log("err readcapsule UNKNOW", temp, 0);
				*/
			return FALSE;
		}

		STRUCT_CAPSULE file;

		_read(Handle, &file, sizeof(STRUCT_CAPSULE));

		_close(Handle);

		// Resposta no layout que o cliente lê (MSG_CAPSULEINFO, 0xDC3). Skills e quest ficam 0: o
		// cliente mostra 9 skills e bits de quest que não têm correspondência direta no personagem.
		MSG_CNFCapsuleInfo sm;
		memset(&sm, 0, sizeof(sm));

		sm.Size = sizeof(MSG_CNFCapsuleInfo);
		sm.Type = _MSG_CNFCapsuleInfo;
		sm.ID = m->ID;

		sm.CIndex = Index;
		sm.Class = file.MOB.Equip[0].sIndex;
		sm.Level = file.MOB.BaseScore.Level;

		sm.sStr = file.MOB.BaseScore.Str;
		sm.sInt = file.MOB.BaseScore.Int;
		sm.sDex = file.MOB.BaseScore.Dex;
		sm.sCon = file.MOB.BaseScore.Con;

		for (int i = 0; i < 4; i++)
			sm.Mastery[i] = file.MOB.BaseScore.Special[i];

		pUser[conn].cSock.SendOneMessage((char*)&sm, sizeof(MSG_CNFCapsuleInfo));
	} break;

	case _MSG_DBPutInCapsule:
	{
		MSG_STANDARDPARM *m = (MSG_STANDARDPARM*)Msg;

		int Slot = m->Parm;

		int Idx = GetIndex(conn, m->ID);
		   
		if (Slot < 0 || Slot >= MOB_PER_ACCOUNT)
		{
			Log("err,putincapsule  slot out of range", pAccountList[Idx].File.Info.AccountName, 0);

			return TRUE;
		}
		LastCapsule++;

		char temp[256];
		sprintf(temp, "./capsule/%d", LastCapsule);

		int Handle = _open(temp, O_RDWR | O_CREAT | O_BINARY, _S_IREAD | _S_IWRITE);

		if(Handle == -1)
		{
			 if (errno == EEXIST)
				 Log("err putincapsulewrite EEXIST", 0, 0);
			 if (errno == EACCES)
				 Log("err putincapsulewrite EACCES", 0, 0);
			 if (errno == EINVAL)
				 Log("err putincapsulewrite EINVAL", 0, 0);
			 if (errno == EMFILE)
				 Log("err putincapsulewrite EMFILE", 0, 0);
			 if (errno == ENOENT)
				 Log("err putincapsulewrite ENOENT", 0, 0);
			 else
				Log("err putincapsulewrite UNKNOWN", 0, 0);

			 return FALSE;
		}

		STRUCT_CAPSULE file;

		file.MOB = pAccountList[Idx].File.Char[Slot];
		file.extra = pAccountList[Idx].File.mobExtra[Slot];

		file.extra.QuestInfo.Arch.MortalSlot = -1;

		_write(Handle, &file, sizeof(STRUCT_CAPSULE));
		_close(Handle);

		memset(pAccountList[Idx].File.ShortSkill[Slot], 0, 16);

		char Temp[128];
		sprintf(Temp, "putchar in capsule [%s]", file.MOB.MobName);		

		Log(Temp, pAccountList[Idx].File.Info.AccountName, 0);

		STRUCT_MOB *mob = &pAccountList[Idx].File.Char[Slot];

		WriteConfig();

		BASE_ClearMob(mob);
		BASE_ClearMobExtra(&pAccountList[Idx].File.mobExtra[Slot]);

		DBWriteAccount(&pAccountList[Idx].File);

		SendDBSignal(conn, m->ID, _MSG_DBCNFCapsuleSucess);

		MSG_CNFDeleteCharacter sm;

		sm.Type = _MSG_DBCNFDeleteCharacter;

		DBGetSelChar(&sm.SelChar, &pAccountList[Idx].File);

		sm.ID = m->ID;

		pUser[conn].cSock.SendOneMessage((char*)&sm, sizeof(MSG_CNFDeleteCharacter));

		sprintf(temp, "../../Common/ImportItem/%d%d%d%d%d", LastCapsule, W2Rand()%255, W2Rand()%255, W2Rand()%255, W2Rand()%255);
		
		FILE *fp = fopen(temp, "a+");
		fprintf(fp, "%s %d %d %d %d %d %d %d", pAccountList[Idx].File.Info.AccountName, 3443, 59, LastCapsule/256, 59,(LastCapsule - ((LastCapsule/256)*256)), 0, 0);
		fclose(fp);
	} break;

	case _MSG_DBOutCapsule:
	{
		MSG_DBOutCapsule *m = (MSG_DBOutCapsule*)Msg;

		int Slot = m->Slot;
		int NovoSlot = 0;
		int Idx = GetIndex(conn, m->ID);
		   
		if (Slot < 0 || Slot >= MOB_PER_ACCOUNT)
		{
			Log("err,outcapsule  slot out of range", pAccountList[Idx].File.Info.AccountName, 0);

			return TRUE;
		}

		for (NovoSlot = 0; NovoSlot < MOB_PER_ACCOUNT; NovoSlot++)
		{
			if (pAccountList[Idx].File.Char[NovoSlot].MobName[0] == 0)
				break;			   
		}
		if (NovoSlot < 0 || NovoSlot >= MOB_PER_ACCOUNT)
		{
			Log("err,newcharcapsule  slot out of range", pAccountList[Idx].File.Info.AccountName, 0);

			SendDBSignal(conn, m->ID, _MSG_DBCNFCapsuleCharacterFail);

			return TRUE;
		}

		int ret = CreateCharacter(pAccountList[Idx].File.Info.AccountName, m->MobName);

		if(ret == 0)
		{
			SendDBSignal(conn, m->ID, _MSG_DBCNFCapsuleCharacterFail2);

			return TRUE;
		}

		STRUCT_ITEM *item = GetItemPointer(&pAccountList[Idx].File.Char[Slot], pAccountList[Idx].File.Cargo, m->SourType, m->SourPos);

		int index = item->stEffect[0].cValue * 256 + item->stEffect[1].cValue;

		char temp[256];
		sprintf(temp, "./capsule/%d", index);

		int Handle = _open(temp, O_RDONLY | O_BINARY);

		if(Handle == -1)
		{
			 if (errno == EEXIST)
				 Log("err putincapsuleread EEXIST", 0, 0);
			 if (errno == EACCES)
				 Log("err putincapsuleread EACCES", 0, 0);
			 if (errno == EINVAL)
				 Log("err putincapsuleread EINVAL", 0, 0);
			 if (errno == EMFILE)
				 Log("err putincapsuleread EMFILE", 0, 0);
			 if (errno == ENOENT)
				 Log("err putincapsuleread ENOENT", 0, 0);
			 else
				Log("err putincapsuleread UNKNOWN", 0, 0);

			 return FALSE;
		}

		STRUCT_CAPSULE file;

		_read(Handle, &file, sizeof(STRUCT_CAPSULE));
		_close(Handle);

		strncpy(file.MOB.MobName, m->MobName, 12);

		STRUCT_MOB *mob;

		mob = &pAccountList[Idx].File.Char[NovoSlot];


		mob->MobName[NAME_LENGTH - 1] = 0;
		mob->MobName[NAME_LENGTH - 2] = 0;

		m->MobName[NAME_LENGTH - 1] = 0;
		m->MobName[NAME_LENGTH - 2] = 0;

		if (mob->MobName[0] != 0)
		{
			SendDBSignal(conn, m->ID, _MSG_DBNewCharacterFail);

			Log("err,newchar already charged", pAccountList[Idx].File.Info.AccountName, 0);
			Log(mob->MobName, m->MobName, 0);

			return TRUE;
		}

		m->MobName[NAME_LENGTH - 1] = 0;
		m->MobName[NAME_LENGTH - 2] = 0;

		int len = strlen(m->MobName);

		for (int i = 0; i < len; i++)
		{
			if (m->MobName[i] == 'í' && m->MobName[i + 1] == 'í')
			{
				SendDBSignal(conn, m->ID, _MSG_DBNewCharacterFail);

				return TRUE;
			}
		}
								  
		STRUCT_MOBEXTRA *extra;

		extra = &pAccountList[Idx].File.mobExtra[NovoSlot];

		mob = &file.MOB;
		extra = &file.extra;

		memset(item, 0, sizeof(STRUCT_ITEM));

		memset(&pAccountList[Idx].File.affect[NovoSlot], 0, sizeof(pAccountList[Idx].File.affect[NovoSlot]));
		memset(&pAccountList[Idx].File.ShortSkill[NovoSlot], -1, 16);

		memset(&mob->Equip[1], 0, sizeof(STRUCT_ITEM)*(MAX_EQUIP - 2));
		memset(&mob->Carry[0], 0, sizeof(STRUCT_ITEM)*(MAX_CARRY - 4));

		mob->Coin = 0;
		mob->Guild = 0;
		mob->GuildLevel = 0;

		memcpy(mob->MobName, m->MobName, NAME_LENGTH);

		ret = DBWriteAccount(&pAccountList[Idx].File);

		if (ret == 0)
		{
			SendDBSignal(conn, m->ID, _MSG_DBNewCharacterFail);

			Log("err,newchar fail - create file", m->MobName, 0);

			return TRUE;
		}
		memcpy(&pAccountList[Idx].File.Char[NovoSlot], mob, sizeof(STRUCT_MOB));
		memcpy(&pAccountList[Idx].File.mobExtra[NovoSlot], extra, sizeof(STRUCT_MOBEXTRA));

		sprintf(temp, "create character [%s] - capsule", mob->MobName);

		Log(temp, pAccountList[Idx].File.Info.AccountName, 0);

		sprintf(temp, "./capsule/%d", index);
		DeleteFileA(temp);


		MSG_CNFNewCharacter sm;

		sm.Type = _MSG_DBCNFNewCharacter;

		DBGetSelChar(&sm.SelChar, &pAccountList[Idx].File);

		sm.ID = m->ID;

		pUser[conn].cSock.SendOneMessage((char*)&sm, sizeof(MSG_CNFNewCharacter));
	} break;

	case _MSG_DBServerChange:
	{
		MSG_DBServerChange *m = (MSG_DBServerChange*)Msg;

		int ID = m->ID;
		int NewServer = m->NewServerID;
		int Slot = m->Slot;

		if (ID <= 0 || ID >= MAX_USER)
			break;

		char temp[256];

		if (NewServer <= 0 || NewServer > MAX_SERVER + 1)
		{
			sprintf(temp, "err,remove server srvnum:%d ", NewServer);
			Log(temp, "system", 0);
			break;
		}
		int Idx = GetIndex(conn, m->ID);

		int Enc[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
		
		int EncResult = GetEncPassword(Idx, Enc);

		if (EncResult)
		{
			MSG_STANDARDPARM sm;
			memset(&sm, 0, sizeof(MSG_STANDARDPARM));

			sm.Type = _MSG_DBServerSend1;
			sm.Size = sizeof(MSG_STANDARDPARM);
			sm.ID = ID;

			sm.Parm = EncResult;

			pUser[conn].cSock.SendOneMessage((char*)&sm, sizeof(MSG_STANDARDPARM));
		}

		else
		{
			SetEncPassword(Idx, NewServer, Enc, m->Slot);
			
			MSG_DBCNFServerChange sm;
			memset(&sm, 0, sizeof(MSG_DBCNFServerChange));
			
			sm.Type = _MSG_DBCNFServerChange;
			sm.Size = sizeof(MSG_DBCNFServerChange);
			sm.ID = ID;

			strcpy(sm.AccountName, pAccountList[Idx].File.Info.AccountName);

			sprintf(sm.Enc, "*%d %d %d %d %d %d %d %d %d %d", m->NewServerID,
				Enc[0], Enc[1], Enc[2], Enc[3], Enc[4], Enc[5], Enc[6], Enc[7], m->Slot);

			memcpy(pAccountList[Idx].File.TempKey, sm.Enc, sizeof(pAccountList[Idx].File.TempKey));

			pUser[conn].cSock.SendOneMessage((char*)&sm, sizeof(MSG_DBCNFServerChange));
		}
	} break;

	case _MSG_DBActivatePinCode:
	{
		MSG_DBActivatePinCode *m = (MSG_DBActivatePinCode*)Msg;

		// CActivePinCode::ProcessMessage(Msg, conn);

	} break;

	case _MSG_DBPrimaryAccount:
	{
		MSG_DBPrimaryAccount *m = (MSG_DBPrimaryAccount*)Msg;

		int Idx = GetIndex(conn, m->ID);

		MSG_DBCheckPrimaryAccount sm;
		memset(&sm, 0, sizeof(MSG_DBCheckPrimaryAccount));

		sm.Size = sizeof(MSG_DBCheckPrimaryAccount);
		sm.Type = _MSG_DBCheckPrimaryAccount;
		
		memcpy(&sm.Mac, m->Mac, sizeof(sm.Mac));

		strncpy(sm.AccountName, pAccountList[Idx].File.Info.AccountName, ACCOUNTNAME_LENGTH);

		for (int i = 0; i < MAX_SERVER; i++)
		{
			if (pUser[i].cSock.Sock == 0 || pUser[i].Mode == USER_EMPTY)
				continue;

			pUser[i].cSock.SendOneMessage((char*)&sm, sizeof(MSG_DBCheckPrimaryAccount));
		}

		MSG_DBClientMessage sm2;

		sm2.Type = _MSG_DBClientMessage;
		sm2.ID = m->ID;
		sm2.Size = sizeof(MSG_DBClientMessage);

		strcpy(sm2.String, "Sua conta agora é a primária.");

		pUser[conn].cSock.SendOneMessage((char*)&sm2, sizeof(MSG_DBClientMessage));
	} break;
	default:
		handled = false;
		break;
	}

	return TRUE;
}
