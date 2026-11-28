/// @file LoginClient.cpp
/// @brief Cliente sintético do login e do jogo, montado com os fontes do cliente (TMProject).
///
/// Usa o `CPSock` e as estruturas de `client/src/common/Basedef.h`, os mesmos que o jogo usa, e faz
/// contra um DBSrv + TMSrv no ar o caminho do login e uma sessão curta de jogo com dois personagens.
///
/// Login, com o personagem A:
///   1. `_MSG_AccountLogin` (0x20D) → `MSG_CNFAccountLogin` (0x10A);
///   2. senha numérica `MSG_CHARPASSWORD` (0xFDE) → eco 0xFDE (o DBSrv só cria ou entra com
///      personagem depois dela);
///   3. `MSG_NewCharacter` (0x20F) → `MSG_CNFNewCharacter` (0x110), na vaga 0;
///   4. outro personagem na vaga 1 e `MSG_DeleteCharacter` (0x211) com a senha da conta →
///      `MSG_CNFDeleteCharacter` (0x112);
///   5. `MSG_CharacterLogin` (0x213) → `MSG_CNFCharacterLogin` (0x114).
///
/// Jogo, com B entrando por outra conta perto de A:
///   6. cada um recebe o `MSG_CreateMob` (0x364) do outro;
///   7. chat de A (0x333) chega a B;
///   8. movimento de A (0x36C) chega a B; o movimento para a casa de B volta para A corrigido;
///   9. A solta a poção de Carry[0] → `MSG_CNFDropItem` (0x175) e `MSG_CreateItem` (0x26E), que
///      B também vê;
///  10. A pega a poção para Carry[5] → `MSG_CNFGetItem` (0x171) com o item, `MSG_SendItem` (0x182) e
///      o 0x16F que tira o item do chão (para B também);
///  11. troca de posição (0x376): de e para Carry[35] é recusada (a bolsa de Carry[60] falta), para
///      Carry[50] passa (a bolsa de Carry[61] está lá);
///  12. A usa a poção de Carry[50] → `MSG_SetHpMp` (0x181);
///  13. A sai do personagem (0x215 → 0x116, B recebe o 0x165) e entra de novo: o 0x114 traz o
///      inventário que o DBSrv gravou (poção gasta, bolsa no lugar).
///  14. /day e combate com um NPC do NPCGener.txt.
///  15. troca de mão dupla (`MSG_Trade`, 0x383): a espada de A (Carry[1]) pela de B; os dois
///      confirmam e recebem o `MSG_Carry` (0x185) com a espada do outro. O smoke confere no w2db
///      que cada espada levou o serial para a outra conta.
///  16. só com o argumento `dupe` (TMSrv com DERUST_TEST_HOOKS): A pede /w2dupe e dá a armadura
///      de Carry[2] a B, ficando com uma cópia, como num bug de duplicação. O smoke confere que o
///      w2db acusa o serial repetido.
///  17. /bug (docs/OPS.md): sem texto volta o uso; com texto, "Relato enviado"; logo depois, o
///      intervalo entre relatos. O smoke confere o relato no w2db e no webhook falso do Discord.
///
/// Cada resposta é conferida campo a campo no layout do cliente. Os valores esperados do
/// personagem vêm do BaseMob sintético que o `tools/server_boot_smoke.py` grava (ver
/// `synthetic_base_mob` lá): se um campo chegar fora do lugar, o valor lido sai errado.
///
/// Uso: LoginClient <ip> <porta> <conta> <senha> <senha numérica> <personagem> <classe>
///                  <conta B> <personagem B> <classe B> [dupe]
/// A conta B usa a mesma senha e a mesma senha numérica. Código de saída 0 quando tudo confere.

#include "core/pch.h"
#include <WinSock2.h>
#include "common/Basedef.h"
#include "net/CPSock.h"

#include <cctype>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <functional>
#include <string>
#include <vector>

extern unsigned int CurrentTime;  // core/TMGlobal.cpp: o CPSock grava em MSG_STANDARD::Tick

namespace
{

// Opcodes que o cliente trata pelo número (TMFieldScene_Packets.cpp, TMHuman_Packets.cpp).
constexpr unsigned short kTypeMessagePanel = MSG_MessagePanel_Opcode;
constexpr unsigned short kTypeCreateItem = 0x26E;
constexpr unsigned short kTypeCNFDropItem = 0x175;
constexpr unsigned short kTypeCNFGetItem = 0x171;
constexpr unsigned short kTypeRemoveItem = 0x16F;
constexpr unsigned short kTypeSetHpMp = 0x181;
constexpr unsigned short kTypeRemoveMob = 0x165;
constexpr unsigned short kTypeCharacterLogout = 0x215;   // TMFieldScene_FrameMove.cpp
constexpr unsigned short kTypeCNFCharacterLogout = 0x116;

constexpr int kStepTimeoutMs = 30000;
constexpr int kPlaceCarry = 1;
constexpr short kPotion = 412;   // PocaoTeste (EF_VOLATILE 1, EF_HP), em Carry[0]
constexpr short kBag = 3467;     // bolsa: libera Carry[45..59] em Carry[61]
constexpr short kSword = 1;      // EspadaTeste em Carry[1], com EF_SANC = classe
constexpr short kArmor = 2;      // ArmaduraTeste em Carry[2], com EF_SANC = classe

int g_failures = 0;

void Fail(const char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	std::printf("FALHA: ");
	std::vprintf(fmt, ap);
	std::printf("\n");
	va_end(ap);
	++g_failures;
}

#define EXPECT(cond, ...) do { if (!(cond)) Fail(__VA_ARGS__); } while (0)

/// @brief Valor esperado de cada campo do BaseMob sintético da classe `cls`.
/// Mantenha igual a `synthetic_base_mob` em tools/server_boot_smoke.py.
struct Expected
{
	int coin;
	long long exp;
	short str, intel, dex, con;
	short equipArmor;      // Equip[1].sIndex
	short carryFirst;      // Carry[0].sIndex
	short carryBag;        // Carry[61].sIndex
	unsigned int learned;  // LearnedSkill[0]
};

Expected ExpectedFor(int cls)
{
	return { 1000 + cls, 5000000000LL + cls, (short)(10 + cls), (short)(20 + cls), (short)(30 + cls),
		(short)(40 + cls), 2, kPotion, kBag, 0x00010203u + (unsigned)cls };
}

bool AllZero(const void* p, size_t n)
{
	const unsigned char* b = static_cast<const unsigned char*>(p);
	for (size_t i = 0; i < n; ++i)
		if (b[i])
			return false;
	return true;
}

using Match = std::function<bool(const char*)>;

class Session
{
public:
	explicit Session(const char* name) : name_(name) {}

