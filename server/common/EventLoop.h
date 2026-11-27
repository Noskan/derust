#pragma once

/// @file EventLoop.h
/// @brief Laço de eventos dos servidores (Linux): timers, sockets, comandos do operador e pedido
///        de encerramento, entregues um de cada vez a um único tratador, na thread principal.
///
/// Segue a ordem do laço de mensagens da janela do Windows de onde o código veio:
/// - os eventos prontos entram numa fila, e a fila é esvaziada antes de esperar de novo;
/// - um socket de dados com bytes para ler gera W2_NET_READ; quando o outro lado fecha (ou há
///   erro), um único W2_NET_CLOSE, depois dos dados que restavam;
/// - um socket que escuta gera W2_NET_ACCEPT quando há uma conexão para aceitar;
/// - SIGINT e SIGTERM geram W2_EVENT_CLOSE, o mesmo caminho do comando Exit;
/// - depois de W2LoopQuit, o laço ainda entrega o que já estava na fila e então volta.
///
/// Comandos: W2LoopAddCommand registra os itens do menu de cada servidor, e cada linha escrita no
/// FIFO de controle (`control.fifo` no diretório do servidor, ou o caminho em W2_CONTROL) com o
/// nome de um item gera o W2_EVENT_COMMAND dele (ver docs/LINUX.md).

/// @brief Códigos de evento que não são de socket. Os sockets usam como código o canal passado a
///        W2LoopWatch (os NET_* de common/CPSock.h), que precisa ser maior que W2_EVENT_LAST.
enum W2EventCode
{
	W2_EVENT_TIMER = 1,	///< `id` é o id do timer (W2LoopSetTimer)
	W2_EVENT_COMMAND,	///< `id` é o id do comando (W2LoopAddCommand)
	W2_EVENT_CLOSE,		///< SIGINT/SIGTERM ou W2LoopPostClose
	W2_EVENT_LAST = 99
};

/// @brief O que aconteceu num socket.
enum W2NetEvent
{
	W2_NET_READ = 0x01,
	W2_NET_ACCEPT = 0x08,
	W2_NET_CLOSE = 0x20,
};

struct W2Event
{
	int code;	///< W2_EVENT_* ou o canal do socket
	int id;		///< timer ou comando
	int fd;		///< socket (só nos eventos de socket)
	int net;	///< W2_NET_* (só nos eventos de socket)
	int error;	///< errno de um W2_NET_CLOSE por erro; 0 quando o outro lado fechou
};

typedef void (*W2EventHandler)(const W2Event& ev);

/// @brief Liga os sinais: SIGINT e SIGTERM pedem o encerramento e SIGPIPE é ignorado (um send()
///        num socket fechado devolve erro). Chamado no início do main, antes de carregar dados.
void W2LoopInit();

/// @brief Registra `fd` no laço: os eventos em `events` (W2_NET_*) chegam com o código `channel`.
///        O socket passa a ser não bloqueante. @return 0, ou -1 se não deu para mudar o socket.
int W2LoopWatch(int fd, int channel, int events);

/// @brief Tira `fd` do laço e o fecha. @return o resultado de close().
int W2CloseSocket(int fd);

/// @brief Cria ou troca o timer `id`: um W2_EVENT_TIMER a cada `ms` milissegundos.
void W2LoopSetTimer(int id, unsigned int ms);

/// @brief Registra um comando do operador. O nome aceito no FIFO é `label` em minúsculas, só com
///        letras e dígitos ("Reload MobBase" -> "reloadmobbase"); o id numérico também vale.
void W2LoopAddCommand(int id, const char* label);

/// @brief Põe um W2_EVENT_CLOSE no fim da fila (o comando Exit).
void W2LoopPostClose();

/// @brief Encerra o laço com `code` depois de entregar o que já está na fila.
void W2LoopQuit(int code);

/// @brief Roda o laço até W2LoopQuit. @return o código passado a W2LoopQuit.
int W2LoopRun(W2EventHandler handler);
