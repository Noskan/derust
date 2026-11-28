/// @file W2DbClient.h
/// @brief Cliente do w2db (docs/W2DB.md) no DBSrv: contas no Postgres em vez de ./account e ./char.
///
/// Liga quando há um socket configurado: a variável W2DB_SOCKET ou a primeira linha de w2db.txt
/// no diretório do DBSrv. Sem isso, o DBSrv segue nos arquivos.
///
/// As chamadas são síncronas, com prazo (W2DB_DEADLINE_MS), na thread do laço: o w2db roda na
/// mesma máquina, a gravação leva poucos milissegundos e o login (argon2id) algumas dezenas. Para
/// o teste fechado (até 30 jogadores) isso não aparece, e a memória do DBSrv fica sempre igual ao
/// banco depois de cada chamada.
///
/// Estado por conta conectada (versão, seriais e os itens da última gravação), pelo nome: a
/// gravação manda a versão esperada e os seriais de cada vaga. Os seriais ficam no DBSrv e são
/// deduzidos a cada gravação: o item que ficou na vaga (igual, ou com o mesmo índice e outros
/// efeitos) ou que mudou de vaga sem mudar mantém o serial. Entre contas, o TMSrv avisa as trocas
/// (STRUCT_ITEMHANDOFF nas gravações) e o item recebido leva o serial que tinha na conta de origem;
/// se quem deu ainda tiver o item, o w2db acusa a duplicação.
#ifndef _W2DB_CLIENT_
#define _W2DB_CLIENT_

#include "common/Basedef.h"

#define W2DB_DEADLINE_MS 5000
#define W2DB_LOGIN_DEADLINE_MS 15000

enum W2DbResult
{
	W2DB_OK = 0,
	W2DB_NO_ACCOUNT,
	W2DB_WRONG_PASSWORD,
	W2DB_LOCKED,
	W2DB_EXISTS,
	W2DB_NAME_TAKEN,
	W2DB_NUMERIC_NOT_SET,
	W2DB_INVALID,
	W2DB_RATE_LIMITED,	///< limite de relatos do /bug do personagem
	W2DB_FAIL,		///< w2db fora do ar, prazo estourado ou erro interno
};

/// @brief Lê a configuração e confere o w2db (Health). @return false se configurado e fora do ar.
bool W2DbInit();

/// @brief O DBSrv está usando o w2db.
bool W2DbOn();

/// @brief Login com senha: a conta vem em `file`, com a senha digitada em AccountPass (os handlers
///        comparam com ela, por exemplo ao apagar personagem). Guarda o estado da conta.
W2DbResult W2DbLogin(const char* name, const char* pass, STRUCT_ACCOUNTFILE* file, int* lockSeconds);

/// @brief Lê a conta sem senha (troca de canal, ferramentas de admin, importações).
///        `keep`: guarda o estado (conta que vai ficar conectada).
W2DbResult W2DbLoad(const char* name, STRUCT_ACCOUNTFILE* file, bool keep);

/// @brief Item que a conta recebeu numa troca (STRUCT_ITEMHANDOFF), com o personagem de quem deu.
struct W2Handoff
{
	char From[ACCOUNTNAME_LENGTH + 1];
	int FromChar;	///< vaga do personagem de quem deu (-1 se não está conectado)
	int FromCarry;
	STRUCT_ITEM Item;
};

/// @brief Grava a conta inteira. Os itens de `handoffs` vieram de outra conta numa troca e levam o
///        serial que tinham lá. Em falha do w2db, grava o arquivo em ./w2db_pending/<CONTA> para
///        importar depois e registra no log.
W2DbResult W2DbSave(const STRUCT_ACCOUNTFILE* file, bool logout, const W2Handoff* handoffs = nullptr,
	int handoffCount = 0);

/// @brief Confere a senha da conta no w2db (conta para o bloqueio por senha errada), sem guardar
///        nada.
W2DbResult W2DbCheckPassword(const char* name, const char* pass);

W2DbResult W2DbCreateAccount(const char* name, const char* pass, bool bonus);
W2DbResult W2DbSetPassword(const char* name, const char* pass);
W2DbResult W2DbCheckNumeric(const char* name, const char* numeric);

/// @brief Conta dona do personagem `ch` (16 bytes em `account`) e a vaga dele (`slot`, pode ser nulo).
W2DbResult W2DbFindCharacter(const char* ch, char* account, int* slot);

/// @brief Apaga o personagem da vaga no banco (os itens dele vão com evento).
W2DbResult W2DbDeleteCharacter(const char* account, int slot);

/// @brief Grava o relato do /bug (bug_reports; o w2db posta no Discord). `text` em CP1252.
W2DbResult W2DbReportBug(const char* account, const char* character, int server, int x, int y,
	const char* text);

/// @brief Esquece o estado da conta (saiu do jogo).
void W2DbForget(const char* name);

#endif
