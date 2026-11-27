/// @file System.cpp
/// @brief Implementação POSIX de System.h e de BASE_InitModuleDir.

#include "common/System.h"
#include "common/Basedef.h"

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <map>
#include <string>
#include <vector>

#include <glob.h>
#include <sys/stat.h>
#include <unistd.h>

namespace
{
struct Listing
{
	std::vector<std::string> paths;
	size_t next = 0;
};

// Handles pequenos (1, 2, ...): as fontes guardam o handle num int.
// Estáticos locais: o RankingSystem do DBSrv lista arquivos no próprio construtor, que roda
// antes dos construtores globais desta TU.
std::map<int, Listing>& Listings()
{
	static std::map<int, Listing> m;
	return m;
}
int g_NextHandle = 1;	// inicialização constante: vale antes de qualquer construtor

void Fill(const std::string& path, W2FindData* d)
{
	memset(d, 0, sizeof(*d));
	size_t slash = path.find_last_of('/');
	std::string name = slash == std::string::npos ? path : path.substr(slash + 1);
	strncpy(d->name, name.c_str(), sizeof(d->name) - 1);
	struct stat st{};
	if (stat(path.c_str(), &st) == 0)
	{
		d->isDir = S_ISDIR(st.st_mode);
		d->size = st.st_size;
		d->writeTime = st.st_mtime;
	}
}

bool IsDotEntry(const std::string& path)
{
	size_t slash = path.find_last_of('/');
	std::string name = slash == std::string::npos ? path : path.substr(slash + 1);
	return name == "." || name == "..";
}
} // namespace

int W2FindFirst(const char* pattern, W2FindData* data)
{
	std::string p = pattern;
	std::replace(p.begin(), p.end(), '\\', '/');
	if (p.size() >= 3 && p.compare(p.size() - 3, 3, "*.*") == 0)
		p.replace(p.size() - 3, 3, "*");

	// GLOB_PERIOD: "*" casa também os nomes que começam com ponto, inclusive "." e "..".
	glob_t g{};
	if (glob(p.c_str(), GLOB_PERIOD, nullptr, &g) != 0 || g.gl_pathc == 0)
	{
		globfree(&g);
		errno = ENOENT;
		return -1;
	}
	Listing l;
	for (size_t i = 0; i < g.gl_pathc; i++)
		l.paths.emplace_back(g.gl_pathv[i]);
	globfree(&g);
	std::stable_partition(l.paths.begin(), l.paths.end(), IsDotEntry);

	int h = g_NextHandle++;
	Listing& stored = Listings()[h] = std::move(l);
	Fill(stored.paths[stored.next++], data);
	return h;
}

int W2FindNext(int handle, W2FindData* data)
{
	auto it = Listings().find(handle);
	if (it == Listings().end() || it->second.next >= it->second.paths.size())
	{
		errno = ENOENT;
		return -1;
	}
	Fill(it->second.paths[it->second.next++], data);
	return 0;
}

void W2FindClose(int handle)
{
	Listings().erase(handle);
}

BOOL W2MoveFile(const char* from, const char* to)
{
	if (access(to, F_OK) == 0)
	{
		errno = EEXIST;
		return FALSE;
	}
	return rename(from, to) == 0 ? TRUE : FALSE;
}

/// Os servidores abrem os arquivos por caminhos relativos ao próprio executável (./, ../../Common).
void BASE_InitModuleDir()
{
	char path[4096];
	ssize_t n = readlink("/proc/self/exe", path, sizeof(path) - 1);
	if (n <= 0)
		return;
	path[n] = 0;
	char* slash = strrchr(path, '/');
	if (slash == nullptr)
		return;
	*slash = 0;
	// Como no original (SetCurrentDirectory sem checagem), uma falha só deixa o diretório atual.
	if (chdir(path[0] ? path : "/") != 0)
		return;
}
