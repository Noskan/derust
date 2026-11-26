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
#include "core/CReadFiles.h"
#include "systems/CWarTower.h"

/// @file ItemSpawnSystem.cpp
/// @brief Criacao, sorteio de adicionais, refinamento, decaimento e combinacao de itens no mundo.

/// Insere item diretamente no inventario do jogador notificando o cliente.
BOOL PutItem(int conn, STRUCT_ITEM *item)
{
	if (conn <= 0 || conn >= MAX_USER)
		return FALSE;

	if (pUser[conn].Mode != USER_PLAY)
		return FALSE;

	int pos = 0;

	for (pos = 0; pos < pMob[conn].MaxCarry && pos < MAX_CARRY; pos++)
	{
		if (pMob[conn].MOB.Carry[pos].sIndex == 0)
			break;
	}

	if (pos >= 0 && pos < pMob[conn].MaxCarry && pos < MAX_CARRY)
	{
		memcpy(&pMob[conn].MOB.Carry[pos], item, sizeof(STRUCT_ITEM));

		SendItem(conn, ITEM_PLACE_CARRY, pos, &pMob[conn].MOB.Carry[pos]);
	}
	else
	{
		SendClientMessage(conn, g_pMessageStringTable[_NN_You_Have_No_Space_To_Trade]);
		return FALSE;
	}
	return TRUE;
}

