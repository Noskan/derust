#pragma once

/// @file Platform.h
/// @brief O que as regras e os servidores usam do sistema e que só depende da biblioteca padrão:
///        gerador de números aleatórios, avisos, relógio e texto. Compila em qualquer plataforma
///        (o tools/divergence_map.py compila as regras de server/common com o mingw). O que é do
///        Linux (arquivos, diretórios, laço de eventos e sockets) fica em System.h, EventLoop.h e
///        CPSock.h.

#include <stddef.h>

/// O código original usa o BOOL do Win32 (um int) com TRUE e FALSE, inclusive em funções que
/// devolvem também -1 (CPSock::Receive): ficam como tipos do projeto.
typedef int BOOL;
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

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

#ifdef __GNUC__
#define W2_PRINTF_FORMAT(f, a) __attribute__((format(printf, f, a)))
#else
#define W2_PRINTF_FORMAT(f, a)
#endif

/// @brief snprintf que diz se o texto coube. O código original usava sprintf, que passava do fim
///        do buffer quando o texto era maior que ele.
///
/// Nos logs e nos avisos ao jogador, cortar no tamanho do buffer é o resultado certo e o retorno
/// pode ser ignorado. Num caminho de arquivo, um caminho cortado aponta para outro arquivo: quem
/// chama trata `false` como falha.
/// @return true se o texto coube inteiro em `buf` (sempre terminado em 0 quando size > 0).
bool W2Format(char* buf, size_t size, const char* fmt, ...) W2_PRINTF_FORMAT(3, 4);

/// @brief Relógio monotônico em milissegundos (volta a zero a cada ~49 dias, como o
///        GetTickCount/timeGetTime do código original): só as diferenças têm sentido.
unsigned int W2TickCount();

/// @brief Suspende a thread por `ms` milissegundos.
void W2SleepMs(unsigned int ms);

/// @brief Passa `s` para maiúsculas no lugar (só ASCII, como o _strupr do CRT no código de
///        página de um byte) e devolve `s`.
char* W2StrUpper(char* s);
