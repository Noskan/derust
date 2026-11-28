/// @file W2DbClient.cpp
/// @brief Implementação de W2DbClient.h.
#include "persistence/W2DbClient.h"

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>
#include <sys/stat.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <grpcpp/grpcpp.h>

#include "net/CUser.h"
#include "core/Server.h"
#include "persistence/W2DbConvert.h"
#include "core/ServerMetrics.h"
#include "w2db/v1/w2db.grpc.pb.h"

namespace pb = w2db::v1;

namespace
{
struct State
{
	uint64_t Version = 0;
	W2Serials Serials;
	// Itens da última gravação (ou da leitura), nas vagas dos seriais.
	STRUCT_ITEM Equip[MOB_PER_ACCOUNT][MAX_EQUIP];
	STRUCT_ITEM Carry[MOB_PER_ACCOUNT][MAX_CARRY];
	STRUCT_ITEM Cargo[MAX_CARGO];
	// Senha numérica que o banco tem, quando o DBSrv a conhece (definida nesta sessão).
	char Numeric[6];
	// Itens que saíram da conta nas últimas gravações, para uma troca achar o serial deles.
	struct Departed
	{
		STRUCT_ITEM Item;
		uint64_t Serial;
		int Char;
		int Carry;	// vaga do Carry, ou -1
	};
	std::vector<Departed> Gone;
};

constexpr size_t kMaxDeparted = 64;

std::unique_ptr<pb::W2Db::Stub> g_stub;
std::unordered_map<std::string, std::unique_ptr<State>> g_states;
std::string g_socket;
// Seriais que uma troca já deu a outra conta enquanto ainda estavam na vaga de quem deu.
std::unordered_set<uint64_t> g_claimed;

std::string Key(const char* name)
{
	char up[ACCOUNTNAME_LENGTH + 1] = { 0 };
	strncpy(up, name, ACCOUNTNAME_LENGTH);
	W2StrUpper(up);
	return up;
}

std::string Bytes(const char* s, size_t max)
{
	size_t n = 0;
	while (n < max && s[n] != 0)
		n++;
	return std::string(s, n);
}

void Deadline(grpc::ClientContext& ctx, int ms)
{
	ctx.set_deadline(std::chrono::system_clock::now() + std::chrono::milliseconds(ms));
}

void LogRpc(const char* what, const char* name, const grpc::Status& st)
{
	char temp[512];
	snprintf(temp, sizeof(temp), "err,w2db %s: %d %s", what, static_cast<int>(st.error_code()),
		st.error_message().c_str());
	Log(temp, const_cast<char*>(name), 0);
	DbMetricAdd(DB_W2DB_FAILURES);
}

W2DbResult FromStatus(int s)
{
	switch (s)
	{
	case pb::STATUS_OK: return W2DB_OK;
	case pb::STATUS_NO_ACCOUNT: return W2DB_NO_ACCOUNT;
	case pb::STATUS_WRONG_PASSWORD: return W2DB_WRONG_PASSWORD;
	case pb::STATUS_LOCKED: return W2DB_LOCKED;
	case pb::STATUS_ACCOUNT_EXISTS: return W2DB_EXISTS;
	case pb::STATUS_NAME_TAKEN: return W2DB_NAME_TAKEN;
	case pb::STATUS_NUMERIC_NOT_SET: return W2DB_NUMERIC_NOT_SET;
	case pb::STATUS_INVALID: return W2DB_INVALID;
	case pb::STATUS_RATE_LIMITED: return W2DB_RATE_LIMITED;
	case pb::STATUS_BANNED: return W2DB_BANNED;
	default: return W2DB_FAIL;
	}
}

std::unique_ptr<State> StateFrom(const pb::Account& acc, STRUCT_ACCOUNTFILE* file)
{
	std::unique_ptr<State> st(new State());
	if (!W2DbProtoToAccount(acc, file, &st->Serials))
		return nullptr;
	st->Version = acc.version();
	memcpy(st->Cargo, file->Cargo, sizeof(st->Cargo));
	for (int s = 0; s < MOB_PER_ACCOUNT; s++)
	{
		memcpy(st->Equip[s], file->Char[s].Equip, sizeof(st->Equip[s]));
		memcpy(st->Carry[s], file->Char[s].Carry, sizeof(st->Carry[s]));
	}
	memset(st->Numeric, 0, sizeof(st->Numeric));
	return st;
}

// Uma vaga de item: o item atual, o serial deduzido e o que havia na última gravação.
struct Slot
{
	const STRUCT_ITEM* now;
	uint64_t* serial;
	const STRUCT_ITEM* before;
	uint64_t beforeSerial;
	int ch;		// personagem (-1 no baú)
	int carry;	// vaga do Carry, ou -1
};

bool Empty(const STRUCT_ITEM& it)
{
	static const STRUCT_ITEM zero = {};
	return memcmp(&it, &zero, sizeof(it)) == 0;
}

bool Same(const STRUCT_ITEM& a, const STRUCT_ITEM& b)
{
	return memcmp(&a, &b, sizeof(STRUCT_ITEM)) == 0;
}

// Item recebido numa troca, já com o serial da conta de origem.
struct Incoming
{
	STRUCT_ITEM Item;
	uint64_t Serial;
};

// Serial do item que veio de outra conta: ainda na vaga de quem deu (quem deu não gravou depois da
// troca) ou entre os que saíram das últimas gravações dele. 0 se não achar.
uint64_t ResolveHandoff(const W2Handoff& h, const std::string& self)
{
	const std::string from = Key(h.From);
	if (from == self)
		return 0;
	auto it = g_states.find(from);
	if (it == g_states.end())
		return 0;
	State& g = *it->second;
	if (h.FromChar >= 0 && h.FromChar < MOB_PER_ACCOUNT && h.FromCarry >= 0 && h.FromCarry < MAX_CARRY)
	{
		const uint64_t serial = g.Serials.Carry[h.FromChar][h.FromCarry];
		if (serial != 0 && Same(g.Carry[h.FromChar][h.FromCarry], h.Item) && g_claimed.count(serial) == 0)
		{
			g_claimed.insert(serial);
			return serial;
		}
	}
	int best = -1;
	for (size_t i = 0; i < g.Gone.size(); i++)
	{
		const State::Departed& d = g.Gone[i];
		if (!Same(d.Item, h.Item))
			continue;
		best = static_cast<int>(i);
		if (d.Char == h.FromChar && d.Carry == h.FromCarry)
			break;
	}
	if (best < 0)
		return 0;
	const uint64_t serial = g.Gone[best].Serial;
	g.Gone.erase(g.Gone.begin() + best);
	return serial;
}

std::vector<Slot> SlotsOf(const STRUCT_ACCOUNTFILE& f, const State& st, W2Serials* out)
{
	std::vector<Slot> slots;
	slots.reserve(MOB_PER_ACCOUNT * (MAX_EQUIP + MAX_CARRY) + MAX_CARGO);
	for (int c = 0; c < MOB_PER_ACCOUNT; c++)
	{
		for (int i = 0; i < MAX_EQUIP; i++)
			slots.push_back({ &f.Char[c].Equip[i], &out->Equip[c][i], &st.Equip[c][i], st.Serials.Equip[c][i], c, -1 });
		for (int i = 0; i < MAX_CARRY; i++)
			slots.push_back({ &f.Char[c].Carry[i], &out->Carry[c][i], &st.Carry[c][i], st.Serials.Carry[c][i], c, i });
	}
	for (int i = 0; i < MAX_CARGO; i++)
		slots.push_back({ &f.Cargo[i], &out->Cargo[i], &st.Cargo[i], st.Serials.Cargo[i], -1, -1 });
	return slots;
}

// Seriais da conta atual a partir da última gravação, em ordem: (1) mesma vaga e mesmo item;
// (2) item recebido numa troca, com o serial da conta de origem; (3) mesma vaga e mesmo índice (o
// item mudou na vaga: refinação, quantidade); (4) mesmo item em outra vaga (mudou de lugar). O
// resto vai com 0 (item novo para o w2db). `taken` diz quais vagas da última gravação continuam.
void InferSerials(const STRUCT_ACCOUNTFILE& f, const State& st, const std::vector<Incoming>& incoming,
	W2Serials* out, std::vector<bool>* taken, std::vector<bool>* used)
{
	memset(out, 0, sizeof(*out));
	std::vector<Slot> slots = SlotsOf(f, st, out);
	taken->assign(slots.size(), false);
	used->assign(incoming.size(), false);
	for (size_t i = 0; i < slots.size(); i++)
	{
		Slot& s = slots[i];
		if (!Empty(*s.now) && s.beforeSerial != 0 && Same(*s.now, *s.before))
		{
			*s.serial = s.beforeSerial;
			(*taken)[i] = true;
		}
	}
	for (size_t k = 0; k < incoming.size(); k++)
	{
		for (Slot& s : slots)
		{
			if (*s.serial == 0 && !Empty(*s.now) && Same(*s.now, incoming[k].Item))
			{
				*s.serial = incoming[k].Serial;
				(*used)[k] = true;
				break;
			}
		}
	}
	for (size_t i = 0; i < slots.size(); i++)
	{
		Slot& s = slots[i];
		if (*s.serial == 0 && !Empty(*s.now) && s.beforeSerial != 0 && !(*taken)[i] && s.now->sIndex == s.before->sIndex)
		{
			*s.serial = s.beforeSerial;
			(*taken)[i] = true;
		}
	}
	for (Slot& s : slots)
	{
		if (Empty(*s.now) || *s.serial != 0)
			continue;
		for (size_t j = 0; j < slots.size(); j++)
		{
			if ((*taken)[j] || slots[j].beforeSerial == 0)
				continue;
			if (Same(*s.now, *slots[j].before))
			{
				*s.serial = slots[j].beforeSerial;
				(*taken)[j] = true;
				break;
			}
		}
	}
}

// Depois de gravar: o que saiu da conta fica em Gone (para uma troca achar o serial), menos o que
// uma troca já levou da vaga.
void RecordDeparted(State* st, const STRUCT_ACCOUNTFILE& f, const std::vector<bool>& taken)
{
	W2Serials scratch;
	std::vector<Slot> slots = SlotsOf(f, *st, &scratch);
	for (size_t i = 0; i < slots.size(); i++)
	{
		const Slot& s = slots[i];
		if (s.beforeSerial == 0)
			continue;
		const bool claimed = g_claimed.erase(s.beforeSerial) != 0;
		if (taken[i] || claimed)
			continue;
		st->Gone.push_back({ *s.before, s.beforeSerial, s.ch, s.carry });
	}
	if (st->Gone.size() > kMaxDeparted)
		st->Gone.erase(st->Gone.begin(), st->Gone.end() - kMaxDeparted);
}

void WritePending(const STRUCT_ACCOUNTFILE* file)
{
	mkdir("./w2db_pending", 0700);
	std::string path = "./w2db_pending/" + Key(file->Info.AccountName);
	FILE* fp = fopen(path.c_str(), "wb");
	if (fp != nullptr)
	{
		fwrite(file, sizeof(*file), 1, fp);
		fclose(fp);
	}
	char temp[256];
	snprintf(temp, sizeof(temp), "err,w2db save failed, account kept in %s", path.c_str());
	Log(temp, const_cast<char*>(file->Info.AccountName), 0);
	ServerAlert(temp, "w2db");
	DbMetricAdd(DB_W2DB_PENDING);
}

W2DbResult LoadState(const char* name, STRUCT_ACCOUNTFILE* file, std::unique_ptr<State>* out)
{
	pb::LoadAccountRequest req;
	req.set_name(Bytes(name, ACCOUNTNAME_LENGTH));
	pb::LoadAccountReply rep;
	grpc::ClientContext ctx;
	Deadline(ctx, W2DB_DEADLINE_MS);
	grpc::Status st = g_stub->LoadAccount(&ctx, req, &rep);
	if (!st.ok())
	{
		LogRpc("load", name, st);
		return W2DB_FAIL;
	}
	if (rep.status() != pb::STATUS_OK)
		return FromStatus(rep.status());
	*out = StateFrom(rep.account(), file);
	return *out ? W2DB_OK : W2DB_INVALID;
}
}

