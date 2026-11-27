/// @file w2_win32.cpp
/// @brief Implementação POSIX do subconjunto Win32 de w2_win32.h (só no build fora do Windows).

#include "w2_win32.h"

#include <algorithm>
#include <cctype>
#include <csignal>
#include <deque>
#include <map>
#include <string>
#include <vector>

#include <glob.h>
#include <poll.h>

/// A única janela: só o endereço importa (as fontes comparam e repassam o HWND).
struct W2HWND__
{
	int unused;
};

namespace
{

W2HWND__ g_MainWindowObject{};
HWND const kMainWindow = &g_MainWindowObject;	// inicialização constante

struct Watch
{
	HWND hwnd;
	UINT msg;
	LONG events;
	bool listening;
};

struct Timer
{
	HWND hwnd;
	UINT ms;
	DWORD due;
};

WNDPROC g_WndProc = nullptr;

// Estáticos locais: construtores globais de outras TUs (o RankingSystem do DBSrv lista
// arquivos no próprio construtor) chamam a camada antes de os globais daqui existirem.
std::map<SOCKET, Watch>& Watches() { static std::map<SOCKET, Watch> m; return m; }
std::map<uintptr_t, Timer>& Timers() { static std::map<uintptr_t, Timer> m; return m; }
std::deque<MSG>& Queue() { static std::deque<MSG> q; return q; }
bool g_Quit = false;
int g_QuitCode = 0;

MSG Make(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

// Itens do menu da janela (AppendMenu), pelo nome normalizado: minúsculas, só letras e dígitos
// ("&SaveAll" -> "saveall"). É o que o FIFO de controle aceita.
struct MenuItem
{
	UINT id;
	std::string label;
};
std::map<std::string, MenuItem>& MenuItems() { static std::map<std::string, MenuItem> m; return m; }

// FIFO de controle: cada linha escrita nele vira o WM_COMMAND do item de menu com esse nome.
int g_ControlFd = -1;
std::string& ControlBuffer() { static std::string b; return b; }

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
	for (const auto& [name, item] : MenuItems())
		names += " " + name;
	std::fprintf(stderr, "controle: comandos:%s\n", names.empty() ? " (nenhum)" : names.c_str());
	std::fflush(stderr);
}

/// @brief Abre o FIFO de controle (W2_CONTROL ou ./control.fifo, no diretório do servidor),
///        criando-o com permissão 0600: só o dono do processo escreve.
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

/// @brief Lê o que chegou no FIFO e enfileira um WM_COMMAND por linha reconhecida.
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
		std::string cmd = NormalizeCommand(line);
		if (cmd.empty())
			continue;
		if (cmd == "help" || cmd == "ajuda")
		{
			ListCommands();
			continue;
		}
		auto it = MenuItems().find(cmd);
		if (it == MenuItems().end())
		{
			// Também pelo id numérico do item.
			for (const auto& [name, item] : MenuItems())
				if (std::to_string(item.id) == cmd)
					it = MenuItems().find(name);
		}
		if (it == MenuItems().end())
		{
			std::fprintf(stderr, "controle: comando desconhecido \"%s\"\n", line.c_str());
			ListCommands();
			continue;
		}
		std::fprintf(stderr, "controle: %s (%u)\n", it->second.label.c_str(), it->second.id);
		std::fflush(stderr);
		Queue().push_back(Make(kMainWindow, WM_COMMAND, (WPARAM)it->second.id, 0));
	}
	if (b.size() > 4096)	// linha sem fim: descarta
		b.clear();
}
volatile sig_atomic_t g_CloseRequested = 0;

void OnSignal(int)
{
	g_CloseRequested = 1;
}

void InstallSignals()
{
	static bool done = false;
	if (done)
		return;
	done = true;
	std::signal(SIGINT, OnSignal);
	std::signal(SIGTERM, OnSignal);
	std::signal(SIGPIPE, SIG_IGN);	// send() num socket fechado devolve erro, como no Winsock
}

MSG Make(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	MSG m{};
	m.hwnd = hwnd;
	m.message = message;
	m.wParam = wParam;
	m.lParam = lParam;
	m.time = GetTickCount();
	return m;
}

bool IsListening(SOCKET s)
{
	int v = 0;
	socklen_t len = sizeof(v);
	return getsockopt(s, SOL_SOCKET, SO_ACCEPTCONN, &v, &len) == 0 && v != 0;
}

