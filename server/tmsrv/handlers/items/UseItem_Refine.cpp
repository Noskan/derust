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

/// @file UseItem_Refine.cpp
/// @brief Sub-handler de poções, PO/PL, refinamentos, fada, adamantita e Poção Kappa.

#include "handlers/items/UseItemInternal.h"

bool UseItem_PotionsAndRefine(int conn, MSG_UseItem* m, STRUCT_ITEM* item, int Vol, int amount)
{
#pragma region POT HP / MP
	if (Vol == 1)
	{
		int Hp = BASE_GetItemAbility(item, EF_HP);

		if(pUser[conn].PotionTime != 0)
		{
			int isTime = GetTickCount() - pUser[conn].PotionTime;			

			if(isTime < PotionDelay)
			{
				SendItem(conn, m->SourType, m->SourPos, item);
				return true;
			}
		}
		pUser[conn].PotionTime = GetTickCount();

		if (Hp && pMob[conn].MOB.CurrentScore.Hp > 0)
		{
			int ReqHp = pUser[conn].ReqHp;
			ReqHp += Hp;

			if (ReqHp > pMob[conn].MOB.CurrentScore.MaxHp)
				ReqHp = pMob[conn].MOB.CurrentScore.MaxHp;

			pUser[conn].ReqHp = ReqHp;
		}
		int Mp = BASE_GetItemAbility(item, EF_MP);

		if (Mp)
		{
			int ReqMp = pUser[conn].ReqMp;
			ReqMp += Mp;

			if (ReqMp > pMob[conn].MOB.CurrentScore.MaxMp)
				ReqMp = pMob[conn].MOB.CurrentScore.MaxMp;

			pUser[conn].ReqMp = ReqMp;
		}
		pUser[conn].Unk_2688 = 1;
		SendSetHpMp(conn);

		if (amount > 1)
			BASE_SetItemAmount(item, amount - 1);

		else
			memset(item, 0, sizeof(STRUCT_ITEM));

		return true;
	}

#pragma endregion
#pragma region PO / PL
	if (Vol == 4 || Vol == 5) // PO - PL
	{
		STRUCT_ITEM *dest = GetItemPointer(&pMob[conn].MOB, pUser[conn].Cargo, m->DestType, m->DestPos);

		if (dest == NULL)
		{
			sprintf(temp, "err,use item fail dest %d %d", m->DestType, m->DestPos);
			ItemLog(temp, "-system", 0);
			return true;
		}

		int Egg = 0;

		if (dest->sIndex >= 2300 && dest->sIndex < 2330)
			Egg = 1;

		if (m->DestType == ITEM_PLACE_CARRY && m->DestPos < pMob[conn].MaxCarry)
		{
			if (dest == NULL)
			{
				SendClientMessage(conn, g_pMessageStringTable[_NN_Only_To_Equips]);
				ItemLog("use item fail - dest", pUser[conn].AccountName, pUser[conn].IP);
				SendItem(conn, m->SourType, m->SourPos, item);
				return true;
			}

			int dstVolatile = BASE_GetItemAbility(dest, EF_VOLATILE);

			if (dstVolatile)
			{
				SendClientMessage(conn, g_pMessageStringTable[_NN_Only_To_Equips]);
				SendItem(conn, m->SourType, m->SourPos, item);
				return true;
			}

			int isSanc = BASE_GetItemAbility(dest, EF_NOSANC);

			if (isSanc)
			{
				SendClientMessage(conn, g_pMessageStringTable[_NN_Cant_Refine_More]);
				SendItem(conn, m->SourType, m->SourPos, item);
				return true;
			}

			int sanc = BASE_GetItemSanc(dest);
			int itemtype = 0;

			if(dest->sIndex >= 1234 && dest->sIndex <= 1237 || dest->sIndex >= 1369 && dest->sIndex <= 1372 || dest->sIndex >= 1519 && dest->sIndex <= 1522 || dest->sIndex >= 1669 && dest->sIndex <= 1672 || dest->sIndex >= 1901 && dest->sIndex <= 1910 || dest->sIndex == 1714)
				itemtype = 5;

			if (sanc >= 6 && Vol == 4)
			{
				SendClientMessage(conn, g_pMessageStringTable[_NN_Cant_Refine_More]);
				SendItem(conn, m->SourType, m->SourPos, item);
				return true;
			}
/*
			if(pUser[conn].UseItemTime != 0)
			{
				int isTime = GetTickCount() - pUser[conn].UseItemTime;			

				if(isTime < 1000)
				{
					SendItem(conn, m->SourType, m->SourPos, item);
					SendClientMessage(conn, "Você deve esperar 1 segundo.");
					return;
				}
			}
			pUser[conn].UseItemTime = GetTickCount();
*/

#pragma region Refinar item Selado no inventário
			if (itemtype == 5)
			{
				if (sanc >= 9)
				{
					SendClientMessage(conn, g_pMessageStringTable[_NN_Cant_Refine_More]);
					SendItem(conn, m->SourType, m->SourPos, item);
					return true;
				}

				if (sanc >= 6 && Vol == 4)
				{
					SendClientMessage(conn, g_pMessageStringTable[_NN_Cant_Refine_More]);
					SendItem(conn, m->SourType, m->SourPos, item);
					return true;
				}

				if (sanc == 0)
				{
					if (dest->stEffect[0].cEffect && dest->stEffect[0].cEffect != 43 && (dest->stEffect[0].cEffect && dest->stEffect[0].cEffect < 116 || dest->stEffect[0].cEffect && dest->stEffect[0].cEffect > 125))
					{
						if (dest->stEffect[1].cEffect && dest->stEffect[1].cEffect != 43 && (dest->stEffect[1].cEffect && dest->stEffect[1].cEffect < 116 || dest->stEffect[1].cEffect && dest->stEffect[1].cEffect > 125))
						{
							if (dest->stEffect[2].cEffect && dest->stEffect[2].cEffect != 43 && (dest->stEffect[2].cEffect && dest->stEffect[2].cEffect < 116 || dest->stEffect[2].cEffect && dest->stEffect[2].cEffect > 125))
							{
								SendClientMessage(conn, g_pMessageStringTable[_NN_Cant_Refine_More]);
								SendItem(conn, m->SourType, m->SourPos, item);
								return true;
							}
							dest->stEffect[2].cEffect = 43;
							dest->stEffect[2].cValue = 0;
						}
						else
						{
							dest->stEffect[1].cEffect = 43;
							dest->stEffect[1].cValue = 0;
						}
					}
					else
					{
						dest->stEffect[0].cEffect = 43;
						dest->stEffect[0].cValue = 0;
					}
				}

				int _rd = rand() % 115;

				if (_rd > 100)
					_rd -= 15;

				int ref = sanc;

				int _chance = g_pCelestialRate[ref];

				if (_rd <= _chance)
				{
					sprintf(temp, "%s", g_pMessageStringTable[_NN_Refine_Success]);
					SendClientMessage(conn, temp);

					BASE_SetItemSanc(dest, ref+1, 0);

					pMob[conn].GetCurrentScore(conn);

					SendScore(conn);
					sprintf(temp, "useitem,refine success %d+%d - (%d,%d,%d)", dest->sIndex, sanc, item->stEffect[0].cEffect, item->stEffect[1].cEffect, item->stEffect[2].cEffect);
					ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);


					if (amount > 1)
						BASE_SetItemAmount(item, amount - 1);

					else
						memset(item, 0, sizeof(STRUCT_ITEM));

									 
					SendItem(conn, m->DestType, m->DestPos, dest);
					SendEmotion(conn, 14, 3);
				}

				else
				{
					sprintf(temp, "%s", g_pMessageStringTable[_NN_Fail_To_Refine]);

					SendClientMessage(conn, temp);
					sprintf(temp, "useitem,refine fail %d+%d - (%d,%d,%d)", dest->sIndex, sanc, item->stEffect[0].cEffect, item->stEffect[1].cEffect, item->stEffect[2].cEffect);
					ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);

					if (amount > 1)
						BASE_SetItemAmount(item, amount - 1);

					else
						memset(item, 0, sizeof(STRUCT_ITEM));

									 
					BASE_ClearItem(dest);
					SendItem(conn, m->DestType, m->DestPos, dest);

					int sFace = pMob[conn].MOB.Equip[0].sIndex / 10;

					if (sFace)
						SendEmotion(conn, 15, 0);

					else
						SendEmotion(conn, 20, 0);
				}

				return true;
			}
			SendClientMessage(conn, g_pMessageStringTable[_NN_Only_To_Equips]);
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
#pragma endregion
		}

		if (m->DestType == ITEM_PLACE_EQUIP && m->DestPos == 0 || m->DestType || m->DestPos > 11 && m->DestPos < 15 && dest->sIndex != 753 && dest->sIndex != 769 && dest->sIndex != 1726 && !Egg)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_Only_To_Equips]);

			ItemLog("err,use item fail - dest", pUser[conn].AccountName, pUser[conn].IP);

			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		if (dest == NULL)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_Only_To_Equips]);
			ItemLog("use item fail - dest", pUser[conn].AccountName, pUser[conn].IP);
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		int dstVolatile = BASE_GetItemAbility(dest, EF_VOLATILE);

		if (dstVolatile)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_Only_To_Equips]);
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		int isSanc = BASE_GetItemAbility(dest, EF_NOSANC);

		if (isSanc)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_Cant_Refine_More]);
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		int sanc = BASE_GetItemSanc(dest);
		int itemtype = BASE_GetItemAbility(dest, EF_MOBTYPE);

		if (sanc >= 6 && Vol == 4)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_Cant_Refine_More]);
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

