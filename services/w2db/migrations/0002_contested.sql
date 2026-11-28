-- Item que estava numa vaga viva de outra conta e passou para quem gravou (a troca em que o outro
-- lado ainda não gravou): a conta que perdeu o item. Se ela gravar de novo com o mesmo serial, é a
-- suspeita de duplicação; se gravar sem ele, a marca sai.
ALTER TABLE items ADD COLUMN contested_from bigint REFERENCES accounts (id) ON DELETE SET NULL;
CREATE INDEX items_contested ON items (contested_from) WHERE contested_from IS NOT NULL;
