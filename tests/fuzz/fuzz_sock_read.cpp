/// @file fuzz_sock_read.cpp
/// @brief Fuzzing de CPSock::ReadMessage: bytes crus da rede, como chegam do cliente.
///
/// O buffer de recepção tem o tamanho exato da entrada, para o ASan acusar qualquer leitura além
/// do que chegou. Cada mensagem devolvida é percorrida até o Size do cabeçalho.
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "common/CPSock.h"

// O CPSock usa o relógio e o log que cada servidor define (core/Server.cpp).
unsigned int LastSendTime = 0;
unsigned int CurrentTime = 0;
void Log(char*, char*, unsigned int) {}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
	if (size == 0 || size > RECV_BUFFER_SIZE)
		return 0;

	CPSock sock;
	free(sock.pRecvBuffer);
	sock.pRecvBuffer = static_cast<char*>(malloc(size));
	memcpy(sock.pRecvBuffer, data, size);
	sock.nRecvPosition = static_cast<int>(size);
	sock.nProcPosition = 0;

	unsigned sum = 0;
	for (;;)
	{
		int error = 0;
		int errorType = 0;
		char* msg = sock.ReadMessage(&error, &errorType);
		if (msg == nullptr)
			break;

		unsigned short msgSize;
		memcpy(&msgSize, msg, sizeof(msgSize));
		for (unsigned i = 0; i < msgSize; i++)
			sum += static_cast<unsigned char>(msg[i]);

		if (error == 2)
			break;
	}

	// Sem isto o compilador pode tirar a leitura acima.
	volatile unsigned sink = sum;
	(void)sink;
	return 0;
}
