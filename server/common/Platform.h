#pragma once

/// @file Platform.h
/// @brief O que os servidores usam do sistema e que muda entre Windows e Linux, com o mesmo
///        comportamento nos dois. Começa pelo gerador de números aleatórios; o port para Linux
///        traz para cá o resto (arquivos, tempo, sockets).

/// @brief Mesmo algoritmo e mesmo intervalo (0..0x7FFF) do W2Rand() do CRT da Microsoft, com a
///        mesma semente inicial (1). O W2Rand() da glibc gera outra sequência com RAND_MAX
///        2147483647, o que mudaria chances de drop, refinação e dano no Linux. O estado é
///        único no processo (os servidores têm uma thread só).
int  W2Rand();
void W2Srand(unsigned int seed);

constexpr int W2_RAND_MAX = 0x7FFF;
