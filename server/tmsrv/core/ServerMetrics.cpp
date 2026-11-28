/// @file ServerMetrics.cpp
/// @brief Métricas do TMSrv (core/ServerMetrics.h).
#include "core/ServerMetrics.h"

#include <chrono>

#include "common/Metrics.h"
#include "common/Basedef.h"
#include "core/Server.h"

namespace
{
W2Metric g_metrics[TM_METRIC_COUNT] = {
	{ "w2_tmsrv_users_online", "gauge", "Jogadores com personagem no mundo.", 0 },
	{ "w2_tmsrv_connections", "gauge", "Conexões de cliente abertas.", 0 },
	{ "w2_tmsrv_tick_seconds_max", "gauge", "Maior ProcessSecTimer desde a gravação anterior.", 0 },
	{ "w2_tmsrv_tick_seconds_total", "counter", "Tempo somado dos ProcessSecTimer.", 0 },
	{ "w2_tmsrv_ticks_total", "counter", "ProcessSecTimer executados.", 0 },
	{ "w2_tmsrv_packets_total", "counter", "Pacotes de cliente lidos.", 0 },
	{ "w2_tmsrv_packets_rejected_total{reason=\"read\"}", "counter", "Pacotes de cliente recusados, pelo motivo.", 0 },
	{ "w2_tmsrv_packets_rejected_total{reason=\"short\"}", "counter", "Pacotes de cliente recusados, pelo motivo.", 0 },
	{ "w2_tmsrv_connections_refused_total", "counter", "Conexões recusadas pela borda de rede (conexões ou ritmo por IP).", 0 },
	{ "w2_tmsrv_logins_rate_limited_total", "counter", "Logins recusados pelo limite por IP.", 0 },
	{ "w2_tmsrv_bug_reports_total", "counter", "Relatos do /bug enviados ao DBSrv.", 0 },
	{ "w2_tmsrv_dbsrv_connected", "gauge", "1 se a conexão com o DBSrv está aberta.", 0 },
};
}

void TmMetricAdd(int id, double n)
{
	if (id >= 0 && id < TM_METRIC_COUNT)
		g_metrics[id].Value += n;
}

void TmMetricsInit()
{
	if (W2MetricsInit("w2_tmsrv", g_metrics, TM_METRIC_COUNT, 15))
		Log("sys,metrics enabled (W2_METRICS_DIR)", "-system", 0);
}

void TmMetricsSecTimer(void (*secTimer)())
{
	auto start = std::chrono::steady_clock::now();
	secTimer();
	double took = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
	g_metrics[TM_TICK_SECONDS].Value += took;
	g_metrics[TM_TICKS].Value += 1;
	if (took > g_metrics[TM_TICK_MAX].Value)
		g_metrics[TM_TICK_MAX].Value = took;

	if (!W2MetricsDue())
		return;

	int online = 0, connections = 0;
	for (int i = 1; i < MAX_USER; i++)
	{
		if (pUser[i].Mode == USER_EMPTY)
			continue;
		connections++;
		if (pUser[i].Mode == USER_PLAY)
			online++;
	}
	g_metrics[TM_USERS_ONLINE].Value = online;
	g_metrics[TM_CONNECTIONS].Value = connections;
	g_metrics[TM_DBSRV_CONNECTED].Value = DBServerSocket.Sock != 0 ? 1 : 0;

	static bool warned = false;
	if (!W2MetricsWrite())
	{
		if (!warned)
			Log("err,metrics write failed (W2_METRICS_DIR)", "-system", 0);
		warned = true;
	}
	else
		warned = false;
	g_metrics[TM_TICK_MAX].Value = 0;
}
