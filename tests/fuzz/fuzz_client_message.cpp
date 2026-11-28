/// @file fuzz_client_message.cpp
/// @brief Fuzzing de ProcessClientMessage: sequências de pacotes do cliente contra o TMSrv inteiro.
///
/// O mundo é o do TMSrv de verdade, carregado de uma árvore sintética
/// (`tools/fuzz_tmsrv.py` monta a árvore e roda o alvo dentro de TMSRV/Run). Não há rede: o
/// jogador é aceito por um socket local de verdade (CUser::AcceptUser), o DBSrv é um socketpair, e
/// o que o servidor envia é drenado e descartado. As respostas do DBSrv do login
/// (`_MSG_DBCNFAccountLogin`, `_MSG_DBCNFCharacterLogin`, `_MSG_DBCNFAccountLogOut`) são
/// fabricadas e passam pelo ProcessDBMessage real.
///
/// Entrada: um byte de estágio (0 = conexão aceita, antes do login; 1 = seleção de personagem;
/// 2 e 3 = em jogo), depois até kMaxPackets registros
/// `[tipo:1][avanço do relógio:1][tamanho:2][corpo]`. O tipo é um índice em kTypes (255 lê o tipo
/// cru em mais 2 bytes). Cada pacote é alocado com o tamanho exato, para o ASan acusar leitura
/// além do que o cliente mandou. A cada entrada o jogador sai e entra de novo, então o estado do
/// jogador não passa de uma entrada para a outra (o do mundo, como itens no chão, passa).
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

#include "common/Basedef.h"
#include "common/CPSock.h"
#include "core/Server.h"
#include "core/CReadFiles.h"
#include "net/ProcessClientMessage.h"
#include "net/ProcessDBMessage.h"
#include "systems/CCastleZakum.h"

namespace {

constexpr int kConn = 1;
constexpr int kMaxPackets = 16;

const short kTypes[] = {
	_MSG_AccountLogin, _MSG_CharacterLogin, _MSG_CharacterLogout, _MSG_DeleteCharacter,
	_MSG_ReqTransper, _MSG_MessageLog, _MSG_CreateCharacter, _MSG_AccountSecure, _MSG_MessageChat,
	_MSG_Action, _MSG_Action2, _MSG_Action3, _MSG_Motion, _MSG_UpdateScore, _MSG_NoViewMob,
	_MSG_Restart, _MSG_Deprivate, _MSG_Challange, _MSG_ChallangeConfirm, _MSG_ReqTeleport,
	_MSG_REQShopList, _MSG_Deposit, _MSG_Withdraw, _MSG_RemoveParty, _MSG_SendReqParty,
	_MSG_AcceptParty, _MSG_TradingItem, _MSG_MessageWhisper, _MSG_ChangeCity, _MSG_PKMode,
	_MSG_ReqTradeList, _MSG_UpdateItem, _MSG_Quest, _MSG_SetShortSkill, _MSG_Attack,
	_MSG_AttackOne, _MSG_AttackTwo, _MSG_DropItem, _MSG_GetItem, _MSG_QuitTrade, _MSG_UseItem,
	_MSG_ApplyBonus, _MSG_SendAutoTrade, _MSG_ReqBuy, _MSG_Buy, _MSG_Sell, _MSG_Trade,
	_MSG_CombineItem, _MSG_ReqRanking, _MSG_CombineItemEhre, _MSG_CombineItemTiny,
	_MSG_CombineItemShany, _MSG_CombineItemAilyn, _MSG_CombineItemAgatha, _MSG_CombineItemOdin,
	_MSG_CombineItemOdin2, _MSG_DeleteItem, _MSG_InviteGuild, _MSG_SplitItem,
	_MSG_CombineItemLindy, _MSG_CombineItemAlquimia, _MSG_CombineItemExtracao, _MSG_GuildAlly,
	_MSG_War, _MSG_CapsuleInfo, _MSG_PutoutSeal,
};
constexpr int kNumTypes = sizeof(kTypes) / sizeof(kTypes[0]);

int g_listenFd = -1;
sockaddr_un g_listenAddr{};
int g_clientFd = -1;  // a ponta do "cliente" da conexão aceita
int g_dbPeerFd = -1;  // a ponta do "DBSrv" do socketpair
STRUCT_MOB g_baseMob;

[[noreturn]] void Fail(const char* what)
{
	fprintf(stderr, "fuzz_client_message: %s\n", what);
	abort();
}

void Drain(int fd)
{
	if (fd < 0)
		return;
	char buf[65536];
	while (recv(fd, buf, sizeof(buf), MSG_DONTWAIT) > 0)
	{
	}
}

void DrainAll()
{
	Drain(g_clientFd);
	Drain(g_dbPeerFd);
}

void InitWorld()
{
	// A mesma sequência de main() (core/Main.cpp), sem as conexões de rede.
	W2LoopInit();
	BASE_InitModuleDir();
	BASE_InitializeHitRate();
	StartLog();
	StartChatLog();
	memset(&g_pTreasure, 0, sizeof(g_pTreasure));
	ReadConfig();
	ReadLevelItemConfig();
	ConfigReady = 1;
	memset(g_pGuildWar, 0, sizeof(g_pGuildWar));
	memset(g_pGuildAlly, 0, sizeof(g_pGuildAlly));
	memset(GuildInfo, 0, sizeof(GuildInfo));
	memset(pMac, 0, sizeof(pMac));
	memset(pMobMerc, 0, sizeof(pMobMerc));
	Reboot();
	SetCastleDoor(1);
	NumServerInGroup = 1;
	SetColoseumDoor(1);
	SetColoseumDoor2(3);
	memset(FailAccount, 0, sizeof(FailAccount));
	CurrentTime = W2TickCount();
	CReadFiles();  // o construtor lê SancRate, QuestsRate, CompRate, Admin, Mac, Chall e Guild
	CCastleZakum::ReadCastleQuest();

	int db[2];
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, db) != 0)
		Fail("socketpair do DBSrv");
	fcntl(db[0], F_SETFL, O_NONBLOCK);
	DBServerSocket.Sock = db[0];
	g_dbPeerFd = db[1];

	// Socket Unix abstrato: uma conexão nova por entrada, sem TIME_WAIT (com TCP as portas
	// efêmeras acabam em segundos).
	g_listenFd = socket(AF_UNIX, SOCK_STREAM, 0);
	g_listenAddr.sun_family = AF_UNIX;
	snprintf(g_listenAddr.sun_path + 1, sizeof(g_listenAddr.sun_path) - 1, "w2fuzz-%d", static_cast<int>(getpid()));
	if (bind(g_listenFd, reinterpret_cast<sockaddr*>(&g_listenAddr), sizeof(g_listenAddr)) != 0 || listen(g_listenFd, 4) != 0)
		Fail("socket de escuta");

	FILE* fp = fopen("../../DBSRV/Run/BaseMob/TK", "rb");
	if (!fp || fread(&g_baseMob, sizeof(g_baseMob), 1, fp) != 1)
		Fail("../../DBSRV/Run/BaseMob/TK (rode pelo tools/fuzz_tmsrv.py)");
	fclose(fp);
	strncpy(g_baseMob.MobName, "Fuzzer", NAME_LENGTH);
}

