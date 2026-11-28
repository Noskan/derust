/// @file NetGuard.h
/// @brief Borda de rede do TMSrv: conexões e tentativas de login por IP, e tempo para logar.
///
/// Limites para o servidor exposto na internet (docs/FASE_2_5.md, B1). Usam pUser[conn].IP.
#ifndef _NETGUARD_
#define _NETGUARD_

#define NETGUARD_MAX_CONN_PER_IP	5		// conexões simultâneas do mesmo IP
#define NETGUARD_ACCEPTS			10		// conexões novas do mesmo IP...
#define NETGUARD_ACCEPT_WINDOW_MS	10000	// ...nesta janela
#define NETGUARD_LOGINS				5		// pedidos de login (0x20D) do mesmo IP...
#define NETGUARD_LOGIN_WINDOW_MS	60000	// ...nesta janela
#define NETGUARD_LOGIN_TIMEOUT_SEC	60		// em SecCounter (meio segundo): 30 s para logar

/// Logo depois do AcceptUser: false se o IP passou dos limites (o chamador fecha a conexão).
bool NetGuardAccept(int conn);

/// No pedido de login da conta: false se o IP passou do limite de tentativas.
bool NetGuardLogin(int conn);

/// No CheckIdle: true se a conexão ainda não terminou o login dentro do tempo.
bool NetGuardLoginExpired(int conn);

#endif