	bool Connect(const char* ip, int port)
	{
		if (!sock_.WSAInitialize())
		{
			Fail("WSAStartup");
			return false;
		}
		// Sem janela (hWndMain nulo) o WSAAsyncSelect não liga as notificações; o socket é
		// deixado bloqueante e lido com select() abaixo.
		if (!sock_.ConnectServer(const_cast<char*>(ip), port, INADDR_ANY, WM_USER + 1))
		{
			Fail("%s: conexão com %s:%d recusada", name_, ip, port);
			return false;
		}
		WSAAsyncSelect(sock_.Sock, nullptr, 0, 0);
		u_long blocking = 0;
		ioctlsocket(sock_.Sock, FIONBIO, &blocking);
		return true;
	}

	void SyncServerClock(unsigned int serverTick)
	{
		serverClockDelta_ = static_cast<long long>(serverTick + 100u) - static_cast<long long>(timeGetTime());
		hasServerClock_ = true;
	}

	unsigned int NowTick() const
	{
		if (!hasServerClock_)
			return timeGetTime();
		return static_cast<unsigned int>(static_cast<long long>(timeGetTime()) + serverClockDelta_);
	}

	/// @brief Envia como o cliente: no jogo, Header.ID é o ClientID; o Tick é o relógio sincronizado
	/// com o servidor após o 0x114.
	void Send(void* msg, int size)
	{
		if (clientId)
			static_cast<MSG_STANDARD*>(msg)->ID = static_cast<unsigned short>(clientId);
		CurrentTime = NowTick();
		sock_.SendOneMessage(static_cast<char*>(msg), size);
	}

	/// @brief Espera um pacote do tipo `type` (e que passe em `match`, se houver). Pacotes que
	/// chegam antes ficam guardados para as esperas seguintes, porque o servidor nem sempre manda
	/// na ordem em que o teste os confere. Registra o texto dos `MSG_MessagePanel`, que é onde o
	/// servidor explica uma recusa. Copia o pacote para `out`.
	bool Wait(unsigned short type, void* out, int outSize, int* gotSize = nullptr, const Match& match = nullptr)
	{
		const DWORD end = GetTickCount() + kStepTimeoutMs;
		for (;;)
		{
			for (auto it = pending_.begin(); it != pending_.end(); ++it)
			{
				const MSG_STANDARD* h = reinterpret_cast<const MSG_STANDARD*>(it->data());
				if (h->Type != type || (match && !match(it->data())))
					continue;
				std::memset(out, 0, outSize);
				std::memcpy(out, it->data(), it->size() < (size_t)outSize ? it->size() : outSize);
				if (gotSize)
					*gotSize = (int)it->size();
				pending_.erase(it);
				return true;
			}
			if ((int)(end - GetTickCount()) <= 0)
				break;
			if (!Pump(type))
				return false;
		}
		Fail("%s: tempo esgotado esperando 0x%X", name_, type);
		return false;
	}

	/// @brief Descarta o que já chegou (pacotes de entrada no jogo que o teste não confere).
	void Drain()
	{
		const DWORD end = GetTickCount() + 500;
		while ((int)(end - GetTickCount()) > 0 && Pump(0))
			;
		pending_.clear();
	}

	const char* name_;
	int clientId = 0;
	CPSock sock_;

private:
	/// @brief Lê o que houver no socket (até 200 ms) para `pending_`.
	bool Pump(unsigned short waiting)
	{
		int err = 0, errType = 0;
		while (char* msg = sock_.ReadMessage(&err, &errType))
		{
			const MSG_STANDARD* h = reinterpret_cast<const MSG_STANDARD*>(msg);
			if (err)
				Fail("%s: pacote 0x%X com erro %d (%d) na leitura", name_, h->Type, err, errType);
			if (h->Type == kTypeMessagePanel)
			{
				const MSG_MessagePanel* p = reinterpret_cast<const MSG_MessagePanel*>(msg);
				std::printf("  %s, servidor: \"%.*s\"\n", name_, (int)sizeof(p->String), p->String);
			}
			pending_.emplace_back(msg, msg + h->Size);
		}
		if (err)
		{
			Fail("%s: leitura do socket: erro %d (%d)", name_, err, errType);
			return false;
		}

		fd_set rd;
		FD_ZERO(&rd);
		FD_SET(sock_.Sock, &rd);
		timeval tv{ 0, 200000 };
		if (select(0, &rd, nullptr, nullptr, &tv) <= 0)
			return true;
		const int before = sock_.nRecvPosition;
		if (sock_.Receive() != 1 || sock_.nRecvPosition == before)
		{
			Fail("%s: o servidor fechou a conexão esperando 0x%X", name_, waiting);
			return false;
		}
		return true;
	}