/// @brief Espera até `timeoutMs` por sockets e enfileira as mensagens dos que ficaram prontos.
void PollSockets(int timeoutMs)
{
	std::vector<pollfd> fds;
	fds.reserve(Watches().size());
	for (const auto& [s, w] : Watches())
	{
		short ev = 0;
		if (w.events & (FD_READ | FD_ACCEPT | FD_CLOSE))
			ev |= POLLIN;
		fds.push_back({ s, ev, 0 });
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
			if ((w.events & FD_ACCEPT) && (p.revents & POLLIN))
				Queue().push_back(Make(w.hwnd, w.msg, (WPARAM)p.fd, WSAMAKESELECTREPLY(FD_ACCEPT, 0)));
			continue;
		}

		// O Winsock entrega os dados que restam como FD_READ e só depois o FD_CLOSE; poll() só diz
		// "legível". Espiar um byte separa os dois casos.
		char c;
		ssize_t r = recv(p.fd, &c, 1, MSG_PEEK | MSG_DONTWAIT);
		if (r > 0)
		{
			if (w.events & FD_READ)
				Queue().push_back(Make(w.hwnd, w.msg, (WPARAM)p.fd, WSAMAKESELECTREPLY(FD_READ, 0)));
		}
		else if (r == 0 || (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR))
		{
			int err = r == 0 ? 0 : errno;
			if (w.events & FD_CLOSE)
				Queue().push_back(Make(w.hwnd, w.msg, (WPARAM)p.fd, WSAMAKESELECTREPLY(FD_CLOSE, err)));
			// Um só FD_CLOSE por socket, como no Winsock: o handler fecha o socket, e se não fechar
			// o laço não deve repetir o aviso.
			it->second.events &= ~(FD_READ | FD_CLOSE);
		}
	}
}

void FireTimers()
{
	DWORD now = GetTickCount();
	for (auto& [id, t] : Timers())
	{
		if ((int)(now - t.due) >= 0)
		{
			Queue().push_back(Make(t.hwnd, WM_TIMER, (WPARAM)id, 0));
			t.due = now + t.ms;
		}
	}
}

int NextTimerMs()
{
	if (Timers().empty())
		return 1000;
	DWORD now = GetTickCount();
	int best = 1000;
	for (const auto& [id, t] : Timers())
		best = std::min(best, std::max(0, (int)(t.due - now)));
	return best;
}

} // namespace

// ---- Janela e mensagens --------------------------------------------------------------------------

BOOL RegisterClass(const WNDCLASS* wc)
{
	g_WndProc = wc->lpfnWndProc;
	InstallSignals();
	return TRUE;
}

HWND CreateWindow(LPCSTR, LPCSTR, DWORD, int, int, int, int, HWND, HMENU, HINSTANCE, LPVOID)
{
	// Como no Windows, o WM_CREATE é tratado dentro do CreateWindow (é onde o menu é montado).
	if (g_WndProc)
		g_WndProc(kMainWindow, WM_CREATE, 0, 0);
	return kMainWindow;
}

BOOL ShowWindow(HWND, int) { return TRUE; }
BOOL UpdateWindow(HWND) { return TRUE; }
BOOL DestroyWindow(HWND hwnd)
{
	Queue().push_back(Make(hwnd, WM_DESTROY, 0, 0));
	return TRUE;
}
BOOL InvalidateRect(HWND, const RECT*, BOOL) { return TRUE; }
BOOL SetWindowText(HWND, LPCSTR) { return TRUE; }
HICON LoadIcon(HINSTANCE, LPCSTR) { return nullptr; }
HCURSOR LoadCursor(HINSTANCE, LPCSTR) { return nullptr; }
HMENU CreateMenu() { return nullptr; }
HMENU CreatePopupMenu() { return nullptr; }
BOOL AppendMenu(HMENU, UINT flags, uintptr_t id, LPCSTR label)
{
	// Só os itens que mandam WM_COMMAND; os submenus (MF_POPUP) não têm comando.
	if (!(flags & (MF_POPUP | MF_SEPARATOR)) && label)
	{
		std::string text;
		for (const char* c = label; *c; ++c)
			if (*c != '&')
				text += *c;
		std::string name = NormalizeCommand(text);
		if (!name.empty())
			MenuItems()[name] = { (UINT)id, text };
	}
	return TRUE;
}
BOOL SetMenu(HWND, HMENU) { return TRUE; }