bool W2DbOn()
{
	return g_stub != nullptr;
}

bool W2DbInit()
{
	const char* env = getenv("W2DB_SOCKET");
	if (env != nullptr && env[0] != 0)
		g_socket = env;
	else if (FILE* fp = fopen("w2db.txt", "rt"))
	{
		char line[512] = { 0 };
		if (fgets(line, sizeof(line), fp) != nullptr)
		{
			line[strcspn(line, "\r\n")] = 0;
			g_socket = line;
		}
		fclose(fp);
	}
	if (g_socket.empty())
		return true;

	std::shared_ptr<grpc::Channel> ch = grpc::CreateChannel("unix:" + g_socket, grpc::InsecureChannelCredentials());
	g_stub = pb::W2Db::NewStub(ch);

	pb::HealthRequest req;
	pb::HealthReply rep;
	grpc::ClientContext ctx;
	Deadline(ctx, W2DB_DEADLINE_MS);
	ctx.set_wait_for_ready(true);
	grpc::Status st = g_stub->Health(&ctx, req, &rep);
	if (!st.ok() || !rep.database_ok())
	{
		char temp[512];
		snprintf(temp, sizeof(temp), "w2db fora do ar em %s: %s", g_socket.c_str(),
			st.ok() ? "banco indisponível" : st.error_message().c_str());
		ServerAlert(temp, "w2db");
		g_stub.reset();
		return false;
	}
	char temp[512];
	snprintf(temp, sizeof(temp), "sys,w2db %s em %s", rep.version().c_str(), g_socket.c_str());
	Log(temp, const_cast<char*>("-system"), 0);

	pb::ResetSessionsRequest rreq;
	pb::StatusReply rrep;
	grpc::ClientContext rctx;
	Deadline(rctx, W2DB_DEADLINE_MS);
	grpc::Status rst = g_stub->ResetSessions(&rctx, rreq, &rrep);
	if (!rst.ok())
		LogRpc("reset sessions", "-system", rst);
	return true;
}

