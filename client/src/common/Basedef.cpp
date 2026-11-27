#include "core/pch.h"
#include "common/Basedef.h"
#include "core/TMGlobal.h"
#include "core/TMLog.h"
#include "common/ItemEffect.h"
#include <WinInet.h>

char g_pAffectTable[MAX_EFFECT_STRING_TABLE][24];
char g_pAffectSubTable[MAX_SUB_EFFECT_STRING_TABLE][24];
int g_pHitRate[1024];

HWND hWndMain;
char EncodeByte[4];
int g_nChannelWidth;
int g_nServerGroupNum;
char g_pServerList[MAX_SERVERGROUP][MAX_SERVERNUMBER][64];
int g_nSelServerWeather;
char g_pMessageStringTable[MAX_STRING][MAX_STRING_LENGTH];
STRUCT_ITEMLIST g_pItemList[MAX_ITEMLIST];
STRUCT_SPELL g_pSpell[MAX_SPELL_LIST];
STRUCT_INITITEM g_pInitItem[100];
int g_itemicon[6500];

STRUCT_GUILDZONE g_pGuildZone[MAX_GUILDZONE] =
{
    {0, 0, 2088, 2148, 2086, 2093, 2052, 2052, 2171, 2163, 197, 213, 238, 230, 205, 220, 228, 220, 5, 0}, // Armia
    {0, 0, 2531, 1700, 2494, 1707, 2432, 1672, 2675, 1767, 197, 149, 238, 166, 205, 157, 228, 157, 5, 0}, // Azran
    {0, 0, 2460, 1976, 2453, 2000, 2448, 1966, 2476, 2024, 141, 213, 182, 230, 146, 220, 173, 220, 5, 0}, // Erion
    {0, 0, 3614, 3124, 3652, 3122, 3605, 3090, 3690, 3260, 141, 149, 182, 166, 146, 157, 173, 157, 5, 0}, // Nippleheim
    {0, 0, 1066, 1760, 1050, 1706, 1036, 1700, 1072, 1760, 4000, 4000, 4010, 4010, 4005, 4005, 4005, 4005, 5, 0} // Noatum
};

float BASE_ScreenResize(float size)
{
	return (float)((float)g_pDevice->m_dwScreenWidth * (float)(size / 800.0f));
}

void BASE_InitModuleDir()
{
    char String[256]{};
    GetModuleFileName(nullptr, String, sizeof(String));

    for (int i = strlen(String) - 1; i > 0; --i)
    {
        if (String[i] == '\\')
        {
            String[i] = 0;
            break;
        }
    }

    SetCurrentDirectory(String);
}

void BASE_InitializeHitRate()
{
    memset(g_pHitRate, 0, sizeof(g_pHitRate));

    int Jump = 512;
    int Start = 0;
    int Quad = 0;

    do
    {
        for (int i = 0; i < 1024; i += Jump)
        {
            if (!g_pHitRate[i])
            {
                if (!Quad)
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
                if (!Quad)
                    ++Start;
            }
        }
        Jump /= 2;
    } while (Jump);

    g_pHitRate[0] = 512;
}

int BASE_ReadItemList()
{
    FILE* fp = nullptr;

    fopen_s(&fp, ".\\ItemList.bin", "rb");

    if (!fp)
    {
        MessageBoxA(0, "Can't read ItemList.bin", "ERROR", 0);
        return 0;
    }

    int size = sizeof(g_pItemList);

    char* temp = (char*)g_pItemList;

    int tsum{};

    fread(g_pItemList, size, 1u, fp);
    fread(&tsum, 4u, 1u, fp);
    fclose(fp);

    int sum = BASE_GetSum2((char*)g_pItemList, size); // Not being used...

#if !defined _DEBUG
    if (tsum != 0x1343B16)
        return 0;
#endif

    for (int i = 0; i < size; ++i)
        temp[i] ^= 0x5A;

    int Handle = _open(".\\ExtraItem.bin", _O_RDONLY | _O_BINARY, 0);

    if (Handle != -1)
    {
        char buff[256]{};

        while (_read(Handle, buff, sizeof(STRUCT_ITEMLIST) + 2) >= sizeof(STRUCT_ITEMLIST) + 2)
        {
            short idx = *(short*)buff;

            if (idx > 0 && idx < MAX_ITEMLIST)
                memcpy(&g_pItemList[idx], &buff[2], sizeof(STRUCT_ITEMLIST));
        }

        _close(Handle);

        for (int j = 0; j < size; ++j)
            temp[j] ^= 0x5A;

        sum = BASE_GetSum2((char*)g_pItemList, size); // Not being used...

#if !defined _DEBUG
        if (tsum != 0x1343B16)
            return 0;
#endif
        for (int j = 0; j < size; ++j)
            temp[j] ^= 0x5A;
    }
    return 1;
}

