#!/usr/bin/env python3
"""Confere a configuração de operação em deploy/ (docs/OPS.md).

1. Toda métrica do derust que as regras de alerta e o painel do Grafana usam existe: as do
   TMSrv e do DBSrv (as tabelas W2Metric de server/*/core/ServerMetrics.cpp), as do w2db
   (services/w2db/src/metrics.rs) e a do backup (deploy/bin/w2-backup-metric). Um nome errado
   num alerta é um alerta que nunca dispara.
2. Com o promtool: a configuração do Prometheus, as regras e os testes delas
   (deploy/prometheus/rules/derust_test.yml).
3. Com o amtool: a configuração do Alertmanager.
4. O painel do Grafana é JSON válido, com ids únicos e a fonte de dados provisionada.
5. Com o caddy: o Caddyfile do HTTPS do painel de GM (deploy/caddy, docs/GM.md).

Uso: ops_check.py [--promtool BIN] [--amtool BIN] [--caddy BIN] [--require-tools]
"""

import argparse
import json
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
DEPLOY = ROOT / "deploy"

# Nomes que as regras e o painel podem usar sem estarem no código do derust.
EXTERNAL = re.compile(r"^(node_|up$|ALERTS)")
OURS = re.compile(r"\b(w2db_[a-z0-9_]+|w2_[a-z0-9_]+)\b")


def defined_metrics():
    names = set()
    for src in ("server/tmsrv/core/ServerMetrics.cpp", "server/dbsrv/core/ServerMetrics.cpp"):
        text = (ROOT / src).read_text()
        names |= set(re.findall(r'\{\s*"(w2_[a-z0-9_]+)', text))
        prefix = re.search(r'W2MetricsInit\("(w2_[a-z0-9_]+)"', text).group(1)
        # Common/Metrics.cpp acrescenta estas a cada arquivo.
        names |= {f"{prefix}_start_time_seconds", f"{prefix}_last_write_timestamp_seconds"}
    rs = (ROOT / "services/w2db/src/metrics.rs").read_text()
    names |= set(re.findall(r'"(w2db_[a-z0-9_]+)', rs))
    # family(&mut o, "w2db_rpc_duration_seconds", "summary", ...): as séries _sum e _count.
    for base in re.findall(r'"(w2db_[a-z0-9_]+)", "summary"', rs):
        names |= {f"{base}_sum", f"{base}_count"}
    names |= set(re.findall(r"^(w2_[a-z0-9_]+)\{", (DEPLOY / "bin/w2-backup-metric").read_text(), re.M))
    return names


def used_metrics():
    uses = []
    rules = DEPLOY / "prometheus/rules/derust.yml"
    # Só as expressões: a linha do `expr:` e, num bloco `expr: >-`, as mais recuadas que ela.
    block = None
    for i, line in enumerate(rules.read_text().splitlines(), 1):
        indent = len(line) - len(line.lstrip())
        if block is not None and line.strip() and indent <= block:
            block = None
        m = re.match(r"^(\s*)expr:\s*(.*)$", line)
        if m:
            if m.group(2).startswith((">", "|")):
                block = len(m.group(1))
            text = m.group(2)
        elif block is not None:
            text = line
        else:
            continue
        for n in OURS.findall(text):
            uses.append((f"{rules.relative_to(ROOT)}:{i}", n))
    dash = json.loads((DEPLOY / "grafana/dashboards/derust.json").read_text())
    for p in dash["panels"]:
        for t in p.get("targets", []):
            for n in OURS.findall(t["expr"]):
                uses.append((f"painel '{p['title']}'", n))
    return uses


def check_dashboard():
    errors = []
    dash = json.loads((DEPLOY / "grafana/dashboards/derust.json").read_text())
    ids = [p["id"] for p in dash["panels"]]
    if len(ids) != len(set(ids)):
        errors.append("painel: ids repetidos")
    ds = re.search(r"uid:\s*(\S+)", (DEPLOY / "grafana/provisioning/datasources/derust.yml").read_text()).group(1)
    for p in dash["panels"]:
        if p["datasource"]["uid"] != ds or any(t["datasource"]["uid"] != ds for t in p["targets"]):
            errors.append(f"painel '{p['title']}': fonte de dados diferente de {ds}")
    return errors


def tool(name, given, required):
    path = given or shutil.which(name)
    if not path and required:
        sys.exit(f"não achei o {name}")
    return path


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--promtool")
    ap.add_argument("--amtool")
    ap.add_argument("--caddy")
    ap.add_argument("--require-tools", action="store_true",
                    help="falha se o promtool, o amtool ou o caddy não existirem")
    args = ap.parse_args()

    errors = []
    defined = defined_metrics()
    for where, name in used_metrics():
        if not EXTERNAL.match(name) and name not in defined:
            errors.append(f"{where}: a métrica {name} não existe no código")
    errors += check_dashboard()
    print(f"métricas do derust: {len(defined)}; usadas nas regras e no painel conferidas")

    promtool = tool("promtool", args.promtool, args.require_tools)
    if promtool:
        with tempfile.TemporaryDirectory() as tmp:
            # As regras ficam em /etc/prometheus/rules na máquina; aqui, numa cópia.
            cfg = (DEPLOY / "prometheus/prometheus.yml").read_text().replace(
                "/etc/prometheus/rules/derust.yml", str(DEPLOY / "prometheus/rules/derust.yml"))
            (Path(tmp) / "prometheus.yml").write_text(cfg)
            for cmd in ([promtool, "check", "config", str(Path(tmp) / "prometheus.yml")],
                        [promtool, "test", "rules", str(DEPLOY / "prometheus/rules/derust_test.yml")]):
                r = subprocess.run(cmd, capture_output=True, text=True)
                if r.returncode != 0:
                    errors.append(f"{' '.join(cmd[1:3])}: {r.stdout}{r.stderr}")
                else:
                    print(f"promtool {' '.join(cmd[1:3])}: ok")
    else:
        print("promtool não encontrado: configuração e regras do Prometheus não conferidas")

    amtool = tool("amtool", args.amtool, args.require_tools)
    if amtool:
        r = subprocess.run([amtool, "check-config", str(DEPLOY / "alertmanager/alertmanager.yml")],
                           capture_output=True, text=True)
        if r.returncode != 0:
            errors.append(f"amtool check-config: {r.stdout}{r.stderr}")
        else:
            print("amtool check-config: ok")
    else:
        print("amtool não encontrado: configuração do Alertmanager não conferida")

    caddy = tool("caddy", args.caddy, args.require_tools)
    if caddy:
        with tempfile.TemporaryDirectory() as tmp:
            r = subprocess.run([caddy, "validate", "--adapter", "caddyfile", "--config",
                                str(DEPLOY / "caddy/Caddyfile")], capture_output=True, text=True,
                               env={"HOME": tmp, "XDG_DATA_HOME": tmp, "XDG_CONFIG_HOME": tmp, "PATH": "/usr/bin:/bin"})
            if r.returncode != 0 or "Valid configuration" not in r.stdout + r.stderr:
                errors.append(f"caddy validate: {r.stdout}{r.stderr}")
            else:
                print("caddy validate: ok")
    else:
        print("caddy não encontrado: o Caddyfile não foi conferido")

    for e in errors:
        print(f"ERRO: {e}")
    print("resultado: " + ("OK" if not errors else f"{len(errors)} erro(s)"))
    return 0 if not errors else 1


if __name__ == "__main__":
    sys.exit(main())