W2DbResult W2DbLogin(const char* name, const char* pass, STRUCT_ACCOUNTFILE* file, int* lockSeconds)
{
	pb::AccountLoginRequest req;
	req.set_name(Bytes(name, ACCOUNTNAME_LENGTH));
	req.set_password(Bytes(pass, ACCOUNTPASS_LENGTH));
	pb::AccountLoginReply rep;
	grpc::ClientContext ctx;
	Deadline(ctx, W2DB_LOGIN_DEADLINE_MS);
	grpc::Status st = g_stub->AccountLogin(&ctx, req, &rep);
	if (!st.ok())
	{
		LogRpc("login", name, st);
		return W2DB_FAIL;
	}
	if (lockSeconds != nullptr)
		*lockSeconds = static_cast<int>(rep.lock_seconds());
	if (rep.status() != pb::STATUS_OK)
		return FromStatus(rep.status());
	std::unique_ptr<State> s = StateFrom(rep.account(), file);
	if (!s)
		return W2DB_INVALID;
	strncpy(file->Info.AccountPass, pass, ACCOUNTPASS_LENGTH);
	file->Info.AccountPass[ACCOUNTPASS_LENGTH - 1] = 0;
	g_states[Key(name)] = std::move(s);
	return W2DB_OK;
}

W2DbResult W2DbLoad(const char* name, STRUCT_ACCOUNTFILE* file, bool keep)
{
	std::unique_ptr<State> s;
	W2DbResult r = LoadState(name, file, &s);
	if (r == W2DB_OK && keep)
	{
		std::unique_ptr<State>& slot = g_states[Key(name)];
		if (slot)
		{
			memcpy(s->Numeric, slot->Numeric, sizeof(s->Numeric));
			s->Gone = std::move(slot->Gone);
		}
		slot = std::move(s);
	}
	return r;
}

