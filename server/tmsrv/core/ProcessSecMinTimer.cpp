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
#include "systems/CWarTower.h"
#include "systems/SecTimerInternal.h"

void ProcessSecTimer()
{
	if (ServerDown == 120) // two minutes untill the server closes
	{
		if (BILLING != 0)
		{
			_AUTH_GAME Unk;

			memset(&Unk, 0, 196);

			SendBilling2(&Unk, 4);

			BILLING = 0;
		}

		while (TRUE)
		{
			if (UserCount >= MAX_USER)
			{
				Log("sys,server down complete", "-system", 0);

				if (hFont != 0)
				{
					DeleteObject(hFont);

					hFont = NULL;
				}

				PostQuitMessage(NULL);

				return;
			}

			if (pUser[UserCount].Mode == USER_EMPTY)
			{
				UserCount++;

				continue;
			}

			break;
		}

		pUser[UserCount].AccountName[ACCOUNTNAME_LENGTH - 1] = 0;
		pUser[UserCount].AccountName[ACCOUNTNAME_LENGTH - 2] = 0;

		sprintf(temp, "sys,save %d - %d - %s", UserCount, pUser[UserCount].Mode, pUser[UserCount].AccountName);
		Log(temp, "-system", 0);

		CloseUser(UserCount);

		UserCount++;

		return;
	}

	if (ServerDown > -1000)
	{
		if (ServerDown <= 0)
		{
			ServerDown++;

			goto lbl_PST1;
		}

		if (ServerDown % 20 == 1)
		{
			int messageId = ServerDown / 20;
			SendNotice(g_pMessageStringTable[messageId + 17]);
		}

		ServerDown++;

		if (ServerDown == 120)
		{
			SetTimer(hWndMain, TIMER_SEC, 200, 0);

			return;
		}
	}

lbl_PST1:

	SecCounter++;

	CurrentTime = timeGetTime();

	if(SecCounter % 120)
		CReadFiles::ReadMacblock();

	if (BILLING != 0 && BillCounter > 0 && BillServerSocket.Sock == 0)
	{
		BillCounter--;

		if (BillCounter <= 0)
		{
			CurrentTime = timeGetTime();

			sprintf(temp, "err,start reconnect BILL %d", CurrentTime);
			Log(temp, "-system", 0);

			int *pip = (int*)LocalIP;

			int ret = BillServerSocket.ConnectBillServer(BillServerAddress, BillServerPort, *pip, WSA_READBILL);

			CurrentTime = timeGetTime();

			sprintf(temp, "err,start reconnect BILL %d", CurrentTime);
			Log(temp, "-system", 0);

			if (ret == 0)
			{
				Log("err,Reconnect BILL Server(x2) fail.", "-system", 0);

				BILLING = 0;
			}
			else
			{
				_AUTH_GAME Unk;

				memset(&Unk, 0, 196);

				SendBilling2(&Unk, 4);
			}
		}
	}
	else
		BillCounter = 0;
	
	if (SecCounter % 8 == 0)
	{
		for (int i = 0; i < MAX_USER; i++)
		{
			if (SaveCount >= MAX_USER)
				SaveCount = 1;

			if (pUser[SaveCount].Mode == USER_PLAY && pMob[SaveCount].Mode != MOB_EMPTY)
			{
				if (BILLING == 2 && pUser[SaveCount].Unk_2728 == 1 && pMob[SaveCount].MOB.CurrentScore.Level >= FREEEXP && (g_Hour <= 12 || g_Hour >= 19))
				{
					SendClientMessage(SaveCount, g_pMessageStringTable[_NN_Child_Pay]);
					CharLogOut(SaveCount);

					SaveCount++;

					break;
				}
				else
				{
					SaveUser(SaveCount, 0);

					SaveCount++;

					break;
				}
			}

			SaveCount++;
		}
	}

	for (int bb = 1; bb < MAX_USER; bb++)
	{
		if (pUser[bb].Mode)
		{
			if (pUser[bb].cSock.nSendPosition)
			{
				BOOL bSend = pUser[bb].cSock.SendMessageA();
				if (!bSend)
				{
					sprintf(temp, "err,send fail close %d/%d %d/%d",
						bb, pUser[bb].cSock.Sock, pUser[bb].cSock.nSendPosition, pUser[bb].cSock.nSentPosition);

					pUser[bb].AccountName[ACCOUNTNAME_LENGTH - 1] = 0;
					pUser[bb].AccountName[ACCOUNTNAME_LENGTH - 2] = 0;

					Log(temp, pUser[bb].AccountName, pUser[bb].IP);

					CloseUser(bb);
				}
			}
		}

		if(pUser[bb].Mode == USER_PLAY)
			pMob[bb].ProcessorSecTimer();

		//Celestial Lv181 dentro da zona do pesadelo
		if((pMob[bb].TargetX/128) == 9 && (pMob[bb].TargetY/128) == 1 || (pMob[bb].TargetX/128) == 8 && (pMob[bb].TargetY/128) == 2 || (pMob[bb].TargetX/128) == 10 && (pMob[bb].TargetY/128) == 2)
		{
			if((pMob[bb].extra.ClassMaster == CELESTIAL || pMob[bb].extra.ClassMaster == CELESTIALCS || pMob[bb].extra.ClassMaster == SCELESTIAL) && pMob[bb].MOB.CurrentScore.Level >= 180)
				DoRecall(bb);
		}

		// Portais das salas da Pista de Runas +5 (Balrog).
		ProcessRuneQuestPortals(bb);
	}

	if(SecCounter % 12 == 0)
	{
		//Carta Sala 1
		int Sala1 = mNPCGen.pList[SECRET_ROOM_N_SALA1_MOB_1].CurrentNumMob + mNPCGen.pList[SECRET_ROOM_N_SALA1_MOB_2].CurrentNumMob + mNPCGen.pList[SECRET_ROOM_M_SALA1_MOB_1].CurrentNumMob + mNPCGen.pList[SECRET_ROOM_M_SALA1_MOB_2].CurrentNumMob + mNPCGen.pList[SECRET_ROOM_A_SALA1_MOB_1].CurrentNumMob + mNPCGen.pList[SECRET_ROOM_A_SALA1_MOB_2].CurrentNumMob;
		int Sala2 = mNPCGen.pList[SECRET_ROOM_N_SALA2_MOB_1].CurrentNumMob + mNPCGen.pList[SECRET_ROOM_N_SALA2_MOB_2].CurrentNumMob + mNPCGen.pList[SECRET_ROOM_M_SALA2_MOB_1].CurrentNumMob + mNPCGen.pList[SECRET_ROOM_M_SALA2_MOB_2].CurrentNumMob + mNPCGen.pList[SECRET_ROOM_A_SALA2_MOB_1].CurrentNumMob + mNPCGen.pList[SECRET_ROOM_A_SALA2_MOB_2].CurrentNumMob;
		int Sala3 = mNPCGen.pList[SECRET_ROOM_N_SALA3_MOB_1].CurrentNumMob + mNPCGen.pList[SECRET_ROOM_N_SALA3_MOB_2].CurrentNumMob + mNPCGen.pList[SECRET_ROOM_M_SALA3_MOB_1].CurrentNumMob + mNPCGen.pList[SECRET_ROOM_M_SALA3_MOB_2].CurrentNumMob + mNPCGen.pList[SECRET_ROOM_A_SALA3_MOB_1].CurrentNumMob + mNPCGen.pList[SECRET_ROOM_A_SALA3_MOB_2].CurrentNumMob;
		int Sala4 = mNPCGen.pList[SECRET_ROOM_N_SALA4_MOB_1].CurrentNumMob + mNPCGen.pList[SECRET_ROOM_N_SALA4_MOB_2].CurrentNumMob + mNPCGen.pList[SECRET_ROOM_N_SALA4_MOB_3].CurrentNumMob + mNPCGen.pList[SECRET_ROOM_N_SALA4_MOB_4].CurrentNumMob + mNPCGen.pList[SECRET_ROOM_M_SALA4_MOB_1].CurrentNumMob + mNPCGen.pList[SECRET_ROOM_M_SALA4_MOB_2].CurrentNumMob + mNPCGen.pList[SECRET_ROOM_M_SALA4_MOB_3].CurrentNumMob + mNPCGen.pList[SECRET_ROOM_M_SALA4_MOB_4].CurrentNumMob + mNPCGen.pList[SECRET_ROOM_A_SALA4_MOB_1].CurrentNumMob + mNPCGen.pList[SECRET_ROOM_A_SALA4_MOB_2].CurrentNumMob + mNPCGen.pList[SECRET_ROOM_A_SALA4_MOB_3].CurrentNumMob + mNPCGen.pList[SECRET_ROOM_A_SALA4_MOB_4].CurrentNumMob;

		SendSignalParmArea(778, 3651, 832, 3699, ESCENE_FIELD, _MSG_MobLeft, Sala1);
		//Carta Sala 2
		SendSignalParmArea(836, 3651, 890, 3699, ESCENE_FIELD, _MSG_MobLeft, Sala2);
		//Carta Sala 3
		SendSignalParmArea(832, 3594, 890, 3644, ESCENE_FIELD, _MSG_MobLeft, Sala3);
		//Carta Sala 4
		SendSignalParmArea(780, 3594, 832, 3645, ESCENE_FIELD, _MSG_MobLeft, Sala4);

		//Pista de runas +6 - Coelho
		SendSignalParmArea(3330, 1475, 3448, 1525, ESCENE_FIELD, _MSG_MobLeft, Pista[6].Party[0].MobCount);

		//RvR
		// O cliente (OnPacketRemainNPCCount) lê um int Parm e mostra "Parm & 0xFF / Parm >> 16": o byte
		// baixo do primeiro short e o segundo short. O +1024 de antes era de outro cliente: o vermelho
		// saía certo (o byte baixo de 1024 é 0) e o azul aparecia somado a 1024.
		SendShortSignalParm2Area(1017, 1911, 1290, 2183, ESCENE_FIELD, _MSG_MobCount, RvRRedPoint, RvRBluePoint);
	}

	if(SecCounter % 1200 == 0)
	{
		ClearAreaQuest(2379, 2076, 2426, 2133);// Coveiro
		ClearAreaQuest(2232, 1564, 2263, 1592);// Capa Verde
		ClearAreaQuest(2640, 1966, 2670, 2004);//Reset habilidades
		ClearAreaQuest(2228, 1700, 2257, 1728);//Carbuncle
		ClearAreaQuest(1950, 1586, 1988, 1614);//Reset habilidades
		ClearAreaQuest(459, 3887, 497, 3916);//Kaizen
		ClearAreaQuest(658, 3728, 703, 3762);//Hidras
		ClearAreaQuest(1312, 4027, 1348, 4055);//Elfos
		ClearAreaQuest(793, 4046, 827, 4080);//Quest Gargula
		ClearArea(3570, 3446, 3965, 3711);//Lanhouse

		for (int x = 1; x < MAX_USER; x++)
		{
			if (pMob[x].QuestFlag && pUser[x].Mode == USER_PLAY)
				pMob[x].QuestFlag = 0;
		}
	}
	if(SecCounter % 120 == 0)
	{
		for(int user = 0; user < MAX_USER; user++)
		{
			if(pUser[user].Mode != USER_PLAY)
				continue;

			if (pMob[user].MOB.Equip[14].sIndex >= 3980 && pMob[user].MOB.Equip[14].sIndex <= 3989 && BASE_CheckItemDate(&pMob[user].MOB.Equip[14]))
			{
				sprintf(temp, "etc,premium item end %d", pMob[user].MOB.Equip[14].sIndex);
				Log(temp, "-system", 0);

				BASE_ClearItem(&pMob[user].MOB.Equip[14]);
				SendItem(user, ITEM_PLACE_EQUIP, 14, &pMob[user].MOB.Equip[14]);
			}

			if (pMob[user].MOB.Equip[12].sIndex >= 4150 && pMob[user].MOB.Equip[12].sIndex <= 4188 && BASE_CheckItemDate(&pMob[user].MOB.Equip[12]))
			{
				sprintf(temp, "etc,premium item end %d", pMob[user].MOB.Equip[12].sIndex);
				Log(temp, "-system", 0);

				BASE_ClearItem(&pMob[user].MOB.Equip[12]);
				SendItem(user, ITEM_PLACE_EQUIP, 12, &pMob[user].MOB.Equip[12]);
			}

			if(pMob[user].MOB.Equip[13].sIndex >= 3900 && pMob[user].MOB.Equip[13].sIndex <= 3913)
			{
				BASE_CheckFairyDate(&pMob[user].MOB.Equip[13]);
				SendItem(user, ITEM_PLACE_EQUIP, 13, &pMob[user].MOB.Equip[13]);
				pMob[user].GetCurrentScore(user);
			}

			if (pMob[user].MOB.Carry[60].sIndex == 3467 && BASE_CheckItemDate(&pMob[user].MOB.Carry[60]))
			{
				BASE_ClearItem(&pMob[user].MOB.Carry[60]);
				SendItem(user, ITEM_PLACE_CARRY, 60, &pMob[user].MOB.Carry[60]);

			}

			if (pMob[user].MOB.Carry[61].sIndex == 3467 && BASE_CheckItemDate(&pMob[user].MOB.Carry[61]))
			{
				BASE_ClearItem(&pMob[user].MOB.Carry[61]);
				SendItem(user, ITEM_PLACE_CARRY, 61, &pMob[user].MOB.Carry[61]);

			}
		}
	}

	if (SecCounter % 2 == 0)
	{
		for (int i = 1; i < MAX_USER; i++)
		{
			if (pUser[i].Mode == USER_PLAY && pMob[i].MOB.CurrentScore.Hp > 0)
			{
				pUser[i].Unk_2688 = 0;

				int retHp = ApplyHp(i);
				int retMp = ApplyMp(i);

				if (retHp != 0)
					SendScore(i);

				else if (retMp != 0)
					SendSetHpMp(i);
			}
		}
	}

	if (SecCounter % 20 == 0)
	{
		for (int i = 1; i < MAX_USER; i++)
		{
			if (pUser[i].Mode == USER_PLAY && pMob[i].MOB.CurrentScore.Hp > 0)
			{
				pUser[i].ReqHp += pMob[i].MOB.BaseScore.Level + 30;
				pUser[i].ReqMp += pMob[i].MOB.BaseScore.Level + 30;

				int retHp = ApplyHp(i);
				int retMp = ApplyMp(i);

				if (retHp != 0)
					SendScore(i);

				else if (retMp != 0)
					SendSetHpMp(i);
			}
		}
	}

	// Eventos agendados por horário (Kefra, Pesa, Pista de Runas): checados a cada 2s.
	if (SecCounter % 2 == 0)
		ProcessScheduledWorldEvents();

	if((SecCounter % 2) == 0)
		ProcessDecayItem();

	int UsersDie = 0;

	int Sec4 = SecCounter % 4;

	if (SecCounter % 4 == 0)
	{
		ProcessRanking();

		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 10; j++)
			{
				if (WaterClear1[i][j] > 1)
					WaterClear1[i][j]--;

				else if (WaterClear1[i][j] == 1 && j <= 7)
				{
					ClearAreaTeleport(WaterScrollPosition[i][j][0] - 8, WaterScrollPosition[i][j][1] - 8, WaterScrollPosition[i][j][0] + 8, WaterScrollPosition[i][j][1] + 8, 1965, 1769);
					WaterClear1[i][j] = 0;
				}
				else if (WaterClear1[i][j] == 1 && j > 7)
				{
					ClearAreaTeleport(WaterScrollPosition[i][j][0] - 12, WaterScrollPosition[i][j][1] - 12, WaterScrollPosition[i][j][0] + 12, WaterScrollPosition[i][j][1] + 12, 1965, 1769);
					WaterClear1[i][j] = 0;
				}
			}
		}

		if (CartaTime > 1)
			CartaTime--;

		if (CartaTime == 1)
		{
			CartaTime = 60;

			if (CartaSala == 1)
				ClearAreaTeleport(778, 3652, 832, 3698, CartaPos[1][0], CartaPos[1][1]);
			
			else if (CartaSala == 2)
				ClearAreaTeleport(836, 3652, 890, 3698, CartaPos[2][0], CartaPos[2][1]);
			
			else if (CartaSala == 3)
				ClearAreaTeleport(834, 3595, 889, 3645, CartaPos[3][0], CartaPos[3][1]);
			
		
			if (CartaSala == 4)
			{
				ClearArea(776, 3595, 834, 3648);
				CartaTime = 0;
				CartaSala = 0;

				sprintf(temp, "etc,clear secretroom");
				Log(temp, "-system", 0);
			}
			
			else
				CartaSala++;

			MSG_STANDARDPARM sm;

			sm.Type = _MSG_StartTime;
			sm.ID = ESCENE_FIELD;
			sm.Size = sizeof(MSG_STANDARDPARM);
			sm.Parm = CartaTime*2;

			MapaMulticast(6, 28, (MSG_STANDARD*)&sm, 0);
		}
	}
	CCastleZakum::ProcessSecTimer();

	int Sec16 = SecCounter % 16;
	int Sec32 = SecCounter % 32;

	int i = 0;
	
	for (i = 1; i < MAX_USER; i++)
	{
		if (i % 32 == Sec32 && pUser[i].Mode && pUser[i].Mode != USER_SAVING4QUIT)
			CheckIdle(i);

		if (i % 16 == Sec16 && pMob[i].MOB.CurrentScore.Hp > 0 && pUser[i].Mode == USER_PLAY)
		{
			RegenMob(i);
			ProcessAffect(i);
			UsersDie++;
		}

		//Coveiro
		if (pMob[i].QuestFlag != 1 && pMob[i].TargetX > 2379 && pMob[i].TargetY > 2076 && pMob[i].TargetX < 2426 && pMob[i].TargetY < 2133 && pMob[i].MOB.BaseScore.Level < 1000)
			DoRecall(i);

		//Jardin
		if (pMob[i].QuestFlag != 2 && pMob[i].TargetX > 2228 && pMob[i].TargetY > 1700 && pMob[i].TargetX < 2257 && pMob[i].TargetY < 1728 && pMob[i].MOB.BaseScore.Level < 1000)
			DoRecall(i);

		//Kaizen
		if (pMob[i].QuestFlag != 3 && pMob[i].TargetX > 459 && pMob[i].TargetY > 3887 && pMob[i].TargetX < 497 && pMob[i].TargetY < 3916 && pMob[i].MOB.BaseScore.Level < 1000)
			DoRecall(i);

		//Hidra
		if (pMob[i].QuestFlag != 4 && pMob[i].TargetX > 658 && pMob[i].TargetY > 3728 && pMob[i].TargetX < 703 && pMob[i].TargetY < 3762 && pMob[i].MOB.BaseScore.Level < 1000)
			DoRecall(i);

		//Elfos
		if (pMob[i].QuestFlag != 5 && pMob[i].TargetX > 1312 && pMob[i].TargetY > 4027 && pMob[i].TargetX < 1348 && pMob[i].TargetY < 4055 && pMob[i].MOB.BaseScore.Level < 1000)
			DoRecall(i);
	}

	if (BrState == 2 && i % 8 == Sec16 && BRItem > 0)
	{
		if (BrMod < 9)
		{
			if (BrMod >= 7)
			{
				SendDamage(2608, 1708, 2622, 1711);
				SendDamage(2608, 1739, 2622, 1743);
				SendEnvEffect(2608, 1708, 2622, 1711, 32, 0);
				SendEnvEffect(2608, 1739, 2622, 1743, 32, 0);
			}
		}
		else
		{
			SendDamage(2608, 1708, 2622, 1718);
			SendDamage(2608, 1733, 2622, 1743);
			SendEnvEffect(2608, 1708, 2622, 1718, 32, 0);
			SendEnvEffect(2608, 1733, 2622, 1743, 32, 0);
		}
	}


	if(SecCounter % 6 == 0)
	{
		SendEnvEffectKingdom(1050, 2108, 1070, 2146, 32, 0, REINO_BLUE);
		SendEnvEffectKingdom(1066, 2133, 1098, 2146, 32, 0, REINO_BLUE);
		SendDamageKingdom(1050, 2108, 1070, 2146, REINO_BLUE);
		SendDamageKingdom(1066, 2133, 1098, 2146, REINO_BLUE);

		SendEnvEffectKingdom(1230, 1947, 1245, 1988, 32, 0, REINO_RED);
		SendEnvEffectKingdom(1204, 1948, 1231, 1962, 32, 0, REINO_RED);
		SendDamageKingdom(1230, 1947, 1245, 1988, REINO_RED);
		SendDamageKingdom(1204, 1948, 1231, 1962, REINO_RED);
	}

	if(SecCounter % 5 == 0)
	{
		SendEnvEffectLeader(3436, 1285, 3450, 1299, 32, 0);
		SendDamageLeader(3436, 1285, 3450, 1299);
		SendEnvEffectLeader(3368, 1285, 3382, 1299, 32, 0);
		SendDamageLeader(3368, 1285, 3382, 1299);
		SendEnvEffectLeader(3384, 1302, 3398, 1316, 32, 0);
		SendDamageLeader(3384, 1302, 3398, 1316);
		SendEnvEffectLeader(3438, 1320, 3452, 1334, 32, 0);
		SendDamageLeader(3438, 1320, 3452, 1334);
		SendEnvEffectLeader(3369, 1320, 3383, 1334, 32, 0);
		SendDamageLeader(3369, 1320, 3383, 1334);
		SendEnvEffectLeader(3385, 1336, 3399, 1350, 32, 0);
		SendDamageLeader(3385, 1336, 3399, 1350);
		SendEnvEffectLeader(3402, 1353, 3416, 1367, 32, 0);
		SendDamageLeader(3402, 1353, 3416, 1367);
		SendEnvEffectLeader(3436, 1371, 3450, 1385, 32, 0);
		SendDamageLeader(3436, 1371, 3450, 1385);
		SendEnvEffectLeader(3368, 1371, 3382, 1385, 32, 0);
		SendDamageLeader(3368, 1371, 3382, 1385);
		SendEnvEffectLeader(3402, 1387, 3416, 1401, 32, 0);
		SendDamageLeader(3402, 1387, 3416, 1401);
		SendEnvEffectLeader(3420, 1286, 3434, 1300, 32, 0);
		SendDamageLeader(3420, 1286, 3434, 1300);
		SendEnvEffectLeader(3351, 1286, 3365, 1300, 32, 0);
		SendDamageLeader(3351, 1286, 3365, 1300);
		SendEnvEffectLeader(3402, 1302, 3416, 1316, 32, 0);
		SendDamageLeader(3402, 1302, 3416, 1316);
		SendEnvEffectLeader(3420, 1320, 3434, 1334, 32, 0);
		SendDamageLeader(3420, 1320, 3434, 1334);
		SendEnvEffectLeader(3371, 1337, 3385, 1351, 32, 0);
		SendDamageLeader(3371, 1337, 3385, 1351);
		SendEnvEffectLeader(3436, 1337, 3450, 1351, 32, 0);
		SendDamageLeader(3436, 1337, 3450, 1351);
		SendEnvEffectLeader(3385, 1353, 3399, 1367, 32, 0);
		SendDamageLeader(3385, 1353, 3399, 1367);
		SendEnvEffectLeader(3352, 1370, 3366, 1384, 32, 0);
		SendDamageLeader(3352, 1370, 3366, 1384);
		SendEnvEffectLeader(3419, 1370, 3433, 1384, 32, 0);
		SendDamageLeader(3419, 1370, 3433, 1384);
		SendEnvEffectLeader(3385, 1387, 3399, 1401, 32, 0);
		SendDamageLeader(3385, 1387, 3399, 1401);
		SendEnvEffectLeader(3402, 1286, 3416, 1300, 32, 0);
		SendDamageLeader(3402, 1286, 3416, 1300);
		SendEnvEffectLeader(3352, 1302, 3366, 1316, 32, 0);
		SendDamageLeader(3352, 1302, 3366, 1316);
		SendEnvEffectLeader(3420, 1302, 3434, 1316, 32, 0);
		SendDamageLeader(3420, 1302, 3434, 1316);
		SendEnvEffectLeader(3401, 1320, 3415, 1334, 32, 0);
		SendDamageLeader(3401, 1320, 3415, 1334);
		SendEnvEffectLeader(3420, 1336, 3434, 1350, 32, 0);
		SendDamageLeader(3420, 1336, 3434, 1350);
		SendEnvEffectLeader(3436, 1353, 3450, 1367, 32, 0);
		SendDamageLeader(3436, 1353, 3450, 1367);
		SendEnvEffectLeader(3368, 1353, 3382, 1367, 32, 0);
		SendDamageLeader(3368, 1353, 3382, 1367);
		SendEnvEffectLeader(3401, 1370, 3415, 1384, 32, 0);
		SendDamageLeader(3401, 1370, 3415, 1384);
		SendEnvEffectLeader(3435, 1387, 3449, 1401, 32, 0);
		SendDamageLeader(3435, 1387, 3449, 1401);
		SendEnvEffectLeader(3369, 1387, 3383, 1401, 32, 0);
		SendDamageLeader(3369, 1387, 3383, 1401);
		SendEnvEffectLeader(3387, 1285, 3401, 1299, 32, 0);
		SendDamageLeader(3387, 1285, 3401, 1299);
		SendEnvEffectLeader(3368, 1302, 3382, 1316, 32, 0);
		SendDamageLeader(3368, 1302, 3382, 1316);
		SendEnvEffectLeader(3436, 1302, 3450, 1316, 32, 0);
		SendDamageLeader(3436, 1302, 3450, 1316);
		SendEnvEffectLeader(3384, 1319, 3398, 1333, 32, 0);
		SendDamageLeader(3384, 1319, 3398, 1333);
		SendEnvEffectLeader(3402, 1335, 3416, 1349, 32, 0);
		SendDamageLeader(3402, 1335, 3416, 1349);
		SendEnvEffectLeader(3419, 1353, 3433, 1367, 32, 0);
		SendDamageLeader(3419, 1353, 3433, 1367);
		SendEnvEffectLeader(3352, 1353, 3366, 1367, 32, 0);
		SendDamageLeader(3352, 1353, 3366, 1367);
		SendEnvEffectLeader(3385, 1370, 3399, 1384, 32, 0);
		SendDamageLeader(3385, 1370, 3399, 1384);
		SendEnvEffectLeader(3419, 1387, 3433, 1401, 32, 0);
		SendDamageLeader(3419, 1387, 3433, 1401);
		SendEnvEffectLeader(3352, 1387, 3366, 1401, 32, 0);
		SendDamageLeader(3352, 1387, 3366, 1401);
		SendEnvEffectLeader(3335, 1286, 3349, 1300, 32, 0);
		SendDamageLeader(3335, 1286, 3349, 1300);
		SendEnvEffectLeader(3335, 1302, 3349, 1316, 32, 0);
		SendDamageLeader(3335, 1302, 3349, 1316);
		SendEnvEffectLeader(3335, 1354, 3349, 1368, 32, 0);
		SendDamageLeader(3335, 1354, 3349, 1368);
		SendEnvEffectLeader(3335, 1371, 3349, 1385, 32, 0);
		SendDamageLeader(3335, 1371, 3349, 1385);
	}

	for (int k = 0; k < MAX_MOB_MERC; k++)
	{
		if (pMobMerc[k].RenewTime == 0 || pMobMerc[k].RebornTime == 0 || pMobMerc[k].GenerateIndex == 0)
			continue;

		if (SecCounter % pMobMerc[k].RenewTime == 0)
			memcpy(pMobMerc[k].Stock, pMobMerc[k].MaxStock, sizeof(pMobMerc[k].Stock));

		if (SecCounter % pMobMerc[k].RebornTime == 0)
			GenerateMob(pMobMerc[k].GenerateIndex, 0, 0);
	}

	// CEncampment::ProcessSecTimer();

	int Sec6 = SecCounter % 6;

	// Movimento / fala dos NPCs (1/6 dos mobs por segundo).
	int initial = Sec6 + MAX_USER;
	ProcessNpcMovementTick(initial);

	Sec4 = SecCounter % 4;

	// Ataque / fala em ação dos mobs em combate (1/4 dos mobs por segundo).
	ProcessMobCombatTick(Sec4, Sec16);
}