	bool hasServerClock_ = false;
	long long serverClockDelta_ = 0;
	std::deque<std::vector<char>> pending_;
};

struct Account
{
	const char* account;
	const char* password;
	const char* numeric;
	const char* charName;
	int cls;
};

/// @brief Personagem no jogo, como o 0x114 o trouxe.
struct InGame
{
	int clientId = 0;
	int posX = 0, posY = 0;
	STRUCT_MOB mob{};
};

/// @brief Confere o 0x114 (entrada no jogo) e guarda o personagem em `out`.
bool CheckCharacterLogin(Session& s, const Account& a, InGame& out)
{
	const Expected want = ExpectedFor(a.cls);
	static MSG_CNFCharacterLogin r;
	int size = 0;
	if (!s.Wait(MSG_CNFCharacterLogin_Opcode, &r, sizeof(r), &size))
		return false;
	EXPECT(size == (int)sizeof(MSG_CNFCharacterLogin), "0x114 com %d bytes, o cliente espera %d", size, (int)sizeof(MSG_CNFCharacterLogin));
	const STRUCT_MOB& mob = r.MOB;
	EXPECT(std::strncmp(mob.MobName, a.charName, 16) == 0, "0x114 MOB.MobName \"%.16s\"", mob.MobName);
	EXPECT(mob.Class == a.cls, "0x114 MOB.Class %d, esperado %d", mob.Class, a.cls);
	EXPECT(mob.Coin == want.coin, "0x114 MOB.Coin %d, esperado %d", mob.Coin, want.coin);
	EXPECT(mob.Exp == want.exp, "0x114 MOB.Exp %lld, esperado %lld", mob.Exp, want.exp);
	EXPECT(mob.BaseScore.Str == want.str && mob.BaseScore.Int == want.intel && mob.BaseScore.Dex == want.dex
		&& mob.BaseScore.Con == want.con, "0x114 MOB.BaseScore Str/Int/Dex/Con %d/%d/%d/%d",
		mob.BaseScore.Str, mob.BaseScore.Int, mob.BaseScore.Dex, mob.BaseScore.Con);
	EXPECT(mob.Equip[1].sIndex == want.equipArmor, "0x114 MOB.Equip[1] %d, esperado %d", mob.Equip[1].sIndex, want.equipArmor);
	EXPECT(mob.Carry[61].sIndex == want.carryBag, "0x114 MOB.Carry[61] %d, esperado %d (bolsa)", mob.Carry[61].sIndex, want.carryBag);
	EXPECT(mob.LearnedSkill[0] == want.learned, "0x114 MOB.LearnedSkill[0] 0x%X, esperado 0x%X", mob.LearnedSkill[0], want.learned);
	EXPECT(mob.LearnedSkill[1] == 0, "0x114 MOB.LearnedSkill[1] 0x%X, esperado 0", mob.LearnedSkill[1]);
	EXPECT(AllZero(mob.dummy, sizeof(mob.dummy)) && mob.CurrentKill == 0 && mob.TotalKill == 0,
		"0x114: MOB.dummy/CurrentKill/TotalKill não vieram zerados");
	EXPECT(r.Slot == 0, "0x114 Slot %d", r.Slot);
	EXPECT(r.ClientID > 0 && r.ClientID < 1000, "0x114 ClientID %d", r.ClientID);
	EXPECT(r.PosX > 0 && r.PosX < 4096 && r.PosY > 0 && r.PosY < 4096, "0x114 posição %d,%d fora do mapa", r.PosX, r.PosY);
	EXPECT(AllZero(&r.Ext1, sizeof(r.Ext1)), "0x114: Ext1 não veio zerado");
	EXPECT(AllZero(&r.Ext2, sizeof(r.Ext2)), "0x114: Ext2 não veio zerado");
	out.clientId = r.ClientID;
	out.posX = r.PosX;
	out.posY = r.PosY;
	out.mob = mob;
	s.SyncServerClock(r.Header.Tick);
	std::printf("   0x114 com %d bytes, \"%.16s\" em %d,%d (ClientID %d)\n", size, mob.MobName, r.PosX, r.PosY, r.ClientID);
	return true;
}

/// @brief Passos 1 a 5: conta, senha numérica, personagem na vaga 0 (e, com `deleteTest`, outro
/// criado e apagado na vaga 1) e entrada no jogo.
bool Login(Session& s, const Account& a, bool deleteTest, InGame& out)
{
	const Expected want = ExpectedFor(a.cls);

	// 1. Conta. Como TMSelectServerScene: Force = 1 e a versão do cliente.
	{
		MSG_AccountLogin m{};
		m.Header.Type = MSG_AccountLogin_Opcode;
		std::strncpy(m.AccountPass, a.password, sizeof(m.AccountPass) - 1);
		std::strncpy(m.AccountName, a.account, sizeof(m.AccountName) - 1);
		m.Version = 1758;
		m.Force = 1;
		s.Send(&m, sizeof(m));

		static MSG_CNFAccountLogin r;
		int size = 0;
		if (!s.Wait(MSG_CNFAccountLogin_Opcode, &r, sizeof(r), &size))
			return false;
		EXPECT(size == (int)sizeof(MSG_CNFAccountLogin), "0x10A com %d bytes, o cliente espera %d", size, (int)sizeof(MSG_CNFAccountLogin));
		char upper[16]{};
		for (int i = 0; i < 15 && a.account[i]; ++i)
			upper[i] = (char)toupper((unsigned char)a.account[i]);
		EXPECT(std::strncmp(r.AccountName, upper, 16) == 0, "0x10A AccountName \"%.16s\", esperado \"%s\"", r.AccountName, upper);
		for (int i = 0; i < 4; ++i)
			EXPECT(r.SelChar.MobName[i][0] == 0, "0x10A: conta nova com personagem na vaga %d", i);
		// Como TMSelectServerScene: a chave do envio vem do SecretCode.
		for (int i = 0; i < 16; ++i)
			s.sock_.SendQueue[i] = r.SecretCode[i];
		s.sock_.SendCount = 0;
		s.sock_.RecvCount = 0;
		std::printf("1. %s, conta: 0x10A com %d bytes\n", s.name_, size);
	}

	// 2. Senha numérica (conta nova: grava a senha e libera a conta).
	{
		MSG_CHARPASSWORD m{};
		m.Header.Type = MSG_CharPassword_Opcode;
		std::strncpy(m.ItemPassWord, a.numeric, 6);
		m.State = 0;
		s.Send(&m, sizeof(m));
		MSG_STANDARD r{};
		if (!s.Wait(MSG_CharPassword_Opcode, &r, sizeof(r)))
			return false;
		std::printf("2. %s, senha numérica: eco 0xFDE\n", s.name_);
	}

	// 3. Personagem na vaga 0.
	{
		MSG_NewCharacter m{};
		m.Header.Type = MSG_NewCharacter_Opcode;
		m.Slot = 0;
		m.Class = a.cls;
		std::snprintf(m.MobName, sizeof(m.MobName), "%s", a.charName);
		s.Send(&m, sizeof(m));

		static MSG_CNFNewCharacter r;
		int size = 0;
		if (!s.Wait(MSG_CNFNewCharacter_Opcode, &r, sizeof(r), &size))
			return false;
		EXPECT(size == (int)sizeof(MSG_CNFNewCharacter), "0x110 com %d bytes, esperado %d", size, (int)sizeof(MSG_CNFNewCharacter));
		const STRUCT_SELCHAR& sel = r.SelChar;
		EXPECT(std::strncmp(sel.MobName[0], a.charName, 16) == 0, "0x110 MobName[0] \"%.16s\"", sel.MobName[0]);
		EXPECT(sel.Coin[0] == want.coin, "0x110 Coin[0] %d, esperado %d", sel.Coin[0], want.coin);
		EXPECT(sel.Exp[0] == want.exp, "0x110 Exp[0] %lld, esperado %lld", sel.Exp[0], want.exp);
		EXPECT(sel.Score[0].Str == want.str && sel.Score[0].Con == want.con, "0x110 Score[0] Str/Con %d/%d", sel.Score[0].Str, sel.Score[0].Con);
		EXPECT(sel.Equip[0][1].sIndex == want.equipArmor, "0x110 Equip[0][1] %d", sel.Equip[0][1].sIndex);
		std::printf("3. %s, personagem \"%s\" criado na vaga 0\n", s.name_, a.charName);
	}

	// 4. Personagem descartável na vaga 1, apagado com a senha da conta.
	if (deleteTest)
	{
		MSG_NewCharacter m{};
		m.Header.Type = MSG_NewCharacter_Opcode;
		m.Slot = 1;
		m.Class = a.cls;
		std::snprintf(m.MobName, sizeof(m.MobName), "%.10sDel", a.charName);
		s.Send(&m, sizeof(m));
		static MSG_CNFNewCharacter r;
		if (!s.Wait(MSG_CNFNewCharacter_Opcode, &r, sizeof(r)))
			return false;
		EXPECT(std::strncmp(r.SelChar.MobName[1], m.MobName, 16) == 0, "0x110 MobName[1] \"%.16s\"", r.SelChar.MobName[1]);

		MSG_DeleteCharacter d{};
		d.Header.Type = MSG_DeleteCharacter_Opcode;
		d.Slot = 1;
		std::memcpy(d.MobName, m.MobName, sizeof(d.MobName));
		std::strncpy(d.Password, a.password, sizeof(d.Password) - 1);
		s.Send(&d, sizeof(d));
		static MSG_CNFDeleteCharacter rd;
		if (!s.Wait(MSG_CNFDeleteCharacter_Opcode, &rd, sizeof(rd)))
			return false;
		EXPECT(rd.SelChar.MobName[1][0] == 0, "0x112: vaga 1 ainda tem \"%.16s\"", rd.SelChar.MobName[1]);
		EXPECT(std::strncmp(rd.SelChar.MobName[0], a.charName, 16) == 0, "0x112: vaga 0 com \"%.16s\"", rd.SelChar.MobName[0]);
		std::printf("4. %s, personagem \"%s\" criado e apagado na vaga 1\n", s.name_, m.MobName);
	}

	// 5. Entrada no jogo.
	{
		MSG_CharacterLogin m{};
		m.Header.Type = MSG_CharacterLogin_Opcode;
		m.Slot = 0;
		s.Send(&m, sizeof(m));
		std::printf("5. %s, entrada no jogo:\n", s.name_);
		if (!CheckCharacterLogin(s, a, out))
			return false;
		EXPECT(out.mob.Carry[0].sIndex == want.carryFirst, "0x114 MOB.Carry[0] %d, esperado %d", out.mob.Carry[0].sIndex, want.carryFirst);
		s.clientId = out.clientId;
	}
	return true;
}

Match FromId(int id)
{
	return [id](const char* p) { return reinterpret_cast<const MSG_STANDARD*>(p)->ID == id; };
}

/// @brief Passos 6 a 13, com A e B no jogo.
bool Play(Session& sa, InGame& a, const Account& accA, Session& sb, InGame& b)
{
	// 6. Cada um vê o outro: B recebe A pelo SendGridMob da entrada; A recebe B pelo GridMulticast.
	{
		auto byMob = [](int id) { return [id](const char* p) { return reinterpret_cast<const MSG_CreateMob*>(p)->MobID == id; }; };
		static MSG_CreateMob r;
		int size = 0;
		if (!sb.Wait(MSG_CreateMob_Opcode, &r, sizeof(r), &size, byMob(a.clientId)))
			return false;
		EXPECT(size == (int)sizeof(MSG_CreateMob), "0x364 com %d bytes, o cliente espera %d", size, (int)sizeof(MSG_CreateMob));
		EXPECT(std::strncmp(r.MobName, a.mob.MobName, 16) == 0, "0x364 (A para B) MobName \"%.16s\"", r.MobName);
		EXPECT(r.PosX == a.posX && r.PosY == a.posY, "0x364 (A para B) em %d,%d, A está em %d,%d", r.PosX, r.PosY, a.posX, a.posY);
		EXPECT(r.Equip[1] == a.mob.Equip[1].sIndex, "0x364 (A para B) Equip[1] %d", r.Equip[1]);
		if (!sa.Wait(MSG_CreateMob_Opcode, &r, sizeof(r), &size, byMob(b.clientId)))
			return false;
		EXPECT(std::strncmp(r.MobName, b.mob.MobName, 16) == 0, "0x364 (B para A) MobName \"%.16s\"", r.MobName);
		EXPECT(r.PosX == b.posX && r.PosY == b.posY, "0x364 (B para A) em %d,%d, B está em %d,%d", r.PosX, r.PosY, b.posX, b.posY);
		std::printf("6. A e B se veem (0x364 com %d bytes)\n", size);
		sa.Drain();
	}

	// 7. Chat de A, que o servidor repassa a quem está perto (sem ecoar para A).
	{
		MSG_MessageChat m{};
		m.Header.Type = MSG_MessageChat_Opcode;
		std::snprintf(m.String, sizeof(m.String), "ola de %s", a.mob.MobName);
		sa.Send(&m, sizeof(m));
		static MSG_MessageChat r;
		if (!sb.Wait(MSG_MessageChat_Opcode, &r, sizeof(r), nullptr, FromId(a.clientId)))
			return false;
		EXPECT(std::strncmp(r.String, m.String, sizeof(m.String)) == 0, "0x333 chegou a B com \"%.128s\"", r.String);
		std::printf("7. chat de A chegou a B: \"%s\"\n", r.String);
	}

	// 8. Movimento de A. Primeiro para a casa de B, ocupada: o TMSrv devolve a A o pacote com outro
	// destino. Depois para uma casa livre, que chega a B.
	{
		MSG_Action m{};
		m.Header.Type = MSG_Action_Opcode;
		m.PosX = (short)a.posX;
		m.PosY = (short)a.posY;
		m.Speed = a.mob.CurrentScore.AttackRun & 0xF;
		m.TargetX = (unsigned short)b.posX;
		m.TargetY = (unsigned short)b.posY;
		sa.Send(&m, sizeof(m));
		static MSG_Action r;
		int size = 0;
		if (!sa.Wait(MSG_Action_Opcode, &r, sizeof(r), &size, FromId(a.clientId)))
			return false;
		EXPECT(size == (int)sizeof(MSG_Action), "0x36C (casa ocupada) com %d bytes, o cliente espera %d", size, (int)sizeof(MSG_Action));
		EXPECT(r.TargetX != b.posX || r.TargetY != b.posY, "0x36C (casa ocupada) manteve o destino %d,%d", r.TargetX, r.TargetY);
		std::printf("8. movimento de A para a casa de B: voltou com destino %d,%d\n", r.TargetX, r.TargetY);

		// Casa livre: 3 casas ao lado de A, do lado oposto ao de B.
		const int dx = b.posX > a.posX ? -3 : 3;
		m.TargetX = (unsigned short)(a.posX + dx);
		m.TargetY = (unsigned short)a.posY;
		Sleep(1000);  // o TMSrv recusa um movimento a menos de 900 ms do anterior
		sa.Send(&m, sizeof(m));
		if (!sb.Wait(MSG_Action_Opcode, &r, sizeof(r), &size, FromId(a.clientId)))
			return false;
		EXPECT(r.TargetX == m.TargetX && r.TargetY == m.TargetY, "0x36C chegou a B com destino %d,%d, esperado %d,%d",
			r.TargetX, r.TargetY, m.TargetX, m.TargetY);
		a.posX = m.TargetX;
		std::printf("   movimento de A para %d,%d chegou a B\n", r.TargetX, r.TargetY);
	}

	// 9. A solta a poção de Carry[0] onde está.
	MSG_CreateItem dropped{};
	{
		MSG_DropItem m{};
		m.Header.Type = MSG_DropItem_Opcode;
		m.SourType = kPlaceCarry;
		m.SourPos = 0;
		m.GridX = (unsigned short)a.posX;
		m.GridY = (unsigned short)a.posY;
		sa.Send(&m, sizeof(m));
		static MSG_CNFDropItem r;
		int size = 0;
		if (!sa.Wait(kTypeCNFDropItem, &r, sizeof(r), &size))
			return false;
		EXPECT(size == (int)sizeof(MSG_CNFDropItem), "0x175 com %d bytes, o cliente espera %d", size, (int)sizeof(MSG_CNFDropItem));
		EXPECT(r.SourType == kPlaceCarry && r.SourPos == 0, "0x175 SourType/SourPos %d/%d", r.SourType, r.SourPos);
		if (!sa.Wait(kTypeCreateItem, &dropped, sizeof(dropped), &size))
			return false;
		EXPECT(size == (int)sizeof(MSG_CreateItem), "0x26E com %d bytes, o cliente espera %d", size, (int)sizeof(MSG_CreateItem));
		EXPECT(dropped.Item.sIndex == kPotion, "0x26E Item.sIndex %d, esperado %d", dropped.Item.sIndex, kPotion);
		EXPECT(dropped.ItemID > 10000, "0x26E ItemID %d", dropped.ItemID);
		EXPECT(dropped.GridX == r.GridX && dropped.GridY == r.GridY, "0x26E em %d,%d, o 0x175 disse %d,%d",
			dropped.GridX, dropped.GridY, r.GridX, r.GridY);
		static MSG_CreateItem rb;
		if (!sb.Wait(kTypeCreateItem, &rb, sizeof(rb)))
			return false;
		EXPECT(rb.ItemID == dropped.ItemID && rb.Item.sIndex == kPotion, "0x26E para B: ItemID %d, item %d", rb.ItemID, rb.Item.sIndex);
		std::printf("9. A soltou a poção em %d,%d (ItemID %d), B viu\n", dropped.GridX, dropped.GridY, dropped.ItemID);
	}

	// 10. A pega a poção para Carry[5].
	{
		MSG_GetItem m{};
		m.Header.Type = MSG_GetItem_Opcode;
		m.DestType = kPlaceCarry;
		m.DestPos = 5;
		m.ItemID = dropped.ItemID;
		m.GridX = dropped.GridX;
		m.GridY = dropped.GridY;
		sa.Send(&m, sizeof(m));
		static MSG_CNFGetItem r;
		int size = 0;
		if (!sa.Wait(kTypeCNFGetItem, &r, sizeof(r), &size))
			return false;
		EXPECT(size == (int)sizeof(MSG_CNFGetItem), "0x171 com %d bytes, o cliente espera %d", size, (int)sizeof(MSG_CNFGetItem));
		EXPECT(r.DestType == kPlaceCarry && r.DestPos == 5, "0x171 DestType/DestPos %d/%d", r.DestType, r.DestPos);
		// OnPacketCNFGetItem põe este item no inventário.
		EXPECT(r.Item.sIndex == kPotion, "0x171 Item.sIndex %d, esperado %d (o cliente põe este item na grade)", r.Item.sIndex, kPotion);
		static MSG_SendItem si;
		if (!sa.Wait(MSG_SendItem_Opcode, &si, sizeof(si), &size))
			return false;
		EXPECT(si.DestType == kPlaceCarry && si.DestPos == 5 && si.Item.sIndex == kPotion, "0x182 %d/%d item %d",
			si.DestType, si.DestPos, si.Item.sIndex);
		MSG_STANDARDPARM rm{};
		if (!sb.Wait(kTypeRemoveItem, &rm, sizeof(rm)))
			return false;
		EXPECT(rm.Parm == dropped.ItemID, "0x16F para B: Parm %d, esperado %d", rm.Parm, dropped.ItemID);
		std::printf("10. A pegou a poção para Carry[5]; o item saiu do chão para B\n");
	}

	// 11. Troca de posição. No 0x376, Sour* é onde o item é solto e Dest* de onde ele sai. Carry[35]
	// depende da bolsa de Carry[60], que A não tem: o TMSrv recusa soltar lá e também tirar de lá
	// (a troca levaria o item de destino para a origem). Carry[50] depende da bolsa de Carry[61],
	// que A tem: o TMSrv ecoa o pedido.
	{
		auto swap = [&](int to, int from) {
			MSG_SwapItem m{};
			m.Header.Type = MSG_SwapItem_Opcode;
			m.SourType = kPlaceCarry;
			m.SourPos = (char)to;
			m.DestType = kPlaceCarry;
			m.DestPos = (char)from;
			sa.Send(&m, sizeof(m));
		};
		swap(35, 5);
		swap(6, 35);
		swap(50, 5);
		static MSG_SwapItem r;
		if (!sa.Wait(MSG_SwapItem_Opcode, &r, sizeof(r)))
			return false;
		EXPECT(r.SourPos == 50 && r.DestPos == 5, "0x376 de Carry[%d] para Carry[%d]: uma troca com Carry[35], sem bolsa, passou",
			r.DestPos, r.SourPos);
		std::printf("11. trocas com Carry[35] recusadas (sem bolsa em Carry[60]); Carry[5] -> Carry[50] aceita\n");
	}

	// 12. A usa a poção de Carry[50].
	{
		MSG_UseItem m{};
		m.Header.Type = MSG_UseItem_Opcode;
		m.SourType = kPlaceCarry;
		m.SourPos = 50;
		m.GridX = (unsigned short)a.posX;
		m.GridY = (unsigned short)a.posY;
		sa.Send(&m, sizeof(m));
		static MSG_SetHpMp r;
		int size = 0;
		if (!sa.Wait(kTypeSetHpMp, &r, sizeof(r), &size))
			return false;
		EXPECT(size == (int)sizeof(MSG_SetHpMp), "0x181 com %d bytes, o cliente espera %d", size, (int)sizeof(MSG_SetHpMp));
		EXPECT(r.Header.ID == a.clientId, "0x181 ID %d, esperado %d", r.Header.ID, a.clientId);
		EXPECT(r.ReqHp >= r.Hp && r.ReqHp <= a.mob.CurrentScore.MaxHp, "0x181 Hp %d ReqHp %d MaxHp %d", r.Hp, r.ReqHp,
			a.mob.CurrentScore.MaxHp);
		std::printf("12. A usou a poção: 0x181 Hp %d ReqHp %d\n", r.Hp, r.ReqHp);
	}

	// 13. A sai do personagem e entra de novo: o inventário volta do DBSrv.
	{
		MSG_STANDARD m{};
		m.Type = kTypeCharacterLogout;
		sa.Send(&m, sizeof(m));
		MSG_STANDARD r{};
		if (!sa.Wait(kTypeCNFCharacterLogout, &r, sizeof(r)))
			return false;
		MSG_STANDARD rm{};
		if (!sb.Wait(kTypeRemoveMob, &rm, sizeof(rm), nullptr, FromId(a.clientId)))
			return false;
		std::printf("13. A saiu do personagem (0x116), B recebeu o 0x165; A entra de novo:\n");

		const int id = sa.clientId;
		sa.clientId = 0;
		MSG_CharacterLogin l{};
		l.Header.Type = MSG_CharacterLogin_Opcode;
		l.Slot = 0;
		sa.Send(&l, sizeof(l));
		InGame again;
		if (!CheckCharacterLogin(sa, accA, again))
			return false;
		sa.clientId = again.clientId;
		a.posX = again.posX;
		a.posY = again.posY;
		EXPECT(again.clientId == id, "0x114 ClientID %d na volta, era %d", again.clientId, id);
		for (int pos : { 0, 5, 35, 50 })
			EXPECT(again.mob.Carry[pos].sIndex == 0, "0x114 na volta: Carry[%d] com %d (a poção foi gasta)", pos, again.mob.Carry[pos].sIndex);
		std::printf("   inventário gravado: poção gasta, bolsa em Carry[61]\n");
	}

	// 14. /day (literal curta em SendClientMessage) e spawn/combate/abate de NPC do NPCGener.txt.
	{
		MSG_MessageWhisper w{};
		w.Header.Type = MSG_MessageWhisper_Opcode;
		std::snprintf(w.MobName, sizeof(w.MobName), "day");
		sa.Send(&w, sizeof(w));

		MSG_MessagePanel mp{};
		auto isDayReply = [](const char* p) {
			return std::strncmp(reinterpret_cast<const MSG_MessagePanel*>(p)->String, "!#11  2", 7) == 0;
		};
		if (!sa.Wait(kTypeMessagePanel, &mp, sizeof(mp), nullptr, isDayReply))
			return false;
		std::printf("14. /day respondido pelo servidor\n");

		static MSG_CreateMob alvo;
		int alvoSize = 0;
		auto isAlvo = [](const char* p) {
			const auto* cm = reinterpret_cast<const MSG_CreateMob*>(p);
			return cm->MobID >= 1000 && std::strncmp(cm->MobName, "AlvoTeste", 16) == 0;
		};
		if (!sa.Wait(MSG_CreateMob_Opcode, &alvo, sizeof(alvo), &alvoSize, isAlvo))
			return false;

		const int goalX = alvo.PosX + (a.posX >= alvo.PosX ? 1 : -1);
		const int goalY = alvo.PosY;
		while (a.posX != goalX || a.posY != goalY)
		{
			int dx = goalX - a.posX;
			int dy = goalY - a.posY;
			if (dx > 4) dx = 4;
			if (dx < -4) dx = -4;
			if (dy > 4) dy = 4;
			if (dy < -4) dy = -4;

			MSG_Action act{};
			act.Header.Type = MSG_Action_Opcode;
			act.PosX = static_cast<short>(a.posX);
			act.PosY = static_cast<short>(a.posY);
			act.TargetX = static_cast<unsigned short>(a.posX + dx);
			act.TargetY = static_cast<unsigned short>(a.posY + dy);
			act.Speed = a.mob.CurrentScore.AttackRun & 0xF;
			Sleep(1000);
			sa.Send(&act, sizeof(act));
			a.posX += dx;
			a.posY += dy;
		}

		bool damaged = false;
		for (int hit = 0; hit < 10 && !damaged; ++hit)
		{
			Sleep(1050);
			MSG_AttackOne atk{};
			atk.Header.Type = MSG_Attack_One_Opcode;
			atk.PosX = static_cast<unsigned short>(a.posX);
			atk.PosY = static_cast<unsigned short>(a.posY);
			atk.TargetX = static_cast<unsigned short>(alvo.PosX);
			atk.TargetY = static_cast<unsigned short>(alvo.PosY);
			atk.AttackerID = static_cast<unsigned short>(a.clientId);
			atk.Motion = 4;
			atk.SkillIndex = -1;
			atk.Dam[0].TargetID = alvo.MobID;
			atk.Dam[0].Damage = -2;
			sa.Send(&atk, sizeof(atk));

			static MSG_AttackOne atkEco;
			auto byAttacker = [&a](const char* p) {
				const auto* rsp = reinterpret_cast<const MSG_AttackOne*>(p);
				return rsp->AttackerID == a.clientId && rsp->Dam[0].TargetID == alvo.MobID;
			};
			if (!sa.Wait(MSG_Attack_One_Opcode, &atkEco, sizeof(atkEco), nullptr, byAttacker))
				return false;
			if (atkEco.Dam[0].Damage > 0)
				damaged = true;
		}
		EXPECT(damaged, "0x39D contra AlvoTeste não causou dano");

		MSG_CNFMobKill mk{};
		auto isAlvoKill = [](const char* p) {
			return reinterpret_cast<const MSG_CNFMobKill*>(p)->KilledMob == alvo.MobID;
		};
		if (!sa.Wait(MSG_CNFMobKill_Opcode, &mk, sizeof(mk), nullptr, isAlvoKill))
			return false;
		EXPECT(mk.Killer == a.clientId, "0x338 Killer %u, esperado %d", mk.Killer, a.clientId);
		std::printf("   AlvoTeste (MobID %u) abatido (Killer=%u)\n", alvo.MobID, mk.Killer);
	}
	return true;
}

/// @brief Acha `item` no inventário (-1 se não há).
int FindCarry(const STRUCT_ITEM* carry, const STRUCT_ITEM& item)
{
	for (int i = 0; i < 64; ++i)
		if (std::memcmp(&carry[i], &item, sizeof(STRUCT_ITEM)) == 0)
			return i;
	return -1;
}

/// @brief Troca entre A e B: cada um oferece o item da vaga dada (-1: nada), A confirma e depois B
/// (o TMSrv fecha a troca na confirmação de B). Devolve o inventário de cada um depois da troca.
bool Trade(Session& sa, InGame& a, int posA, Session& sb, InGame& b, int posB, STRUCT_ITEM* carryA, STRUCT_ITEM* carryB)
{
	auto offer = [](const InGame& who, const InGame& other, int pos) {
		MSG_Trade m{};
		m.Header.Type = MSG_Trade_Opcode;
		for (int i = 0; i < 15; ++i)
			m.CarryPos[i] = -1;
		if (pos >= 0)
		{
			m.Item[0] = who.mob.Carry[pos];
			m.CarryPos[0] = (char)pos;
		}
		m.OpponentID = (unsigned short)other.clientId;
		return m;
	};
	MSG_Trade ma = offer(a, b, posA);
	MSG_Trade mb = offer(b, a, posB);
	static MSG_Trade r;

	sa.Send(&ma, sizeof(ma));
	if (!sb.Wait(MSG_Trade_Opcode, &r, sizeof(r)))
		return false;
	sb.Send(&mb, sizeof(mb));
	if (!sa.Wait(MSG_Trade_Opcode, &r, sizeof(r)))
		return false;
	ma.MyCheck = 1;
	sa.Send(&ma, sizeof(ma));
	if (!sb.Wait(MSG_Trade_Opcode, &r, sizeof(r)))
		return false;
	mb.MyCheck = 1;
	sb.Send(&mb, sizeof(mb));

	static MSG_Carry ca, cb;
	if (!sa.Wait(MSG_UpdateCarry_Opcode, &ca, sizeof(ca)) || !sb.Wait(MSG_UpdateCarry_Opcode, &cb, sizeof(cb)))
		return false;
	std::memcpy(carryA, ca.Carry, sizeof(ca.Carry));
	std::memcpy(carryB, cb.Carry, sizeof(cb.Carry));
	std::memcpy(a.mob.Carry, ca.Carry, sizeof(ca.Carry));
	std::memcpy(b.mob.Carry, cb.Carry, sizeof(cb.Carry));
	return true;
}

/// @brief Passos 15 e 16: troca de mão dupla e, com `dupe`, a duplicação forçada.
bool Trades(Session& sa, InGame& a, Session& sb, InGame& b, bool dupe)
{
	sa.Drain();
	sb.Drain();
	{
		const STRUCT_ITEM swordA = a.mob.Carry[1];
		const STRUCT_ITEM swordB = b.mob.Carry[1];
		EXPECT(swordA.sIndex == kSword && swordB.sIndex == kSword, "Carry[1] de A/B com %d/%d, esperado a espada %d",
			swordA.sIndex, swordB.sIndex, kSword);
		STRUCT_ITEM ca[64], cb[64];
		if (!Trade(sa, a, 1, sb, b, 1, ca, cb))
			return false;
		EXPECT(FindCarry(ca, swordB) >= 0 && FindCarry(ca, swordA) < 0, "0x185 de A: sem a espada de B ou ainda com a sua");
		EXPECT(FindCarry(cb, swordA) >= 0 && FindCarry(cb, swordB) < 0, "0x185 de B: sem a espada de A ou ainda com a sua");
		std::printf("15. troca de mão dupla: a espada de A (Carry[1]) foi para Carry[%d] de B, a de B para Carry[%d] de A\n",
			FindCarry(cb, swordA), FindCarry(ca, swordB));
	}
	if (dupe)
	{
		MSG_MessageWhisper w{};
		w.Header.Type = MSG_MessageWhisper_Opcode;
		std::snprintf(w.MobName, sizeof(w.MobName), "w2dupe");
		sa.Send(&w, sizeof(w));
		MSG_MessagePanel mp{};
		auto isHook = [](const char* p) {
			return std::strncmp(reinterpret_cast<const MSG_MessagePanel*>(p)->String, "w2dupe", 6) == 0;
		};
		if (!sa.Wait(kTypeMessagePanel, &mp, sizeof(mp), nullptr, isHook))
		{
			Fail("A: /w2dupe sem resposta (TMSrv sem DERUST_TEST_HOOKS?)");
			return false;
		}
		const STRUCT_ITEM armorA = a.mob.Carry[2];
		EXPECT(armorA.sIndex == kArmor, "Carry[2] de A com %d, esperado a armadura %d", armorA.sIndex, kArmor);
		STRUCT_ITEM ca[64], cb[64];
		if (!Trade(sa, a, 2, sb, b, -1, ca, cb))
			return false;
		EXPECT(FindCarry(cb, armorA) >= 0, "0x185 de B: sem a armadura de A");
		EXPECT(FindCarry(ca, armorA) >= 0, "0x185 de A: o gancho /w2dupe não deixou a cópia da armadura");
		std::printf("16. duplicação forçada: A deu a armadura a B e ficou com uma cópia em Carry[%d]\n", FindCarry(ca, armorA));
	}
	return true;
}

/// O texto do /bug do passo 17. Igual ao BUG_TEXT de tools/server_boot_smoke.py, que o procura no
/// banco e no webhook (o @everyone confere que o Discord não menciona ninguém).
const char kBugText[] = "smoke: a porta do castelo nao abre @everyone";

bool Whisper(Session& s, const char* command, const char* text, const char* reply)
{
	MSG_MessageWhisper w{};
	w.Header.Type = MSG_MessageWhisper_Opcode;
	std::snprintf(w.MobName, sizeof(w.MobName), "%s", command);
	std::snprintf(w.String, sizeof(w.String), "%s", text);
	s.Send(&w, sizeof(w));
	MSG_MessagePanel mp{};
	auto isReply = [reply](const char* p) {
		return std::strncmp(reinterpret_cast<const MSG_MessagePanel*>(p)->String, reply, std::strlen(reply)) == 0;
	};
	if (!s.Wait(kTypeMessagePanel, &mp, sizeof(mp), nullptr, isReply))
	{
		Fail("%s: /%s %s sem a resposta \"%s\"", s.name_, command, text, reply);
		return false;
	}
	return true;
}

/// @brief Passo 17: o /bug.
bool BugReport(Session& sa)
{
	sa.Drain();
	if (!Whisper(sa, "bug", "   ", "Uso: /bug") || !Whisper(sa, "bug", kBugText, "Relato enviado.")
		|| !Whisper(sa, "bug", "outro relato", "Aguarde 1 minuto"))
		return false;
	std::printf("17. /bug: uso sem texto, relato enviado, segundo relato barrado pelo intervalo\n");
	return true;
}

} // namespace

