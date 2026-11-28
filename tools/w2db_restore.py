#!/usr/bin/env python3
"""Restauração do backup do w2db (pgBackRest) e conferência do banco restaurado (docs/OPS.md).

restore: o roteiro de quem opera, numa instância separada (outra máquina, ou a mesma num
diretório novo). Restaura o último backup, ou um instante (--target), num diretório novo, sobe
um Postgres só no socket, sem arquivar WAL (não manda nada de volta para o repositório), espera
o fim da recuperação, roda `w2db check` e desliga. Com --keep, deixa o diretório e diz como
subir de novo.

selftest: o caminho inteiro numa pasta temporária, com um repositório local do pgBackRest no
lugar do bucket: Postgres com archive_command, contas sintéticas (services/w2db/tests/data)
importadas, backup completo, diferencial, escritas antes e depois de um instante T, e duas
restaurações: até T (o que veio depois de T não pode estar lá) e até o fim. Também confere as
métricas do arquivamento no /metrics do w2db. É o teste do CI.

Uso:
  w2db_restore.py restore --w2db BIN [--config /etc/pgbackrest/pgbackrest.conf] [--target TEMPO]
                          [--into DIR] [--keep] [--db-user postgres] [--pg-bin DIR]
  w2db_restore.py selftest --w2db BIN [--pgbackrest BIN] [--pg-bin DIR] [--keep]
"""

import argparse
import getpass
import os
import re
import shutil
import socket
import subprocess
import sys
import tempfile
import time
import urllib.request
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
STANZA = "w2db"


def find_pg_bin(given):
    """Diretório com initdb, pg_ctl e psql: o dado, o do pg_config ou o maior /usr/lib/postgresql/N/bin."""
    if given:
        return Path(given)
    if shutil.which("initdb"):
        return Path(shutil.which("initdb")).parent
    found = sorted(Path("/usr/lib/postgresql").glob("*/bin/initdb"), key=lambda p: int(p.parent.parent.name))
    if not found:
        sys.exit("não achei o initdb do Postgres (--pg-bin)")
    return found[-1].parent


def run(cmd, **kw):
    r = subprocess.run([str(c) for c in cmd], capture_output=True, text=True, **kw)
    if r.returncode != 0:
        raise RuntimeError(f"{' '.join(str(c) for c in cmd)} saiu com {r.returncode}:\n{r.stdout}{r.stderr}")
    return r.stdout


class Pg:
    """Um Postgres só no socket de `sock`."""

    def __init__(self, pg_bin, data, sock, user, port=5432):
        self.pg_bin, self.data, self.sock, self.user, self.port = pg_bin, Path(data), Path(sock), user, port

    def start(self, extra="", log=None):
        opts = f"-c listen_addresses='' -k {self.sock} -p {self.port} {extra}"
        run([self.pg_bin / "pg_ctl", "-D", self.data, "-l", log or self.data.parent / "pg.log", "-o", opts, "-w",
             "-t", "600", "start"])

    def stop(self):
        subprocess.run([str(self.pg_bin / "pg_ctl"), "-D", str(self.data), "-m", "fast", "-w", "stop"],
                       capture_output=True)

    def url(self, db="w2db"):
        return f"postgres://{self.user}@localhost:{self.port}/{db}?host={self.sock}"

    def sql(self, query, db="w2db"):
        out = run([self.pg_bin / "psql", "-h", self.sock, "-p", self.port, "-U", self.user, "-d", db, "-X", "-q",
                   "-tA", "-F", "\t", "-v", "ON_ERROR_STOP=1", "-c", query])
        return [tuple(line.split("\t")) for line in out.splitlines() if line]


