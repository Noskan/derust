/// @file ServerMetrics.h
/// @brief Métricas do DBSrv (common/Metrics.h): w2_dbsrv.prom em $W2_METRICS_DIR, para o
///        coletor textfile do node_exporter (docs/OPS.md).
#ifndef _DBSRV_METRICS_
#define _DBSRV_METRICS_

enum DbMetric
{
	DB_ACCOUNTS_ONLINE,			///< contas conectadas (pAccountList)
	DB_TMSRV_CONNECTED,			///< TMSrv conectados
	DB_LOGIN_FAIL_PASSWORD,		///< senha errada
	DB_LOGIN_FAIL_NO_ACCOUNT,	///< conta que não existe
	DB_LOGIN_FAIL_LOCKED,		///< conta bloqueada no w2db por senhas erradas
	DB_LOGIN_FAIL_BLOCKED,		///< conta bloqueada pela data (Info.Year)
	DB_LOGIN_FAIL_NUMERIC,		///< senha numérica errada
	DB_SAVES,					///< gravações de conta
	DB_W2DB_FAILURES,			///< chamadas ao w2db que falharam (fora do ar, prazo, erro)
	DB_W2DB_PENDING,			///< contas gravadas em ./w2db_pending por falha do w2db
	DB_DUPE_SUSPECTS,			///< itens que o w2db marcou como suspeita de duplicação
	DB_BUG_REPORTS,				///< relatos do /bug gravados
	DB_METRIC_COUNT
};

void DbMetricAdd(int id, double n = 1);

/// @brief Liga a gravação (W2_METRICS_DIR) no boot.
void DbMetricsInit();

/// @brief No timer de segundo: grava o arquivo quando é hora.
void DbMetricsTick();

#endif
