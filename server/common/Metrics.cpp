/// @file Metrics.cpp
/// @brief Gravação das métricas no formato de texto do Prometheus (Metrics.h).
#include "common/Metrics.h"

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <process.h>
#define w2_getpid _getpid
#else
#include <unistd.h>
#define w2_getpid getpid
#endif

#include "common/Platform.h"

namespace
{
const char* g_prefix = nullptr;
W2Metric* g_table = nullptr;
int g_count = 0;
unsigned int g_intervalMs = 15000;
unsigned int g_lastWrite = 0;
long long g_start = 0;
char g_path[512];
char g_tmp[600];

/// Tamanho do nome sem os rótulos.
size_t BaseLength(const char* name)
{
	const char* brace = strchr(name, '{');
	return brace ? (size_t)(brace - name) : strlen(name);
}

/// Número no formato do Prometheus: inteiros sem casas (contadores grandes sem notação
/// científica), o resto com 9 algarismos significativos.
void FormatValue(char* out, size_t size, double v)
{
	if (isnan(v))
		snprintf(out, size, "NaN");
	else if (isinf(v))
		snprintf(out, size, v > 0 ? "+Inf" : "-Inf");
	else if (v == floor(v) && fabs(v) < 1e15)
		snprintf(out, size, "%.0f", v);
	else
		snprintf(out, size, "%.9g", v);
}

struct Out
{
	char* p;
	int size;
	int len;
	bool ok;

	void Add(const char* fmt, ...) W2_PRINTF_FORMAT(2, 3);
};

void Out::Add(const char* fmt, ...)
{
	if (!ok)
		return;
	va_list ap;
	va_start(ap, fmt);
	int n = vsnprintf(p + len, (size_t)(size - len), fmt, ap);
	va_end(ap);
	if (n < 0 || n >= size - len)
	{
		ok = false;
		return;
	}
	len += n;
}
}

bool W2MetricsInit(const char* prefix, W2Metric* table, int count, int intervalSec)
{
	g_prefix = prefix;
	g_table = table;
	g_count = count;
	g_intervalMs = (unsigned int)(intervalSec > 0 ? intervalSec : 15) * 1000u;
	g_start = (long long)time(nullptr);
	g_lastWrite = W2TickCount();

	const char* dir = getenv("W2_METRICS_DIR");
	if (dir == nullptr || dir[0] == 0)
	{
		g_path[0] = 0;
		return false;
	}
	if (!W2Format(g_path, sizeof(g_path), "%s/%s.prom", dir, prefix) ||
		!W2Format(g_tmp, sizeof(g_tmp), "%s/.%s.prom.%d.tmp", dir, prefix, (int)w2_getpid()))
	{
		g_path[0] = 0;
		return false;
	}
	return true;
}

int W2MetricsRender(char* out, int size, long long now)
{
	if (size <= 0)
		return -1;
	Out o = {out, size, 0, true};
	out[0] = 0;
	char value[64];

	for (int i = 0; i < g_count; i++)
	{
		const W2Metric& m = g_table[i];
		size_t base = BaseLength(m.Name);
		bool first = i == 0 || BaseLength(g_table[i - 1].Name) != base ||
			strncmp(g_table[i - 1].Name, m.Name, base) != 0;
		if (first)
		{
			o.Add("# HELP %.*s %s\n", (int)base, m.Name, m.Help);
			o.Add("# TYPE %.*s %s\n", (int)base, m.Name, m.Type);
		}
		FormatValue(value, sizeof(value), m.Value);
		o.Add("%s %s\n", m.Name, value);
	}
	o.Add("# HELP %s_start_time_seconds Início do processo (Unix).\n", g_prefix);
	o.Add("# TYPE %s_start_time_seconds gauge\n", g_prefix);
	o.Add("%s_start_time_seconds %lld\n", g_prefix, g_start);
	o.Add("# HELP %s_last_write_timestamp_seconds Gravação deste arquivo (Unix).\n", g_prefix);
	o.Add("# TYPE %s_last_write_timestamp_seconds gauge\n", g_prefix);
	o.Add("%s_last_write_timestamp_seconds %lld\n", g_prefix, now);
	return o.ok ? o.len : -1;
}

bool W2MetricsWrite()
{
	g_lastWrite = W2TickCount();
	if (g_path[0] == 0)
		return true;

	static char text[32768];
	int len = W2MetricsRender(text, sizeof(text), (long long)time(nullptr));
	if (len < 0)
		return false;

	FILE* fp = fopen(g_tmp, "wb");
	if (fp == nullptr)
		return false;
	bool ok = fwrite(text, 1, (size_t)len, fp) == (size_t)len;
	ok = fclose(fp) == 0 && ok;
	// rename troca o arquivo de uma vez no POSIX (o coletor lê o antigo ou o novo, inteiro).
	if (!ok || rename(g_tmp, g_path) != 0)
	{
		remove(g_tmp);
		return false;
	}
	return true;
}

bool W2MetricsDue()
{
	return g_table != nullptr && W2TickCount() - g_lastWrite >= g_intervalMs;
}