#pragma region Refinar item celestial / HC
		if (itemtype == 3)
		{
			if (sanc >= REF_15)
			{
				SendClientMessage(conn, g_pMessageStringTable[_NN_Cant_Refine_More]);
				SendItem(conn, m->SourType, m->SourPos, item);
				return true;
			}

			if (sanc >= 6 && Vol == 4)
			{
				SendClientMessage(conn, g_pMessageStringTable[_NN_Cant_Refine_More]);
				SendItem(conn, m->SourType, m->SourPos, item);
				return true;
			}

			if (sanc == 0)
			{
				if (dest->stEffect[0].cEffect && dest->stEffect[0].cEffect != 43 && (dest->stEffect[0].cEffect && dest->stEffect[0].cEffect < 116 || dest->stEffect[0].cEffect && dest->stEffect[0].cEffect > 125))
				{
					if (dest->stEffect[1].cEffect && dest->stEffect[1].cEffect != 43 && (dest->stEffect[1].cEffect && dest->stEffect[1].cEffect < 116 || dest->stEffect[1].cEffect && dest->stEffect[1].cEffect > 125))
					{
						if (dest->stEffect[2].cEffect && dest->stEffect[2].cEffect != 43 && (dest->stEffect[2].cEffect && dest->stEffect[2].cEffect < 116 || dest->stEffect[2].cEffect && dest->stEffect[2].cEffect > 125))
						{
							SendClientMessage(conn, g_pMessageStringTable[_NN_Cant_Refine_More]);
							SendItem(conn, m->SourType, m->SourPos, item);
							return true;
						}
						dest->stEffect[2].cEffect = 43;
						dest->stEffect[2].cValue = 0;
					}
					else
					{
						dest->stEffect[1].cEffect = 43;
						dest->stEffect[1].cValue = 0;
					}
				}
				else
				{
					dest->stEffect[0].cEffect = 43;
					dest->stEffect[0].cValue = 0;
				}
			}

			int _rd = rand() % 115;

			if (_rd > 100)
				_rd -= 15;

			int ref = sanc;

			if (ref == REF_10)
				ref = 10;

			else if (ref == REF_11)
				ref = 11;

			else if (ref == REF_12)
				ref = 12;

			else if (ref == REF_13)
				ref = 13;

			else if (ref == REF_14)
				ref = 14;

			else if (ref == REF_15)
				ref = 15;

			int _chance = g_pCelestialRate[ref];

			if (_rd <= _chance)
			{
				sprintf(temp, "%s", g_pMessageStringTable[_NN_Refine_Success]);
				SendClientMessage(conn, temp);

									 
				BASE_SetItemSanc(dest, ref+1, 0);

				pMob[conn].GetCurrentScore(conn);

				SendScore(conn);
				sprintf(temp, "useitem,refine success %d+%d - (%d,%d,%d)", dest->sIndex, sanc, item->stEffect[0].cEffect, item->stEffect[1].cEffect, item->stEffect[2].cEffect);
				ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);


				//SendNotice(g_pMessageStringTable[_SS_Combin_12Succ]);

				if (amount > 1)
					BASE_SetItemAmount(item, amount - 1);

				else
					memset(item, 0, sizeof(STRUCT_ITEM));

									 
				SendItem(conn, m->DestType, m->DestPos, dest);
				SendEmotion(conn, 14, 3);
			}

			else
			{
				sprintf(temp, "%s", g_pMessageStringTable[_NN_Fail_To_Refine]);

				SendClientMessage(conn, temp);
				sprintf(temp, "useitem,refine fail %d+%d - (%d,%d,%d)", dest->sIndex, sanc, item->stEffect[0].cEffect, item->stEffect[1].cEffect, item->stEffect[2].cEffect);
				ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);

				if (amount > 1)
					BASE_SetItemAmount(item, amount - 1);

				else
					memset(item, 0, sizeof(STRUCT_ITEM));

									 
				BASE_SetItemSanc(dest, 0, 0);
				SendItem(conn, m->DestType, m->DestPos, dest);

				int sFace = pMob[conn].MOB.Equip[0].sIndex / 10;

				if (sFace)
					SendEmotion(conn, 15, 0);

				else
					SendEmotion(conn, 20, 0);
			}

			return true;
		}
