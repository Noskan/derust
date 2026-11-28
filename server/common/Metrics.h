/// @file Metrics.h
/// @brief Métricas dos servidores para o coletor textfile do node_exporter (docs/OPS.md).
///
/// Cada servidor tem uma tabela fixa de W2Metric e a grava, de tempos em tempos, em
/// `$W2_METRICS_DIR/<arquivo>.prom` no formato de texto do Prometheus: escreve num temporário e
/// renomeia, para o coletor nunca ler um arquivo pela metade. Sem W2_METRICS_DIR, nada é gravado
/// e as contagens só ficam na memória.
///
/// Além da tabela, o arquivo leva `<prefixo>_start_time_seconds` e
/// `<prefixo>_last_write_timestamp_seconds` (o alerta de servidor parado olha esta).
#ifndef _W2_METRICS_
#define _W2_METRICS_

struct W2Metric
{
	/// Nome com os rótulos, se houver: `w2_dbsrv_login_failures_total{reason="password"}`.
	/// Os de mesmo nome (com rótulos diferentes) ficam juntos na tabela.
	const char* Name;
	const char* Type;	///< "counter" ou "gauge"
	const char* Help;
	double Value;
};

/// @brief Liga a gravação se W2_METRICS_DIR estiver definida.
/// @param prefix prefixo das métricas e nome do arquivo (`w2_tmsrv` grava w2_tmsrv.prom).
/// @param intervalSec intervalo entre gravações (W2MetricsDue).
/// @return true se vai gravar.
bool W2MetricsInit(const char* prefix, W2Metric* table, int count, int intervalSec);

/// @brief Passou o intervalo desde a última gravação: o servidor atualiza os valores do momento
///        (jogadores, conexões) e chama W2MetricsWrite. Chamado no timer de segundo.
bool W2MetricsDue();

/// @brief Grava agora (e conta o intervalo a partir daqui).
/// @return false se não conseguiu gravar (diretório sem permissão, disco cheio).
bool W2MetricsWrite();

/// @brief O texto que iria para o arquivo (os testes e a gravação usam).
/// @return o tamanho escrito em `out` (sempre terminado em 0), ou -1 se não coube.
int W2MetricsRender(char* out, int size, long long now);

#endif