/// Sorteia adicionais aleatorios de drop (baseados em nivel e bonus) para um item.
void SetItemBonus(STRUCT_ITEM *Dest, int Level, int a3, int DropBonus)
{
	int addDropBonus = DropBonus / 8;
	
	if (DropBonus / 8 < 0)
		addDropBonus = 0;

	if (addDropBonus > 2)
		addDropBonus = 2;

	int add1 = -1;
	int add2 = -1;

	if (Dest->stEffect[0].cEffect >= EF_GRADE0 && Dest->stEffect[0].cEffect <= EF_GRADE5)
	{
		add1 = Dest->stEffect[0].cEffect - EF_GRADE0;
		add2 = Dest->stEffect[0].cValue;
   
		Dest->stEffect[0].cEffect = 0;
		Dest->stEffect[0].cValue = 0;
	}

	g_dLevel1 = 0;
	g_dLevel2 = 0;

	if(!a3 && Level >= 210)
		Level -= 47;

	int ItemId = Dest->sIndex;
	int nUnique = g_pItemList[ItemId].nUnique;
	int nReqLv = g_pItemList[ItemId].ReqLvl;
	int nPos = g_pItemList[ItemId].nPos;

	int lvdif = (Level - nReqLv + 1) / 25;

	if(add1 != -1)
		lvdif = add1;

	g_dLevel = lvdif;

	int pForc = 0;

	if ( lvdif >= 4 )
		pForc = 1;

	if ( lvdif < 0 )
		lvdif = 0;

	if ( lvdif > 3 )
		lvdif = 3;

	if ( a3 && lvdif >= 3 )
		lvdif = 2;

	if (nPos & 0xFE && !LOBYTE(Dest->stEffect[0].sValue) && nPos != 128 )
	{
    
	int v38 = 59;
    int v37 = 0;
    int v36 = 0;
    int v35 = rand() % 101;
    int v34 = 100;

    if ( a3 )
    {
      v34 = v35 % 3;
    }
    else
    {
      if ( lvdif )
      {
        if ( lvdif == 1 )
        {
          v34 = v35 % (6 - addDropBonus);
        }
        else
        {
          if ( lvdif == 2 )
          {
            v34 = v35 % (6 - addDropBonus);
          }
          else
          {
            if ( lvdif >= 3 )
              v34 = v35 % 4;
          }
        }
      }
      else
      {
        v34 = v35 % (8 - addDropBonus);
      }
    }
    switch ( nPos )
    {
      case 2:
        if ( v34 )
        {
          if ( v34 == 1 )
          {
            v38 = 60;
            v36 = 2;
          }
        }
        else
        {
          v38 = 26;
          v36 = 3;
        }
        break;
      case 4:
        v38 = 71;
        v36 = 10;
        v37 = 1;
        break;
      case 8:
        v38 = 71;
        v36 = 10;
        v37 = 1;
        break;
      case 16:
        v38 = 72;
        v36 = 5;
        break;
      case 32:
        v38 = 73;
        v36 = 6;
        v37 = -1;
        break;
      case 64:
      case 192:
        if ( nUnique != 44 && nUnique != 47 )
        {
          if ( v34 )
          {
            if ( v34 == 1 )
            {
              v38 = 2;
              v36 = 9;
              v37 = -1;
            }
            else
            {
              if ( v34 == 2 )
              {
                v38 = 74;
                v36 = 3;
              }
            }
          }
          else
          {
            v38 = 26;
            v36 = 3;
            v37 = 1;
          }
        }
        else
        {
          if ( v34 )
          {
            if ( v34 == 1 )
            {
              v38 = 74;
              v36 = 3;
              v37 = 1;
            }
          }
          else
          {
            v38 = 60;
            v36 = 4;
            v37 = -1;
          }
        }
        break;
    }
    int v33 = rand() % 100;
    if ( a3 )
      v33 = 2 * v33 / 3;
    int v32 = 100;
    if ( a3 )
    {
      v32 = v33 % 4;
    }
    else
    {
      if ( lvdif )
      {
        if ( lvdif == 1 )
        {
          v32 = v33 % 6;
        }
        else
        {
          if ( lvdif == 2 )
          {
            v32 = v33 % 6;
          }
          else
          {
            if ( lvdif >= 3 )
              v32 = v33 % 4;
          }
        }
      }
      else
      {
        v32 = v33 % 8;
      }
    }
    int v31 = 59;
    int v30 = 0;
    int v29 = 0;
    switch ( nPos )
    {
      case 2:
        if ( v32 )
        {
          if ( v32 == 1 )
          {
            v31 = 3;
            v30 = 5;
            v29 = -1;
          }
        }
        else
        {
          v31 = 4;
          v30 = 10;
        }
        break;
      case 4:
        if ( v32 )
        {
          if ( v32 == 1 )
          {
            v31 = 2;
            v30 = 6;
            v29 = -1;
          }
          else
          {
            if ( v32 == 2 )
            {
              v31 = 3;
              v30 = 5;
              v29 = -1;
            }
          }
        }
        else
        {
          v31 = 60;
          v30 = 2;
          v29 = -1;
        }
        break;
      case 8:
        if ( v32 )
        {
          if ( v32 == 1 )
          {
            v31 = 2;
            v30 = 6;
            v29 = -1;
          }
          else
          {
            if ( v32 == 2 )
            {
              v31 = 3;
              v30 = 5;
              v29 = -1;
            }
          }
        }
        else
        {
          v31 = 60;
          v30 = 2;
          v29 = -1;
        }
        break;
      case 16:
        if ( v32 )
        {
          switch ( v32 )
          {
            case 1:
              v31 = 2;
              v30 = 6;
              v29 = -1;
              break;
            case 2:
              v31 = 74;
              v30 = 3;
              break;
            case 3:
              v31 = 54;
              v30 = 3;
              break;
          }
        }
        else
        {
          v31 = 60;
          v30 = 2;
          v29 = -1;
        }
        break;
      case 32:
        if ( v32 )
        {
          if ( v32 == 1 )
          {
            v31 = 74;
            v30 = 3;
          }
        }
        else
        {
          v31 = 60;
          v30 = 2;
          v29 = -1;
        }
        break;
      case 64:
      case 128:
      case 192:
        if ( nUnique != 44 && nUnique != 47 )
        {
          if ( v32 )
          {
            if ( v32 == 1 )
            {
              v31 = 2;
              v30 = 9;
              v29 = -1;
            }
            else
            {
              if ( v32 == 2 )
              {
                v31 = 74;
                v30 = 3;
                v29 = 1;
              }
            }
          }
          else
          {
            v31 = 26;
            v30 = 3;
            v29 = 1;
          }
        }
        else
        {
          if ( v32 )
          {
            if ( v32 == 1 )
            {
              v31 = 74;
              v30 = 3;
              v29 = 1;
            }
          }
          else
          {
            v31 = 60;
            v30 = 4;
            v29 = -1;
          }
        }
        break;
    }
    int v28 = 0;
    v35 = rand() % 100;
    if ( lvdif )
    {
      switch ( lvdif )
      {
        case 1:
          if ( v35 >= 1 )
          {
            if ( v35 >= 5 )
            {
              if ( v35 >= 24 )
              {
                if ( v35 >= 65 )
                  v28 = 1;
                else
                  v28 = 2;
              }
              else
              {
                v28 = 3;
              }
            }
            else
            {
              v28 = 4;
            }
          }
          else
          {
            v28 = 5;
          }
          break;
        case 2:
          if ( v35 >= 2 )
          {
            if ( v35 >= 16 )
            {
              if ( v35 >= 60 )
                v28 = 2;
              else
                v28 = 3;
            }
            else
            {
              v28 = 4;
            }
          }
          else
          {
            v28 = 5;
          }
          break;
        case 3:
          if ( v35 >= 2 )
          {
            if ( v35 >= 9 )
            {
              if ( v35 >= 45 )
              {
                if ( v35 >= 75 )
                  v28 = 2;
                else
                  v28 = 3;
              }
              else
              {
                v28 = 4;
              }
            }
            else
            {
              v28 = 5;
            }
          }
          else
          {
            v28 = 6;
          }
          break;
        default:
          if ( lvdif >= 4 )
          {
            if ( v35 >= 2 )
            {
              if ( v35 >= 10 )
              {
                if ( v35 >= 30 )
                {
                  if ( v35 >= 75 )
                    v28 = 3;
                  else
                    v28 = 4;
                }
                else
                {
                  v28 = 5;
                }
              }
              else
              {
                v28 = 6;
              }
            }
            else
            {
              v28 = 7;
            }
          }
          break;
      }
    }
    else
    {
      if ( v35 >= 2 )
      {
        if ( v35 >= 6 )
        {
          if ( v35 >= 24 )
            v28 = v35 < 55;
          else
            v28 = 2;
        }
        else
        {
          v28 = 3;
        }
      }
      else
      {
        v28 = 4;
      }
    }
    if ( pForc && v28 < 4 )
      v28 = 4;
    v28 += v37;
    if ( a3 && !v28 )
      v28 = 1;
    if ( LOBYTE(Dest->stEffect[1].sValue) || v28 <= 0 )
    {
      if ( LOBYTE(Dest->stEffect[1].sValue) || v28 > 0 || nPos != 32 )
      {
        if ( !LOBYTE(Dest->stEffect[1].sValue) )
        {
			Dest->stEffect[1].cEffect = 59;
          int v4 = rand() & 0x8000007F;
          if ( v4 < 0 )
            v4 = ((unsigned __int8)(v4 - 1) | 0x80) + 1;
		  Dest->stEffect[1].cValue = v4;
        }
      }
      else
      {
		  Dest->stEffect[1].cEffect = v38;
		  Dest->stEffect[1].cValue = 0;
      }
    }
    else
    {
		Dest->stEffect[1].cEffect = v38;
		Dest->stEffect[1].cValue = v36 * v28;
    }
    g_dLevel1 = v28;
    int v27 = 0;
    v33 = rand() % 100;
    if ( lvdif )
    {
      switch ( lvdif )
      {
        case 1:
          if ( v33 >= 1 )
          {
            if ( v33 >= 5 )
            {
              if ( v33 >= 24 )
              {
                if ( v33 >= 65 )
                  v27 = 1;
                else
                  v27 = 2;
              }
              else
              {
                v27 = 3;
              }
            }
            else
            {
              v27 = 4;
            }
          }
          else
          {
            v27 = 5;
          }
          break;
        case 2:
          if ( v33 >= 2 )
          {
            if ( v33 >= 16 )
            {
              if ( v33 >= 60 )
                v27 = 2;
              else
                v27 = 3;
            }
            else
            {
              v27 = 4;
            }
          }
          else
          {
            v27 = 5;
          }
          break;
        case 3:
          if ( v33 >= 2 )
          {
            if ( v33 >= 9 )
            {
              if ( v33 >= 45 )
              {
                if ( v33 >= 75 )
                  v27 = 2;
                else
                  v27 = 3;
              }
              else
              {
                v27 = 4;
              }
            }
            else
            {
              v27 = 5;
            }
          }
          else
          {
            v27 = 6;
          }
          break;
        default:
          if ( lvdif >= 4 )
          {
            if ( v33 >= 2 )
            {
              if ( v33 >= 10 )
              {
                if ( v33 >= 30 )
                {
                  if ( v33 >= 75 )
                    v27 = 3;
                  else
                    v27 = 4;
                }
                else
                {
                  v27 = 5;
                }
              }
              else
              {
                v27 = 6;
              }
            }
            else
            {
              v27 = 7;
            }
          }
          break;
      }
    }
    else
    {
      if ( v35 >= 2 )
      {
        if ( v33 >= 6 )
        {
          if ( v33 >= 24 )
            v27 = v33 < 55;
          else
            v27 = 2;
        }
        else
        {
          v27 = 3;
        }
      }
      else
      {
        v28 = 4;
      }
    }
    if ( pForc && v27 < 3 )
      v27 = 3;
    if ( a3 && v27 >= 5 )
      v27 = 4;
    v27 += v29;
    if ( addDropBonus && !v27 )
      v27 = addDropBonus;
    if ( a3 && !v27 )
      v27 = 1;
    if ( v27 <= 0 || LOBYTE(Dest->stEffect[2].sValue) )
    {
      if ( !LOBYTE(Dest->stEffect[2].sValue) )
      {
        Dest->stEffect[2].cEffect = 59;
        int v5 = rand() & 0x8000007F;
        if ( v5 < 0 )
          v5 = ((unsigned __int8)(v5 - 1) | 0x80) + 1;
		Dest->stEffect[2].cValue = v5;
      }
    }
    else
    {
		Dest->stEffect[2].cEffect = v31;
		Dest->stEffect[2].cValue = v30 * v27;
    }
    g_dLevel2 = v27;

    if ( !LOBYTE(Dest->stEffect[0].sValue) )
    {
      int v26 = rand() % 100;
      if ( a3 )
        v26 /= 2;
      int v25 = 1;
      int v24 = 12;
      int v23 = 45;
      int v22 = 70;
      if ( lvdif >= 3 )
      {
        v25 = 6;
        v24 = 35;
        v23 = 85;
        v22 = 100;
      }
      if ( lvdif == 2 )
      {
        v25 = 6;
        v24 = 35;
        v23 = 85;
        v22 = 100;
      }
      if ( lvdif == 1 )
      {
        v25 = 6;
        v24 = 22;
        v23 = 75;
        v22 = 90;
      }
      if ( !lvdif )
      {
        v25 = 6;
        v24 = 22;
        v23 = 75;
        v22 = 90;
      }
      if ( v26 >= v25 )
      {
        if ( v26 >= v24 )
        {
          if ( v26 >= v23 )
          {
            if ( v26 >= v22 )
            {
				Dest->stEffect[0].cEffect = 59;
              int v6 = rand() & 0x8000007F;
              if ( v6 < 0 )
                v6 = ((unsigned __int8)(v6 - 1) | 0x80) + 1;
			  Dest->stEffect[0].cValue = v6;
            }
            else
            {
              int v20 = rand() % 10;
              int v19 = g_pBonusType[v20];
              int v18 = lvdif;
              int v17 = g_pBonusValue[v20][lvdif][0];
              int v16 = g_pBonusValue[v20][lvdif][1];
              int v15 = v16 + 1 - v17;
              int v14 = rand() % v15 + v17;
              Dest->stEffect[0].cEffect = v19;
              Dest->stEffect[0].cValue = v14;
            }
          }
          else
          {
            Dest->stEffect[0].cEffect = 43;
            Dest->stEffect[0].cValue = 0;
          }
        }
        else
        {
          Dest->stEffect[0].cEffect = 43;
          Dest->stEffect[0].cValue = 1;
        }
      }
      else
      {
        Dest->stEffect[0].cEffect = 43;
        Dest->stEffect[0].cValue = 2;
        if ( add2 > 2 )
        {
          int v21 = rand() % 100;
          switch ( add2 )
          {
            case 3:
              if ( v21 < 30 )
                Dest->stEffect[0].cValue = 3;
              break;
            case 4:
              if ( v21 >= 10 )
              {
                if ( v21 < 40 )
                  Dest->stEffect[0].cValue = 3;
              }
              else
              {
                Dest->stEffect[0].cValue = 4;
              }
              break;
            case 5:
              if ( v21 >= 10 )
              {
                if ( v21 >= 30 )
                {
                  if ( v21 < 60 )
                    Dest->stEffect[0].cValue = 3;
                }
                else
                {
                  Dest->stEffect[0].cValue = 4;
                }
              }
              else
              {
                Dest->stEffect[0].cValue = 5;
              }
              break;
            case 6:
              if ( v21 >= 10 )
              {
                if ( v21 >= 20 )
                {
                  if ( v21 >= 40 )
                  {
                    if ( v21 < 60 )
                      Dest->stEffect[0].cValue = 3;
                  }
                  else
                  {
                    Dest->stEffect[0].cValue = 4;
                  }
                }
                else
                {
                  Dest->stEffect[0].cValue = 5;
                }
              }
              else
              {
                Dest->stEffect[0].cValue = 6;
              }
              break;
            case 7:
              if ( v21 >= 4 )
              {
                if ( v21 >= 10 )
                {
                  if ( v21 >= 20 )
                  {
                    if ( v21 >= 35 )
                    {
                      if ( v21 < 60 )
                        Dest->stEffect[0].cValue = 3;
                    }
                    else
                    {
                      Dest->stEffect[0].cValue = 4;
                    }
                  }
                  else
                  {
                    Dest->stEffect[0].cValue = 5;
                  }
                }
                else
                {
                  Dest->stEffect[0].cValue = 6;
                }
              }
              else
              {
                Dest->stEffect[0].cValue = 7;
              }
              break;
          }
        }
      }
    }
  }

  for (int i = 0; i < 12; ++i )
  {
    int v12 = g_pItemList[ItemId].ReqLvl;
    switch ( g_pItemList[ItemId].stEffect[i].sEffect )
    {
      case 0x2B:
        Dest->stEffect[0].cEffect = 43;
		Dest->stEffect[0].cValue = (unsigned char)g_pItemList[ItemId].stEffect[i].sValue;
        break;
      case 0x3D:
        Dest->stEffect[0].cEffect = 61;
        Dest->stEffect[0].cValue = LOBYTE(g_pItemList[ItemId].stEffect[i].sValue);
        break;
      case 0x4E:
        Dest->stEffect[0].cEffect = 78;
        int v11 = rand() % 4 + g_pItemList[ItemId].stEffect[i].sValue;
        if ( v11 > 9 )
          v11 = 9;
        Dest->stEffect[0].cValue = v11;
        break;
    }
  }
  if ( Dest->sIndex == 412 || Dest->sIndex == 413 || Dest->sIndex == 419 || Dest->sIndex == 420 || Dest->sIndex == 753 )
  {
    if ( !Dest->stEffect[0].cEffect )
    {
      Dest->stEffect[0].cEffect = 59;
      Dest->stEffect[0].cValue = rand();
    }
    if ( !Dest->stEffect[1].cEffect)
    {
      Dest->stEffect[1].cEffect = 59;
      Dest->stEffect[01].cValue = rand();
    }
    if ( !Dest->stEffect[2].cEffect)
    {
      Dest->stEffect[2].cEffect = 59;
      Dest->stEffect[2].cValue = rand();
    }
  }
  if ( Dest->sIndex >= 447 && Dest->sIndex <= 450
    || Dest->sIndex >= 692 && Dest->sIndex <= 695)
  {
    if ( !Dest->stEffect[0].cEffect )
    {
      Dest->stEffect[0].cEffect = 59;
      Dest->stEffect[0].cValue = rand();
    }
    if ( !Dest->stEffect[1].cEffect)
    {
      Dest->stEffect[1].cEffect = 59;
      Dest->stEffect[01].cValue = rand();
    }
    if ( !Dest->stEffect[2].cEffect)
    {
      Dest->stEffect[2].cEffect = 59;
      Dest->stEffect[2].cValue = rand()%256;
    }
  }
}