#pragma endregion
#pragma region Pedras Arch
		if(dest->sIndex >= 1752 && dest->sIndex <= 1759)
		{
			int _rd = rand() % 115;

			if (_rd > 100)
				_rd -= 15;

			int NextPedra = 1744;
			int RateSucess = 0;

			if(dest->sIndex == 1752)//Orc Tropper
			{
				if(_rd < 56)
					NextPedra = 1744;//Inteligencia
									 
				else if(_rd < 80)
					NextPedra = 1745;//Sabedoria

				else if(_rd < 90)
					NextPedra = 1746;//Misericórdia

				else if(_rd < 93)
					NextPedra = 1747;//Abismo

				RateSucess = 93;
			}

			else if(dest->sIndex == 1753)//Esqueleto
			{
				if(_rd < 21)
					NextPedra = 1744;//Inteligencia
									 
				else if(_rd < 76)
					NextPedra = 1745;//Sabedoria

				else if(_rd < 86)
					NextPedra = 1746;//Misericórdia

				else if(_rd < 90)
					NextPedra = 1747;//Abismo

				RateSucess = 90;
			}

			else if(dest->sIndex == 1754)//Dragão Lich
			{
				if(_rd < 3)
					NextPedra = 1744;//Inteligencia
									 
				else if(_rd < 21)
					NextPedra = 1745;//Sabedoria

				else if(_rd < 76)
					NextPedra = 1746;//Misericórdia

				else if(_rd < 85)
					NextPedra = 1747;//Abismo

				RateSucess = 85;
			}

			else if(dest->sIndex == 1755)//DemonLord
			{
				if(_rd < 3)
					NextPedra = 1744;//Inteligencia
									 
				else if(_rd < 10)
					NextPedra = 1745;//Sabedoria

				else if(_rd < 25)
					NextPedra = 1746;//Misericórdia

				else if(_rd < 80)
					NextPedra = 1747;//Abismo

				RateSucess = 80;
			}

			else if(dest->sIndex == 1756)//Manticora
			{
				if(_rd < 50)
					NextPedra = 1748;//Beleza
									 
				else if(_rd < 62)
					NextPedra = 1749;//Vitória

				else if(_rd < 68)
					NextPedra = 1750;//Originalidade

				else if(_rd < 70)
					NextPedra = 1751;//Reino

				RateSucess = 70;
			}

			else if(dest->sIndex == 1757)//Gargula de fogo
			{
				if(_rd < 9)
					NextPedra = 1748;//Beleza
									 
				else if(_rd < 59)
					NextPedra = 1749;//Vitória

				else if(_rd < 63)
					NextPedra = 1750;//Originalidade

				else if(_rd < 65)
					NextPedra = 1751;//Reino

				RateSucess = 65;
			}

			else if(dest->sIndex == 1758)//Lugefer
			{
				if(_rd < 2)
					NextPedra = 1748;//Beleza
									 
				else if(_rd < 8)
					NextPedra = 1749;//Vitória

				else if(_rd < 58)
					NextPedra = 1750;//Originalidade

				else if(_rd < 62)
					NextPedra = 1751;//Reino

				RateSucess = 62;
			}

			else if(dest->sIndex == 1759)//DemonLord
			{
				if(_rd < 2)
					NextPedra = 1748;//Beleza
									 
				else if(_rd < 5)
					NextPedra = 1749;//Vitória

				else if(_rd < 10)
					NextPedra = 1750;//Originalidade

				else if(_rd < 60)
					NextPedra = 1751;//Reino

				RateSucess = 60;
			}

			if (_rd <= RateSucess)
			{
				sprintf(temp, "%s", g_pMessageStringTable[_NN_Refine_Success]);
				SendClientMessage(conn, temp);

				dest->sIndex = NextPedra; 

				pMob[conn].GetCurrentScore(conn);

				SendScore(conn);
				sprintf(temp, "useitem,refine success %d+%d - (%d,%d,%d)", dest->sIndex, sanc, item->stEffect[0].cEffect, item->stEffect[1].cEffect, item->stEffect[2].cEffect);
				ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);

				if (amount > 1)
					BASE_SetItemAmount(item, amount - 1);

				else
					memset(item, 0, sizeof(STRUCT_ITEM));

									 
				SendItem(conn, m->DestType, m->DestPos, dest);
				SendEmotion(conn, 14, 3);
			}

			else
			{
				sprintf(temp, "%s", g_pMessageStringTable[_NN_Fail_To_Refine]);

				SendClientMessage(conn, temp);
				sprintf(temp, "useitem,refine fail %d+%d - (%d,%d,%d)", dest->sIndex, sanc, item->stEffect[0].cEffect, item->stEffect[1].cEffect, item->stEffect[2].cEffect);
				ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);

				if (amount > 1)
					BASE_SetItemAmount(item, amount - 1);

				else
					memset(item, 0, sizeof(STRUCT_ITEM));

									 
				BASE_SetItemSanc(dest, 0, 0);
				SendItem(conn, m->DestType, m->DestPos, dest);

				int sFace = pMob[conn].MOB.Equip[0].sIndex / 10;

				if (sFace)
					SendEmotion(conn, 15, 0);

				else
					SendEmotion(conn, 20, 0);
			}

			return true;
		}
