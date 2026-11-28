/// @file RateLimit.cpp
/// @brief Implementação de W2RateLimit (common/RateLimit.h).
#include "common/RateLimit.h"

#include <cstring>

W2RateLimit::W2RateLimit(int limit, unsigned int windowMs) : limit_(limit), windowMs_(windowMs)
{
	Clear();
}

void W2RateLimit::Clear()
{
	memset(slots_, 0, sizeof(slots_));
}

bool W2RateLimit::Allow(unsigned int key, unsigned int nowMs)
{
	// Mistura os bits do IP (os da rede variam pouco) antes de escolher o slot.
	unsigned int h = key * 2654435761u;
	Slot& s = slots_[(h >> 22) % kSlots];

	// A subtração sem sinal cobre a volta do relógio de 32 bits.
	if (s.count == 0 || s.key != key || nowMs - s.start >= windowMs_)
	{
		s.key = key;
		s.start = nowMs;
		s.count = 0;
	}

	if (s.count < 0x7FFFFFFF)
		s.count++;

	return s.count <= limit_;
}