int BASE_ReadSkillBin()
{
    int size = sizeof(STRUCT_SPELL) * MAX_SPELL_LIST;
    char* temp = (char*)g_pSpell;
    int tsum = 0;

    FILE* fp = fopen(SkillData_Path, "rb");

    if (fp != NULL)
    {
        fread(g_pSpell, size, 1, fp);
        fread(&tsum, sizeof(tsum), 1, fp);

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

    for (int i = 0; i < size; i++)
    {
        temp[i] = temp[i] ^ 0x5A;
    }

    return TRUE;
}

int BASE_ReadInitItem()
{
    // Check if is really necessarily
    return 1;
}

void BASE_InitialItemRePrice()
{
    g_pItemList[412].nPrice = 4000000;
    g_pItemList[413].nPrice = 8000000;
    g_pItemList[419].nPrice = 400000;
    g_pItemList[420].nPrice = 800000;
}

int BASE_GetSum(char* p, int size)
{
	int sum = 0;
	for (int i = 0; i < size; ++i)
	{
		int mod = i % 7;
		if (!(i % 7))
			sum += p[i] / 2;
		if (mod == 1)
			sum += p[i] ^ 0xFF;
		if (mod == 2)
			sum += 3 * p[i];
		if (mod == 3)
			sum += 2 * p[i];
		if (mod == 4)
			sum -= p[i] / 7;
		if (mod == 5)
			sum -= p[i];
		else
			sum += p[i] / 3;
	}

	return sum;
}

int BASE_GetSum2(char* p, int size)
{
    int sum = 0;

    for (int i = 0; i < size; ++i)
    {
        int mod = i % 9;
        if (mod == 0)
            sum += 2 * p[i];
        if (mod == 1)
            sum += p[i] ^ 0xFF;
        if (mod == 2)
            sum += p[i] / 3;
        if (mod == 3)
            sum += 2 * p[i];
        if (mod == 4)
            sum -= p[i] ^ 0x5A;
        if (mod == 5)
            sum -= p[i];
        else
            sum += p[i] / 5;
    }

    return sum;
}

int BASE_ReadMessageBin()
{
	memset(g_pMessageStringTable, 0, sizeof g_pMessageStringTable);

	int size = 256000;
	FILE* pFile = nullptr;
	fopen_s(&pFile, Strdef_Path, "rb");

	int checksum = 0;	
	if (pFile)
	{
		fread(g_pMessageStringTable, size, 1u, pFile);
		fread(&checksum, 4, 1, pFile);
		fclose(pFile);
	}

	if (checksum != BASE_GetSum((char*)g_pMessageStringTable, size))
		return 0;

	for (int i = 0; i < 2000; ++i)
		for (int k = 0; k < 128; ++k)
			g_pMessageStringTable[i][k] ^= 0x5A;

	return 1;
}

void BASE_InitEffectString()
{
    FILE* fpEffectString = nullptr;
    fopen_s(&fpEffectString, EffectString_Path, "rt");

    if (fpEffectString)
    {
        for (int i = 1; i < MAX_EFFECT_STRING_TABLE; ++i)
            fscanf(fpEffectString, "%24s", &g_pAffectTable[i][0]);
   
        fclose(fpEffectString);
    }

    FILE* fpEffectSubString = nullptr;
    fopen_s(&fpEffectSubString, EffectSubString_Path, "rt");

    if (fpEffectSubString)
    {
        for (int j = 0; j < MAX_SUB_EFFECT_STRING_TABLE; ++j)
            fscanf(fpEffectSubString, "%s", &g_pAffectSubTable[j][0]);

        fclose(fpEffectSubString);
    }

    /* There's a loading of the GuildString.txt file, but is not used */
}

int BASE_InitializeBaseDef()
{
    int ret = 0;
	ret = BASE_InitializeServerList() & 1;
    ret = BASE_ReadSkillBin() & ret;
    ret = BASE_ReadItemList() & ret;
    ret = BASE_ReadInitItem() & ret;
    ret = BASE_InitializeAttribute() & ret;

    BASE_InitialItemRePrice();

	return ret;
}

void BASE_ReadItemPrice()
{
    int itemprice[MAX_ITEM_PRICE_REPLACE][2]{};

    FILE* fp = nullptr;
    fopen_s(&fp, ItemPrice_Path, "rb");

    if (fp)
    {
        fread(itemprice, sizeof(itemprice), 1, fp);
        fclose(fp);
    }

    for (int k = 0; k < MAX_ITEM_PRICE_REPLACE && itemprice[k][0]; ++k)
    {
        int idx = itemprice[k][0];
        int bufprice = g_pItemList[idx].nPrice;
        g_pItemList[idx].nPrice = itemprice[k][1];   
    }
}

void BASE_UnderBarToSpace(char* szStr)
{
	while (*szStr)
	{
		if (*szStr == '_')
			*szStr = ' ';

		++szStr;
	}
}

int BASE_ParseUserCount(const char* szPage, int* pnUserCount, int nMax, int* pnWeek, int* pnDay)
{
    for (int i = 0; i < nMax; ++i)
        pnUserCount[i] = -1;
    if (pnWeek)
        *pnWeek = -1;
    if (pnDay)
        *pnDay = -1;

    if (szPage == nullptr || nMax < 1)
        return 0;

    // A página do DBSrv (CReadFiles::UpdateConnection) traz uma linha por canal, a linha k para o
    // canal k + 1, e opcionalmente a semana e o dia do castelo. As cenas indexam a contagem pelo
    // número do canal (1 a MAX_SERVER), então a linha k vai para pnUserCount[k + 1].
    const int nChannels = nMax - 1;
    const char* szCursor = szPage;
    int nRead = 0;
    for (int k = 0; k < nChannels + 2; ++k)
    {
        char* szEnd = nullptr;
        long nValue = strtol(szCursor, &szEnd, 10);
        if (szEnd == szCursor)
            break;

        szCursor = szEnd;
        if (k < nChannels)
            pnUserCount[k + 1] = static_cast<int>(nValue);
        else if (k == nChannels && pnWeek)
            *pnWeek = static_cast<int>(nValue);
        else if (k == nChannels + 1 && pnDay)
            *pnDay = static_cast<int>(nValue);
        ++nRead;
    }

    return nRead;
}

int BASE_GetHttpRequest(char* httpname, char* Request, int MaxBuffer)
{   
    auto hSession = InternetOpen("MS", 0, 0, 0, 0);
    if (!hSession)
        return 0;

    auto hHttpFile = InternetOpenUrl(hSession, httpname, 0, 0, 0x4000000u, 0);

    if (!hHttpFile)
    {
        GetLastError();
        InternetCloseHandle(hSession);
        return 0;
    }

    DWORD dwBytesRead = 0;
    InternetReadFile(hHttpFile, Request, MaxBuffer, &dwBytesRead);
    InternetCloseHandle(hHttpFile);
    if (dwBytesRead >= 1024)
        dwBytesRead = 1023;

    Request[dwBytesRead] = 0;
    InternetCloseHandle(hSession);

    return 1;
}

int BASE_GetWeekNumber()
{
	time_t now;
	time(&now);

	unsigned int week = 86400;
	return (int)(now / week - 3);
}

int ReadItemicon()
{
    FILE* fpBin = nullptr;
    fopen_s(&fpBin, ItemIcon_Path, "rb");
    if (fpBin)
    {
        fread(g_itemicon, sizeof(g_itemicon), 1u, fpBin);
        fclose(fpBin);
    }

    return 1;
}

void ReadItemName()
{
    FILE* fpBin = nullptr;
    fopen_s(&fpBin, ItemName_Path, "rb");

    if (!fpBin)
        return;

    for (int i = 0; i < MAX_ITEMLIST; ++i)
    {
        int Index = -1;
        char Name[256]{};

        if (!fread(&Index, 4, 1, fpBin) || !fread(Name, 64, 1, fpBin))
            break;

        for (int nTemp = 0; nTemp < 62; ++nTemp)
            Name[nTemp] -= nTemp;

        if (Index != -1 && Index < 6500)
        {
            if (strlen(Name) >= 63)
            {
                Name[63] = 0;
                Name[62] = 0;
            }

            strcpy(g_pItemList[Index].Name, Name);
        }
    }

    fclose(fpBin);
}

void ReadUIString()
{
	FILE* pFile = nullptr;
	fopen_s(&pFile, UIString_Path, "rt");

	if (pFile)
	{
		char str[128]{ 0 };
		while (fgets(str, sizeof str, pFile))
		{
			char part[64]{ 0 };
			int Index = 0;

			int ret = sscanf_s(str, "%d %s", &Index, part, 64);
			if (ret != 2)
				continue;

			strncpy_s(g_UIString[Index], part, sizeof part);
		}

		fclose(pFile);
	}
}

char ReadNameFiltraDataBase()
{
	return 1;
}

char ReadChatFiltraDataBase()
{
	return 1;
}

void EnableSysKey()
{
}

bool CheckOS()
{
	return false;
}

void DisableSysKey()
{
}

int IsClearString(char* str, int target)
{
	int len = strlen(str);
	for (int pos = 0; pos < len; ++pos)
	{
		if (str[pos] >= 0)
		{
			if (pos >= target)
				return 1;
		}
		else
		{
			if (pos == target)
				return 0;
			if (pos == target + 1)
				return 1;

			++pos;
		}
	}

	return 1;
}

int IsClearString2(char* str, int nTarget)
{
	if (!str)
		return 1;

	char* pNextRightChar = CharNext(&str[nTarget]);
	int nLen = pNextRightChar - &str[nTarget];
	int nLen2 = pNextRightChar - CharPrev(str, pNextRightChar);

	if (nLen == 1 && nLen2 == 2)
		return 0;

	if (nLen != 2 || nLen2 != 1)
		return 1;

	return 0;
}

int BASE_InitializeServerList()
{
	FILE* fpBin = nullptr;
	fopen_s(&fpBin, "./serverlist.bin", "rb");

	if (fpBin)
	{
		char szList[65] = { "¤¡¤¤¤§¤©¤±¤²¤µ¤·¤¸¤º¤»¤¼¤½¤¾¤¿¤Á¤Ã¤Å¤Ç¤Ë¤Ì¤Ð¤Ñ¤Ó¤¿¤Ä¤Ó¤Ç¤Ì°¡³ª´Ù"};

		memset(&g_pServerList, 0, sizeof g_pServerList);
		fread(g_pServerList, 0x6E, 0x40u, fpBin);
		fclose(fpBin);

		for (int k = 0; k < MAX_SERVERGROUP; k++)
			for (int j = 0; j < MAX_SERVERNUMBER; j++)
				for (int i = 0; i < 64; i++)
					g_pServerList[k][j][i] -= szList[63 - i];
		return 1;
	}

	return 0;
}

char BASE_CheckValidString(char* name)
{
    int size = strlen(name);
    if (size < 4 || size >= 16)
        return 0;

    for (int j = 0; j < size; ++j)
    {
        char x = name[j];
        if (x < 0)
        {
            if (!name[++j])
                return 0;
        }
        else if ((x < 'a' || x > 'z') && (x < 'A' || x > 'Z') && (x < '0' || x > '9') && x != '-')
            return 0;
    }

    return 1;
}

char* BASE_TransCurse(char* sz)
{
    if (sz == nullptr)
        return 0;
    
    bool bFind = false;
    do
    {
        bFind = false;
        for (size_t i = 0; i < g_pCurseList.dnum; ++i)
        {
            if (strlen(g_pCurseList.pCurseList[i].szOriginal) == 0)
                return sz;

            char* szDest = strstr(sz, g_pCurseList.pCurseList[i].szOriginal);
            if (szDest == nullptr)
                continue;

            int nOriLen = strlen(g_pCurseList.pCurseList[i].szOriginal);
            if (!IsClearString2(sz, szDest - sz))
                continue;

            char szNext[128]{};
            memcpy(szNext, sz, szDest - sz);
            strcat(szNext, g_pCurseList.pCurseList[i].szTrans);
            strcat(szNext, &szDest[nOriLen]);
            sprintf(sz, szNext);
            bFind = true;
            break;
        }
    } while (bFind == true);
    return sz;
}

int BASE_ReadTOTOList(char* szFileName)
{
    return 0;
}

char BASE_CheckChatValid(const char* Chat)
{
    return 1;
}

char CheckGuildName(const char* GuildName, bool bSubguild)
{
    int nLen = strlen(GuildName);
    if (!bSubguild)
    {
        for (int i = 0; i < nLen; ++i)
        {
            if (GuildName[i] == ' ')
                return 0;

            if (GuildName[i] == '_')
                return 0;
        }
    }

    if (nLen < 2)
        return 1;

    for (int i = 1; i < nLen; ++i)
        if (GuildName[i] == -95 && GuildName[i - 1] == -95)
            return 0;

    return 1;
}