/// Aplica adicionais secundarios/especiais ao item.
void SetItemBonus2(STRUCT_ITEM *Dest)
{
	int nPos = g_pItemList[Dest->sIndex].nPos;

	if(nPos == 2)//Elmo
	{
		int _rand = rand()%25;

		if(Dest->stEffect[0].cEffect == EF_SANC)
		{
			int sanc = BASE_GetItemSanc(Dest);

			if (sanc < 6)
			{
				sanc += rand() % 2;

				if (sanc >= 6)
					sanc = 6;

				BASE_SetItemSanc(Dest, sanc, 0);
			}
		}
		else 
		{
			Dest->stEffect[0].cEffect = EF_SANC;
			Dest->stEffect[0].cValue = rand()%2;
		}
		Dest->stEffect[1].cEffect = g_pBonusValue3[_rand][0];
		Dest->stEffect[1].cValue = g_pBonusValue3[_rand][1];

		Dest->stEffect[2].cEffect = g_pBonusValue3[_rand][2];
		Dest->stEffect[2].cValue = g_pBonusValue3[_rand][3];
	}
	//Peito calça
	if(nPos == 4 || nPos == 8)
	{
		int _rand = rand()%48;

		if(Dest->stEffect[0].cEffect == EF_SANC)
		{
			int sanc = BASE_GetItemSanc(Dest);

			if (sanc < 6)
			{
				sanc += rand() % 2;

				if (sanc >= 6)
					sanc = 6;

				BASE_SetItemSanc(Dest, sanc, 0);
			}
		}
		else 
		{
			Dest->stEffect[0].cEffect = EF_SANC;
			Dest->stEffect[0].cValue = rand()%2;
		}

		Dest->stEffect[1].cEffect = g_pBonusValue2[_rand][0];
		Dest->stEffect[1].cValue = g_pBonusValue2[_rand][1];

		Dest->stEffect[2].cEffect = g_pBonusValue2[_rand][2];
		Dest->stEffect[2].cValue = g_pBonusValue2[_rand][3];
	}

	//Luva
	if(nPos == 16)
	{
		int _rand = rand()%30;

		if(Dest->stEffect[0].cEffect == EF_SANC)
		{
			int sanc = BASE_GetItemSanc(Dest);

			if (sanc < 6)
			{
				sanc += rand() % 2;

				if (sanc >= 6)
					sanc = 6;

				BASE_SetItemSanc(Dest, sanc, 0);
			}
		}
		else 
		{
			Dest->stEffect[0].cEffect = EF_SANC;
			Dest->stEffect[0].cValue = rand()%2;
		}

		Dest->stEffect[1].cEffect = g_pBonusValue4[_rand][0];
		Dest->stEffect[1].cValue = g_pBonusValue4[_rand][1];

		Dest->stEffect[2].cEffect = g_pBonusValue4[_rand][2];
		Dest->stEffect[2].cValue = g_pBonusValue4[_rand][3];
	}

	//Bota
	if(nPos == 32)
	{
		int _rand = rand()%30;

		if(Dest->stEffect[0].cEffect == EF_SANC)
		{
			int sanc = BASE_GetItemSanc(Dest);

			if (sanc < 6)
			{
				sanc += rand() % 2;

				if (sanc >= 6)
					sanc = 6;

				BASE_SetItemSanc(Dest, sanc, 0);
			}
		}
		else 
		{
			Dest->stEffect[0].cEffect = EF_SANC;
			Dest->stEffect[0].cValue = rand()%2;
		}

		Dest->stEffect[1].cEffect = g_pBonusValue5[_rand][0];
		Dest->stEffect[1].cValue = g_pBonusValue5[_rand][1];

		Dest->stEffect[2].cEffect = g_pBonusValue5[_rand][2];
		Dest->stEffect[2].cValue = g_pBonusValue5[_rand][3];

		if(Dest->stEffect[1].cEffect == EF_DAMAGE && BASE_GetItemAbility(Dest, EF_DAMAGE) > 30)
		{
			Dest->stEffect[1].cValue -= Dest->stEffect[1].cValue < (BASE_GetItemAbility(Dest, EF_DAMAGE)-30) ? Dest->stEffect[1].cValue : (BASE_GetItemAbility(Dest, EF_DAMAGE)-30);
			if(Dest->stEffect[1].cValue < 0)
				Dest->stEffect[1].cValue = 0;
		}

		if(Dest->stEffect[2].cEffect == EF_DAMAGE && BASE_GetItemAbility(Dest, EF_DAMAGE) > 30)
		{
			Dest->stEffect[2].cValue -= Dest->stEffect[2].cValue < (BASE_GetItemAbility(Dest, EF_DAMAGE)-30) ? Dest->stEffect[2].cValue : (BASE_GetItemAbility(Dest, EF_DAMAGE)-30);
			if(Dest->stEffect[2].cValue < 0)
				Dest->stEffect[2].cValue = 0;
		}
	}
}