def restore(pgbackrest, config, pg_bin, into, w2db, db_user, target=None, keep=False):
    """Restaura em `into` (vazio), sobe, confere com `w2db check` e desliga. Devolve (ok, Pg, relatório)."""
    into = Path(into)
    into.mkdir(parents=True, exist_ok=True)
    data = into / "data"
    if data.exists() and any(data.iterdir()):
        sys.exit(f"{data} não está vazio")
    cmd = [pgbackrest, f"--config={config}", f"--stanza={STANZA}", f"--pg1-path={data}", "--log-level-console=warn"]
    if target:
        cmd += ["--type=time", f"--target={target}", "--target-action=promote"]
    cmd.append("restore")
    print(f"restaurando {'até ' + target if target else 'o último estado'} em {data}")
    run(cmd)

    # No Debian a configuração fica em /etc, fora do backup: o mínimo para a instância de teste.
    if not (data / "postgresql.conf").exists():
        (data / "postgresql.conf").write_text("# instância restaurada por tools/w2db_restore.py\n")
    (data / "pg_hba.conf").write_text("local all all trust\n")
    os.chmod(data, 0o700)

    sock = Path(tempfile.mkdtemp(prefix="w2rst_"))
    pg = Pg(pg_bin, data, sock, db_user, port=5432)
    # archive_mode=off: a instância restaurada não manda WAL para o repositório de produção.
    pg.start("-c archive_mode=off -c hba_file=" + str(data / "pg_hba.conf"), log=into / "pg.log")
    try:
        end = time.time() + 600
        while pg.sql("SELECT pg_is_in_recovery()", db="postgres")[0][0] == "t":
            if time.time() > end:
                raise RuntimeError("a recuperação não terminou em 10 min")
            time.sleep(0.5)
        r = subprocess.run([str(w2db), "check"], capture_output=True, text=True,
                           env=dict(os.environ, DATABASE_URL=pg.url(), NO_COLOR="1", RUST_LOG="warn"))
        report = r.stdout + r.stderr
        print(report.rstrip())
        ok = r.returncode == 0
        return ok, pg, report
    finally:
        if not keep:
            pg.stop()
            shutil.rmtree(into, ignore_errors=True)
            shutil.rmtree(sock, ignore_errors=True)
        else:
            print(f"instância restaurada no ar em {sock} (porta 5432); pare com: "
                  f"{pg_bin / 'pg_ctl'} -D {data} stop")


def free_port():
    with socket.socket() as s:
        s.bind(("127.0.0.1", 0))
        return s.getsockname()[1]