#pragma endregion

#pragma region Refinar brincos +10+
		if(sanc >= 9 && sanc <= REF_14 && Vol == 5 && m->DestPos == 8 && g_pItemList[dest->sIndex].nPos == 256)
		{
			int RateSucess = 15;
			int _rand = rand() % 100;

			int ref = sanc;

			if (ref == 9)
				ref = 9;

			else if (ref == REF_10)
				ref = 10;

			else if (ref == REF_11)
				ref = 11;

			else if (ref == REF_12)
				ref = 12;

			else if (ref == REF_13)
				ref = 13;

			else if (ref == REF_14)
				ref = 14;

			else if (ref == REF_15)
				ref = 15;

			int _chance = 15;

			if (_rand <= _chance)
			{
				sprintf(temp, "%s", g_pMessageStringTable[_NN_Refine_Success]);
				SendClientMessage(conn, temp);

									 
				BASE_SetItemSanc(dest, ref+1, 0);

				pMob[conn].GetCurrentScore(conn);

				SendScore(conn);
				sprintf(temp, "useitem,refine success %d+%d - (%d,%d,%d)", dest->sIndex, sanc, dest->stEffect[0].cEffect, dest->stEffect[1].cEffect, dest->stEffect[2].cEffect);
				ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);

				if (amount > 1)
					BASE_SetItemAmount(item, amount - 1);

				else
					memset(item, 0, sizeof(STRUCT_ITEM));

									 
				SendItem(conn, m->DestType, m->DestPos, dest);
				SendEmotion(conn, 14, 3);
			}

			else
			{
				sprintf(temp, "%s", g_pMessageStringTable[_NN_Fail_To_Refine]);

				SendClientMessage(conn, temp);
				sprintf(temp, "useitem,refine fail %d+%d - (%d,%d,%d)", dest->sIndex, sanc, dest->stEffect[0].cEffect, dest->stEffect[1].cEffect, dest->stEffect[2].cEffect);
				ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);

				if (amount > 1)
					BASE_SetItemAmount(item, amount - 1);

				else
					memset(item, 0, sizeof(STRUCT_ITEM));

									 
				BASE_ClearItem(dest);
				SendItem(conn, m->DestType, m->DestPos, dest);

				int sFace = pMob[conn].MOB.Equip[0].sIndex / 10;

				if (sFace)
					SendEmotion(conn, 15, 0);

				else
					SendEmotion(conn, 20, 0);
			}
			return true;
		}
