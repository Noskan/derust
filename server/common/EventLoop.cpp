/// @file EventLoop.cpp
/// @brief Implementação POSIX de EventLoop.h: poll() nos sockets e no FIFO de controle, timers
///        pelo relógio monotônico.

#include "common/EventLoop.h"
#include "common/Platform.h"

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <map>
#include <string>
#include <vector>

#include <fcntl.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

namespace
{
struct Watch
{
	int channel;
	int events;
	bool listening;
};

struct Timer
{
	unsigned int ms;
	unsigned int due;
};

struct Command
{
	int id;
	std::string label;
};

// Estáticos locais: construtores globais de outras TUs podem chegar aqui antes dos globais desta.
std::map<int, Watch>& Watches() { static std::map<int, Watch> m; return m; }
std::map<int, Timer>& Timers() { static std::map<int, Timer> m; return m; }
std::deque<W2Event>& Queue() { static std::deque<W2Event> q; return q; }
std::map<std::string, Command>& Commands() { static std::map<std::string, Command> m; return m; }
std::string& ControlBuffer() { static std::string b; return b; }

bool g_Quit = false;
int g_QuitCode = 0;
int g_ControlFd = -1;
volatile sig_atomic_t g_CloseRequested = 0;

W2Event Make(int code)
{
	W2Event ev{};
	ev.code = code;
	ev.fd = -1;
	return ev;
}

W2Event MakeSocket(const Watch& w, int fd, int net, int error)
{
	W2Event ev = Make(w.channel);
	ev.fd = fd;
	ev.net = net;
	ev.error = error;
	return ev;
}

std::string NormalizeCommand(const std::string& s)
{
	std::string out;
	for (unsigned char c : s)
		if (std::isalnum(c))
			out += (char)std::tolower(c);
	return out;
}

void ListCommands()
{
	std::string names;
	for (const auto& [name, cmd] : Commands())
		names += " " + name;
	std::fprintf(stderr, "controle: comandos:%s\n", names.empty() ? " (nenhum)" : names.c_str());
	std::fflush(stderr);
}

/// @brief Abre o FIFO de controle (W2_CONTROL ou ./control.fifo), criando-o com permissão 0600:
///        só o dono do processo escreve.
void OpenControl()
{
	static bool tried = false;
	if (tried)
		return;
	tried = true;

	const char* env = std::getenv("W2_CONTROL");
	std::string path = env && *env ? env : "control.fifo";
	struct stat st{};
	if (stat(path.c_str(), &st) == 0)
	{
		if (!S_ISFIFO(st.st_mode))
		{
			std::fprintf(stderr, "controle: %s existe e não é um FIFO; comandos desligados\n", path.c_str());
			return;
		}
	}
	else if (mkfifo(path.c_str(), 0600) != 0)
	{
		std::fprintf(stderr, "controle: não foi possível criar %s (%s); comandos desligados\n", path.c_str(), std::strerror(errno));
		return;
	}
	// O_RDWR: o próprio processo mantém o FIFO aberto para escrita, então não há EOF quando quem
	// escreveu fecha, e o open não bloqueia esperando um escritor.
	g_ControlFd = open(path.c_str(), O_RDWR | O_NONBLOCK | O_CLOEXEC);
	if (g_ControlFd == -1)
		std::fprintf(stderr, "controle: não foi possível abrir %s (%s)\n", path.c_str(), std::strerror(errno));
	std::fflush(stderr);
}

/// @brief Lê o que chegou no FIFO e enfileira um W2_EVENT_COMMAND por linha reconhecida.
void ReadControl()
{
	char buf[512];
	for (;;)
	{
		ssize_t n = read(g_ControlFd, buf, sizeof(buf));
		if (n <= 0)
			break;
		ControlBuffer().append(buf, (size_t)n);
	}
	std::string& b = ControlBuffer();
	size_t eol;
	while ((eol = b.find('\n')) != std::string::npos)
	{
		std::string line = b.substr(0, eol);
		b.erase(0, eol + 1);
		std::string name = NormalizeCommand(line);
		if (name.empty())
			continue;
		if (name == "help" || name == "ajuda")
		{
			ListCommands();
			continue;
		}
		auto it = Commands().find(name);
		if (it == Commands().end())
		{
			// Também pelo id numérico do comando.
			for (const auto& [key, cmd] : Commands())
				if (std::to_string(cmd.id) == name)
					it = Commands().find(key);
		}
		if (it == Commands().end())
		{
			std::fprintf(stderr, "controle: comando desconhecido \"%s\"\n", line.c_str());
			ListCommands();
			continue;
		}
		std::fprintf(stderr, "controle: %s (%d)\n", it->second.label.c_str(), it->second.id);
		std::fflush(stderr);
		W2Event ev = Make(W2_EVENT_COMMAND);
		ev.id = it->second.id;
		Queue().push_back(ev);
	}
	if (b.size() > 4096)	// linha sem fim: descarta
		b.clear();
}

void OnSignal(int)
{
	g_CloseRequested = 1;
}

bool IsListening(int fd)
{
	int v = 0;
	socklen_t len = sizeof(v);
	return getsockopt(fd, SOL_SOCKET, SO_ACCEPTCONN, &v, &len) == 0 && v != 0;
}

/// @brief Espera até `timeoutMs` por sockets e enfileira os eventos dos que ficaram prontos.
void PollSockets(int timeoutMs)
{
	std::vector<pollfd> fds;
	fds.reserve(Watches().size() + 1);
	for (const auto& [fd, w] : Watches())
	{
		short ev = 0;
		if (w.events & (W2_NET_READ | W2_NET_ACCEPT | W2_NET_CLOSE))
			ev |= POLLIN;
		fds.push_back({ fd, ev, 0 });
	}
	if (g_ControlFd != -1)
		fds.push_back({ g_ControlFd, POLLIN, 0 });

	int n = poll(fds.data(), fds.size(), timeoutMs);
	if (n <= 0)
		return;

	for (const pollfd& p : fds)
	{
		if (!p.revents)
			continue;
		if (p.fd == g_ControlFd)
		{
			ReadControl();
			continue;
		}
		auto it = Watches().find(p.fd);
		if (it == Watches().end())
			continue;
		const Watch& w = it->second;

		if (w.listening)
		{
			if ((w.events & W2_NET_ACCEPT) && (p.revents & POLLIN))
				Queue().push_back(MakeSocket(w, p.fd, W2_NET_ACCEPT, 0));
			continue;
		}

		// poll() só diz "legível". Espiar um byte separa os dados (W2_NET_READ) do fechamento
		// (0 bytes, ou um erro), que só é avisado depois que os dados acabam.
		char c;
		ssize_t r = recv(p.fd, &c, 1, MSG_PEEK | MSG_DONTWAIT);
		if (r > 0)
		{
			if (w.events & W2_NET_READ)
				Queue().push_back(MakeSocket(w, p.fd, W2_NET_READ, 0));
		}
		else if (r == 0 || (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR))
		{
			int err = r == 0 ? 0 : errno;
			if (w.events & W2_NET_CLOSE)
				Queue().push_back(MakeSocket(w, p.fd, W2_NET_CLOSE, err));
			// Um só aviso de fechamento por socket: o tratador fecha o socket, e se não fechar o
			// laço não deve repetir o aviso.
			it->second.events &= ~(W2_NET_READ | W2_NET_CLOSE);
		}
	}
}

void FireTimers()
{
	unsigned int now = W2TickCount();
	for (auto& [id, t] : Timers())
	{
		if ((int)(now - t.due) >= 0)
		{
			W2Event ev = Make(W2_EVENT_TIMER);
			ev.id = id;
			Queue().push_back(ev);
			t.due = now + t.ms;
		}
	}
}

int NextTimerMs()
{
	if (Timers().empty())
		return 1000;
	unsigned int now = W2TickCount();
	int best = 1000;
	for (const auto& [id, t] : Timers())
		best = std::min(best, std::max(0, (int)(t.due - now)));
	return best;
}
} // namespace

