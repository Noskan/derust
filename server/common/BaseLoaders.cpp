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

/// @file BaseLoaders.cpp
/// @brief Carga e gravação das tabelas de dados do jogo em w2pp_common: lista de itens,
///        skills, mensagens, nomes de mobs/efeitos, itens iniciais, guildas, servidores,
///        taxa de acerto e atributos do mapa (formatos texto e .bin com checksum).
///
/// Extraído de `Basedef.cpp` na Fase 2 (Onda 4) sem alteração de comportamento.

#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>
#include <stdio.h> 
#include <fcntl.h>
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/timeb.h>
#include <string.h>        
#include <time.h>
#include <mbstring.h>

#include "common/Basedef.h"
#include "common/ItemEffect.h"
#include "common/BasedefInternal.h"

void BASE_WriteSkillBin()
{
	int size = sizeof(STRUCT_SPELL) * MAX_SPELL_LIST;
	char *temp = (char*)g_pSpell;

	for(int i = 0; i < size; i++)
	{
		temp[i] = temp[i] ^ 0x5A;
	}

	FILE *fp = fopen("SkillData.bin", "wb");

	if(fp != NULL)
	{	
		fwrite(g_pSpell, sizeof(STRUCT_SPELL), MAX_SPELL_LIST , fp);

		int sum = BASE_GetSum2((char*)g_pSpell, size);
		int nRand = W2Rand();

		fwrite(&nRand, sizeof(nRand), 1 , fp);
		fclose(fp);

		FILE *fpCSum = fopen("../SkillCSum.h", "wt");

		if(fpCSum != NULL)
		{
			fprintf(fpCSum, "#define SKILL_CHECKSUM %d", sum);
			fclose(fpCSum);	// antes fclose(fp) de novo, e o fpCSum ficava aberto
		}
	}

	for(int i = 0; i < size; i++)
	{ 
		temp[i] = temp[i] ^ 0x5A;
	}

}

void BASE_WriteItemList(int nItemList)
{
	int size = sizeof(STRUCT_ITEMLIST)*MAX_ITEMLIST;
	char *temp = (char*)&g_pItemList;

	for(int i = 0; i < size; i++)
	{
		temp[i] = temp[i] ^ 0x5A;
	}

	FILE *fp = NULL;

	if(nItemList == 0)
	{
		fp = fopen("../../Common/ItemList.bin", "wb");

		if(fp == NULL)
		{
			MessageBox(NULL, "ItemList.binÀ» »ý¼ºÇÒ¼ö ¾ø½À´Ï´Ù. ", "ERROR", NULL);
			return;
		}

		fwrite(g_pItemList, sizeof(STRUCT_ITEMLIST)*MAX_ITEMLIST, 1, fp);
	}

	int sum = BASE_GetSum2((char*)g_pItemList, size);

	int nRand = W2Rand();

	if(nItemList == 0)
	{
		fwrite(&nRand, sizeof(nRand), 1 , fp);
		fclose(fp);
	}

	FILE* fpCSum = NULL;

	if(nItemList == 0)
		fpCSum = fopen("../ItemCSum.h", "wt");
	else
		fpCSum = fopen("../ItemCSum.h", "a+");

	if(fpCSum != NULL)
	{
		if(nItemList == 0)
			fprintf(fpCSum, "#define ITEM_CHECKSUM %d\n", sum);
		else
			fprintf(fpCSum, "#define ITEM_CHECKSUM2 %d\n", sum);

		fclose(fpCSum);
	}

    for(int i = 0; i < size; i++)
	{
		temp[i] = temp[i]^0x5A;
	}
}

int BASE_ReadSkillBin()
{
	int size = sizeof(STRUCT_SPELL) * MAX_SPELL_LIST;
	char *temp = (char*)g_pSpell;
	int tsum = 0;

	FILE *fp = fopen("SkillData.bin", "rb");

	if(fp != NULL)
	{
		fread(g_pSpell, size , 1 , fp);
		fread(&tsum, sizeof(tsum), 1 , fp);

		fclose(fp);
	}
	else
	{
		MessageBox(NULL, "Can't read SkillData.bin", "ERROR", NULL);
		return FALSE;
	}

	int sum = BASE_GetSum2((char*)g_pSpell, size);

#ifndef _DEBUG
	//if(SKILL_CHECKSUM != sum) 
	//	return FALSE;
#endif

	for(int i = 0; i < size;i++)
	{	
		temp[i] = temp[i] ^ 0x5A;
	}

	return TRUE;
}

