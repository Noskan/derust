#pragma once

/// @file System.h
/// @brief Serviços do Linux que os servidores usam: listagem de diretório e troca de nome de
///        arquivo com as regras que o código original espera. A implementação (System.cpp) usa
///        POSIX e só entra no build dos servidores; as regras de server/common não dependem dela.

#include "common/Platform.h"

/// @brief Entrada de diretório devolvida por W2FindFirst/W2FindNext.
struct W2FindData
{
	char name[260];		///< só o nome, sem o diretório
	bool isDir;
	long long size;
	long long writeTime;	///< última gravação, em segundos desde 1970 (UTC)
};

/// @brief Abre a listagem dos nomes que casam com `pattern` (`dir/*` ou `dir/*.ext`; `*.*` casa
///        também nomes sem ponto) e devolve a primeira entrada em `data`.
///
/// O código foi escrito para o FindFirstFile do Windows, que devolve "." e ".." antes dos
/// arquivos. Os laços que pulam nomes começados por ponto contam com isso (o `while (W2FindNext())`
/// do DayLog descarta a primeira entrada, e o limite de 10 entradas por chamada do ImportItem
/// conta as duas), então elas vêm primeiro aqui também. Os demais nomes seguem a ordem do glob().
/// @return o handle da listagem, ou -1 se nada casou.
int W2FindFirst(const char* pattern, W2FindData* data);

/// @brief Próxima entrada da listagem. @return 0, ou -1 no fim (ou com um handle inválido).
int W2FindNext(int handle, W2FindData* data);

/// @brief Fecha a listagem (aceita -1).
void W2FindClose(int handle);

/// @brief Troca o nome de `from` para `to` e falha se `to` já existir, como o MoveFile do
///        Windows (o rename() do POSIX substituiria o destino). Os arquivos que não puderam ser
///        importados vão para a pasta de erro com esse nome, e um segundo com o mesmo nome fica
///        onde está. @return TRUE se moveu; senão FALSE, com o motivo em errno.
BOOL W2MoveFile(const char* from, const char* to);