/// Abre arquivo de relatorio/dump de itens escrevendo cabecalho.
FILE*  WriteHeader(char *fileName)
{
	FILE *fp = NULL;

	fp = fopen(fileName, "wt");

	fprintf(fp, "<script>\n");
	fprintf(fp, "function f_show(idx) \n");
	fprintf(fp, "{\n");
	fprintf(fp, "opener.document.frmRegist.item_code.value=idx;\n");
	fprintf(fp, "self.close();\n");
	fprintf(fp, "}\n");
	fprintf(fp, "</script>\n");
	fprintf(fp, "<style><!--\n");
	fprintf(fp, "A:link {color:#111111;font-size:9pt;text-decoration:none}\n");
	fprintf(fp, "A:visited {color:#333333;font-size:9pt;text-decoration:none}\n");
	fprintf(fp, "A:active {color:red;font-size:9pt}\n");
	fprintf(fp, "A:hover {text-color:red;text-decoration:underline}\n");
	fprintf(fp, "BODY,TD,TH {font-size: 9pt}\n");
	fprintf(fp, "--></style>\n");

	return fp;
}

/// Exporta tabela de armaduras para arquivo.
void  WriteArmor(FILE *fp)
{
}

/// Exporta tabela de armas para arquivo.
void WriteWeapon(FILE *fp)
{
	fprintf(fp, "<body leftmargin=0 topmargin=0 marginwidth=0 marginheight=0><table>\n");

	for (int i = 0; i < 32; i++)
	{
		fprintf(fp, "<tr  height=10>");

		for (int j = 0; j < 9; j++)
		{
			char dest[256];

			int item = weapon[i][j];

			if (j == 0)
				strcpy(dest, "#F6EED9");
			if (j == 1)
				strcpy(dest, "#DFDFDF");
			if (j == 2)
				strcpy(dest, "#D9EAF6");
			if (j == 3)
				strcpy(dest, "#DFE9D0");
			if (j == 4)
				strcpy(dest, "#F6D9E0");
			if (j == 5)
				strcpy(dest, "#F6EED9");
			if (j == 6)
				strcpy(dest, "#DFDFDF");
			if (j == 7)
				strcpy(dest, "#D9EAF6");
			if (j == 8)
				strcpy(dest, "#DFE9D0");

			int reqlv = g_pItemList[item].ReqLvl;
			int reqstatus = g_pItemList[item].ReqCon + g_pItemList[item].ReqDex + g_pItemList[item].ReqInt + g_pItemList[item].ReqStr;

			if (reqlv || reqstatus <= 75)
			{
				if (reqlv || reqstatus <= 20)
					strcpy(temp, "#000000");
				else
					strcpy(temp, "#00BF00");
			}
			else
				strcpy(temp, "#FF00FF");

			fprintf(fp, "<td width=140 height=16 bgcolor='%s'><a href=\"javascript:f_show(%d); title='%d'><font color='%s'>%16.16s</font></a></td>",
				&dest, item, item, &temp, &g_pItemList[item].Name);
		}
		fprintf(fp, "</tr>\n");
	}
	fprintf(fp, "</table></body>\n");
}

