/// @file ServerMetrics.h
/// @brief Métricas do TMSrv (common/Metrics.h): w2_tmsrv.prom em $W2_METRICS_DIR, para o
///        coletor textfile do node_exporter (docs/OPS.md).
#ifndef _TMSRV_METRICS_
#define _TMSRV_METRICS_

enum TmMetric
{
	TM_USERS_ONLINE,		///< jogadores com personagem no mundo
	TM_CONNECTIONS,			///< conexões de cliente abertas
	TM_TICK_MAX,			///< maior ProcessSecTimer desde a última gravação
	TM_TICK_SECONDS,		///< soma do tempo dos ProcessSecTimer
	TM_TICKS,				///< ProcessSecTimer executados
	TM_PACKETS,				///< pacotes de cliente lidos
	TM_REJECTED_READ,		///< pacotes recusados na leitura (tamanho, checksum)
	TM_REJECTED_SHORT,		///< pacotes menores que a estrutura do tipo
	TM_CONNECTIONS_REFUSED,	///< conexões recusadas pela borda de rede (por IP)
	TM_LOGINS_LIMITED,		///< logins recusados pelo limite por IP
	TM_BUG_REPORTS,			///< relatos do /bug enviados ao DBSrv
	TM_DBSRV_CONNECTED,		///< 1 se a conexão com o DBSrv está aberta
	TM_METRIC_COUNT
};

void TmMetricAdd(int id, double n = 1);

/// @brief Liga a gravação (W2_METRICS_DIR) no boot.
void TmMetricsInit();

/// @brief Roda o ProcessSecTimer medindo o tempo, e grava o arquivo quando é hora.
void TmMetricsSecTimer(void (*secTimer)());

#endif
