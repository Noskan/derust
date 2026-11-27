#pragma once

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

/// Atalho de desenvolvimento: entrada automática no jogo, sem cliques.
///
/// Com a variável de ambiente `W2_AUTOLOGIN=<ip>,<conta>,<senha>,<senha numérica>,<vaga>`, a cena
/// de seleção de servidor conecta no IP dado e envia o login da conta, e a cena de seleção de
/// personagem envia a senha numérica e entra com o personagem da vaga (0 a 3). As cenas chamam o
/// mesmo código dos botões (`B_LOGIN_OK`, o OK da senha numérica e o botão de entrar), então o
/// caminho de rede é o do jogador. Sem a variável, nada muda.
///
/// Serve para medir o cliente com dados novos sob o wine/Xvfb, onde clicar às cegas é frágil.
struct TMAutoLogin
{
	char szIP[64];
	char szID[16];
	char szPW[16];
	char szNumericPW[10];
	int nSlot;
};

/// Copia o próximo campo de `*pszCursor` (até a vírgula ou o fim) para `szOut`, truncando em
/// `nSize - 1`, e avança o cursor. Devolve 0 quando não há mais campos.
inline int TMAutoLogin_NextField(const char** pszCursor, char* szOut, size_t nSize)
{
	const char* szStart = *pszCursor;
	if (szStart == nullptr)
		return 0;

	const char* szComma = strchr(szStart, ',');
	size_t nLen = szComma ? (size_t)(szComma - szStart) : strlen(szStart);
	if (nLen >= nSize)
		nLen = nSize - 1;

	memcpy(szOut, szStart, nLen);
	szOut[nLen] = 0;
	*pszCursor = szComma ? szComma + 1 : nullptr;
	return 1;
}

/// Lê `W2_AUTOLOGIN`. Devolve 1 quando a variável existe, tem os cinco campos e a vaga é 0 a 3.
inline int TMAutoLogin_Read(TMAutoLogin* pOut)
{
	const char* szEnv = getenv("W2_AUTOLOGIN");
	if (szEnv == nullptr || pOut == nullptr)
		return 0;

	memset(pOut, 0, sizeof(*pOut));

	char szSlot[8]{};
	const char* szCursor = szEnv;
	if (!TMAutoLogin_NextField(&szCursor, pOut->szIP, sizeof(pOut->szIP)) ||
		!TMAutoLogin_NextField(&szCursor, pOut->szID, sizeof(pOut->szID)) ||
		!TMAutoLogin_NextField(&szCursor, pOut->szPW, sizeof(pOut->szPW)) ||
		!TMAutoLogin_NextField(&szCursor, pOut->szNumericPW, sizeof(pOut->szNumericPW)) ||
		!TMAutoLogin_NextField(&szCursor, szSlot, sizeof(szSlot)))
		return 0;

	pOut->nSlot = atoi(szSlot);
	return pOut->szIP[0] && pOut->szID[0] && pOut->szPW[0] && pOut->nSlot >= 0 && pOut->nSlot < 4;
}