int BASE_ReadItemList()
{
	int size = sizeof(STRUCT_ITEMLIST) * MAX_ITEMLIST;
	char *temp = (char*)g_pItemList;
	int tsum = 0;

	FILE *fp;
	fp = fopen("../../Common/ItemList.bin", "rb");

	if(fp == NULL) 
	{
		MessageBox(NULL, "Can't read ItemList.bin", "ERROR", NULL);

		return FALSE;
	}

	fread(g_pItemList, sizeof(STRUCT_ITEMLIST)*MAX_ITEMLIST, 1, fp);
	fread(&tsum, sizeof(tsum), 1, fp);

	fclose(fp);

	int sum = BASE_GetSum2((char*)g_pItemList, size);

#ifndef _DEBUG
	//if (ITEM_CHECKSUM != sum)
	//	return FALSE;
#endif

	for(int i = 0; i < size; i++)
	{ 
		temp[i] = temp[i] ^ 0x5A;
	}

	int Handle = _open("../../Common/ExtraItem.bin", _O_RDONLY|_O_BINARY, 0);

	if(Handle != -1)
	{
		while(1)
		{ 
			char buff[256];

			STRUCT_ITEMLIST *item;

			int sz = _read(Handle, buff, sizeof(STRUCT_ITEMLIST)+2);

			if(sz < sizeof(STRUCT_ITEMLIST)+2) 
				break;

			short idx = *((short*)buff);

			if(idx <= 0 || idx >= MAX_ITEMLIST) 
				continue;

			item = (STRUCT_ITEMLIST *)(buff+2);

			g_pItemList[idx] = *item;
		}

		_close(Handle);
#ifndef _DEBUG
		for(int i = 0; i < size; i++)
		{ 
			temp[i] = temp[i] ^ 0x5A;
		}

		int sum = BASE_GetSum2((char*)g_pItemList, size);

		//if(ITEM_CHECKSUM2 != sum) 
		//	return FALSE;

		for(int i = 0; i < size; i++)
		{
			temp[i] = temp[i] ^ 0x5A;
		}
#endif
	}

	return TRUE;
}

void BASE_InitializeMessage(char *file)
{
	memset(g_pMessageStringTable, MAX_STRING, 128);

	FILE *fp = fopen(file, "rt");

	if(fp == NULL)
		return;

	 char str[1024];
	 char part1[256];
	 char *cont;

	while(1)
	{
		int Index = -1;
		char *ret = fgets((char*)str, 1024, fp);

		if(ret == NULL)
			break;

        sscanf(str, "%d %s", &Index, part1);

		str[510] = 0; 
		str[511] = 0; 

		if(Index <= 0 || Index > MAX_STRING) 
			continue;

		int TabPos = 0;

		for(int i = 0; i < 256; i++)
		{   
			if(str[i] == '\t') 
			{
				TabPos = i;

				break;
			} 
		}

		if(TabPos == 0)
		{  
			MessageBox(NULL, str, "Can't parse String", MB_OK);

		   continue;
		}

        cont = (char*)(str + TabPos + 1);

		for(int i = 0; i < 128; i++)
		{   
			if (cont[i] == '\t' || cont[i] == '\n' || cont[i] == 0)
			{ 
				cont[i] = 0;
				
				break;
			} 
		}

        int len = (int)strlen(cont);

		if(len <= 0 || len >= 128)
		{  
			MessageBox(NULL, str, "Empty or Long String", MB_OK);

			continue;
		}

		strcpy(g_pMessageStringTable[Index], cont);
	}

	fclose(fp);
}