W2DbResult W2DbSave(const STRUCT_ACCOUNTFILE* file, bool logout, const W2Handoff* handoffs, int handoffCount)
{
	const char* name = file->Info.AccountName;
	const std::string key = Key(name);

	// Os itens recebidos em trocas levam o serial da conta de origem (uma vez só, mesmo se a
	// gravação tiver de ser refeita).
	std::vector<Incoming> incoming;
	for (int i = 0; i < handoffCount; i++)
	{
		const uint64_t serial = ResolveHandoff(handoffs[i], key);
		if (serial != 0)
		{
			incoming.push_back({ handoffs[i].Item, serial });
			continue;
		}
		char temp[256];
		snprintf(temp, sizeof(temp), "err,w2db handoff not resolved: item %d from %.16s carry %d",
			handoffs[i].Item.sIndex, handoffs[i].From, handoffs[i].FromCarry);
		Log(temp, const_cast<char*>(name), 0);
	}

	for (int attempt = 0; attempt < 2; attempt++)
	{
		auto it = g_states.find(key);
		std::unique_ptr<State> loaded;
		State* st = nullptr;
		if (it != g_states.end() && attempt == 0)
			st = it->second.get();
		else
		{
			// Sem estado (conta fora do jogo) ou versão velha: lê a do banco.
			static STRUCT_ACCOUNTFILE scratch;
			W2DbResult r = LoadState(name, &scratch, &loaded);
			if (r != W2DB_OK)
			{
				if (r == W2DB_FAIL)
					WritePending(file);
				return r;
			}
			if (it != g_states.end())
			{
				memcpy(loaded->Numeric, it->second->Numeric, sizeof(loaded->Numeric));
				loaded->Gone = std::move(it->second->Gone);
				it->second = std::move(loaded);
				st = it->second.get();
			}
			else
				st = loaded.get();
		}

		W2Serials serials;
		std::vector<bool> taken, used;
		InferSerials(*file, *st, incoming, &serials, &taken, &used);
		for (size_t k = 0; k < incoming.size(); k++)
		{
			if (used[k])
				continue;
			char temp[256];
			snprintf(temp, sizeof(temp), "err,w2db traded item %d (serial %llu) not found in the account",
				incoming[k].Item.sIndex, static_cast<unsigned long long>(incoming[k].Serial));
			Log(temp, const_cast<char*>(name), 0);
		}

		pb::SaveAccountRequest req;
		W2DbAccountToProto(*file, serials, st->Version, req.mutable_account());
		req.set_logout(logout);
		req.set_reason(logout ? "saída" : "gravação");
		pb::SaveAccountReply rep;
		grpc::ClientContext ctx;
		Deadline(ctx, W2DB_DEADLINE_MS);
		grpc::Status gs = g_stub->SaveAccount(&ctx, req, &rep);
		if (!gs.ok())
		{
			LogRpc("save", name, gs);
			WritePending(file);
			return W2DB_FAIL;
		}
		if (rep.status() == pb::STATUS_VERSION_CONFLICT)
		{
			Log(const_cast<char*>("err,w2db version conflict, reloading"), const_cast<char*>(name), 0);
			continue;
		}
		if (rep.status() != pb::STATUS_OK)
			return FromStatus(rep.status());

		// O estado passa a ser o desta gravação.
		RecordDeparted(st, *file, taken);
		for (const pb::AssignedSerial& a : rep.assigned())
		{
			const unsigned c = a.character_slot();
			const unsigned s = a.slot();
			if (a.container() == pb::CONTAINER_EQUIP && c < MOB_PER_ACCOUNT && s < MAX_EQUIP)
				serials.Equip[c][s] = a.serial();
			else if (a.container() == pb::CONTAINER_CARRY && c < MOB_PER_ACCOUNT && s < MAX_CARRY)
				serials.Carry[c][s] = a.serial();
			else if (a.container() == pb::CONTAINER_CARGO && s < MAX_CARGO)
				serials.Cargo[s] = a.serial();
		}
		if (rep.dupe_suspects_size() > 0)
		{
			// O mesmo serial vivo em duas contas: o w2db deu serial novo a este e guardou o caso em
			// item_events (dupe_suspect).
			char temp[256];
			snprintf(temp, sizeof(temp), "err,w2db %d dupe suspect(s) in %.16s, new serial %llu", rep.dupe_suspects_size(),
				name, static_cast<unsigned long long>(rep.dupe_suspects(0)));
			Log(temp, const_cast<char*>(name), 0);
			ServerAlert(temp, "w2db");
			DbMetricAdd(DB_DUPE_SUSPECTS, rep.dupe_suspects_size());
		}
		st->Version = rep.version();
		st->Serials = serials;
		memcpy(st->Cargo, file->Cargo, sizeof(st->Cargo));
		for (int s = 0; s < MOB_PER_ACCOUNT; s++)
		{
			memcpy(st->Equip[s], file->Char[s].Equip, sizeof(st->Equip[s]));
			memcpy(st->Carry[s], file->Char[s].Carry, sizeof(st->Carry[s]));
		}

		// Senha numérica nova (o DBSrv a grava no arquivo da conta; o w2db guarda o hash).
		const char* tok = file->Info.NumericToken;
		if (tok[0] != -1 && tok[0] != 0 && memcmp(tok, st->Numeric, sizeof(st->Numeric)) != 0)
		{
			pb::SetNumericPasswordRequest nreq;
			nreq.set_name(Bytes(name, ACCOUNTNAME_LENGTH));
			nreq.set_numeric(Bytes(tok, 6));
			pb::StatusReply nrep;
			grpc::ClientContext nctx;
			Deadline(nctx, W2DB_LOGIN_DEADLINE_MS);
			grpc::Status ns = g_stub->SetNumericPassword(&nctx, nreq, &nrep);
			if (!ns.ok() || nrep.status() != pb::STATUS_OK)
			{
				LogRpc("numeric", name, ns);
				return W2DB_FAIL;
			}
			memcpy(st->Numeric, tok, sizeof(st->Numeric));
		}
		return W2DB_OK;
	}
	Log(const_cast<char*>("err,w2db version conflict twice"), const_cast<char*>(name), 0);
	WritePending(file);
	return W2DB_FAIL;
}