void W2LoopInit()
{
	static bool done = false;
	if (done)
		return;
	done = true;
	std::signal(SIGINT, OnSignal);
	std::signal(SIGTERM, OnSignal);
	std::signal(SIGPIPE, SIG_IGN);
}

int W2LoopWatch(int fd, int channel, int events)
{
	int fl = fcntl(fd, F_GETFL, 0);
	if (fl == -1 || fcntl(fd, F_SETFL, fl | O_NONBLOCK) == -1)
		return -1;
	Watches()[fd] = { channel, events, IsListening(fd) };
	return 0;
}

int W2CloseSocket(int fd)
{
	Watches().erase(fd);
	return close(fd);
}

void W2LoopSetTimer(int id, unsigned int ms)
{
	if (ms == 0)
		ms = 1;
	Timers()[id] = { ms, W2TickCount() + ms };
}

void W2LoopAddCommand(int id, const char* label)
{
	std::string name = NormalizeCommand(label);
	if (!name.empty())
		Commands()[name] = { id, label };
}

void W2LoopPostClose()
{
	Queue().push_back(Make(W2_EVENT_CLOSE));
}

void W2LoopQuit(int code)
{
	g_Quit = true;
	g_QuitCode = code;
}

int W2LoopRun(W2EventHandler handler)
{
	W2LoopInit();
	OpenControl();
	for (;;)
	{
		if (g_CloseRequested)
		{
			g_CloseRequested = 0;
			Queue().push_back(Make(W2_EVENT_CLOSE));
		}
		if (!Queue().empty())
		{
			W2Event ev = Queue().front();
			Queue().pop_front();
			handler(ev);
			continue;
		}
		if (g_Quit)
			return g_QuitCode;
		FireTimers();
		if (!Queue().empty())
			continue;
		PollSockets(NextTimerMs());
		FireTimers();
	}
}
