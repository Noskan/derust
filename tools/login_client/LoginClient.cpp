/// @file LoginClient.cpp
/// @brief Cliente sintético do login, montado com os fontes do cliente (TMProject).
///
/// Usa o `CPSock` e as estruturas de `client/src/common/Basedef.h`, os mesmos que o jogo usa, e faz
/// o caminho do login de ponta a ponta contra um DBSrv + TMSrv no ar:
///   1. `_MSG_AccountLogin` (0x20D) → `MSG_CNFAccountLogin` (0x10A);
///   2. senha numérica `MSG_CHARPASSWORD` (0xFDE) → eco 0xFDE (o DBSrv só cria ou entra com
///      personagem depois dela);
///   3. `MSG_NewCharacter` (0x20F) → `MSG_CNFNewCharacter` (0x110), na vaga 0;
///   4. outro personagem na vaga 1 e `MSG_DeleteCharacter` (0x211) com a senha da conta →
///      `MSG_CNFDeleteCharacter` (0x112);
///   5. `MSG_CharacterLogin` (0x213) → `MSG_CNFCharacterLogin` (0x114).
///
/// Cada resposta é conferida campo a campo no layout do cliente. Os valores esperados do
/// personagem vêm do BaseMob sintético que o `tools/server_boot_smoke.py` grava (ver
/// `synthetic_base_mob` lá): se um campo chegar fora do lugar, o valor lido sai errado.
///
/// Uso: LoginClient <ip> <porta> <conta> <senha> <senha numérica> <personagem> <classe>
/// Código de saída 0 quando todas as etapas conferem.

#include "core/pch.h"
#include <WinSock2.h>
#include "common/Basedef.h"
#include "net/CPSock.h"

#include <cctype>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace
{

constexpr unsigned short kTypeMessagePanel = MSG_MessagePanel_Opcode;
constexpr int kStepTimeoutMs = 30000;

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
	unsigned int learned;  // LearnedSkill[0]
};

Expected ExpectedFor(int cls)
{
	return { 1000 + cls, 5000000000LL + cls, (short)(10 + cls), (short)(20 + cls), (short)(30 + cls),
		(short)(40 + cls), 2, 412, 0x00010203u + (unsigned)cls };
}

bool AllZero(const void* p, size_t n)
{
	const unsigned char* b = static_cast<const unsigned char*>(p);
	for (size_t i = 0; i < n; ++i)
		if (b[i])
			return false;
	return true;
}

class Session
{
public:
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
			Fail("conexão com %s:%d recusada", ip, port);
			return false;
		}
		WSAAsyncSelect(sock_.Sock, nullptr, 0, 0);
		u_long blocking = 0;
		ioctlsocket(sock_.Sock, FIONBIO, &blocking);
		return true;
	}

	void Send(void* msg, int size)
	{
		sock_.SendOneMessage(static_cast<char*>(msg), size);
	}

	/// @brief Espera um pacote do tipo `type`, ignorando os demais (registra o texto dos
	/// `MSG_MessagePanel`, que é onde o servidor explica uma recusa). Copia o pacote para `out`.
	bool Wait(unsigned short type, void* out, int outSize, int* gotSize = nullptr)
	{
		const DWORD end = GetTickCount() + kStepTimeoutMs;
		while ((int)(end - GetTickCount()) > 0)
		{
			int err = 0, errType = 0;
			while (char* msg = sock_.ReadMessage(&err, &errType))
			{
				const MSG_STANDARD* h = reinterpret_cast<const MSG_STANDARD*>(msg);
				if (err)
					Fail("pacote 0x%X com erro %d (%d) na leitura", h->Type, err, errType);
				if (h->Type == kTypeMessagePanel)
				{
					const MSG_MessagePanel* p = reinterpret_cast<const MSG_MessagePanel*>(msg);
					std::printf("  servidor: \"%.*s\"\n", (int)sizeof(p->String), p->String);
				}
				if (h->Type == type)
				{
					std::memset(out, 0, outSize);
					std::memcpy(out, msg, h->Size < outSize ? h->Size : outSize);
					if (gotSize)
						*gotSize = h->Size;
					return true;
				}
				std::printf("  (pacote 0x%X, %d bytes)\n", h->Type, h->Size);
			}
			if (err)
			{
				Fail("leitura do socket: erro %d (%d)", err, errType);
				return false;
			}

			fd_set rd;
			FD_ZERO(&rd);
			FD_SET(sock_.Sock, &rd);
			timeval tv{ 0, 200000 };
			if (select(0, &rd, nullptr, nullptr, &tv) <= 0)
				continue;
			const int before = sock_.nRecvPosition;
			if (sock_.Receive() != 1 || sock_.nRecvPosition == before)
			{
				Fail("o servidor fechou a conexão esperando 0x%X", type);
				return false;
			}
		}
		Fail("tempo esgotado esperando 0x%X", type);
		return false;
	}

	CPSock sock_;
};

} // namespace