#pragma endregion

		if (sanc == 9 || sanc >= REF_11 || sanc < 0 || sanc >= 9 && dest->sIndex == 769)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_Cant_Refine_More]);
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		int sancsucess = BASE_GetItemSancSuccess(dest);

		if (sanc < 0)
			sanc = 0;

		if (sanc == 0)
		{
			if (dest->stEffect[0].cEffect && dest->stEffect[0].cEffect != 43 && (dest->stEffect[0].cEffect && dest->stEffect[0].cEffect < 116 || dest->stEffect[0].cEffect && dest->stEffect[0].cEffect > 125))
			{
				if (dest->stEffect[1].cEffect && dest->stEffect[1].cEffect != 43 && (dest->stEffect[1].cEffect && dest->stEffect[1].cEffect < 116 || dest->stEffect[1].cEffect && dest->stEffect[1].cEffect > 125))
				{
					if (dest->stEffect[2].cEffect && dest->stEffect[2].cEffect != 43 && (dest->stEffect[2].cEffect && dest->stEffect[2].cEffect < 116 || dest->stEffect[2].cEffect && dest->stEffect[2].cEffect > 125))
					{
						SendClientMessage(conn, g_pMessageStringTable[_NN_Cant_Refine_More]);
						SendItem(conn, m->SourType, m->SourPos, item);
						return true;
					}
					dest->stEffect[2].cEffect = 43;
					dest->stEffect[2].cValue = 0;
				}
				else
				{
					dest->stEffect[1].cEffect = 43;
					dest->stEffect[1].cValue = 0;
				}
			}
			else
			{
				dest->stEffect[0].cEffect = 43;
				dest->stEffect[0].cValue = 0;
			}
		}

		if (dest->sIndex >= 2300 && dest->sIndex < 2330 && pMob[conn].MOB.BaseScore.Level < 999 && BASE_GetBonusItemAbility(dest, EF_INCUDELAY) > 0)
		{
			SendClientMessage(conn, g_pMessageStringTable[_NN_Incu_Wait_More]);
			SendItem(conn, m->SourType, m->SourPos, item);
			return true;
		}

		int OriLacto = Vol - 4;
		int RateSucess = item->sIndex != 4141 ? BASE_GetSuccessRate(dest, Vol - 4) : BASE_GetSuccessRate(dest, 2);
		int _rand = rand() % 100;

		int ItemGrade = BASE_GetItemAbility(dest, EF_ITEMLEVEL);

		if (_rand <= RateSucess && RateSucess)
		{
			if(sanc == REF_10)
				BASE_SetItemSanc(dest, 11, BASE_GetItemGem(dest));
								 
			else
			{
				if (ItemGrade >= 1 && ItemGrade <= 5)
				{
					ItemGrade--;
					sanc += g_pSancGrade[OriLacto][ItemGrade];


					if (sanc >= 6 && Vol == 4)
						sanc = 6;

					else if (sanc > 9)
						sanc = 9;
				}
				else
					sanc++;


				sancsucess = 0;

				BASE_SetItemSanc(dest, sanc, 0);
			}
			pMob[conn].GetCurrentScore(conn);

			SendScore(conn);
			sprintf(temp, "useitem,refine success %d+%d - (%d,%d,%d)", dest->sIndex, sanc, dest->stEffect[0].cEffect, dest->stEffect[1].cEffect, dest->stEffect[2].cEffect);
			ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);

			SendClientMessage(conn, g_pMessageStringTable[_NN_Refine_Success]);

			if (dest->sIndex >= 2300 && dest->sIndex < 2330)
			{
				dest->stEffect[2].cEffect = 84;

				int incurand = rand() & 0x80000003;

				if (incurand < 0)
					incurand = ((incurand - 1) | 0xFC) + 1;

				dest->stEffect[2].cValue = incurand + 6;

				int incubate = BASE_GetBonusItemAbility(dest, EF_INCUBATE);

				if (sanc >= incubate)
				{
					dest->sIndex += 30;

					dest->stEffect[0].sValue = 20000;
					dest->stEffect[1].cEffect = 1;
					dest->stEffect[1].cValue = rand() % 20 + 10;
					dest->stEffect[2].cEffect = 30;
					dest->stEffect[2].cValue = 1;

					SendClientMessage(conn, g_pMessageStringTable[_NN_INCUBATED]);
					MountProcess(conn, 0);

					SendItem(conn, m->DestType, m->DestPos, dest);
				}
			}
			SendItem(conn, m->DestType, m->DestPos, dest);
			SendEmotion(conn, 14, 3);

			if (amount > 1)
				BASE_SetItemAmount(item, amount - 1);

			else
				memset(item, 0, sizeof(STRUCT_ITEM));
		}

		else
		{
			sprintf(temp, "%s", g_pMessageStringTable[_NN_Fail_To_Refine]);

			SendClientMessage(conn, temp);
			sprintf(temp, "useitem,refine fail %d+%d - (%d,%d,%d)", dest->sIndex, sanc, dest->stEffect[0].cEffect, dest->stEffect[1].cEffect, dest->stEffect[2].cEffect);
			ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);

			if (amount > 1)
				BASE_SetItemAmount(item, amount - 1);

			else
				memset(item, 0, sizeof(STRUCT_ITEM));

			if (rand() % 4 <= 2)
			{
				if (OriLacto == 1)
					++sancsucess;

				else if (sanc <= 5 && !OriLacto)
					++sancsucess;
			}
			if (dest->sIndex >= 2300 && dest->sIndex < 2330)
			{
				dest->stEffect[2].cEffect = 84;

				int incu = rand() & 0x80000003;
				if (incu < 0)
					incu = ((incu - 1) | 0xFC) + 1;

				dest->stEffect[2].cValue = incu;
			}

			if(sanc != REF_10)
				BASE_SetItemSanc(dest, sanc, sancsucess);

			SendItem(conn, m->DestType, m->DestPos, dest);

			int sFace = pMob[conn].MOB.Equip[0].sIndex / 10;

			if (sFace)
			SendEmotion(conn, 15, 0);

			else
				SendEmotion(conn, 20, 0);
		}

		return true;
	}

