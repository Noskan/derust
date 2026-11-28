/// @file W2DbConvert.h
/// @brief Arquivo de conta do DBSrv (STRUCT_ACCOUNTFILE) <-> conta do contrato do w2db (w2db.v1.Account).
///
/// Produz exatamente o proto que o codec do importador (services/w2db/src/legacy.rs) produz do
/// mesmo arquivo: o teste w2db_convert_tests confere contra os .pb gerados por ele. A senha, a
/// senha numérica e os dados pessoais não vão para o proto.
#ifndef _W2DB_CONVERT_
#define _W2DB_CONVERT_

#include <cstdint>

#include "common/Basedef.h"

namespace w2db { namespace v1 { class Account; } }

/// @brief Seriais dos itens de uma conta, nas mesmas vagas de STRUCT_ACCOUNTFILE (0: sem serial).
struct W2Serials
{
	uint64_t Equip[MOB_PER_ACCOUNT][MAX_EQUIP];
	uint64_t Carry[MOB_PER_ACCOUNT][MAX_CARRY];
	uint64_t Cargo[MAX_CARGO];
};

/// @brief Conta do arquivo para o proto, com os seriais e a versão esperada.
void W2DbAccountToProto(const STRUCT_ACCOUNTFILE& file, const W2Serials& serials, uint64_t version,
	w2db::v1::Account* out);

/// @brief Proto para a conta do arquivo. Senha e dados pessoais ficam zerados; a senha numérica fica
///        em -1 no primeiro byte quando a conta não tem uma (como a conta nova do DBSrv) e zerada
///        quando tem (o DBSrv só conhece o hash). @return false se o proto não cabe no arquivo.
bool W2DbProtoToAccount(const w2db::v1::Account& in, STRUCT_ACCOUNTFILE* file, W2Serials* serials);

#endif
