#pragma once

/// @file Platform.h
/// @brief O que as regras e os servidores usam do sistema e que só depende da biblioteca padrão:
///        gerador de números aleatórios, avisos, relógio e texto. Compila em qualquer plataforma
///        (o tools/divergence_map.py compila as regras de server/common com o mingw). O que é do
///        Linux (arquivos, diretórios, laço de eventos e sockets) fica em System.h, EventLoop.h e
///        CPSock.h.

/// @brief Mesmo algoritmo e mesmo intervalo (0..0x7FFF) do W2Rand() do CRT da Microsoft, com a
///        mesma semente inicial (1). O W2Rand() da glibc gera outra sequência com RAND_MAX
///        2147483647, o que mudaria chances de drop, refinação e dano no Linux. O estado é
///        único no processo (os servidores têm uma thread só).
int  W2Rand();
void W2Srand(unsigned int seed);

constexpr int W2_RAND_MAX = 0x7FFF;

/// @brief Aviso ao operador: escreve "[título] texto" no stderr. É o que sobrou das MessageBox do
///        código original; o servidor segue depois do aviso, como seguia depois da caixa. O teste
///        de partida (tools/server_boot_smoke.py) procura essas linhas no stderr.
void ServerAlert(const char* text, const char* caption);