/// Cria um item caido no chao na posicao (x, y) respeitando o grid de itens.
int  CreateItem(int x, int y, STRUCT_ITEM *item, int rotate, int Create)
{
	if (item->sIndex <= 0 || item->sIndex >= MAX_ITEMLIST)
		return FALSE;

	GetEmptyItemGrid(&x, &y);

//	if (pHeightGrid[y][x] >= -15 || pHeightGrid[y][x] >= 90)
//		return FALSE;

	if (pItemGrid[y][x])
		return FALSE;

	int empty = GetEmptyItem();

	if (empty == 0)
		return FALSE;

	pItem[empty].Mode = 1;

	pItem[empty].PosX = x;
	pItem[empty].PosY = y;

	memcpy(&pItem[empty].ITEM, item, sizeof(STRUCT_ITEM));

	pItem[empty].Rotate = rotate;
	pItem[empty].State = STATE_OPEN;

	pItem[empty].Delay = 90;
	pItem[empty].Decay = 0;

	pItem[empty].GridCharge = BASE_GetItemAbility(item, EF_GROUND);

	pItemGrid[y][x] = empty;

	pItem[empty].Height = pHeightGrid[y][x];

	MSG_CreateItem sm;
	memset(&sm, 0, sizeof(MSG_CreateItem));

	sm.Type = _MSG_CreateItem;
	sm.Size = sizeof(MSG_CreateItem);
	sm.ID = ESCENE_FIELD;

	sm.ItemID = empty + 10000;

	memcpy(&sm.Item, &pItem[empty].ITEM, sizeof(STRUCT_ITEM));

	sm.GridX = x;
	sm.GridY = y;

	sm.Rotate = rotate;
	sm.State = pItem[empty].State;
//	sm.Create = Create;

//	sm.Height = pItem[empty].Height;

	if (Create == 2)//Bau
		sm.Height = -204;

	GridMulticast(x, y, (MSG_STANDARD*)&sm, 0);

	return TRUE;
}