#pragma endregion
#pragma region Pilula Orc
	if (Vol == 6) // Pilula Orc
	{
		if(pMob[conn].extra.QuestInfo.Mortal.PilulaOrc)
			return true;

		pMob[conn].MOB.SkillBonus += 9;
		pMob[conn].extra.QuestInfo.Mortal.PilulaOrc = 1;
		SendEmotion(conn, 14, 3);
		SendEtc(conn);

		if (amount > 1)
			BASE_SetItemAmount(item, amount - 1);
		else
			memset(item, 0, sizeof(STRUCT_ITEM));

		sprintf(temp, "useitem,pilula_orc quest complete.");
		ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);
		return true;
	}
#pragma endregion
#pragma region Poeira de Fada
	if (Vol == 7)//Poeira de Fada
	{
		int Level = pMob[conn].MOB.BaseScore.Level;
		int maxlevel = pMob[conn].extra.ClassMaster == MORTAL || pMob[conn].extra.ClassMaster == ARCH ? MAX_LEVEL : MAX_CLEVEL;

		if (Level >= maxlevel || Level < 0 || rand() % 2)
		{
			int sFace = pMob[conn].MOB.Equip[0].sIndex / 10;

			if (sFace)
				SendEmotion(conn, 15, 0);
			else
				SendEmotion(conn, 20, 0);

			SendClientMessage(conn, g_pMessageStringTable[_NN_Fail_To_Level_Up]);
		}
		else
		{
			pMob[conn].MOB.Exp = pMob[conn].extra.ClassMaster == MORTAL || pMob[conn].extra.ClassMaster == ARCH ? g_pNextLevel[Level+1] : g_pNextLevel_2[Level+1];

			if (pMob[conn].CheckGetLevel())
			{
				SendClientMessage(conn, "*** Level UP !!! ***");

				SendEmotion(conn, 14, 3);

				SendScore(conn);
				SendEtc(conn);

				if(pMob[conn].extra.ClassMaster == MORTAL)
					DoItemLevel(conn);
			}
		}

		if (amount > 1)
			BASE_SetItemAmount(item, amount - 1);

		else
			memset(item, 0, sizeof(STRUCT_ITEM));

		sprintf(temp, "useitem,poeira_de_fada classmaster:%d level:%d", pMob[conn].extra.ClassMaster, pMob[conn].MOB.BaseScore.Level);
		ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);
		return true;
	}
	#pragma endregion
