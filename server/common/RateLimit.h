/// @file RateLimit.h
/// @brief Limite de eventos por chave (IP) em janela fixa, para a borda de rede dos servidores.
#ifndef _W2_RATELIMIT_
#define _W2_RATELIMIT_

/// Conta eventos por chave numa janela fixa de `windowMs` e recusa a partir de `limit + 1`.
/// A tabela tem tamanho fixo (sem alocação): duas chaves no mesmo slot dividem a contagem do
/// slot mais recente, o que só deixa o limite mais frouxo para elas, nunca bloqueia além dele.
class W2RateLimit
{
public:
	W2RateLimit(int limit, unsigned int windowMs);

	/// Registra um evento da chave no instante `nowMs` (relógio em ms, pode dar a volta) e diz se
	/// ele cabe no limite. O evento recusado também conta.
	bool Allow(unsigned int key, unsigned int nowMs);

	/// Esquece todas as chaves.
	void Clear();

private:
	struct Slot
	{
		unsigned int key;
		unsigned int start;
		int count;
	};

	static const int kSlots = 1024;

	int limit_;
	unsigned int windowMs_;
	Slot slots_[kSlots];
};

#endif
