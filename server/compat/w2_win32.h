#pragma once

/// @file w2_win32.h
/// @brief Subconjunto do Win32 que o TMSrv e o DBSrv usam, emulado sobre POSIX para o build Linux.
///
/// Só entra no build fora do Windows: server/compat/posix tem `Windows.h`, `io.h`, `winsock2.h` e
/// os outros nomes que as fontes incluem, todos apontando para este arquivo. No Windows nada disto
/// é compilado e as fontes usam o SDK de verdade.
///
/// O que é emulado, e como:
/// - Laço de mensagens: `RegisterClass` guarda o `lpfnWndProc`, `CreateWindow` devolve uma janela
///   fictícia, e `GetMessage` espera com `poll()` pelos sockets registrados com `WSAAsyncSelect` e
///   pelos timers de `SetTimer`. Cada evento vira a mensagem que o Windows entregaria
///   (`WSA_*` com wParam = socket e lParam = WSAMAKESELECTREPLY(evento, erro); `WM_TIMER` com
///   wParam = id), despachada pelo mesmo `MainWndProc`. SIGINT e SIGTERM viram `WM_CLOSE`.
/// - Sockets: `SOCKET` é o descritor; `WSAAsyncSelect` deixa o socket não bloqueante, como no
///   Winsock. Para um socket de dados com leitura pronta, o laço espia um byte (`MSG_PEEK`):
///   0 bytes é `FD_CLOSE`, dados são `FD_READ`, que é o que o Winsock entrega.
/// - Interface: `MessageBox` escreve no stderr; desenho (GDI), menus e janelas não fazem nada.
/// - CRT: `_open`/`_read`/... e `_findfirst` sobre POSIX; `_strupr`, `_stricmp`, `GetTickCount`,
///   `timeGetTime`, `GetLocalTime`, `Sleep`.

#ifdef _WIN32
#error "w2_win32.h é só para o build fora do Windows"
#endif

#include <cerrno>
#include <cmath>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <strings.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// ---- Tipos -------------------------------------------------------------------------------------

typedef int BOOL;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int DWORD;
typedef unsigned int UINT;
typedef int INT;
typedef int LONG;	// 32 bits como no Win32 (o `long` do Linux 64 bits tem 8)
typedef unsigned int ULONG;
typedef char CHAR;
typedef float FLOAT;
#define __int8 char	// `unsigned __int8` das fontes decompiladas
#define __int16 short
#define __int32 int
typedef long long __int64;
typedef char* LPSTR;
typedef const char* LPCSTR;
typedef void* LPVOID;
typedef unsigned int WPARAM;	// as fontes declaram MainWndProc(HWND, UINT, UINT wParam, LONG lParam)
typedef int LPARAM;
typedef int LRESULT;
typedef void* HANDLE;
typedef struct W2HWND__* HWND;
typedef void* HINSTANCE;
typedef void* HMENU;
typedef void* HDC;
struct HFONT__;
typedef HFONT__* HFONT;	// Server.h declara `HFONT__* GetAFont()`
typedef void* HGDIOBJ;
typedef void* HICON;
typedef void* HCURSOR;
typedef void* HBRUSH;
typedef unsigned int COLORREF;

#define TRUE 1
#define FALSE 0
#define MAX_PATH 260
#define WINAPI
#define FAR
#define NEAR
#define APIENTRY
#define CALLBACK
#define UNREFERENCED_PARAMETER(p) (void)(p)

#define MAKEWORD(a, b) ((WORD)(((BYTE)(a)) | ((WORD)((BYTE)(b))) << 8))
#define LOBYTE(w) ((BYTE)((w) & 0xFF))
#define HIBYTE(w) ((BYTE)(((w) >> 8) & 0xFF))
#define LOWORD(l) ((WORD)((l) & 0xFFFF))
#define HIWORD(l) ((WORD)(((l) >> 16) & 0xFFFF))
#define RGB(r, g, b) ((COLORREF)(((BYTE)(r) | ((WORD)((BYTE)(g)) << 8)) | (((DWORD)(BYTE)(b)) << 16)))

// ---- Mensagens e janela ------------------------------------------------------------------------