W2DbResult W2DbCheckPassword(const char* name, const char* pass)
{
	pb::AccountLoginRequest req;
	req.set_name(Bytes(name, ACCOUNTNAME_LENGTH));
	req.set_password(Bytes(pass, ACCOUNTPASS_LENGTH));
	pb::AccountLoginReply rep;
	grpc::ClientContext ctx;
	Deadline(ctx, W2DB_LOGIN_DEADLINE_MS);
	grpc::Status st = g_stub->AccountLogin(&ctx, req, &rep);
	if (!st.ok())
	{
		LogRpc("password check", name, st);
		return W2DB_FAIL;
	}
	return FromStatus(rep.status());
}

W2DbResult W2DbCreateAccount(const char* name, const char* pass, bool bonus)
{
	pb::CreateAccountRequest req;
	req.set_name(Bytes(name, ACCOUNTNAME_LENGTH));
	req.set_password(Bytes(pass, ACCOUNTPASS_LENGTH));
	req.set_bonus(bonus);
	pb::CreateAccountReply rep;
	grpc::ClientContext ctx;
	Deadline(ctx, W2DB_LOGIN_DEADLINE_MS);
	grpc::Status st = g_stub->CreateAccount(&ctx, req, &rep);
	if (!st.ok())
	{
		LogRpc("create account", name, st);
		return W2DB_FAIL;
	}
	return FromStatus(rep.status());
}