void BASE_InitializeMobname(char *file, int offset)
{
	if(offset == 0)
		memset(g_pEnglish, 0, 19200);

	FILE *fp = fopen(file, "rt");

	if(fp == NULL)
		return;

	int Index = 0;

	char str[1024];
	char part1[256];
	char *cont;

	if(offset == 0)
	{
		while(1)
		{
			char *ret = fgets((char*)str, 1024, fp);

			if(ret == NULL)
				break;

			sscanf(str, "%s", part1);

			str[510] = 0;
			str[511] = 0;

			int TabPos = 0;

			for(int i = 0; i < 256; i++)
			{   
				if(str[i] == '\t') 
				{
					TabPos = i;

					break;
				} 
			}

			if(TabPos == 0)
			{  
				MessageBox(NULL, str, "Can't parse String", MB_OK);

				continue;
			}

			cont = (char*)(str + TabPos + 1);

			for(int i = 0; i < 128; i++)
			{   
				if(cont[i] == '\t' || cont[i] == '\n' || cont[i] == 0)
				{ 
					cont[i] = 0;

					break;
				} 
			}

			int len = 0;
			len = (int)strlen(part1);

			if(len <= 0 || len >= 16)
			{  
				MessageBox(NULL, str, "Empty or Long String-1st", MB_OK);

				continue;
			}

			len = (int)strlen(cont);

			if(len <= 0 || len >= 16)
			{  
				MessageBox(NULL, str, "Empty or Long String-2nd", MB_OK);

				continue;
			}


			if(Index < 0 || Index >= MAX_STRING) 
			{
				MessageBox(NULL, str, "Too much items in mobname.txt", MB_OK);

				break;
			}

			strcpy(g_pEnglish[Index][0], part1);
			strcpy(g_pEnglish[Index][1], cont);
			strcpy(g_pEnglish[Index][2], part1);

			Index++;
		}
		
	}
	else
	{
		while(1)
		{
			char *ret = fgets((char*)str, 1024, fp);

			if(ret == NULL)
				break;

			sscanf(str, "%s", part1);

			str[510] = 0;
			str[511] = 0;

			int TabPos = 0;

			for(int i = 0; i < 256; i++)
			{   
				if(str[i] == '\t') 
				{
					TabPos = i;

					break;
				} 
			}

			if(TabPos == 0)
			{  
				MessageBox(NULL, str, "Can't parse String", MB_OK);

				continue;
			}

			cont = (char*)(str + TabPos + 1);

			for(int i = 0; i < 128; i++)
			{   
				if(cont[i] == '\t' || cont[i] == '\n' || cont[i] == 0)
				{ 
					cont[i] = 0;

					break;
				} 
			}

			int len = 0;
			len = strlen(part1);

			if(len <= 0 || len >= NAME_LENGTH)
			{  
				MessageBox(NULL, str, "Empty or Long String-1st", MB_OK);

				continue;
			}

			len = (int)strlen(cont);

			if(len <= 0 || len >= NAME_LENGTH)
			{  
				MessageBox(NULL, str, "Empty or Long String-2nd", MB_OK);

				continue;
			}


			if(Index < 0 || Index >= MAX_STRING) 
			{
				MessageBox(NULL, str, "Too much items in mobname.txt", MB_OK);

				break;
			}


			for(Index = 0; Index < MAX_STRING; Index++)
			{
				if(!strcmp(part1, g_pEnglish[Index][0]))
					break;

				if(!strcmp(part1, g_pEnglish[Index][1]))
					break;
			}

			if(Index == MAX_STRING)
			{
                MessageBoxA(NULL, str, "Can't Find match mobname", MB_OK);

                break;
			}

			strcpy(g_pEnglish[Index][2], cont);

		}
	}
}

void BASE_WriteMessageBin()
{
	int size = sizeof(char) * MAX_STRING * 128;
	char *temp = (char *)g_pMessageStringTable;

	for(int i = 0; i < size; i++)
	{ 
		temp[i] = temp[i] ^ 0x5A;
	}

	FILE *fp = fopen("UI/strdef.bin", "wb");

	if(fp != NULL)
	{	
		fwrite(g_pMessageStringTable, sizeof(char), MAX_STRING * 128, fp);

		int sum = BASE_GetSum((char*)g_pMessageStringTable, size);
		fwrite(&sum, sizeof(sum), 1 , fp);

		fclose(fp);
	}

	for(int i = 0; i < size; i++)
	{
		temp[i] = temp[i] ^ 0x5A;
	}

}

int BASE_ReadMessageBin()
{
	memset(g_pMessageStringTable, MAX_STRING, 128);

	int size = sizeof(char) * MAX_STRING * 128;
	char *temp = (char *)g_pMessageStringTable;
	int tsum = 0;

	FILE *fp = fopen("UI/strdef.bin", "rb");

	if(fp != NULL)
	{
		fread(g_pMessageStringTable, size , 1, fp);
		fread(&tsum, sizeof(tsum), 1, fp);

		fclose(fp);
	}

	int sum = BASE_GetSum((char*)g_pMessageStringTable, size);
	/*
	if(sum != tsum) 
		return FALSE;
	*/
	for(int i = 0; i < size; i++)
	{ 
		temp[i] = temp[i] ^ 0x5A;
	}

	return TRUE;
}