#define WM_CREATE 0x0001
#define WM_DESTROY 0x0002
#define WM_PAINT 0x000F
#define WM_CLOSE 0x0010
#define WM_QUIT 0x0012
#define WM_COMMAND 0x0111
#define WM_TIMER 0x0113
#define WM_USER 0x0400

typedef LONG (*WNDPROC)(HWND, UINT, UINT, LONG);

struct POINT { LONG x, y; };
struct RECT { LONG left, top, right, bottom; };

struct MSG
{
	HWND hwnd;
	UINT message;
	WPARAM wParam;
	LPARAM lParam;
	DWORD time;
	POINT pt;
};

struct WNDCLASS
{
	UINT style;
	WNDPROC lpfnWndProc;
	int cbClsExtra;
	int cbWndExtra;
	HINSTANCE hInstance;
	HICON hIcon;
	HCURSOR hCursor;
	HBRUSH hbrBackground;
	LPCSTR lpszMenuName;
	LPCSTR lpszClassName;
};

struct PAINTSTRUCT
{
	HDC hdc;
	BOOL fErase;
	RECT rcPaint;
};

#define CS_VREDRAW 0x0001
#define CS_HREDRAW 0x0002
#define CS_DBLCLKS 0x0008
#define WS_OVERLAPPED 0x00000000
#define WS_CAPTION 0x00C00000
#define WS_SYSMENU 0x00080000
#define WS_THICKFRAME 0x00040000
#define WS_MINIMIZEBOX 0x00020000
#define WS_MAXIMIZEBOX 0x00010000
#define WS_OVERLAPPEDWINDOW 0x00CF0000
#define WS_CLIPCHILDREN 0x02000000
#define CW_USEDEFAULT ((int)0x80000000)
#define IDC_ARROW ((LPCSTR)32512)
#define WHITE_BRUSH 0
#define MF_STRING 0x0000
#define MF_POPUP 0x0010
#define MF_SEPARATOR 0x0800
#define SW_SHOW 5

#define MB_OK 0x0000
#define MB_OKCANCEL 0x0001
#define MB_YESNO 0x0004
#define MB_ICONERROR 0x0010
#define MB_SYSTEMMODAL 0x1000
#define MB_ICONQUESTION 0x0020
#define MB_ICONWARNING 0x0030
#define MB_ICONINFORMATION 0x0040
#define IDOK 1
#define IDCANCEL 2
#define IDYES 6
#define IDNO 7

BOOL RegisterClass(const WNDCLASS* wc);
HWND CreateWindow(LPCSTR cls, LPCSTR title, DWORD style, int x, int y, int w, int h, HWND parent, HMENU menu,
	HINSTANCE inst, LPVOID param);
BOOL ShowWindow(HWND, int);
BOOL UpdateWindow(HWND);
BOOL DestroyWindow(HWND);
BOOL InvalidateRect(HWND, const RECT*, BOOL);
BOOL SetWindowText(HWND, LPCSTR);
#define SetWindowTextA SetWindowText
HICON LoadIcon(HINSTANCE, LPCSTR);
HCURSOR LoadCursor(HINSTANCE, LPCSTR);
HMENU CreateMenu();
HMENU CreatePopupMenu();
BOOL AppendMenu(HMENU, UINT, uintptr_t, LPCSTR);
BOOL SetMenu(HWND, HMENU);

BOOL GetMessage(MSG* msg, HWND, UINT, UINT);
BOOL TranslateMessage(const MSG*);
LRESULT DispatchMessage(const MSG* msg);
BOOL PostMessage(HWND, UINT, WPARAM, LPARAM);
LRESULT SendMessageA(HWND, UINT, WPARAM, LPARAM);
#define SendMessage SendMessageA	// como no SDK: CPSock::SendMessage vira CPSock::SendMessageA
void PostQuitMessage(int code);
LRESULT DefWindowProc(HWND, UINT, WPARAM, LPARAM);

uintptr_t SetTimer(HWND, uintptr_t id, UINT ms, void* proc);
BOOL KillTimer(HWND, uintptr_t id);

/// @brief Escreve "[título] texto" no stderr em vez de abrir uma caixa e devolve a resposta
///        afirmativa (IDYES para MB_YESNO, IDOK nos outros casos).
int MessageBox(HWND, LPCSTR text, LPCSTR caption, UINT type);
#define MessageBoxA MessageBox