W2DbResult W2DbSetPassword(const char* name, const char* pass)
{
	pb::SetPasswordRequest req;
	req.set_name(Bytes(name, ACCOUNTNAME_LENGTH));
	req.set_password(Bytes(pass, ACCOUNTPASS_LENGTH));
	pb::StatusReply rep;
	grpc::ClientContext ctx;
	Deadline(ctx, W2DB_LOGIN_DEADLINE_MS);
	grpc::Status st = g_stub->SetPassword(&ctx, req, &rep);
	if (!st.ok())
	{
		LogRpc("set password", name, st);
		return W2DB_FAIL;
	}
	return FromStatus(rep.status());
}

W2DbResult W2DbCheckNumeric(const char* name, const char* numeric)
{
	auto it = g_states.find(Key(name));
	if (it != g_states.end() && it->second->Numeric[0] != 0)
		return memcmp(it->second->Numeric, numeric, sizeof(it->second->Numeric)) == 0 ? W2DB_OK : W2DB_WRONG_PASSWORD;

	pb::CheckNumericPasswordRequest req;
	req.set_name(Bytes(name, ACCOUNTNAME_LENGTH));
	req.set_numeric(Bytes(numeric, 6));
	pb::StatusReply rep;
	grpc::ClientContext ctx;
	Deadline(ctx, W2DB_LOGIN_DEADLINE_MS);
	grpc::Status st = g_stub->CheckNumericPassword(&ctx, req, &rep);
	if (!st.ok())
	{
		LogRpc("numeric check", name, st);
		return W2DB_FAIL;
	}
	if (rep.status() == pb::STATUS_OK && it != g_states.end())
		strncpy(it->second->Numeric, numeric, sizeof(it->second->Numeric));
	return FromStatus(rep.status());
}