def selftest(args):
    pg_bin = find_pg_bin(args.pg_bin)
    pgbackrest = args.pgbackrest or shutil.which("pgbackrest") or sys.exit("não achei o pgbackrest (--pgbackrest)")
    w2db = Path(args.w2db).resolve()
    work = Path(tempfile.mkdtemp(prefix="w2bk_"))
    user = getpass.getuser()
    src_data = work / "src"
    sock = work / "s"
    sock.mkdir()
    repo = work / "repo"
    config = work / "pgbackrest.conf"
    # O mesmo pgbackrest.conf de deploy/pgbackrest, com o repositório numa pasta no lugar do bucket.
    config.write_text(f"""[global]
repo1-type=posix
repo1-path={repo}
repo1-cipher-type=aes-256-cbc
repo1-cipher-pass=senha-so-do-teste
repo1-retention-full-type=count
repo1-retention-full=4
compress-type=zst
start-fast=y
log-path={work}/log
lock-path={work}/lock
spool-path={work}/spool
log-level-console=warn

[{STANZA}]
pg1-path={src_data}
pg1-socket-path={sock}
pg1-port=5432
pg1-user={user}
""")
    run([pg_bin / "initdb", "-D", src_data, "-U", user, "--auth=trust", "-E", "UTF8", "--locale=C", "--no-sync"])
    archive = f"{pgbackrest} --config={config} --stanza={STANZA} archive-push %p"
    # Os mesmos parâmetros de deploy/postgres/derust.conf.
    (src_data / "postgresql.conf").open("a").write(
        "\nwal_level = replica\narchive_mode = on\n"
        f"archive_command = '{archive}'\narchive_timeout = 60\nmax_wal_senders = 3\n")
    src = Pg(pg_bin, src_data, sock, user)
    src.start(log=work / "src.log")
    metrics_port = free_port()
    serve = None
    failures = []
    try:
        src.sql("CREATE DATABASE w2db", db="postgres")
        env = dict(os.environ, DATABASE_URL=src.url(), NO_COLOR="1", RUST_LOG="warn")
        run([w2db, "migrate"], env=env)
        print(run([w2db, "import", ROOT / "services/w2db/tests/data"], env=env).strip())

        bk = [pgbackrest, f"--config={config}", f"--stanza={STANZA}"]
        run(bk + ["stanza-create"])
        run(bk + ["check"])
        run(bk + ["--type=full", "backup"])
        print("backup completo feito")
        run([w2db, "invite", "depois-do-completo"], env=env)
        run(bk + ["--type=diff", "backup"])
        print("backup diferencial feito")
        run([w2db, "invite", "antes-do-alvo"], env=env)
        src.sql("SELECT pg_switch_wal()")
        time.sleep(1.1)
        target = src.sql("SELECT now()::text")[0][0]
        time.sleep(1.1)
        run([w2db, "invite", "depois-do-alvo"], env=env)
        src.sql("SELECT pg_switch_wal()")
        # O WAL de depois do alvo tem de estar no repositório (a recuperação para no primeiro
        # commit depois dele).
        run(bk + ["check"])

        # As métricas do arquivamento pelo /metrics do w2db.
        serve = subprocess.Popen([str(w2db), "serve", "--socket", str(sock / "w2db.sock"), "--metrics-addr",
                                  f"127.0.0.1:{metrics_port}"], env=env, stdout=subprocess.DEVNULL,
                                 stderr=subprocess.DEVNULL)
        text = ""
        for _ in range(100):
            try:
                text = urllib.request.urlopen(f"http://127.0.0.1:{metrics_port}/metrics", timeout=2).read().decode()
                break
            except OSError:
                time.sleep(0.1)
        m = re.search(r"^w2db_wal_archived_total (\d+)$", text, re.M)
        f = re.search(r"^w2db_wal_archive_failures_total (\d+)$", text, re.M)
        if not m or int(m.group(1)) == 0 or not f or int(f.group(1)) != 0:
            failures.append(f"métricas do arquivamento: {m and m.group(0)} {f and f.group(0)}")
        else:
            print(f"/metrics: {m.group(0)}, {f.group(0)}")

        def invites(pg):
            return {r[0] for r in pg.sql("SELECT discord_id FROM invites")}

        for label, tgt, want, unwanted in (
            ("até o alvo", target, {"depois-do-completo", "antes-do-alvo"}, {"depois-do-alvo"}),
            ("até o fim", None, {"depois-do-completo", "antes-do-alvo", "depois-do-alvo"}, set()),
        ):
            ok, pg, report = restore(pgbackrest, config, pg_bin, work / "restore", w2db, user, target=tgt, keep=True)
            try:
                got = invites(pg)
                accounts = int(pg.sql("SELECT count(*) FROM accounts")[0][0])
                if not ok:
                    failures.append(f"restauração {label}: w2db check falhou")
                if not want <= got or got & unwanted:
                    failures.append(f"restauração {label}: convites {sorted(got)}, esperado {sorted(want)}")
                if accounts != 2:
                    failures.append(f"restauração {label}: {accounts} conta(s), esperado 2")
                if ok and want <= got and not got & unwanted and accounts == 2:
                    print(f"restauração {label}: ok ({accounts} contas, convites {sorted(got)})")
            finally:
                pg.stop()
                shutil.rmtree(work / "restore", ignore_errors=True)
                shutil.rmtree(pg.sock, ignore_errors=True)
    finally:
        if serve is not None:
            serve.terminate()
            serve.wait(10)
        src.stop()
        if not args.keep:
            shutil.rmtree(work, ignore_errors=True)
        else:
            print(f"pasta do teste: {work}")

    for f in failures:
        print(f"FALHOU: {f}")
    print("resultado: " + ("OK" if not failures else f"{len(failures)} falha(s)"))
    return 0 if not failures else 1


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)
    r = sub.add_parser("restore", help="restaura numa instância nova e confere")
    r.add_argument("--w2db", required=True, help="binário do w2db (para o w2db check)")
    r.add_argument("--config", default="/etc/pgbackrest/pgbackrest.conf")
    r.add_argument("--pgbackrest", help="binário do pgbackrest (padrão: o do PATH)")
    r.add_argument("--target", help="instante (ex.: '2026-12-01 21:30:00-03'); sem ele, o último estado")
    r.add_argument("--into", help="diretório novo da instância (padrão: temporário)")
    r.add_argument("--keep", action="store_true", help="deixa a instância restaurada no ar")
    r.add_argument("--db-user", default="postgres", help="superusuário do cluster restaurado")
    r.add_argument("--pg-bin", help="diretório do initdb/pg_ctl/psql")
    s = sub.add_parser("selftest", help="backup e restauração de ponta a ponta numa pasta temporária")
    s.add_argument("--w2db", required=True)
    s.add_argument("--pgbackrest")
    s.add_argument("--pg-bin")
    s.add_argument("--keep", action="store_true", help="deixa a pasta do teste")
    args = ap.parse_args()

    if args.cmd == "selftest":
        return selftest(args)
    pgbackrest = args.pgbackrest or shutil.which("pgbackrest") or sys.exit("não achei o pgbackrest (--pgbackrest)")
    into = args.into or tempfile.mkdtemp(prefix="w2db_restore_")
    ok, _, _ = restore(pgbackrest, args.config, find_pg_bin(args.pg_bin), into, Path(args.w2db).resolve(),
                       args.db_user, target=args.target, keep=args.keep)
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