void BASE_InitializeItemList()
{
	int size = sizeof(STRUCT_ITEMLIST)*MAX_ITEMLIST;

	memset(g_pItemList, 0, size);

	int ret = 1;

	ret = BASE_ReadItemListFile("../../Common/itemlist.csv", 1);

	if(ret == FALSE)
	{
		MessageBox(NULL, "There is no file.", "itemlist.csv", MB_OK);

		return;
	}

	char extraitem[128];

	strcpy(extraitem, "../../Common/extraitem.csv");

	ret = BASE_ReadItemListFile(extraitem, 0);
}

int BASE_ReadItemListFile(char *filename, int Build)
{
	FILE *wfp = NULL;

	if(Build != 0)
	{	
		BASE_WriteItemList(1);

		wfp =  fopen("../../Common/ExtraItem.bin", "wb");	// mesmo nome que a leitura: no Linux a caixa conta

		if(wfp == NULL)
		{
			MessageBox(NULL, "Can't write ExtraItem.bin", "ERROR", MB_OK);

			return FALSE;
		}
	}

	char temp[256];

	sprintf(temp, "%s", filename);

	FILE *fp = fopen(temp, "rb");

    if(fp == NULL) 
	{
		sprintf(temp, "../../TMSRV/Run/%s", filename);
		
		fp = fopen(temp, "rb");
	}

	if(fp == NULL)
		return FALSE;

	while(1)
	{
		int  Index = -1;

      	char Name[256];

		memset(Name, 0, 256);

	    char nE[MAX_STATICEFFECT][256];
		memset(nE, 0, sizeof(nE));

	    int  vE[MAX_STATICEFFECT];
		memset(vE, 0, sizeof(vE));

		int ve = sizeof(vE);
		int ne = sizeof(nE);

		char str[1024];
		char *ret = fgets((char*)str, 1024, fp);

		if(ret == NULL)
			break;

		for(int i = 0; i < 1024; i++)
		{
			if (str[i] == ',')
				str[i] = ' ';
		}

		char temp1[256];
		temp1[0] = 0;

		char temp2[256];
		temp2[0] = 0;

		int unique = 0;
		int nprice = 0;
		int npos = 0;
		int	Grade = 0;
		int	Extra = 0;

		sscanf(str, "%d %s %s %s %d %d %d %d %d %s %d %s %d %s %d %s %d %s %d %s %d %s %d %s %d %s %d %s %d %s %d %s %d", &Index, Name, temp1, temp2, &unique, &nprice, &npos, &Extra, &Grade, 
			nE[0], &vE[0], nE[1], &vE[1], nE[2], &vE[2], nE[3], &vE[3], nE[4], &vE[4], nE[5], &vE[5], nE[6], &vE[6], nE[7], &vE[7], nE[8], &vE[8], nE[9], &vE[9], nE[10], &vE[10], nE[11], &vE[11]);

		if(Index == -1)
			continue;

		if(Index >= MAX_ITEMLIST)
		{
			MessageBox(NULL, str, "check MAX_ITEMLIST", MB_OK);

			continue;
		}

		int len = strlen(Name);

        if (len >= ITEMNAME_LENGTH-1)
		{
			MessageBox(NULL, "too long Item Name", Name, MB_OK);

			Name[ITEMNAME_LENGTH-1] = 0;
			Name[ITEMNAME_LENGTH-2] = 0;
		}

		strcpy(g_pItemList[Index].Name, Name);
 
		
		int  Mesh = 0;
	    int  Texture = 0;
		int  ReqLvl = 0;
        int  ReqStr = 0;
		int  ReqInt = 0;
		int  ReqDex = 0;
		int  ReqCon = 0;

		if(temp1[0] == '.')
			temp1[0] = 32;
		
		if(temp2[0] == '.') 
			temp2[0] = 32;

		sscanf(temp1, "%d.%d", &Mesh, &Texture);
		sscanf(temp2, "%d.%d.%d.%d.%d", &ReqLvl, &ReqStr, &ReqInt, &ReqDex, &ReqCon);

		g_pItemList[Index].nIndexMesh          = Mesh;
		g_pItemList[Index].nIndexTexture       = Texture;
		g_pItemList[Index].nReqLvl             = ReqLvl;
	    g_pItemList[Index].nReqStr             = ReqStr;
	    g_pItemList[Index].nReqInt             = ReqInt;
	    g_pItemList[Index].nReqDex             = ReqDex;
	    g_pItemList[Index].nReqCon             = ReqCon;
		g_pItemList[Index].nUnique             = unique;
		g_pItemList[Index].nPrice              = nprice;
		g_pItemList[Index].nPos                = npos;
		g_pItemList[Index].nGrade			   = Grade;
		g_pItemList[Index].nExtra			   = Extra;

   	    g_pItemList[Index].nIndexVisualEffect  = 0;


        for(int j = 0; j < MAX_STATICEFFECT; j++)
		{
			if(nE[j][0] == 0)
				continue;

			int v;

		    for(v = 0; v < MAX_EFFECTINDEX; v++)
			{
				if (!strcmp(EffectNameTable[v], nE[j]))
					break;
			}

		    if (j == MAX_EFFECTINDEX)
			{
				MessageBox(0, nE[j], "Unregisterd effect", 0);

                continue;
			}   

			g_pItemList[Index].stEffect[j].sEffect = v;
			g_pItemList[Index].stEffect[j].sValue  = vE[j];
		}

		if(Build != 0)
		{
			fwrite(&Index, 2, 1, wfp);
			fwrite(&g_pItemList[Index], sizeof(STRUCT_ITEMLIST), 1, wfp);
		}
	}

	fclose(fp);

	if(Build != 0)
	{
		fclose(wfp);
		BASE_WriteItemList(0);
	}

	return TRUE;
}