void SendFromClient(void* msg, int size)
{
	static_cast<MSG_STANDARD*>(msg)->Size = static_cast<short>(size);
	static_cast<MSG_STANDARD*>(msg)->ID = kConn;
	ProcessClientMessage(kConn, static_cast<char*>(msg), FALSE);
	DrainAll();
}

void SendFromDB(void* msg)
{
	ProcessDBMessage(static_cast<char*>(msg));
	DrainAll();
}

void ResetUser()
{
	if (pUser[kConn].Mode != USER_EMPTY)
		CloseUser(kConn);
	if (pUser[kConn].Mode == USER_SAVING4QUIT)
	{
		MSG_STANDARD sm{};
		sm.Type = _MSG_DBCNFAccountLogOut;
		sm.ID = kConn;
		sm.Size = sizeof(sm);
		SendFromDB(&sm);
	}
	if (pUser[kConn].Mode != USER_EMPTY)
		CloseUser(kConn);
	if (g_clientFd >= 0)
		close(g_clientFd);
	g_clientFd = -1;
	if (pUser[kConn].Mode != USER_EMPTY)
		Fail("o jogador não voltou a USER_EMPTY");
}

void Accept()
{
	g_clientFd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (connect(g_clientFd, reinterpret_cast<sockaddr*>(&g_listenAddr), sizeof(g_listenAddr)) != 0)
		Fail("connect");
	if (!pUser[kConn].AcceptUser(g_listenFd))
		Fail("AcceptUser");
	// O accept de um socket Unix não traz endereço IPv4. Um IP por entrada (10.x.x.x), para o
	// limite de login por IP (net/NetGuard.h) não recusar as entradas seguintes.
	static unsigned int s_ip = 0;
	pUser[kConn].IP = htonl(0x0A000000u | (++s_ip & 0x00FFFFFFu));
	fcntl(pUser[kConn].cSock.Sock, F_SETFL, O_NONBLOCK);
}