/// Atualiza estado/altura de objetos interativos (portoes) no mapa.
int UpdateItem(int Gate, int state, int *height)
{
	int rotate = pItem[Gate].Rotate;

	int update = BASE_UpdateItem(pItem[Gate].GridCharge, pItem[Gate].State, state, pItem[Gate].PosX, pItem[Gate].PosY, (char*)&pHeightGrid, rotate, height);

	if (update == 0)
		return FALSE;

	pItem[Gate].State = state;
	pItem[Gate].Height = *height;

	int GridCharge = pItem[Gate].GridCharge;
	int itemstate = pItem[Gate].State;

	pItem[Gate].Delay = 0;

	if (GridCharge >= 6 || GridCharge < 0)
	{
		Log("err,updateItem maskidx illegal", "-system", 0);
		return TRUE;
	}

	if (itemstate == state)
	{
		int keyid = BASE_GetItemAbility(&pItem[Gate].ITEM, EF_KEYID);

		if (keyid == 15 && itemstate == 1 && state == 3)
			CreateMob("GATE", pItem[Gate].PosX, pItem[Gate].PosY, "npc", 0);

		pItem[Gate].State = state;

		return TRUE;
	}

	int posX = pItem[Gate].PosX;
	int posY = pItem[Gate].PosY;

	for (int i = 0; i <= MAX_GROUNDMASK - 2; i++)
	{
		for (int j = 0; j <= MAX_GROUNDMASK - 2; j++)
		{
			int val = g_pGroundMask[GridCharge][rotate][i][j];

			int iX = posX + j - 2;
			int jY = posY + i - 2;

			if (iX >= 1 && jY >= 1 && iX <= 4094 && jY <= 4094 && val)
			{
				int tmob = pMobGrid[jY][iX];

				if (tmob != 0)
				{
					if (pMob[tmob].MOB.Equip[0].sIndex == 220)
						DeleteMob(tmob, 3);

					else
					{
						int mobgrid = GetEmptyMobGrid(tmob, &iX, &jY);

						if (mobgrid != 0)
						{
							pMob[tmob].Route[0] = 0;

							MSG_Action sm;
							memset(&sm, 0, sizeof(MSG_Action));

							GetAction(tmob, iX, jY, &sm);

							int unk = 20;
							int unk2 = 0;

							GridMulticast(iX, jY, (MSG_STANDARD*)&sm, 0);

							if (tmob < MAX_USER)
								pUser[tmob].cSock.SendOneMessage((char*)&sm, sizeof(sm));
						}
					}
				}
			}
		}
	}

	return TRUE;
}

/// Remove itens caidos no chao cujo tempo de expiracao venceu.
void ProcessDecayItem()
{
	for (int i = 0; i < ItemCount; i++)
	{
		ItemCount++;

		if (ItemCount >= 5000)
			ItemCount = g_dwInitItem + 1;

		if (pItem[ItemCount].ITEM.sIndex == 1727)
			continue;

		if (pItem[ItemCount].Mode == 0 || pItem[ItemCount].Mode == 2)
			continue;

		if (pItem[ItemCount].Delay >= 1)
		{
			pItem[ItemCount].Delay--;
			continue;
		}

		if (pItem[ItemCount].Decay != -1)
		{
			int itemPosX = pItem[ItemCount].PosX;
			int itemPosY = pItem[ItemCount].PosY;

			BASE_ClearItem(&pItem[ItemCount].ITEM);

			pItemGrid[itemPosX][itemPosY] = 0;
			pItem[ItemCount].Mode = 0;

			MSG_DecayItem sm;
			memset(&sm, 0, sizeof(MSG_DecayItem));

			sm.Type = _MSG_DecayItem;
			sm.Size = sizeof(MSG_DecayItem);
			sm.ID = ESCENE_FIELD;

			sm.ItemID = ItemCount + 10000;

			GridMulticast(itemPosX, itemPosY, (MSG_STANDARD*)&sm, 0);
		}
	}
}

/// Valida e combina fragmentos de mapa do tesouro.
int  CombineTreasureMap(int conn)
{
	for (int i = 0; i < MAX_CARRY; i++)
	{
		if (pMob[conn].MOB.Carry[i].sIndex != 788)
			continue;

		if (i + 1 >= MAX_CARRY)
			continue;

		if (pMob[conn].MOB.Carry[i + 1].sIndex != 789)
			continue;

		if (i + 9 >= MAX_CARRY)
			continue;

		if (pMob[conn].MOB.Carry[i + 9].sIndex != 790)
			continue;

		if (i + 10 >= MAX_CARRY)
			continue;

		if (pMob[conn].MOB.Carry[i + 10].sIndex != 791)
			continue;

		if (i + 18 >= MAX_CARRY)
			continue;

		if (pMob[conn].MOB.Carry[i + 18].sIndex != 792)
			continue;

		if (i + 19 >= MAX_CARRY)
			continue;

		if (pMob[conn].MOB.Carry[i + 19].sIndex != 793)
			continue;

		MSG_MessageDBRecord sm;
		memset(&sm, 0, sizeof(MSG_MessageDBRecord));

		sm.Type = _MSG_MessageDBRecord;
		sm.ID = 0;

		sm.Size = sizeof(MSG_MessageDBRecord);

		sprintf(sm.Record, "ETC treasure_map %d %s %s", ServerIndex, pUser[conn].AccountName, pMob[conn].MOB.MobName);

		DBServerSocket.SendOneMessage((char*)&sm, sizeof(MSG_MessageDBRecord));

		if (i >= MAX_CARRY)
			continue;

		pMob[conn].MOB.Carry[i].sIndex = 0;
		SendItem(conn, ITEM_PLACE_CARRY, i, &pMob[conn].MOB.Carry[i]);

		if (i + 1 >= MAX_CARRY)
			continue;

		pMob[conn].MOB.Carry[i + 1].sIndex = 0;
		SendItem(conn, ITEM_PLACE_CARRY, i + 1, &pMob[conn].MOB.Carry[i + 1]);

		if (i + 9 >= MAX_CARRY)
			continue;

		pMob[conn].MOB.Carry[i + 9].sIndex = 0;
		SendItem(conn, ITEM_PLACE_CARRY, i + 9, &pMob[conn].MOB.Carry[i + 9]);

		if (i + 10 >= MAX_CARRY)
			continue;

		pMob[conn].MOB.Carry[i + 10].sIndex = 0;
		SendItem(conn, ITEM_PLACE_CARRY, i + 10, &pMob[conn].MOB.Carry[i + 10]);

		if (i + 18 >= MAX_CARRY)
			continue;

		pMob[conn].MOB.Carry[i + 18].sIndex = 0;
		SendItem(conn, ITEM_PLACE_CARRY, i + 18, &pMob[conn].MOB.Carry[i + 18]);

		if (i + 19 >= MAX_CARRY)
			continue;

		pMob[conn].MOB.Carry[i + 19].sIndex = 0;
		SendItem(conn, ITEM_PLACE_CARRY, i + 19, &pMob[conn].MOB.Carry[i + 19]);

		SendClientMessage(conn, g_pMessageStringTable[_NN_Treasure_Map]);

		return TRUE;
	}

	int itemCount = 0;

	for (int i = 0; i < MAX_CARRY; i++)
	{
		if (pMob[conn].MOB.Carry[i].sIndex >= 788 && pMob[conn].MOB.Carry[i].sIndex <= 793)
		{
			pMob[conn].MOB.Carry[i].sIndex = 485;

			SendItem(conn, ITEM_PLACE_CARRY, i, &pMob[conn].MOB.Carry[i]);

			itemCount++;
		}
	}

	if (itemCount <= 0)
		return 0;

	SendClientMessage(conn, g_pMessageStringTable[_NN_Congratulations]);
	return TRUE;
}