void BASE_InitializeEffectName()
{
	for(int i = 0; i < MAX_EFFECTINDEX; i++)
		EffectNameTable[i][0] = 0;

	FILE *fp = NULL;

	fp = fopen("../../Common/ItemEffect.h", "rb");

	if(fp == NULL)
		fp = fopen("../../TMSRV/Run/ItemEffect.h", "rb");

	if(fp == NULL)
	{
		MessageBoxA(NULL, "There is no file", "ItemEffect", MB_OK);

		return;
	}

	char temp[256];

	char str5[256];
	char str4[256];
	char str3[256];
	char str2[256];
	char str1[256];

	while(1)
	{
		char *ret = fgets(temp, 255, fp);

		if(ret == NULL)
			break;

		if(temp[0] == '\r')
			continue;

		str1[0] = 0;

		sscanf(temp, "%s", str1);

		if(strcmp(str1, "#include") == 0)
		{
			sscanf(temp, "%s %s", str1, str2);

			str2[1 + strlen(str2)] = 0;

			BASE_InitializeEffectName();

			continue;
		}

		else if(strcmp(str1, "#define") != 0)
			continue;

		sscanf(temp, "%s %s %s %s %s", str1, str2, str3, str4, str5);

		int val = atoi(str3);

		if(val < 0 || val > MAX_EFFECTINDEX)
		{
			MessageBox(NULL, temp,"Effect.hÀÇ Define Value°¡ ¼ýÀÚ°¡ ¾Æ´Ï°Å³ª 0ÀÌÇÏ ¶Ç´Â MAX_EFFECTINDEXÀÌ»óÀÌ´Ù", MB_OK);

			continue;
		}

		if(EffectNameTable[val][0] != 0)
		{
			MessageBox(NULL, temp, EffectNameTable[val], MB_OK);

			continue;
		}
		
		memcpy(EffectNameTable[val], str2, 24);

		continue;
	}

	fclose(fp);
}

void BASE_InitializeHitRate()
{
	// Regra do cliente: cada passada visita os índices de Jump em Jump (o servidor percorria todos com
	// i++, e a tabela saía em outra ordem).
	memset(g_pHitRate, 0, sizeof(g_pHitRate));

	int Jump = 512;
	int Start = 0;
	int Quad = 0;

	do
	{
		for (int i = 0; i < 1024; i += Jump)
		{
			if (g_pHitRate[i])
				continue;

			if (Quad == 0)
				g_pHitRate[i] = Start;
			else if (Quad == 1)
				g_pHitRate[i] = 512 - Start;
			else if (Quad == 2)
				g_pHitRate[i] = Start + 512;
			else
				g_pHitRate[i] = 1024 - Start;

			if (g_pHitRate[i] > 999)
				g_pHitRate[i] = 999;

			if (++Quad >= 4)
				Quad = 0;

			if (Quad == 0)
				++Start;
		}

		Jump /= 2;
	} while (Jump);

	g_pHitRate[0] = 512;
}