/// @brief Caminho do executável (/proc/self/exe), com '/'.
DWORD GetModuleFileName(HINSTANCE, char* buf, DWORD size);
BOOL SetCurrentDirectory(LPCSTR path);
DWORD GetCurrentDirectory(DWORD size, char* buf);
/// @brief Próximo caractere no código de página ANSI de um byte (ver _mbschr).
inline char* CharNext(const char* p) { return (char*)(*p ? p + 1 : p); }

// ---- GDI: o painel do servidor não existe fora do Windows ----------------------------------------

#define FW_LIGHT 300
#define FW_NORMAL 400
#define DEFAULT_CHARSET 1
#define OUT_DEVICE_PRECIS 5
#define CLIP_DEFAULT_PRECIS 0
#define DEFAULT_QUALITY 0
#define DEFAULT_PITCH 0
#define TRANSPARENT 1
#define OPAQUE 2

HDC GetDC(HWND);
int ReleaseDC(HWND, HDC);
HDC BeginPaint(HWND, PAINTSTRUCT*);
BOOL EndPaint(HWND, const PAINTSTRUCT*);
HFONT CreateFont(int, int, int, int, int, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, LPCSTR);
HGDIOBJ SelectObject(HDC, HGDIOBJ);
HGDIOBJ GetStockObject(int);
BOOL DeleteObject(HGDIOBJ);
BOOL TextOut(HDC, int, int, LPCSTR, int);
#define TextOutA TextOut
COLORREF SetTextColor(HDC, COLORREF);
COLORREF SetBkColor(HDC, COLORREF);
int SetBkMode(HDC, int);

// ---- Tempo ---------------------------------------------------------------------------------------

struct SYSTEMTIME
{
	WORD wYear, wMonth, wDayOfWeek, wDay, wHour, wMinute, wSecond, wMilliseconds;
};

DWORD GetTickCount();
DWORD timeGetTime();
void Sleep(DWORD ms);
void GetLocalTime(SYSTEMTIME* st);

// ---- CRT -----------------------------------------------------------------------------------------

#define _O_RDONLY O_RDONLY
#define _O_WRONLY O_WRONLY
#define _O_RDWR O_RDWR
#define _O_CREAT O_CREAT
#define _O_TRUNC O_TRUNC
#define _O_APPEND O_APPEND
#define _O_EXCL O_EXCL
#define _O_BINARY 0
#define _O_TEXT 0
#define O_BINARY 0
#define _S_IREAD S_IRUSR
#define _S_IWRITE S_IWUSR

inline int _open(const char* path, int flags, int mode = 0666) { return open(path, flags, mode ? mode | 0644 : 0644); }
inline int _read(int fd, void* buf, unsigned int n) { return (int)read(fd, buf, n); }
inline int _write(int fd, const void* buf, unsigned int n) { return (int)write(fd, buf, n); }
inline int _close(int fd) { return close(fd); }
inline long _lseek(int fd, long off, int whence) { return (long)lseek(fd, off, whence); }
inline long _filelength(int fd)
{
	struct stat st;
	return fstat(fd, &st) == 0 ? (long)st.st_size : -1L;
}
inline int _access(const char* path, int mode) { return access(path, mode); }
inline int _unlink(const char* path) { return unlink(path); }
inline int _mkdir(const char* path) { return mkdir(path, 0755); }

/// @brief `_findfirst`/`_findnext` sobre glob(): o padrão é o do Windows (`dir/*.*` ou `dir/*`), e
///        `name` é só o nome do arquivo, sem o diretório.
struct _finddata_t
{
	unsigned attrib;
	long long time_create;
	long long time_access;
	long long time_write;
	long long size;
	char name[260];
};
#define _A_NORMAL 0x00
#define _A_SUBDIR 0x10

intptr_t _findfirst(const char* pattern, _finddata_t* data);
int _findnext(intptr_t handle, _finddata_t* data);
int _findclose(intptr_t handle);

char* _strupr(char* s);
char* _strlwr(char* s);
#define strupr _strupr
#define strlwr _strlwr
#define _stricmp strcasecmp
#define _strnicmp strncasecmp
#define stricmp strcasecmp
#define strnicmp strncasecmp
#define _snprintf snprintf
#define _vsnprintf vsnprintf
char* _itoa(int value, char* buf, int radix);