W2DbResult W2DbFindCharacter(const char* ch, char* account, int* slot)
{
	pb::FindCharacterRequest req;
	req.set_name(Bytes(ch, NAME_LENGTH));
	pb::FindCharacterReply rep;
	grpc::ClientContext ctx;
	Deadline(ctx, W2DB_DEADLINE_MS);
	grpc::Status st = g_stub->FindCharacter(&ctx, req, &rep);
	if (!st.ok())
	{
		LogRpc("find character", ch, st);
		return W2DB_FAIL;
	}
	if (rep.status() != pb::STATUS_OK)
		return FromStatus(rep.status());
	memset(account, 0, ACCOUNTNAME_LENGTH);
	memcpy(account, rep.account_name().data(),
		rep.account_name().size() < ACCOUNTNAME_LENGTH - 1 ? rep.account_name().size() : ACCOUNTNAME_LENGTH - 1);
	if (slot != nullptr)
		*slot = static_cast<int>(rep.slot());
	return W2DB_OK;
}

W2DbResult W2DbDeleteCharacter(const char* account, int slot)
{
	auto it = g_states.find(Key(account));
	std::unique_ptr<State> loaded;
	State* st = nullptr;
	if (it != g_states.end())
		st = it->second.get();
	else
	{
		static STRUCT_ACCOUNTFILE scratch;
		W2DbResult r = LoadState(account, &scratch, &loaded);
		if (r != W2DB_OK)
			return r;
		st = loaded.get();
	}
	for (int attempt = 0; attempt < 2; attempt++)
	{
		pb::DeleteCharacterRequest req;
		req.set_account_name(Bytes(account, ACCOUNTNAME_LENGTH));
		req.set_expected_version(st->Version);
		req.set_slot(slot);
		pb::SaveAccountReply rep;
		grpc::ClientContext ctx;
		Deadline(ctx, W2DB_DEADLINE_MS);
		grpc::Status gs = g_stub->DeleteCharacter(&ctx, req, &rep);
		if (!gs.ok())
		{
			LogRpc("delete character", account, gs);
			return W2DB_FAIL;
		}
		if (rep.status() == pb::STATUS_VERSION_CONFLICT)
		{
			st->Version = rep.version();
			continue;
		}
		if (rep.status() != pb::STATUS_OK)
			return FromStatus(rep.status());
		st->Version = rep.version();
		// Os itens do personagem saíram do banco: a vaga fica sem seriais.
		memset(st->Serials.Equip[slot], 0, sizeof(st->Serials.Equip[slot]));
		memset(st->Serials.Carry[slot], 0, sizeof(st->Serials.Carry[slot]));
		memset(st->Equip[slot], 0, sizeof(st->Equip[slot]));
		memset(st->Carry[slot], 0, sizeof(st->Carry[slot]));
		return W2DB_OK;
	}
	return W2DB_FAIL;
}

W2DbResult W2DbReportBug(const char* account, const char* character, int server, int x, int y,
	const char* text)
{
	pb::ReportBugRequest req;
	req.set_account_name(Bytes(account, ACCOUNTNAME_LENGTH));
	req.set_character_name(Bytes(character, NAME_LENGTH));
	req.set_server(server);
	req.set_pos_x(x);
	req.set_pos_y(y);
	req.set_text(Bytes(text, CHAT_LENGTH));
	pb::ReportBugReply rep;
	grpc::ClientContext ctx;
	Deadline(ctx, W2DB_DEADLINE_MS);
	grpc::Status st = g_stub->ReportBug(&ctx, req, &rep);
	if (!st.ok())
	{
		LogRpc("bug report", account, st);
		return W2DB_FAIL;
	}
	return FromStatus(rep.status());
}

void W2DbSetSession(const char* name, bool online)
{
	pb::SetSessionRequest req;
	req.set_name(Bytes(name, ACCOUNTNAME_LENGTH));
	req.set_online(online);
	pb::StatusReply rep;
	grpc::ClientContext ctx;
	Deadline(ctx, W2DB_DEADLINE_MS);
	grpc::Status st = g_stub->SetSession(&ctx, req, &rep);
	if (!st.ok())
		LogRpc(online ? "session on" : "session off", name, st);
}

void W2DbForget(const char* name)
{
	g_states.erase(Key(name));
}