void BASE_WriteInitItem()
{
	int size = sizeof(STRUCT_INITITEM) * MAX_INITITEM;
	char *temp = (char *)g_pInitItem;

	for(int i = 0; i < size; i++)
	{
		temp[i] = temp[i] ^ 0xFF;
	} //STRUCT_INITITEM g_pInitItem[MAX_INITITEM];

	FILE *fp = fopen("../../Common/InitItem.bin", "wb");

	if(fp == NULL) 
	{
		MessageBox(NULL, "Can't write inititem.bin", "ERROR", MB_OK | MB_SYSTEMMODAL);

		return;
	}

	fwrite(g_pInitItem, size,1, fp);

	fclose(fp);

	for(int i = 0; i < size; i++)
	{
		temp[i] = temp[i] ^ 0xFF;
	}
}

int BASE_ReadInitItem()
{
	char szPathName[256];
	char szFileName[256];

	memset(szPathName, 0, 256);
	memset(szFileName, 0, 256);

	GetModuleFileName(NULL, szPathName, 256);

	int len = strlen(szPathName);

	for(int nIndex = len-1; nIndex > 0; nIndex--)
	{
		if(szPathName[nIndex]=='\\' || szPathName[nIndex]=='/')
		{
			sprintf(szFileName, "%s", &szPathName[nIndex+1]);

			break;
		}
	}

#ifdef _ADMIN_TOOL
#ifdef _CHINESESIMPLE
	sprintf(szFileName, "WYDSC.EXE");
#elif _CHINESETRADITION
	sprintf(szFileName, "WYDTC.EXE");
#else
	sprintf(szFileName, "WYD.EXE");
#endif
#endif

	int size = sizeof(STRUCT_INITITEM) * MAX_INITITEM;
	char *temp = (char *)g_pInitItem;

	FILE *fp = fopen(szFileName, "rb");

	if(fp == NULL) 
	{
		MessageBox(NULL, "Can't read inititem", "ERROR", MB_OK | MB_SYSTEMMODAL);

		return FALSE;
	}

	fseek(fp, -size, SEEK_END);
	fread(g_pInitItem, size, 1, fp);

	fclose(fp);

	for(int i = 0; i < size; i++)
	{ 
		temp[i]= temp[i] ^ 0xFF;
	}

	for(int i = 0; i < MAX_INITITEM; i++)
	{
		if(g_pInitItem[i].PosX <= 0)
		{
			g_dwInitItem = i;

			break;
		}
	}

	return TRUE;
} 

void BASE_InitializeInitItem()
{
	FILE *fp = NULL;

	fp = fopen("../../Common/InitItem.csv", "rb");

	if(fp == NULL)
		fp = fopen("../../TMSRV/Run/InitItem.csv", "rb");

	if(fp == NULL)
	{
		MessageBox(NULL, "There is no file", "InitItem.csv", MB_OK);

		return;
	}
	
	char str[1024];

	g_dwInitItem = 0;

	while(1)
	{
		char *ret = fgets(str, 1024, fp);

		if(ret == 0)
			break;

		for(int i = 0; i < 1024; i++)
		{
			if(str[i] == ',')
				str[i] = ' ';
		}

		int Index = -1;
		int PosX = 0;
		int PosY = 0;
		int Rotate = 0;

		sscanf(str, "%d %d %d %d", &Index, &PosX, &PosY, &Rotate);

		if(Index == -1)
			continue;

		g_pInitItem[g_dwInitItem].PosX = PosX;
		g_pInitItem[g_dwInitItem].PosY = PosY;
		g_pInitItem[g_dwInitItem].sIndex = Index;
		g_pInitItem[g_dwInitItem].Rotate = Rotate;

		g_dwInitItem++;
	}

	fclose(fp);
}