/// Executa combinacao simples de item para geracao de recompensa.
void Combine(int conn, int item, int PrizeItem)
{
	int founditem = 0;

	if (conn < 0 || conn >= MAX_USER)
		return;

	for (int i = 0; i < MAX_CARRY; i++)
	{
		int index = pMob[conn].MOB.Carry[i].sIndex;

		if (index == item)
			founditem += pMob[conn].MOB.Carry[i].stEffect[0].cEffect == 61 ? pMob[conn].MOB.Carry[i].stEffect[0].cValue : 1;

		if (founditem >= 10)
			break;
	}

	if (founditem < 10)
		return;

	int clearitem = 0;

	for (int i = 0; i < MAX_CARRY; i++)
	{
		int index = pMob[conn].MOB.Carry[i].sIndex;

		if (index != item)
			continue;

		if (clearitem >= founditem)
			break;

		clearitem += pMob[conn].MOB.Carry[i].stEffect[0].cEffect == 61 ? pMob[conn].MOB.Carry[i].stEffect[0].cValue : 1;

		if (pMob[conn].MOB.Carry[i].stEffect[0].cEffect == 61 && pMob[conn].MOB.Carry[i].stEffect[0].cValue > 10)
			pMob[conn].MOB.Carry[i].stEffect[0].cValue -= 10;

		else
			BASE_ClearItem(&pMob[conn].MOB.Carry[i]);


		if (clearitem < 10)
			continue;
	}

	STRUCT_ITEM Prize;

	Prize.sIndex = PrizeItem;

	Prize.stEffect[0].cEffect = 59;
	Prize.stEffect[0].cValue = rand() % 100;

	Prize.stEffect[1].cEffect = 59;
	Prize.stEffect[1].cValue = rand() % 100;

	Prize.stEffect[2].cEffect = 59;
	Prize.stEffect[2].cValue = rand() % 50 + 50;

	int i = 0;

	for (i = 0; i < MAX_CARRY; i++)
	{
		int error = -1;

		//if (BASE_CanCarry(&Prize, pMob[conn].MOB.Carry, i % 9, i / 9, &error) == 1)
		//	break;

		if (pMob[conn].MOB.Carry[i].sIndex == 0)
			break;
	}

	if (i >= 0 && i < MAX_CARRY)
	{
		*(int*)&pMob[conn].MOB.Carry[i] = *(int*)&Prize;
		*(int*)((int)&pMob[conn].MOB.Carry[i] + 4) = *(int*)((int)&Prize + 4);
	}

	else
		CreateItem(pMob[conn].TargetX, pMob[conn].TargetY, &Prize, rand() % 4, 1);
}

/// Evolui/ajusta nivel de item conforme tabela configurada.
void DoItemLevel(int conn)
{
	int type = 0;

	if(pMob[conn].MOB.BaseScore.Str > pMob[conn].MOB.BaseScore.Int && pMob[conn].MOB.BaseScore.Str > pMob[conn].MOB.BaseScore.Dex && pMob[conn].MOB.BaseScore.Str > pMob[conn].MOB.BaseScore.Con)
		type = 0;
			
	else if(pMob[conn].MOB.BaseScore.Int > pMob[conn].MOB.BaseScore.Str && pMob[conn].MOB.BaseScore.Int > pMob[conn].MOB.BaseScore.Dex && pMob[conn].MOB.BaseScore.Int > pMob[conn].MOB.BaseScore.Con)
		type = 1;

	else if(pMob[conn].MOB.BaseScore.Dex > pMob[conn].MOB.BaseScore.Str && pMob[conn].MOB.BaseScore.Dex > pMob[conn].MOB.BaseScore.Int && pMob[conn].MOB.BaseScore.Dex > pMob[conn].MOB.BaseScore.Con)
		type = 2;

	else
		type = 3;

	if(LevelItem[pMob[conn].MOB.Class][type][pMob[conn].MOB.BaseScore.Level].sIndex != 0)
	{
		for (int i = 0; i < MAX_CARGO - 2; i++)
		{
			int retsour = pUser[conn].Cargo[i].sIndex;

			if (retsour != 0)
				continue;

			pUser[conn].Cargo[i] = LevelItem[pMob[conn].MOB.Class][type][pMob[conn].MOB.BaseScore.Level];

			SendItem(conn, ITEM_PLACE_CARGO, i, &LevelItem[pMob[conn].MOB.Class][type][pMob[conn].MOB.BaseScore.Level]);

			char cMsg[128];

			sprintf(cMsg, "%s [%s]", g_pMessageStringTable[_NN_Item_Arrived], g_pItemList[LevelItem[pMob[conn].MOB.Class][type][pMob[conn].MOB.BaseScore.Level].sIndex].Name);

			SendClientMessage(conn, cMsg);

			char ItemCode[256];

			sprintf(temp, "tra,%s %d", pUser[conn].AccountName, i);
			BASE_GetItemCode(&LevelItem[pMob[conn].MOB.Class][type][pMob[conn].MOB.BaseScore.Level], ItemCode);
			strcat(temp, ItemCode);
			Log(temp, "_lvlreward_", pUser[conn].IP);

			if (pUser[conn].Mode == USER_SELCHAR)
				SaveUser(conn, 0);

			return;
		}
	}
}