BOOL GetMessage(MSG* msg, HWND, UINT, UINT)
{
	InstallSignals();
	OpenControl();
	for (;;)
	{
		if (g_CloseRequested)
		{
			g_CloseRequested = 0;
			Queue().push_back(Make(kMainWindow, WM_CLOSE, 0, 0));
		}
		if (!Queue().empty())
		{
			*msg = Queue().front();
			Queue().pop_front();
			return TRUE;
		}
		if (g_Quit)
		{
			*msg = Make(kMainWindow, WM_QUIT, (WPARAM)g_QuitCode, 0);
			return FALSE;
		}
		FireTimers();
		if (!Queue().empty())
			continue;
		PollSockets(NextTimerMs());
		FireTimers();
	}
}

BOOL TranslateMessage(const MSG*) { return TRUE; }

LRESULT DispatchMessage(const MSG* msg)
{
	if (!g_WndProc)
		return 0;
	return g_WndProc(msg->hwnd, msg->message, msg->wParam, msg->lParam);
}

BOOL PostMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	Queue().push_back(Make(hwnd, message, wParam, lParam));
	return TRUE;
}

LRESULT SendMessageA(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return g_WndProc ? g_WndProc(hwnd, message, wParam, lParam) : 0;
}

void PostQuitMessage(int code)
{
	g_Quit = true;
	g_QuitCode = code;
}

LRESULT DefWindowProc(HWND hwnd, UINT message, WPARAM, LPARAM)
{
	// No Windows, WM_CLOSE sem tratamento destrói a janela; os servidores tratam WM_CLOSE e
	// WM_DESTROY e chamam PostQuitMessage.
	if (message == WM_CLOSE)
		DestroyWindow(hwnd);
	return 0;
}

uintptr_t SetTimer(HWND hwnd, uintptr_t id, UINT ms, void*)
{
	Timers()[id] = { hwnd, ms ? ms : 1, GetTickCount() + (ms ? ms : 1) };
	return id;
}

BOOL KillTimer(HWND, uintptr_t id)
{
	return Timers().erase(id) ? TRUE : FALSE;
}

int MessageBox(HWND, LPCSTR text, LPCSTR caption, UINT type)
{
	std::fprintf(stderr, "[%s] %s\n", caption ? caption : "", text ? text : "");
	std::fflush(stderr);
	// Sem ninguém para responder, a resposta é a afirmativa. As duas perguntas dos servidores são
	// as de desligamento (WM_CLOSE), que no Linux vem de SIGINT/SIGTERM: o operador já decidiu.
	return (type & 0x0F) == MB_YESNO ? IDYES : IDOK;
}

DWORD GetModuleFileName(HINSTANCE, char* buf, DWORD size)
{
	if (!size)
		return 0;
	ssize_t n = readlink("/proc/self/exe", buf, size - 1);
	if (n < 0)
		n = 0;
	buf[n] = 0;
	return (DWORD)n;
}

BOOL SetCurrentDirectory(LPCSTR path)
{
	return chdir(path) == 0 ? TRUE : FALSE;
}

DWORD GetCurrentDirectory(DWORD size, char* buf)
{
	return getcwd(buf, size) ? (DWORD)strlen(buf) : 0;
}

// ---- GDI -----------------------------------------------------------------------------------------

HDC GetDC(HWND) { return nullptr; }
int ReleaseDC(HWND, HDC) { return 1; }
HDC BeginPaint(HWND, PAINTSTRUCT* ps)
{
	if (ps)
		memset(ps, 0, sizeof(*ps));
	return nullptr;
}
BOOL EndPaint(HWND, const PAINTSTRUCT*) { return TRUE; }
HFONT CreateFont(int, int, int, int, int, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, LPCSTR) { return nullptr; }
HGDIOBJ SelectObject(HDC, HGDIOBJ) { return nullptr; }
HGDIOBJ GetStockObject(int) { return nullptr; }
BOOL DeleteObject(HGDIOBJ) { return TRUE; }
BOOL TextOut(HDC, int, int, LPCSTR, int) { return TRUE; }
COLORREF SetTextColor(HDC, COLORREF) { return 0; }
COLORREF SetBkColor(HDC, COLORREF) { return 0; }
int SetBkMode(HDC, int) { return OPAQUE; }