void BASE_InitializeSkill()
{
     FILE *fp  =  fopen("../../Common/SkillData.csv", "rb");

     if(fp == NULL)
		 fp = fopen("../../TMSRV/Run/SkillData.csv", "rb");

	 if(fp == NULL)
	 {
		 MessageBox(NULL, "There is no file", "SkillData.csv", NULL);

		 return;
	 }	

	 char str[1024];

     memset(g_pSpell, 0, sizeof(g_pSpell));


	 while(1)
	 {
		char * ret = fgets((char*)str, 1024, fp);

		if(ret == NULL)
			break;     

		for(int i = 0; i < 1024; i++)
		{
			if (str[i] == ',')
				str[i] = ' ';

		    if(str[i] == -0)
				break;
		}

		STRUCT_SPELL temp;
		
		memset(&temp, 0, sizeof(temp));

        int Index = -1;
        char act1[256] = "", act2[256] = "";

		sscanf(str, "%d %d %d %d %d %d %d %d %d %d %d %d %d %s %s %d %d %d %d %d %d %d", 
								&Index,				&temp.SkillPoint,	&temp.TargetType,	&temp.ManaSpent,	&temp.Delay,
								&temp.Range,		&temp.InstanceType,	&temp.InstanceValue,&temp.TickType,		&temp.TickValue,
								&temp.AffectType,   &temp.AffectValue, &temp.AffectTime,    act1,				act2, &temp.InstanceAttribute,
								&temp.TickAttribute,&temp.Aggressive,	&temp.MaxTarget,	&temp.bParty,		&temp.AffectResist,&temp.Passive);
		
		// A tabela tem as 248 entradas do cliente; o combate só aceita as primeiras MAX_SKILLINDEX.
		if(Index < 0 || Index >= MAX_SPELL_LIST)
			continue;

		temp.AffectTime = temp.AffectTime / 4;

		// As duas colunas de movimentos ("1.2.3.4.5.6"), como Act1 e Act2 do cliente. Antes as duas iam
		// para o mesmo buffer e só a segunda ficava. O memset acima zera o resto.
		sscanf(act1, "%hhd.%hhd.%hhd.%hhd.%hhd.%hhd", &temp.Act1[0], &temp.Act1[1], &temp.Act1[2], &temp.Act1[3], &temp.Act1[4], &temp.Act1[5]);
		sscanf(act2, "%hhd.%hhd.%hhd.%hhd.%hhd.%hhd", &temp.Act2[0], &temp.Act2[1], &temp.Act2[2], &temp.Act2[3], &temp.Act2[4], &temp.Act2[5]);

		g_pSpell[Index] = temp;
	 }

	 fclose(fp);

	 return;
}

void BASE_InitializeClientGuildName(int group)
{
	FILE *fp = NULL;

	fp = fopen("../../Common/Guilds.txt", "rb");

	if(fp == NULL)
		fp = fopen("./Guilds.txt", "rb");

	if(fp == NULL)
		fp = fopen("../../TMSRV/Run/Guilds.txt", "rb");

	if(fp == NULL)
		return;

	memset(g_pClientGuildName, 0, sizeof(g_pClientGuildName));

	while(1)
	{
		int Group = -1;
		int Server = -1;
		int Guild = -1;

		char szGuild[4][4096];
		char str[1024];


		char *ret = fgets((char*)str, 1024, fp);

		if(ret == NULL)
			break;

		sscanf(str, "%d %d %d %s %s %s %s", &Group, &Server, &Guild, szGuild[0], szGuild[1], szGuild[2], szGuild[3]);

		if((Group < 0 || Group >= MAX_SERVERGROUP || Server < 0 || Server >= 16) || (Guild < 0 || Guild >= MAX_GUILD) || szGuild[0][0] == 0)
		{
			MessageBoxA(NULL, str, "Can`t parse string in Guilds.txt", MB_OK);
			continue;
		}

		if(Group != group)
			continue;

		for(int i = 0; i < 4; i++)
		{
			int len = strlen(szGuild[i]);

			if(len > 11)
			{
				szGuild[i][11] = 0;
				len = 11;
			}
			else if(len > 0)
			{
				if(!IsClearString3(szGuild[i], len - 1))
					szGuild[i][len - 1] = 0;
			}

			strcpy(g_pClientGuildName[Server][Guild][i], szGuild[i]);
		}
	}

	fclose(fp);

	CurrentClientGuild = group;
}

void BASE_InitializeGuildName()
{
	FILE *fp = fopen("../../Common/Guilds.txt", "rb");

	if(fp == NULL)
		fp = fopen("./Guilds.txt", "rb");

	if(fp == NULL)
		fp = fopen("../../TMSRV/Run/Guilds.txt", "rb");

	if(fp == NULL)
	{
		MessageBox(0, "can't find guilds.txt", "error", MB_OK | MB_SYSTEMMODAL);
		memset(g_pGuildName, 0, sizeof(g_pGuildName));

		return;
	}

	int nSize = sizeof(g_pGuildName);

	memset(g_pGuildName, 0, nSize);

	char str[1024];

	while(1)
	{
		int Group = -1;
		int Server = -1;
		int Guild = -1;
		char szGuild[256] = {0,};

		char *ret = fgets(str, 1024, fp);

		if(ret == NULL)
			break;

		sscanf(str, "%d %d %d %s", &Group, &Server, &Guild, szGuild);

		if((Group < 0 || Group >= MAX_SERVERGROUP) || (Server < 0 || Server >= 16) || (Guild < 0 || Guild >= MAX_GUILD) || szGuild[0] == 0)
		{
			MessageBox(0, str, "Can't parse string in Guilds.txt", 0);

			break;
		}

		int len = strlen(szGuild);

		if(len > GUILDNAME_LENGTH-1)
		{
			szGuild[GUILDNAME_LENGTH-1] = 0;
			len = GUILDNAME_LENGTH-1;
		}

		if(len > 0)
		{
			if(!IsClearString3(szGuild, len - 1))
				ret[3 + len] = 0;
		}

		strncpy(g_pGuildName[Group][Server][Guild], szGuild, GUILDNAME_LENGTH);
	};

	fclose(fp);
}