#pragma region Olho Crescente
	if (Vol == 8)
	{
		if(pMob[conn].extra.ClassMaster != MORTAL)
			return true;

		pMob[conn].MOB.Exp += 2500;

		sprintf(temp, g_pMessageStringTable[_NS_GETEXP], 2500);
		SendClientMessage(conn, temp);

		SendEmotion(conn, 14, 3);

		if (pMob[conn].CheckGetLevel() == 4)
		{
			SetCircletSubGod(conn);
			SendClientMessage(conn, g_pMessageStringTable[_NN_Level_Up]);
			SendScore(conn);

			if(pMob[conn].extra.ClassMaster == MORTAL)
				DoItemLevel(conn);
		}
		SendEtc(conn);

		if (amount > 1)
			BASE_SetItemAmount(item, amount-1);
							 
		else
			memset(item, 0, sizeof(STRUCT_ITEM));
		
		sprintf(temp, "useitem,olho_crescente level:%d exp recebida:2500", pMob[conn].MOB.BaseScore.Level);
		ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);
		return true;
	}
#pragma endregion
#pragma region Adamantita/beril etc LE
	if (Vol == 9) // Adamantita / Beril etc
	{
		STRUCT_ITEM *dest = GetItemPointer(&pMob[conn].MOB, pUser[conn].Cargo, m->DestType, m->DestPos);

		if (dest == NULL)
		{
			sprintf(temp, "err,use item fail dest %d %d", m->DestType, m->DestPos);
			ItemLog(temp, "-system", 0);
			return true;
		}

		int sourIndex = item->sIndex;
		int destIndex = dest->sIndex;
		int Type = sourIndex - 575;

		if (sourIndex - 575 >= 0 && Type < 4)
		{
			int UniqueType = -1;
			int nUnique = g_pItemList[destIndex].nUnique;

			if (nUnique == 5 || nUnique == 14 || nUnique == 24 || nUnique == 34)
				UniqueType = 0;

			if (nUnique == 6 || nUnique == 15 || nUnique == 25 || nUnique == 35)
				UniqueType = 1;

			if (nUnique == 7 || nUnique == 16 || nUnique == 26 || nUnique == 36)
				UniqueType = 2;

			if (nUnique == 8 || nUnique == 17 || nUnique == 27 || nUnique == 37)
				UniqueType = 3;

			if (nUnique == 10 || nUnique == 20 || nUnique == 30 || nUnique == 40)
				UniqueType = 3;

			if (nUnique == -1)
			{
				SendItem(conn, m->SourType, m->SourPos, item);
				SendClientMessage(conn, g_pMessageStringTable[Type + _NN_Only_Level1_Armor]);
			}
			else
			{
				if (UniqueType == Type)
				{
					int Grade = g_pItemList[destIndex].Grade;

					if (Grade > 0 && Grade < 4)
					{
						int _rand = rand() % 100;
						int chance = 50;
						if (_rand <= 50)
						{
							int Extra = g_pItemList[destIndex].Extra;

							dest->sIndex = Extra;

							SendItem(conn, m->DestType, m->DestPos, dest);

							if (amount > 1)
								BASE_SetItemAmount(item, amount - 1);

							else
								memset(item, 0, sizeof(STRUCT_ITEM));

							SendScore(conn);
							SendEmotion(conn, 14, 0);

							sprintf(temp, "useitem,regendary success %d - (%d,%d,%d)", dest->sIndex,
								item->stEffect[0].cEffect, item->stEffect[1].cEffect, item->stEffect[2].cEffect);

							ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);
							sprintf(temp, "%s(%d/%d)", g_pMessageStringTable[_NN_Refine_Success], _rand, chance);

							SendClientMessage(conn, temp);
						}
						else
						{
							if (pMob[conn].MOB.Equip[0].sIndex / 10)
								SendEmotion(conn, 15, 0);

							else
								SendEmotion(conn, 20, 0);

							sprintf(temp, "useitem,regendary fail %d - (%d,%d,%d)",
								dest->sIndex, item->stEffect[0].cEffect, item->stEffect[1].cEffect, item->stEffect[2].cEffect);

							ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);

							sprintf(temp, "%s(%d/%d)", g_pMessageStringTable[_NN_Fail_To_Refine], _rand, chance);
							SendClientMessage(conn, temp);

							if (amount > 1)
								BASE_SetItemAmount(item, amount - 1);

							else
								memset(item, 0, sizeof(STRUCT_ITEM));
						}
					}
					else
					{
						SendItem(conn, m->SourType, m->SourPos, item);
						SendClientMessage(conn, g_pMessageStringTable[_NN_Cant_Refine_More]);
					}
				}
				else
				{
					SendItem(conn, m->SourType, m->SourPos, item);
					SendClientMessage(conn, g_pMessageStringTable[Type + _NN_Only_Level1_Armor]);
				}
			}
		}
		else
			SendItem(conn, m->SourType, m->SourPos, item);

		return true;
	}
	#pragma endregion
