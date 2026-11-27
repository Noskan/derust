/// @file w2_main.cpp
/// @brief Ponto de entrada do TMSrv e do DBSrv fora do Windows: chama o WinMain das fontes com a
///        linha de comando. Fica fora de w2pp_common para não colidir com o main dos testes.

#include "w2_win32.h"

#include <string>

int main(int argc, char** argv)
{
	std::string cmd;
	for (int i = 1; i < argc; i++)
	{
		if (i > 1)
			cmd += ' ';
		cmd += argv[i];
	}
	return WinMain(nullptr, nullptr, cmd.data(), 0);
}