/// Configura atributos da coroa/tiara de sub-deus.
void SetCircletSubGod(int conn)
{
	if(pMob[conn].extra.ClassMaster == SCELESTIAL && pMob[conn].MOB.BaseScore.Level == 120 && pMob[conn].extra.QuestInfo.Celestial.Add120 == 0)
	{
		pMob[conn].extra.QuestInfo.Celestial.Add120 = 1;

		if(pMob[conn].MOB.Equip[1].sIndex != 0)
		{
			pMob[conn].MOB.Equip[1].stEffect[1].cEffect = EF_AC;
			pMob[conn].MOB.Equip[1].stEffect[1].cValue = 30;
			pMob[conn].MOB.Equip[1].stEffect[2].cEffect = EF_HP;
			pMob[conn].MOB.Equip[1].stEffect[2].cValue = 40;
			SendItem(conn, ITEM_PLACE_EQUIP, 1, &pMob[conn].MOB.Equip[1]);
		}
	}

	if(pMob[conn].extra.ClassMaster == SCELESTIAL && pMob[conn].MOB.BaseScore.Level == 150 && pMob[conn].extra.QuestInfo.Celestial.Add150 == 0)
	{
		pMob[conn].extra.QuestInfo.Celestial.Add150 = 1;

		if(pMob[conn].MOB.Equip[1].sIndex != 0)
		{
			pMob[conn].MOB.Equip[1].stEffect[1].cEffect = EF_AC;
			pMob[conn].MOB.Equip[1].stEffect[1].cValue = 35;
			pMob[conn].MOB.Equip[1].stEffect[2].cEffect = EF_HP;
			pMob[conn].MOB.Equip[1].stEffect[2].cValue = 50;
			SendItem(conn, ITEM_PLACE_EQUIP, 1, &pMob[conn].MOB.Equip[1]);
		}
	}

	if(pMob[conn].extra.ClassMaster == SCELESTIAL && pMob[conn].MOB.BaseScore.Level == 180 && pMob[conn].extra.QuestInfo.Celestial.Add180 == 0)
	{
		pMob[conn].extra.QuestInfo.Celestial.Add180 = 1;

		if(pMob[conn].MOB.Equip[1].sIndex != 0)
		{
			pMob[conn].MOB.Equip[1].stEffect[1].cEffect = EF_AC;
			pMob[conn].MOB.Equip[1].stEffect[1].cValue = 45;
			pMob[conn].MOB.Equip[1].stEffect[2].cEffect = EF_HP;
			pMob[conn].MOB.Equip[1].stEffect[2].cValue = 60;
			SendItem(conn, ITEM_PLACE_EQUIP, 1, &pMob[conn].MOB.Equip[1]);
		}
	}

	if(pMob[conn].extra.ClassMaster == SCELESTIAL && pMob[conn].MOB.BaseScore.Level == 199 && pMob[conn].extra.QuestInfo.Celestial.Add200 == 0)
	{
		pMob[conn].extra.QuestInfo.Celestial.Add200 = 1;

		if(pMob[conn].MOB.Equip[1].sIndex != 0)
		{
			pMob[conn].MOB.Equip[1].stEffect[1].cEffect = EF_AC;
			pMob[conn].MOB.Equip[1].stEffect[1].cValue = 50;
			pMob[conn].MOB.Equip[1].stEffect[2].cEffect = EF_HP;
			pMob[conn].MOB.Equip[1].stEffect[2].cValue = 70;
			SendItem(conn, ITEM_PLACE_EQUIP, 1, &pMob[conn].MOB.Equip[1]);
		}
	}
}

/// Cria bau de tesouro no mapa.
int  CreateTreasureBox(int x, int y, STRUCT_ITEM *item, int rotate, int State)
{
	if (item->sIndex <= 0 || item->sIndex >= MAX_ITEMLIST)
		return FALSE;

	GetEmptyItemGrid(&x, &y);


	if (pItemGrid[y][x])
		return FALSE;

	int empty = GetEmptyItem();

	if (empty == 0)
		return FALSE;

	pItem[empty].Mode = 2;

	pItem[empty].PosX = x;
	pItem[empty].PosY = y;

	memcpy(&pItem[empty].ITEM, item, sizeof(STRUCT_ITEM));

	pItem[empty].Rotate = rotate;
	pItem[empty].State = State;

	pItem[empty].Delay = 0;
	pItem[empty].Decay = 0;

	pItem[empty].GridCharge = BASE_GetItemAbility(item, EF_GROUND);

	pItemGrid[y][x] = empty;

	pItem[empty].Height = pHeightGrid[y][x];

	MSG_CreateItem sm;
	memset(&sm, 0, sizeof(MSG_CreateItem));

	sm.Type = _MSG_CreateItem;
	sm.Size = sizeof(MSG_CreateItem);
	sm.ID = ESCENE_FIELD;

	sm.ItemID = empty + 10000;

	memcpy(&sm.Item, &pItem[empty].ITEM, sizeof(STRUCT_ITEM));

	sm.GridX = x;
	sm.GridY = y;

	sm.Rotate = rotate;
	sm.State = pItem[empty].State;

	sm.Height = -204;

	GridMulticast(x, y, (MSG_STANDARD*)&sm, 0);

	return TRUE;
}
