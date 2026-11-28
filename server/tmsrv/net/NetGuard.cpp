/// @file NetGuard.cpp
/// @brief Implementação da borda de rede do TMSrv (net/NetGuard.h).
#include "net/NetGuard.h"

#include "common/Basedef.h"
#include "common/RateLimit.h"
#include "core/Server.h"
#include "core/ServerMetrics.h"

namespace
{
W2RateLimit g_acceptLimit(NETGUARD_ACCEPTS, NETGUARD_ACCEPT_WINDOW_MS);
W2RateLimit g_loginLimit(NETGUARD_LOGINS, NETGUARD_LOGIN_WINDOW_MS);
}

bool NetGuardAccept(int conn)
{
	const unsigned int ip = pUser[conn].IP;

	int sameIP = 0;
	for (int i = 1; i < MAX_USER; i++)
	{
		if (i != conn && pUser[i].Mode != USER_EMPTY && pUser[i].IP == ip)
			sameIP++;
	}

	if (sameIP >= NETGUARD_MAX_CONN_PER_IP)
	{
		Log("err,netguard too many connections from ip", "-system", ip);
		TmMetricAdd(TM_CONNECTIONS_REFUSED);
		return false;
	}

	if (!g_acceptLimit.Allow(ip, W2TickCount()))
	{
		Log("err,netguard accept rate", "-system", ip);
		TmMetricAdd(TM_CONNECTIONS_REFUSED);
		return false;
	}

	return true;
}

bool NetGuardLogin(int conn)
{
	if (g_loginLimit.Allow(pUser[conn].IP, W2TickCount()))
		return true;

	Log("err,netguard login rate", pUser[conn].AccountName, pUser[conn].IP);
	TmMetricAdd(TM_LOGINS_LIMITED);
	return false;
}

bool NetGuardLoginExpired(int conn)
{
	const int mode = pUser[conn].Mode;
	if (mode != USER_ACCEPT && mode != USER_LOGIN)
		return false;

	return SecCounter - pUser[conn].AcceptSec > NETGUARD_LOGIN_TIMEOUT_SEC;
}
