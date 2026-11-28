/// @file W2DbClient_none.cpp
/// @brief W2DbClient.h sem o gRPC (DERUST_W2DB=OFF): o DBSrv fica nos arquivos e não sobe se houver
///        um w2db configurado.
#include "persistence/W2DbClient.h"

#include <cstdio>
#include <cstdlib>

#include "common/Platform.h"

bool W2DbInit()
{
	const char* env = getenv("W2DB_SOCKET");
	bool configured = env != nullptr && env[0] != 0;
	if (FILE* fp = fopen("w2db.txt", "rt"))
	{
		int c = fgetc(fp);
		configured = configured || (c != EOF && c != '\n' && c != '\r');
		fclose(fp);
	}
	if (configured)
		ServerAlert("w2db configurado, mas este DBSrv foi compilado sem o gRPC (DERUST_W2DB=OFF)", "w2db");
	return !configured;
}

bool W2DbOn() { return false; }
W2DbResult W2DbLogin(const char*, const char*, STRUCT_ACCOUNTFILE*, int*) { return W2DB_FAIL; }
W2DbResult W2DbLoad(const char*, STRUCT_ACCOUNTFILE*, bool) { return W2DB_FAIL; }
W2DbResult W2DbSave(const STRUCT_ACCOUNTFILE*, bool, const W2Handoff*, int) { return W2DB_FAIL; }
W2DbResult W2DbCheckPassword(const char*, const char*) { return W2DB_FAIL; }
W2DbResult W2DbCreateAccount(const char*, const char*, bool) { return W2DB_FAIL; }
W2DbResult W2DbSetPassword(const char*, const char*) { return W2DB_FAIL; }
W2DbResult W2DbCheckNumeric(const char*, const char*) { return W2DB_FAIL; }
W2DbResult W2DbFindCharacter(const char*, char*, int*) { return W2DB_FAIL; }
W2DbResult W2DbDeleteCharacter(const char*, int) { return W2DB_FAIL; }
W2DbResult W2DbReportBug(const char*, const char*, int, int, int, const char*) { return W2DB_FAIL; }
void W2DbForget(const char*) {}
