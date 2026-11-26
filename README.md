# derust

Monorepo unificando o servidor (**W2PP**) e o cliente (**TMProject**) do *With Your Destiny (WYD)*, preservando o histórico completo de commits de ambos os repositórios originais.

## Estrutura do Projeto

- [`server/`](server/README.md) — Servidor C++ importado de [Noskan/W2PP](https://github.com/Noskan/W2PP), organizado em módulos:
  - `server/common/` (`w2pp_common`) — Tipos base, pacotes, socket (`Basedef`, `CPSock`, `ItemEffect`) e trava de layout ABI.
  - `server/dbsrv/` (`DBSrv`) — Servidor de banco de dados em arquivos (`core/`, `persistence/`, `net/`, `resources/`).
  - `server/tmsrv/` (`TMSrv`) — Game server (`core/`, `entities/`, `net/`, `systems/`, `handlers/{account,combat,items,combine,trade,social,world}/`, `resources/`).
  - `server/tools/client_patch/` (`ClientPatch_v7662`) — DLL opcional de patch para cliente v7662 (`-DW2PP_BUILD_CLIENT_PATCH=ON`).
- [`client/`](client/README.md) — Cliente C++ DirectX 9 importado de [Noskan/tm-project](https://github.com/Noskan/tm-project), organizado em:
  - `client/src/` (`TMProject`) — `core/`, `common/`, `net/`, `crypto/` (`tm_crypto`), `render/`, `audio/`, `ui/`, `scene/`, `world/`, `entities/`, `skills/`, `effects/`, `gameplay/` e `resources/`.
  - `client/third_party/directx/` (`tm_directx`) — Headers e bibliotecas do DirectX 9 SDK.
  - `client/docs/` — Documentação auxiliar do cliente.
- [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md) — Mapa completo de módulos, diagrama de dependências, travas de ABI, regras de fronteira e roteiro da Fase 2.
- [`tools/check_boundaries.py`](tools/check_boundaries.py) — Verificador automático de fronteiras de dependência entre módulos.

## Build (Windows Win32 / MSVC)

```powershell
# Configurar (Visual Studio 2022 Win32)
cmake --preset win32-msvc

# Ou autodetectando a versão do Visual Studio instalada
cmake -S . -B build/win32-msvc -A Win32

# Compilar Debug ou Release
cmake --build build/win32-msvc --config Debug --parallel
cmake --build build/win32-msvc --config Release --parallel
```

Para verificar localmente as fronteiras de `#include`:

```bash
python3 tools/check_boundaries.py
```

## Status da Modularização

- **Fase 1 (concluída)**: Reorganização modular via `git mv`, migração para CMake + presets Win32, pipeline CI no GitHub Actions (`windows-latest` Debug/Release), verificador de fronteiras e travas de ABI em `ProtocolLayoutCheck.cpp`.
- **Fase 2 — Onda 1 (concluída)**: Limpeza de `ItemEffect.h` para PT-BR, `.clang-format`, documentação Doxygen/inline em PT-BR e fatiamento dos 4 maiores *god-files*: `server/common/Basedef.cpp` (`Basedef`/`BaseItem`/`BaseCombat`), `tmsrv/core/Server.cpp` (5 sistemas em `tmsrv/systems/`), `TMFieldScene.cpp` (5 TUs) e `TMHuman.cpp` (5 TUs).
- **Fase 2 — Onda 2 (próxima etapa)**: Fatiamento de `_MSG_UseItem.cpp`, `SGrid.cpp`, `TMGround.cpp`, `_MSG_Quest.cpp`, `CFileDB.cpp`, `MobKilled.cpp` e testes CTest nos módulos extraídos.

## Histórico e Licença

- O histórico completo de commits foi preservado via `git mv` (use `git log --follow -- <arquivo>`).
- Ambos os projetos originais são licenciados sob **GNU GPL v3** (veja [`server/license.txt`](server/license.txt)).