void LoginAccount()
{
	MSG_AccountLogin m{};
	m.Type = _MSG_AccountLogin;
	m.Version = APP_VERSION;
	strncpy(m.AccountName, "FUZZ", sizeof(m.AccountName) - 1);
	strncpy(m.AccountPass, "senha", sizeof(m.AccountPass) - 1);
	SendFromClient(&m, sizeof(m));

	static MSG_DBCNFAccountLogin cnf;
	memset(&cnf, 0, sizeof(cnf));
	cnf.Type = _MSG_DBCNFAccountLogin;
	cnf.ID = kConn;
	cnf.Size = sizeof(cnf);
	strncpy(cnf.AccountName, "FUZZ", sizeof(cnf.AccountName) - 1);
	strncpy(cnf.sel.MobName[0], g_baseMob.MobName, NAME_LENGTH);
	cnf.sel.Score[0] = g_baseMob.CurrentScore;
	SendFromDB(&cnf);
	if (pUser[kConn].Mode != USER_SELCHAR)
		Fail("o login da conta não chegou a USER_SELCHAR");
}

void LoginCharacter()
{
	MSG_CharacterLogin m{};
	m.Type = _MSG_CharacterLogin;
	m.Slot = 0;
	SendFromClient(&m, sizeof(m));

	static MSG_CNFCharacterLogin cnf;
	memset(&cnf, 0, sizeof(cnf));
	cnf.Type = _MSG_DBCNFCharacterLogin;
	cnf.ID = kConn;
	cnf.Size = sizeof(cnf);
	cnf.PosX = 2100;
	cnf.PosY = 2100;
	cnf.mob = g_baseMob;
	cnf.Slot = 0;
	SendFromDB(&cnf);
	if (pUser[kConn].Mode != USER_PLAY)
		Fail("o login do personagem não chegou a USER_PLAY");
}

}  // namespace

/// Sementes: um pacote de cada tipo, no tamanho que o cliente envia (ClientMessageMinSize), em
/// cada estágio. Gravadas em W2_FUZZ_WRITE_SEEDS pelo tools/fuzz_tmsrv.py.
void WriteSeeds(const char* dir)
{
	for (int stage = 0; stage < 4; stage++)
	{
		for (int i = 0; i < kNumTypes; i++)
		{
			const int body = ClientMessageMinSize(kTypes[i]) - static_cast<int>(sizeof(MSG_STANDARD));
			std::vector<uint8_t> seed = { static_cast<uint8_t>(stage), static_cast<uint8_t>(i), 20,
				static_cast<uint8_t>(body & 0xFF), static_cast<uint8_t>(body >> 8) };
			seed.resize(seed.size() + body);
			char path[4096];
			snprintf(path, sizeof(path), "%s/stage%d-type%04x", dir, stage, static_cast<unsigned short>(kTypes[i]));
			FILE* fp = fopen(path, "wb");
			if (!fp)
				Fail("não abriu o arquivo de semente");
			fwrite(seed.data(), 1, seed.size(), fp);
			fclose(fp);
		}
	}
}

extern "C" int LLVMFuzzerInitialize(int*, char***)
{
	if (const char* dir = getenv("W2_FUZZ_WRITE_SEEDS"))
	{
		WriteSeeds(dir);
		exit(0);
	}

	InitWorld();
	// Confere uma vez que o caminho de login funciona; senão o fuzzing só veria a rejeição.
	ResetUser();
	Accept();
	LoginAccount();
	LoginCharacter();
	ResetUser();
	return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
	if (size < 1)
		return 0;

	ResetUser();
	Accept();
	const int stage = data[0] % 4;
	if (stage >= 1)
		LoginAccount();
	if (stage >= 2)
		LoginCharacter();

	size_t pos = 1;
	for (int n = 0; n < kMaxPackets && pos + 4 <= size; n++)
	{
		short type;
		const uint8_t pick = data[pos++];
		if (pick == 255)
		{
			if (pos + 2 > size)
				break;
			memcpy(&type, data + pos, 2);
			pos += 2;
		}
		else
			type = kTypes[pick % kNumTypes];

		CurrentTime += data[pos++] * 50u;

		if (pos + 2 > size)
			break;
		unsigned short bodyLen;
		memcpy(&bodyLen, data + pos, 2);
		pos += 2;
		bodyLen %= (MAX_MESSAGE_SIZE - sizeof(MSG_STANDARD) + 1);
		if (bodyLen > size - pos)
			bodyLen = static_cast<unsigned short>(size - pos);

		const int msgSize = static_cast<int>(sizeof(MSG_STANDARD)) + bodyLen;
		std::vector<char> msg(msgSize);
		memcpy(msg.data() + sizeof(MSG_STANDARD), data + pos, bodyLen);
		pos += bodyLen;

		MSG_STANDARD* h = reinterpret_cast<MSG_STANDARD*>(msg.data());
		h->Type = type;
		h->ClientTick = CurrentTime;
		SendFromClient(msg.data(), msgSize);
	}
	return 0;
}
