/// @file w2db_convert_tests.cpp
/// @brief Conversão STRUCT_ACCOUNTFILE <-> w2db.v1.Account do DBSrv (server/dbsrv/persistence/W2DbConvert.cpp)
///        contra o codec do importador (services/w2db/src/legacy.rs).
///
/// Os .pb de services/w2db/tests/data/golden são o que o codec do Rust produz das contas de teste
/// (services/w2db/tests/data/account/D) (teste golden_protos de services/w2db/tests/roundtrip.rs). O C++ tem de produzir o
/// mesmo proto, e voltar do proto ao mesmo arquivo, exceto a senha, a senha numérica e os dados
/// pessoais, que não vão para o proto.
///
/// Uso: w2db_convert_tests <services/w2db/tests/data/account/D> <services/w2db/tests/data/golden>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>

#include <google/protobuf/util/message_differencer.h>

#include "persistence/W2DbConvert.h"
#include "w2db/v1/w2db.pb.h"

namespace
{
int g_failures = 0;

#define CHECK(cond, ...)                         \
	do                                           \
	{                                            \
		if (!(cond))                             \
		{                                        \
			fprintf(stderr, "FALHOU: " __VA_ARGS__); \
			fprintf(stderr, "\n");               \
			g_failures++;                        \
		}                                        \
	} while (0)

bool ReadFile(const std::string& path, std::string* out)
{
	std::ifstream in(path, std::ios::binary);
	if (!in)
		return false;
	std::ostringstream ss;
	ss << in.rdbuf();
	*out = ss.str();
	return true;
}

// Bytes do arquivo que não vão para o proto: da senha (logo depois do nome) ao fim da senha
// numérica, contíguos em STRUCT_ACCOUNTINFO (shared/protocol/account_layout.txt, NOT_ROUND_TRIPPED).
void Mask(STRUCT_ACCOUNTFILE* f)
{
	const size_t from = offsetof(STRUCT_ACCOUNTINFO, AccountPass);
	const size_t to = offsetof(STRUCT_ACCOUNTINFO, NumericToken) + sizeof(f->Info.NumericToken);
	memset(reinterpret_cast<char*>(&f->Info) + from, 0, to - from);
}

std::string Diff(const google::protobuf::Message& a, const google::protobuf::Message& b)
{
	google::protobuf::util::MessageDifferencer d;
	std::string report;
	d.ReportDifferencesToString(&report);
	return d.Compare(a, b) ? std::string() : report;
}

void TestAccount(const std::string& dir, const std::string& goldenDir, const char* name)
{
	std::string raw, golden;
	if (!ReadFile(dir + "/" + name, &raw) || !ReadFile(goldenDir + "/" + name + ".pb", &golden))
	{
		CHECK(false, "%s: não li a conta ou o .pb em %s", name, dir.c_str());
		return;
	}
	CHECK(raw.size() == sizeof(STRUCT_ACCOUNTFILE), "%s: %zu bytes, esperado %zu", name, raw.size(),
		sizeof(STRUCT_ACCOUNTFILE));
	if (raw.size() != sizeof(STRUCT_ACCOUNTFILE))
		return;

	static STRUCT_ACCOUNTFILE file;
	memcpy(&file, raw.data(), sizeof(file));

	w2db::v1::Account want;
	CHECK(want.ParseFromString(golden), "%s.pb não decodifica", name);

	// Arquivo -> proto, como o importador.
	static W2Serials zero;
	w2db::v1::Account got;
	W2DbAccountToProto(file, zero, 0, &got);
	std::string d = Diff(got, want);
	CHECK(d.empty(), "%s: proto diferente do codec do Rust:\n%s", name, d.c_str());

	// Proto -> arquivo: igual ao original, fora a parte que não vai para o proto.
	static STRUCT_ACCOUNTFILE back;
	static W2Serials serials;
	CHECK(W2DbProtoToAccount(want, &back, &serials), "%s: proto não cabe no arquivo", name);
	static STRUCT_ACCOUNTFILE masked;
	masked = file;
	Mask(&masked);
	static STRUCT_ACCOUNTFILE backMasked;
	backMasked = back;
	Mask(&backMasked);
	if (memcmp(&masked, &backMasked, sizeof(masked)) != 0)
	{
		const char* a = reinterpret_cast<const char*>(&masked);
		const char* b = reinterpret_cast<const char*>(&backMasked);
		size_t at = 0;
		while (a[at] == b[at])
			at++;
		CHECK(false, "%s: proto -> arquivo difere no byte %zu", name, at);
	}

	// Seriais e versão vão e voltam nas mesmas vagas.
	static W2Serials marks;
	memset(&marks, 0, sizeof(marks));
	uint64_t next = 1000;
	for (int c = 0; c < MOB_PER_ACCOUNT; c++)
	{
		for (int i = 0; i < MAX_EQUIP; i++)
			marks.Equip[c][i] = file.Char[c].Equip[i].sIndex != 0 ? next++ : 0;
		for (int i = 0; i < MAX_CARRY; i++)
			marks.Carry[c][i] = file.Char[c].Carry[i].sIndex != 0 ? next++ : 0;
	}
	for (int i = 0; i < MAX_CARGO; i++)
		marks.Cargo[i] = file.Cargo[i].sIndex != 0 ? next++ : 0;
	w2db::v1::Account withSerials;
	W2DbAccountToProto(file, marks, 77, &withSerials);
	CHECK(withSerials.version() == 77, "%s: versão não foi para o proto", name);
	static W2Serials again;
	CHECK(W2DbProtoToAccount(withSerials, &back, &again), "%s: proto com seriais não cabe", name);
	CHECK(memcmp(&marks, &again, sizeof(marks)) == 0, "%s: seriais mudaram de vaga", name);
	CHECK(next > 1000, "%s: conta de teste sem itens", name);

	// A conta sem senha numérica volta com -1 no primeiro byte, como a conta nova do DBSrv.
	w2db::v1::Account noNumeric = want;
	noNumeric.set_numeric_set(false);
	CHECK(W2DbProtoToAccount(noNumeric, &back, &again), "%s: sem senha numérica", name);
	CHECK(back.Info.NumericToken[0] == -1, "%s: NumericToken[0] = %d sem senha numérica", name,
		back.Info.NumericToken[0]);
}

void TestRejects()
{
	w2db::v1::Account acc;
	w2db::v1::StoredItem* it = acc.add_cargo();
	it->set_slot(MAX_CARGO);
	it->mutable_item()->set_index(1);
	static STRUCT_ACCOUNTFILE file;
	static W2Serials serials;
	CHECK(!W2DbProtoToAccount(acc, &file, &serials), "vaga de baú fora do limite foi aceita");

	w2db::v1::Account big;
	big.set_name(std::string(ACCOUNTNAME_LENGTH, 'A'));
	CHECK(!W2DbProtoToAccount(big, &file, &serials), "nome de conta sem lugar para o terminador foi aceito");
}
}

int main(int argc, char** argv)
{
	if (argc != 3)
	{
		fprintf(stderr, "uso: %s <services/w2db/tests/data/account/D> <services/w2db/tests/data/golden>\n", argv[0]);
		return 2;
	}
	TestAccount(argv[1], argv[2], "DERUST");
	TestAccount(argv[1], argv[2], "DERUST2");
	TestRejects();
	if (g_failures != 0)
	{
		fprintf(stderr, "%d falha(s)\n", g_failures);
		return 1;
	}
	printf("w2db_convert_tests: ok\n");
	return 0;
}