/// @brief `_mbschr` no código de página ANSI de um Windows ocidental (1252, de um byte só), que é o
///        padrão do CRT ao iniciar: igual a strchr.
inline unsigned char* _mbschr(const unsigned char* s, unsigned int c) { return (unsigned char*)strchr((const char*)s, (int)c); }
#define itoa _itoa

/// @brief `MoveFile`/`DeleteFile` do Win32: MoveFile falha se o destino existe.
BOOL MoveFile(LPCSTR from, LPCSTR to);
inline BOOL DeleteFile(LPCSTR path) { return unlink(path) == 0 ? TRUE : FALSE; }
#define DeleteFileA DeleteFile
#define MoveFileA MoveFile
inline DWORD GetLastError() { return (DWORD)errno; }

/// @brief `FindFirstFile`/`FindNextFile` sobre _findfirst.
struct FILETIME { DWORD dwLowDateTime, dwHighDateTime; };
#define FILE_ATTRIBUTE_DIRECTORY 0x10
#define FILE_ATTRIBUTE_NORMAL 0x80
#define INVALID_HANDLE_VALUE ((HANDLE)(intptr_t)-1)
struct WIN32_FIND_DATA
{
	DWORD dwFileAttributes;
	FILETIME ftCreationTime;
	FILETIME ftLastAccessTime;
	FILETIME ftLastWriteTime;
	DWORD nFileSizeHigh;
	DWORD nFileSizeLow;
	char cFileName[MAX_PATH];
};
HANDLE FindFirstFile(LPCSTR pattern, WIN32_FIND_DATA* data);
BOOL FindNextFile(HANDLE h, WIN32_FIND_DATA* data);
BOOL FindClose(HANDLE h);
/// @brief Converte o FILETIME (UTC, 100 ns desde 1601) em SYSTEMTIME UTC, como no Win32.
BOOL FileTimeToSystemTime(const FILETIME* ft, SYSTEMTIME* st);

inline int fopen_s(FILE** fp, const char* path, const char* mode)
{
	*fp = fopen(path, mode);
	return *fp ? 0 : errno;
}

// ---- Winsock -------------------------------------------------------------------------------------

typedef int SOCKET;
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)

typedef struct sockaddr SOCKADDR;
typedef struct sockaddr* PSOCKADDR;
typedef struct sockaddr* LPSOCKADDR;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct hostent HOSTENT;
typedef struct hostent* LPHOSTENT;
typedef struct in_addr IN_ADDR;

#define FD_READ 0x01
#define FD_WRITE 0x02
#define FD_OOB 0x04
#define FD_ACCEPT 0x08
#define FD_CONNECT 0x10
#define FD_CLOSE 0x20

#define WSAMAKESELECTREPLY(event, error) ((LONG)(((WORD)(event)) | (((DWORD)(WORD)(error)) << 16)))
#define WSAGETSELECTEVENT(lParam) LOWORD(lParam)
#define WSAGETSELECTERROR(lParam) HIWORD(lParam)

#define WSAEWOULDBLOCK EWOULDBLOCK
#define WSAEINPROGRESS EINPROGRESS
#define WSAECONNRESET ECONNRESET
#define WSAENOTCONN ENOTCONN

struct WSADATA
{
	WORD wVersion;
	WORD wHighVersion;
	char szDescription[257];
	char szSystemStatus[129];
};

inline int WSAStartup(WORD, WSADATA* d) { memset(d, 0, sizeof(*d)); return 0; }
inline int WSACleanup() { return 0; }
inline int WSAGetLastError() { return errno; }

/// @brief Fecha o descritor e o tira do laço de mensagens.
int closesocket(SOCKET s);
inline int ioctlsocket(SOCKET s, long cmd, unsigned long* arg) { return ioctl(s, cmd, arg); }

/// @brief Registra o socket no laço: os eventos em `events` (FD_*) chegam como a mensagem `msg`.
///        Como no Winsock, o socket passa a ser não bloqueante. `events` = 0 tira o registro.
int WSAAsyncSelect(SOCKET s, HWND hwnd, UINT msg, LONG events);

// ---- Entrada -------------------------------------------------------------------------------------

/// @brief As fontes definem WinMain; o `main` de w2_win32.cpp a chama com a linha de comando.
int WinMain(HINSTANCE, HINSTANCE, LPSTR, int);