int BASE_InitializeServerList()
{
	FILE *fp = NULL;

	fp = fopen("./serverlist.txt", "rt");

	if(fp == NULL)
		fp = fopen("../../Common/serverlist.txt", "rt");

	if(fp == NULL)
	{
		MessageBoxA(NULL, "Can't open server list.txt", "adress", MB_OK);

		return FALSE;
	}
	
	memset(g_pServerList, 0, sizeof(g_pServerList));

	char str[256];
	char address[64];

	int ServerGroup;
	int ServerNumber;

	while(1)
	{
		char *ret = fgets((char*)str, 255, fp);

		if(ret == NULL)
			break;

		ServerGroup = -1;
		ServerNumber = -1;

		address[0] = 0;

		sscanf(str, "%d %d %s", &ServerGroup, &ServerNumber, address);

		if((ServerGroup < 0 ||ServerGroup >= MAX_SERVERGROUP) || (ServerNumber < 0 || ServerNumber > MAX_SERVERNUMBER))
			  break;

		strcpy(g_pServerList[ServerGroup][ServerNumber], address);
	};

	fclose(fp);

	FILE *fpBin = fopen("./serverlist.bin", "w");

	if(fp != NULL)
	{
		int szList[64] = {
			0xA4, 0xA1, 0xA4, 0xA4, 0xA4, 0xA7, 0xA4, 0xA9, 0xA4, 0xB1, 0xA4, 0xB2, 0xA4, 0xB5, 0xA4, 0xB7, 
			0xA4, 0xB8, 0xA4, 0xBA, 0xA4, 0xBB, 0xA4, 0xBC, 0xA4, 0xBD, 0xA4, 0xBE, 0xA4, 0xBF, 0xA4, 0xC1, 
			0xA4, 0xC3, 0xA4, 0xC5, 0xA4, 0xC7, 0xA4, 0xCB, 0xA4, 0xCC, 0xA4, 0xD0, 0xA4, 0xD1, 0xA4, 0xD3, 
			0xA4, 0xBF, 0xA4, 0xC4, 0xA4, 0xD3, 0xA4, 0xC7, 0xA4, 0xCC, 0xB0, 0xA1, 0xB3, 0xAA, 0xB4, 0xD9
		};

		for(int k = 0; k < MAX_SERVERGROUP; k++)
		{
			for(int j = 0; j < MAX_SERVERNUMBER; j++)
			{
				for(int i = 0; i < 64; i++)
				{
					g_pServerList[k][j][i] += szList[63-i];

				}
			}
		}

		fwrite(g_pServerList, MAX_SERVERGROUP*MAX_SERVERNUMBER, 64, fpBin);
		fclose(fpBin);

		for(int k = 0; k < MAX_SERVERGROUP; k++)
		{
			for(int j = 0; j < MAX_SERVERNUMBER; j++)
			{
				for(int i = 0; i < 64; i++)
				{
					g_pServerList[k][j][i] -= szList[63-i];
				}
			}
		}

	}

	return TRUE;
		
}

int BASE_InitializeAttribute()
{
	int tsum = 0;
	int sum = 0;

	char FileName[128];

	strcpy(FileName, "./AttributeMap.dat");

	FILE *fp = NULL;

	fp = fopen(FileName, "rb");

	if (fp == NULL)
		fp = fopen("../../TMSRV/Run/AttributeMap.dat", "rb");
	
	if(fp == NULL)
	{
		MessageBoxA(NULL, "There is no file", "AttributeMap.dat", MB_OK);

		return FALSE;
	}

	fread(g_pAttribute, 1024, 1024, fp);

	fclose(fp);

	return TRUE;
}
