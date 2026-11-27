#include "common/Platform.h"

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