#pragma region Poção Kappa
	if (Vol == 10 || Vol == 55 || Vol == 200 || Vol == 201 || Vol == 56 || Vol == 52 || Vol == 53 || Vol == 57 || Vol == 202)
	{
		int value = 0;

		int tempo = 80;

		//Kappa
		if(item->sIndex == 787)
			value = 1;

		//Combatente
		else if(item->sIndex == 1764)
			value = 2;

		//Mental
		else if(item->sIndex == 1765)
			value = 3;

		//Mental 60m
		else if(item->sIndex == 3312)
		{
			tempo = AFFECT_1H;
			value = 3;
		}
		//Combatente 60m
		else if(item->sIndex == 3311)
		{
			tempo = AFFECT_1H;
			value = 2;
		}

		//Kappa 30m
		else if(item->sIndex == 3310)
		{
			tempo = AFFECT_1H/2;
			value = 1;
		}

		//Mental 20h
		else if(item->sIndex == 3321)
		{
			tempo = AFFECT_1H*20;
			value = 3;
		}

		//Combatente 20h
		else if(item->sIndex == 3320)
		{
			tempo = AFFECT_1H*20;
			value = 2;
		}

		//Kappa 20h
		else if(item->sIndex == 3319)
		{
			tempo = AFFECT_1H*20;
			value = 1;
		}

		int EmptyAffect = -1;

		for(int i = 0; i < MAX_AFFECT; i++)
		{
			if(pMob[conn].Affect[i].Type == 4 && pMob[conn].Affect[i].Level == value)
			{
				EmptyAffect = i;
				break;
			}
		}

		for(int i = 0; i < MAX_AFFECT && EmptyAffect == -1; i++)
		{
			if(pMob[conn].Affect[i].Type == 0)
			{
				EmptyAffect = i;
				break;
			}
		}

		if(EmptyAffect != -1)
		{
			pMob[conn].Affect[EmptyAffect].Type = 4;
			pMob[conn].Affect[EmptyAffect].Value = 0;
			pMob[conn].Affect[EmptyAffect].Level = value;
			pMob[conn].Affect[EmptyAffect].Time = tempo;

			pMob[conn].GetCurrentScore(conn);
			SendScore(conn);
		}

							
		if (amount > 1)
			BASE_SetItemAmount(item, amount - 1);

		else
			memset(item, 0, sizeof(STRUCT_ITEM));

		sprintf(temp, "useitem,poção_kappa affectslot:%d value:%d time:%d", EmptyAffect, value, tempo);
		ItemLog(temp, pUser[conn].AccountName, pUser[conn].IP);
		return true;
	}
#pragma endregion

	return false;
}
