/// @file ServerMetrics.cpp
/// @brief Métricas do DBSrv (core/ServerMetrics.h).
#include "core/ServerMetrics.h"

#include "common/Basedef.h"
#include "common/Metrics.h"
#include "net/CUser.h"
#include "persistence/CFileDB.h"
#include "core/Server.h"

namespace
{
W2Metric g_metrics[DB_METRIC_COUNT] = {
	{ "w2_dbsrv_accounts_online", "gauge", "Contas conectadas.", 0 },
	{ "w2_dbsrv_tmsrv_connected", "gauge", "TMSrv conectados.", 0 },
	{ "w2_dbsrv_login_failures_total{reason=\"password\"}", "counter", "Logins recusados, pelo motivo.", 0 },
	{ "w2_dbsrv_login_failures_total{reason=\"no_account\"}", "counter", "Logins recusados, pelo motivo.", 0 },
	{ "w2_dbsrv_login_failures_total{reason=\"locked\"}", "counter", "Logins recusados, pelo motivo.", 0 },
	{ "w2_dbsrv_login_failures_total{reason=\"blocked\"}", "counter", "Logins recusados, pelo motivo.", 0 },
	{ "w2_dbsrv_login_failures_total{reason=\"numeric\"}", "counter", "Logins recusados, pelo motivo.", 0 },
	{ "w2_dbsrv_saves_total", "counter", "Gravações de conta.", 0 },
	{ "w2_dbsrv_w2db_failures_total", "counter", "Chamadas ao w2db que falharam (fora do ar, prazo estourado, erro).", 0 },
	{ "w2_dbsrv_w2db_pending_total", "counter", "Contas gravadas em ./w2db_pending por falha do w2db.", 0 },
	{ "w2_dbsrv_dupe_suspects_total", "counter", "Itens marcados pelo w2db como suspeita de duplicação.", 0 },
	{ "w2_dbsrv_bug_reports_total", "counter", "Relatos do /bug gravados.", 0 },
};
}

void DbMetricAdd(int id, double n)
{
	if (id >= 0 && id < DB_METRIC_COUNT)
		g_metrics[id].Value += n;
}

void DbMetricsInit()
{
	if (W2MetricsInit("w2_dbsrv", g_metrics, DB_METRIC_COUNT, 15))
		Log(const_cast<char*>("sys,metrics enabled (W2_METRICS_DIR)"), const_cast<char*>("-system"), 0);
}

void DbMetricsTick()
{
	if (!W2MetricsDue())
		return;

	int accounts = 0;
	for (int i = 0; i < MAX_DBACCOUNT; i++)
	{
		if (cFileDB.pAccountList[i].Login != 0)
			accounts++;
	}
	int servers = 0;
	for (int i = 0; i < MAX_SERVER; i++)
	{
		if (pUser[i].Mode != USER_EMPTY && pUser[i].cSock.Sock != 0)
			servers++;
	}
	g_metrics[DB_ACCOUNTS_ONLINE].Value = accounts;
	g_metrics[DB_TMSRV_CONNECTED].Value = servers;

	static bool warned = false;
	if (!W2MetricsWrite())
	{
		if (!warned)
			Log(const_cast<char*>("err,metrics write failed (W2_METRICS_DIR)"), const_cast<char*>("-system"), 0);
		warned = true;
	}
	else
		warned = false;
}