// ---- Tempo ---------------------------------------------------------------------------------------

DWORD GetTickCount()
{
	timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (DWORD)((unsigned long long)ts.tv_sec * 1000ull + (unsigned long long)ts.tv_nsec / 1000000ull);
}

DWORD timeGetTime()
{
	return GetTickCount();
}

void Sleep(DWORD ms)
{
	timespec ts{ (time_t)(ms / 1000), (long)(ms % 1000) * 1000000L };
	while (nanosleep(&ts, &ts) == -1 && errno == EINTR)
	{
	}
}

void GetLocalTime(SYSTEMTIME* st)
{
	timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	tm when;
	localtime_r(&ts.tv_sec, &when);
	st->wYear = (WORD)(when.tm_year + 1900);
	st->wMonth = (WORD)(when.tm_mon + 1);
	st->wDayOfWeek = (WORD)when.tm_wday;
	st->wDay = (WORD)when.tm_mday;
	st->wHour = (WORD)when.tm_hour;
	st->wMinute = (WORD)when.tm_min;
	st->wSecond = (WORD)when.tm_sec;
	st->wMilliseconds = (WORD)(ts.tv_nsec / 1000000L);
}

// ---- CRT -----------------------------------------------------------------------------------------

namespace
{
struct Finder
{
	std::vector<std::string> paths;
	size_t next = 0;
};

// Handles pequenos (1, 2, ...): as fontes guardam o retorno de _findfirst num int
// (`int hFindFile = _findfirst(...)`), onde um ponteiro de 64 bits não cabe.
std::map<intptr_t, Finder>& Finders() { static std::map<intptr_t, Finder> m; return m; }
intptr_t g_NextFinder = 1;	// inicialização constante: vale antes de qualquer construtor

void FillFindData(const std::string& path, _finddata_t* d)
{
	memset(d, 0, sizeof(*d));
	size_t slash = path.find_last_of('/');
	std::string name = slash == std::string::npos ? path : path.substr(slash + 1);
	strncpy(d->name, name.c_str(), sizeof(d->name) - 1);
	struct stat st{};
	if (stat(path.c_str(), &st) == 0)
	{
		d->attrib = S_ISDIR(st.st_mode) ? _A_SUBDIR : _A_NORMAL;
		d->size = st.st_size;
		d->time_write = st.st_mtime;
		d->time_access = st.st_atime;
		d->time_create = st.st_ctime;
	}
}

bool IsDotEntry(const std::string& path)
{
	size_t slash = path.find_last_of('/');
	std::string name = slash == std::string::npos ? path : path.substr(slash + 1);
	return name == "." || name == "..";
}
} // namespace

intptr_t _findfirst(const char* pattern, _finddata_t* data)
{
	std::string p = pattern;
	std::replace(p.begin(), p.end(), '\\', '/');
	// No Windows, "*.*" casa também nomes sem ponto.
	if (p.size() >= 3 && p.compare(p.size() - 3, 3, "*.*") == 0)
		p.replace(p.size() - 3, 3, "*");

	// GLOB_PERIOD: como no Windows, "*" casa nomes que começam com ponto, inclusive "." e "..",
	// que o Windows devolve primeiro. As fontes contam com isso: `while (FindNextFile(...))`
	// pula a primeira entrada.
	glob_t g{};
	if (glob(p.c_str(), GLOB_PERIOD, nullptr, &g) != 0 || g.gl_pathc == 0)
	{
		globfree(&g);
		errno = ENOENT;
		return -1;
	}
	Finder f;
	for (size_t i = 0; i < g.gl_pathc; i++)
		f.paths.emplace_back(g.gl_pathv[i]);
	globfree(&g);
	std::stable_partition(f.paths.begin(), f.paths.end(), IsDotEntry);

	intptr_t h = g_NextFinder++;
	Finder& stored = Finders()[h] = std::move(f);
	FillFindData(stored.paths[stored.next++], data);
	return h;
}

int _findnext(intptr_t handle, _finddata_t* data)
{
	auto it = Finders().find(handle);
	if (it == Finders().end() || it->second.next >= it->second.paths.size())
	{
		errno = ENOENT;
		return -1;
	}
	FillFindData(it->second.paths[it->second.next++], data);
	return 0;
}

int _findclose(intptr_t handle)
{
	return Finders().erase(handle) ? 0 : -1;
}

