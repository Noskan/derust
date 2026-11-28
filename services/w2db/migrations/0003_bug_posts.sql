-- Relatos de bug postados no Discord (docs/OPS.md): a fila é o que ainda não foi postado, e o
-- limite por personagem conta os relatos da última hora.
ALTER TABLE bug_reports ADD COLUMN posted_at timestamptz;
ALTER TABLE bug_reports ADD COLUMN post_attempts integer NOT NULL DEFAULT 0;
CREATE INDEX bug_reports_unposted ON bug_reports (id) WHERE posted_at IS NULL;
CREATE INDEX bug_reports_character ON bug_reports (character_name, created_at);
