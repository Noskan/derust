# derust

Monorepo unificando o servidor (**W2PP**) e o cliente (**TMProject**) do *With Your Destiny (WYD)*, preservando o histórico completo de commits de ambos os repositórios originais.

## Estrutura

- `server/` — Servidor C++ importado de [Noskan/W2PP](https://github.com/Noskan/W2PP) (`master`), incluindo `W2PP Code Project.sln`, `Code/TMSrv`, `Code/DBSrv` e `Code/ClientPatch_v7662`. Veja [`server/README.md`](server/README.md).
- `client/` — Cliente C++ importado de [Noskan/tm-project](https://github.com/Noskan/tm-project) (`main`), incluindo `TM.sln`, `Projects/TMProject`, `Dependencies/` e `Infos/`. Veja [`client/README.md`](client/README.md).

## Histórico e notas

- O histórico completo de commits de ambos os projetos foi preservado sob `server/` e `client/` (ex.: `git log server/` ou `git log client/`).
- Os workflows originais de CI do cliente permanecem preservados em `client/.github/workflows/` (não são executados automaticamente na raiz do monorepo).
- Ambos os subprojetos originais estão licenciados sob GNU GPL v3.