char* _strupr(char* s)
{
	for (char* p = s; *p; ++p)
		*p = (char)std::toupper((unsigned char)*p);
	return s;
}

char* _strlwr(char* s)
{
	for (char* p = s; *p; ++p)
		*p = (char)std::tolower((unsigned char)*p);
	return s;
}

char* _itoa(int value, char* buf, int radix)
{
	if (radix == 10)
	{
		std::sprintf(buf, "%d", value);
		return buf;
	}
	char tmp[40];
	unsigned int v = (unsigned int)value;
	int n = 0;
	do
	{
		int d = (int)(v % (unsigned)radix);
		tmp[n++] = (char)(d < 10 ? '0' + d : 'a' + d - 10);
		v /= (unsigned)radix;
	} while (v);
	for (int i = 0; i < n; i++)
		buf[i] = tmp[n - 1 - i];
	buf[n] = 0;
	return buf;
}

BOOL MoveFile(LPCSTR from, LPCSTR to)
{
	if (access(to, F_OK) == 0)
	{
		errno = EEXIST;
		return FALSE;
	}
	return rename(from, to) == 0 ? TRUE : FALSE;
}

namespace
{
void ToFileTime(long long unixTime, FILETIME* ft)
{
	unsigned long long v = (unsigned long long)(unixTime + 11644473600ll) * 10000000ull;
	ft->dwLowDateTime = (DWORD)v;
	ft->dwHighDateTime = (DWORD)(v >> 32);
}

void ToFindData(const _finddata_t& f, WIN32_FIND_DATA* d)
{
	memset(d, 0, sizeof(*d));
	d->dwFileAttributes = (f.attrib & _A_SUBDIR) ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL;
	ToFileTime(f.time_create, &d->ftCreationTime);
	ToFileTime(f.time_access, &d->ftLastAccessTime);
	ToFileTime(f.time_write, &d->ftLastWriteTime);
	d->nFileSizeLow = (DWORD)f.size;
	d->nFileSizeHigh = (DWORD)((unsigned long long)f.size >> 32);
	strncpy(d->cFileName, f.name, sizeof(d->cFileName) - 1);
}
} // namespace

HANDLE FindFirstFile(LPCSTR pattern, WIN32_FIND_DATA* data)
{
	_finddata_t f;
	intptr_t h = _findfirst(pattern, &f);
	if (h == -1)
		return INVALID_HANDLE_VALUE;
	ToFindData(f, data);
	return (HANDLE)h;
}

BOOL FindNextFile(HANDLE h, WIN32_FIND_DATA* data)
{
	_finddata_t f;
	if (_findnext((intptr_t)h, &f) != 0)
		return FALSE;
	ToFindData(f, data);
	return TRUE;
}

BOOL FindClose(HANDLE h)
{
	return _findclose((intptr_t)h) == 0 ? TRUE : FALSE;
}

BOOL FileTimeToSystemTime(const FILETIME* ft, SYSTEMTIME* st)
{
	unsigned long long v = ((unsigned long long)ft->dwHighDateTime << 32) | ft->dwLowDateTime;
	time_t secs = (time_t)(v / 10000000ull) - 11644473600ll;
	tm when;
	gmtime_r(&secs, &when);
	st->wYear = (WORD)(when.tm_year + 1900);
	st->wMonth = (WORD)(when.tm_mon + 1);
	st->wDayOfWeek = (WORD)when.tm_wday;
	st->wDay = (WORD)when.tm_mday;
	st->wHour = (WORD)when.tm_hour;
	st->wMinute = (WORD)when.tm_min;
	st->wSecond = (WORD)when.tm_sec;
	st->wMilliseconds = (WORD)((v / 10000ull) % 1000ull);
	return TRUE;
}

// ---- Winsock -------------------------------------------------------------------------------------

int closesocket(SOCKET s)
{
	Watches().erase(s);
	return close(s);
}

int WSAAsyncSelect(SOCKET s, HWND hwnd, UINT msg, LONG events)
{
	if (events == 0)
	{
		Watches().erase(s);
		return 0;
	}
	int fl = fcntl(s, F_GETFL, 0);
	if (fl == -1 || fcntl(s, F_SETFL, fl | O_NONBLOCK) == -1)
		return SOCKET_ERROR;
	Watches()[s] = { hwnd, msg, events, IsListening(s) };
	return 0;
}