void ProcessMinTimer()
{
	time_t rawtime;
	tm *timeinfo;

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	if (timeinfo->tm_mday != LastLogDay)
		StartLog();

	if (timeinfo->tm_mday != LastChatLogDay)
		StartChatLog();

	if (timeinfo->tm_mday != LastItemLogDay)
		StartItemLog();

	if ((MinCounter % 2) == 0)
	{
		int NewbieServerID = (timeinfo->tm_mday - 1) % NumServerInGroup;

		if (LOCALSERVER != 0 || ServerIndex == NewbieServerID)
			NewbieEventServer = 1;

		else if (NewbieEventServer == 1 && ServerIndex != NewbieServerID)
		{
			// CEncampment::ChangeNewBieChannel();

			for (int i = 1; i < MAX_USER; i++)
			{
				if (pUser[i].Mode == USER_EMPTY || pUser[i].cSock.Sock == 0)
					continue;

				if (pUser[i].TradeMode == 1)
					RemoveTrade(i);
			}

			NewbieEventServer = 0;
		}


		if (1 != 0) // Likely to be a debug flag of sorts.. TODO: Check and decide if it stays
		{
			int wNum = BASE_GetWeekNumber();
			int wMod = wNum % 7;
			int wDay = wNum / 7;

			if (wMod == 0 && ((wDay % 2) == (ServerIndex % 2)))
				CastleServer = 1;
			else
				CastleServer = 0;

		}
	}

	for(int i = 0; i < MAX_GUILDZONE; i++)
	{
		if(GuildImpostoID[i] == 0)
			continue;

		if(pMob[GuildImpostoID[i]].MOB.CurrentScore.Hp <= 0)
			continue;

		strcpy(temp, "./npc/");

		char name[NAME_LENGTH];

		sprintf(name, "%s", pMob[GuildImpostoID[i]].MOB.MobName);

		for(int j = 0; j < NAME_LENGTH; j++)
		{
			if(name[j] == ' ')
				name[j] = '_';
		}

		strcat(temp, name);

		int handle = _open(temp, _O_CREAT | _O_RDWR | _O_BINARY, _S_IREAD | _S_IWRITE);
		if (handle == -1)
		{
			Log("-system", "fail - save npc imposto file", 0);
			return;
		}
		_write(handle, (void*)&pMob[GuildImpostoID[i]].MOB, sizeof(pMob[GuildImpostoID[i]].MOB));
		_close(handle);
	}


	if (ForceWeekMode != -1)
		timeinfo->tm_wday = -1;

	CCastleZakum::ProcessMinTimer();

#pragma region Clear Area
	if (Kingdom1Clear == 1)
	{
		Kingdom1Clear = 2;
	}
	else if (Kingdom1Clear == 2)
	{
		Kingdom1Clear = 0;

		ClearArea(1676, 1556, 1776, 1636);
	}

	if (Kingdom2Clear == 1)
	{
		Kingdom2Clear = 2;
	}
	else if (Kingdom2Clear == 2)
	{
		Kingdom2Clear = 0;

		ClearArea(1676, 1816, 1776, 1892);
	}
#pragma endregion

	if ((MinCounter % 10) == 0)
		memset(FailAccount, 0, sizeof(FailAccount));

	CurrentTime = timeGetTime();

#pragma region Close Gates
	for (int i = 17; i < g_dwInitItem; i++)
	{
		int ipx = g_pInitItem[i].PosX; // Item Pos X
		int ipy = g_pInitItem[i].PosY; // Item Pos Y
		int ipg = pItemGrid[ipy][ipx]; // Item Pos Grid

		if (pItem[ipg].ITEM.sIndex <= 0 || pItem[ipg].ITEM.sIndex >= MAX_ITEMLIST)
			continue;

		CItem *tItem = &pItem[ipg];

		int iKey = BASE_GetItemAbility(&pItem[ipg].ITEM, 39);

		if (iKey != 0)
		{
			if (pItem[ipg].State == STATE_OPEN && iKey < 15)
			{
				if (pItem[ipg].Delay == 0)
				{
					pItem[ipg].Delay = 1;

					continue;
				}

				UpdateItem(ipg, 3, &pItem[ipg].Height);
				/*
				MSG_UpdateItem sm;

				sm.ID = ESCENE_FIELD;
				sm.Type = _MSG_UpdateItem;

				sm.ItemID = ipg + 10000;

				sm.Size = sizeof(MSG_UpdateItem);

				//sm.Height = pItem[ipg].Height;
				*/
				pItem[ipg].State = STATE_LOCKED;
				/*
				sm.State = pItem[ipg].State;

				GridMulticast(ipx, ipy, (MSG_STANDARD*)&sm, 0);
				*/
				pItem[ipg].Delay = 0;

				MSG_CreateItem sm;
				memset(&sm, 0, sizeof(MSG_CreateItem)); // Height, Create e Owner iam sem valor

				sm.Type = _MSG_CreateItem;
				sm.Size = sizeof(MSG_CreateItem);
				sm.ID = ESCENE_FIELD;

				sm.ItemID = ipg + 10000;

				memcpy(&sm.Item, &pItem[ipg].ITEM, sizeof(STRUCT_ITEM));

				sm.GridX = ipx;
				sm.GridY = ipy;

				sm.Rotate = pItem[ipg].Rotate;
				sm.State = pItem[ipg].State;
				//sm.Create = Create;

				//sm.Height = pItem[empty].Height;

				GridMulticast(ipx, ipy, (MSG_STANDARD*)&sm, 0);
			}
		}
	}
#pragma endregion
#pragma region GenerateMobs
	for (int i = 0; i < mNPCGen.NumList; i++)
	{
		int MinuteGenerate = mNPCGen.pList[i].MinuteGenerate;

		if (i != 0 && i != 1 && i != 2 && i != 5 && i != 6 && i != 7 && i != -1)
		{
			if (MinuteGenerate <= 0)
				continue;
			

			int mod = i % MinuteGenerate;

			if (MinCounter % MinuteGenerate == mod)
			{
				GenerateMob(i, 0, 0);
				
				if (MinuteGenerate >= 500 && MinuteGenerate < 1000)
				{
					mNPCGen.pList[i].MinuteGenerate = W2Rand() % 500 + 500;

					continue;
				}

				if (MinuteGenerate >= 1000 && MinuteGenerate < 2000)
				{
					mNPCGen.pList[i].MinuteGenerate = W2Rand() % 1000 + 1000;

					continue;
				}

				if (MinuteGenerate >= 2000 && MinuteGenerate < 3800)
				{
					mNPCGen.pList[i].MinuteGenerate = W2Rand() % 1800 + 2000;

					continue;
				}

				if (MinuteGenerate >= 3800)
				{
					mNPCGen.pList[i].MinuteGenerate = W2Rand() % 1000 + 3800;

					if (DUNGEONEVENT)
					{
						int RndPos = W2Rand() % 30;
						int dpX = DungeonPos[RndPos][0];
						int dpY = DungeonPos[RndPos][1];

						int RndL = W2Rand() % 5 + 5;

						for (int j = 0; j < RndL; j++)
						{
							STRUCT_ITEM PrizeItem;

							memset(&PrizeItem, 0, sizeof(STRUCT_ITEM));

							PrizeItem.sIndex = DungeonItem[W2Rand() % 10];

							SetItemBonus(&PrizeItem, 0, 0, 0);

							int rnd = W2Rand() % 4;

							CreateItem(dpX, dpY, &PrizeItem, rnd, 1);
						}
					}
				}
				
			}
		}
	}
#pragma endregion
	MinCounter++;

	int rndWeather = W2Rand() % 1200;

	if (ForceWeather == -1)
	{
		if (rndWeather >= 0 && rndWeather < 260 && CurrentWeather != 0)
		{
			CurrentWeather = 0;
			SendWeather();
		}
		else if (rndWeather >= 30 && rndWeather < 50 && CurrentWeather != 1)
		{
			CurrentWeather = 1;
			SendWeather();
		}
		else if (rndWeather >= 55 && rndWeather < 60 && CurrentWeather != 2)
		{
			CurrentWeather = 2;
			SendWeather();
		}
	}
	else
	{
		if (ForceWeather != CurrentWeather)
		{
			CurrentWeather = ForceWeather;
			SendWeather();
		}
	}
 
	GuildProcess();
}