int main(int argc, char** argv)
{
	if (argc != 8)
	{
		std::printf("uso: %s <ip> <porta> <conta> <senha> <senha numérica> <personagem> <classe>\n", argv[0]);
		return 2;
	}
	setvbuf(stdout, nullptr, _IONBF, 0);
	const char* ip = argv[1];
	const int port = std::atoi(argv[2]);
	const char* account = argv[3];
	const char* password = argv[4];
	const char* numeric = argv[5];
	const char* charName = argv[6];
	const int cls = std::atoi(argv[7]);
	const Expected want = ExpectedFor(cls);

	Session s;
	if (!s.Connect(ip, port))
		return 1;

	// 1. Conta. Como TMSelectServerScene: Force = 1 e a versão do cliente.
	{
		MSG_AccountLogin m{};
		m.Header.Type = MSG_AccountLogin_Opcode;
		std::strncpy(m.AccountPass, password, sizeof(m.AccountPass) - 1);
		std::strncpy(m.AccountName, account, sizeof(m.AccountName) - 1);
		m.Version = 1758;
		m.Force = 1;
		s.Send(&m, sizeof(m));

		static MSG_CNFAccountLogin r;
		int size = 0;
		if (!s.Wait(MSG_CNFAccountLogin_Opcode, &r, sizeof(r), &size))
			return 1;
		EXPECT(size == (int)sizeof(MSG_CNFAccountLogin), "0x10A com %d bytes, o cliente espera %d", size, (int)sizeof(MSG_CNFAccountLogin));
		char upper[16]{};
		for (int i = 0; i < 15 && account[i]; ++i)
			upper[i] = (char)toupper((unsigned char)account[i]);
		EXPECT(std::strncmp(r.AccountName, upper, 16) == 0, "0x10A AccountName \"%.16s\", esperado \"%s\"", r.AccountName, upper);
		for (int i = 0; i < 4; ++i)
			EXPECT(r.SelChar.MobName[i][0] == 0, "0x10A: conta nova com personagem na vaga %d", i);
		// Como TMSelectServerScene: a chave do envio vem do SecretCode.
		for (int i = 0; i < 16; ++i)
			s.sock_.SendQueue[i] = r.SecretCode[i];
		s.sock_.SendCount = 0;
		s.sock_.RecvCount = 0;
		std::printf("1. conta: 0x10A com %d bytes\n", size);
	}

	// 2. Senha numérica (conta nova: grava a senha e libera a conta).
	{
		MSG_CHARPASSWORD m{};
		m.Header.Type = MSG_CharPassword_Opcode;
		std::strncpy(m.ItemPassWord, numeric, 6);
		m.State = 0;
		s.Send(&m, sizeof(m));
		MSG_STANDARD r{};
		if (!s.Wait(MSG_CharPassword_Opcode, &r, sizeof(r)))
			return 1;
		std::printf("2. senha numérica: eco 0xFDE\n");
	}

	// 3. Personagem na vaga 0.
	{
		MSG_NewCharacter m{};
		m.Header.Type = MSG_NewCharacter_Opcode;
		m.Slot = 0;
		m.Class = cls;
		std::snprintf(m.MobName, sizeof(m.MobName), "%s", charName);
		s.Send(&m, sizeof(m));

		static MSG_CNFNewCharacter r;
		int size = 0;
		if (!s.Wait(MSG_CNFNewCharacter_Opcode, &r, sizeof(r), &size))
			return 1;
		EXPECT(size == (int)sizeof(MSG_CNFNewCharacter), "0x110 com %d bytes, esperado %d", size, (int)sizeof(MSG_CNFNewCharacter));
		const STRUCT_SELCHAR& sel = r.SelChar;
		EXPECT(std::strncmp(sel.MobName[0], charName, 16) == 0, "0x110 MobName[0] \"%.16s\"", sel.MobName[0]);
		EXPECT(sel.Coin[0] == want.coin, "0x110 Coin[0] %d, esperado %d", sel.Coin[0], want.coin);
		EXPECT(sel.Exp[0] == want.exp, "0x110 Exp[0] %lld, esperado %lld", sel.Exp[0], want.exp);
		EXPECT(sel.Score[0].Str == want.str && sel.Score[0].Con == want.con, "0x110 Score[0] Str/Con %d/%d", sel.Score[0].Str, sel.Score[0].Con);
		EXPECT(sel.Equip[0][1].sIndex == want.equipArmor, "0x110 Equip[0][1] %d", sel.Equip[0][1].sIndex);
		std::printf("3. personagem \"%s\" criado na vaga 0\n", charName);
	}

	// 4. Personagem descartável na vaga 1, apagado com a senha da conta.
	{
		MSG_NewCharacter m{};
		m.Header.Type = MSG_NewCharacter_Opcode;
		m.Slot = 1;
		m.Class = cls;
		std::snprintf(m.MobName, sizeof(m.MobName), "%.10sDel", charName);
		s.Send(&m, sizeof(m));
		static MSG_CNFNewCharacter r;
		if (!s.Wait(MSG_CNFNewCharacter_Opcode, &r, sizeof(r)))
			return 1;
		EXPECT(std::strncmp(r.SelChar.MobName[1], m.MobName, 16) == 0, "0x110 MobName[1] \"%.16s\"", r.SelChar.MobName[1]);

		MSG_DeleteCharacter d{};
		d.Header.Type = MSG_DeleteCharacter_Opcode;
		d.Slot = 1;
		std::memcpy(d.MobName, m.MobName, sizeof(d.MobName));
		std::strncpy(d.Password, password, sizeof(d.Password) - 1);
		s.Send(&d, sizeof(d));
		static MSG_CNFDeleteCharacter rd;
		if (!s.Wait(MSG_CNFDeleteCharacter_Opcode, &rd, sizeof(rd)))
			return 1;
		EXPECT(rd.SelChar.MobName[1][0] == 0, "0x112: vaga 1 ainda tem \"%.16s\"", rd.SelChar.MobName[1]);
		EXPECT(std::strncmp(rd.SelChar.MobName[0], charName, 16) == 0, "0x112: vaga 0 com \"%.16s\"", rd.SelChar.MobName[0]);
		std::printf("4. personagem \"%s\" criado e apagado na vaga 1\n", m.MobName);
	}

	// 5. Entrada no jogo.
	{
		MSG_CharacterLogin m{};
		m.Header.Type = MSG_CharacterLogin_Opcode;
		m.Slot = 0;
		s.Send(&m, sizeof(m));

		static MSG_CNFCharacterLogin r;
		int size = 0;
		if (!s.Wait(MSG_CNFCharacterLogin_Opcode, &r, sizeof(r), &size))
			return 1;
		EXPECT(size == (int)sizeof(MSG_CNFCharacterLogin), "0x114 com %d bytes, o cliente espera %d", size, (int)sizeof(MSG_CNFCharacterLogin));
		const STRUCT_MOB& mob = r.MOB;
		EXPECT(std::strncmp(mob.MobName, charName, 16) == 0, "0x114 MOB.MobName \"%.16s\"", mob.MobName);
		EXPECT(mob.Class == cls, "0x114 MOB.Class %d, esperado %d", mob.Class, cls);
		EXPECT(mob.Coin == want.coin, "0x114 MOB.Coin %d, esperado %d", mob.Coin, want.coin);
		EXPECT(mob.Exp == want.exp, "0x114 MOB.Exp %lld, esperado %lld", mob.Exp, want.exp);
		EXPECT(mob.BaseScore.Str == want.str && mob.BaseScore.Int == want.intel && mob.BaseScore.Dex == want.dex
			&& mob.BaseScore.Con == want.con, "0x114 MOB.BaseScore Str/Int/Dex/Con %d/%d/%d/%d",
			mob.BaseScore.Str, mob.BaseScore.Int, mob.BaseScore.Dex, mob.BaseScore.Con);
		EXPECT(mob.Equip[1].sIndex == want.equipArmor, "0x114 MOB.Equip[1] %d, esperado %d", mob.Equip[1].sIndex, want.equipArmor);
		EXPECT(mob.Carry[0].sIndex == want.carryFirst, "0x114 MOB.Carry[0] %d, esperado %d", mob.Carry[0].sIndex, want.carryFirst);
		EXPECT(mob.LearnedSkill[0] == want.learned, "0x114 MOB.LearnedSkill[0] 0x%X, esperado 0x%X", mob.LearnedSkill[0], want.learned);
		EXPECT(mob.LearnedSkill[1] == 0, "0x114 MOB.LearnedSkill[1] 0x%X, esperado 0", mob.LearnedSkill[1]);
		EXPECT(AllZero(mob.dummy, sizeof(mob.dummy)) && mob.CurrentKill == 0 && mob.TotalKill == 0,
			"0x114: MOB.dummy/CurrentKill/TotalKill não vieram zerados");
		EXPECT(r.Slot == 0, "0x114 Slot %d", r.Slot);
		EXPECT(r.ClientID > 0 && r.ClientID < 1000, "0x114 ClientID %d", r.ClientID);
		EXPECT(r.PosX > 0 && r.PosX < 4096 && r.PosY > 0 && r.PosY < 4096, "0x114 posição %d,%d fora do mapa", r.PosX, r.PosY);
		EXPECT(AllZero(&r.Ext1, sizeof(r.Ext1)), "0x114: Ext1 não veio zerado");
		EXPECT(AllZero(&r.Ext2, sizeof(r.Ext2)), "0x114: Ext2 não veio zerado");
		std::printf("5. entrada no jogo: 0x114 com %d bytes, \"%.16s\" em %d,%d (ClientID %d)\n",
			size, mob.MobName, r.PosX, r.PosY, r.ClientID);
	}

	s.sock_.CloseSocket();
	if (g_failures)
	{
		std::printf("login: %d conferência(s) falharam\n", g_failures);
		return 1;
	}
	std::printf("login: OK\n");
	return 0;
}