int main(int argc, char** argv)
{
	if (argc != 11 && !(argc == 12 && std::strcmp(argv[11], "dupe") == 0))
	{
		std::printf("uso: %s <ip> <porta> <conta> <senha> <senha numérica> <personagem> <classe>"
			" <conta B> <personagem B> <classe B> [dupe]\n", argv[0]);
		return 2;
	}
	const bool dupe = argc == 12;
	setvbuf(stdout, nullptr, _IONBF, 0);
	const char* ip = argv[1];
	const int port = std::atoi(argv[2]);
	const Account accA{ argv[3], argv[4], argv[5], argv[6], std::atoi(argv[7]) };
	const Account accB{ argv[8], argv[4], argv[5], argv[9], std::atoi(argv[10]) };

	Session sa("A"), sb("B");
	InGame a, b;
	bool ok = sa.Connect(ip, port) && Login(sa, accA, true, a) && sb.Connect(ip, port) && Login(sb, accB, false, b)
		&& Play(sa, a, accA, sb, b) && Trades(sa, a, sb, b, dupe) && BugReport(sa);
	sa.sock_.CloseSocket();
	sb.sock_.CloseSocket();
	if (!ok || g_failures)
	{
		std::printf("login e jogo: %d conferência(s) falharam\n", g_failures);
		return 1;
	}
	std::printf("login e jogo: OK\n");
	return 0;
}
