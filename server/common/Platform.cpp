#include "common/Platform.h"

#include <cctype>
#include <chrono>
#include <cstdio>
#include <thread>

namespace
{
unsigned int g_RandState = 1;
}

int W2Rand()
{
	g_RandState = g_RandState * 214013u + 2531011u;
	return (int)((g_RandState >> 16) & 0x7FFF);
}

void W2Srand(unsigned int seed)
{
	g_RandState = seed;
}

void ServerAlert(const char* text, const char* caption)
{
	std::fprintf(stderr, "[%s] %s\n", caption ? caption : "", text ? text : "");
	std::fflush(stderr);
}

unsigned int W2TickCount()
{
	auto since = std::chrono::steady_clock::now().time_since_epoch();
	return (unsigned int)std::chrono::duration_cast<std::chrono::milliseconds>(since).count();
}

void W2SleepMs(unsigned int ms)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

char* W2StrUpper(char* s)
{
	for (char* p = s; *p; ++p)
		*p = (char)std::toupper((unsigned char)*p);
	return s;
}
