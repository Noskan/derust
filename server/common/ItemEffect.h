/*
*   Copyright (C) {2015}  {VK, Charles TheHouse}
*
*   This program is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program.  If not, see [http://www.gnu.org/licenses/].
*
*   Contact at:
*/

/// @file ItemEffect.h
/// @brief Constantes de efeitos (EF_*) usadas em ItemList.csv e nos modificadores stEffect[3] dos itens no servidor.

//#define EF_NONE     			  0	// Efeito 0 indica ausência de efeito; não soma valor em Score[0].
// Atributos de Score (1..14)
#define EF_LEVEL     			  1	// Nível base do item/mob (soma ao Score[1..15]).
#define EF_DAMAGE                 2 // Dano físico adicional
#define EF_AC       			  3	// Defesa física (AC)
#define EF_HP        			  4	// Bônus de HP máximo
#define EF_MP       			  5	// Bônus de MP máximo
#define EF_EXP      			  6	// Experiência concedida / bônus de EXP
#define EF_STR      		      7	// Força (STR)
#define EF_INT      		      8	// Inteligência (INT)
#define EF_DEX       		      9	// Destreza (DEX)
#define EF_CON      		     10	// Constituição (CON)
#define EF_SPECIAL1    		     11	// Linhagem/Maestria 1
#define EF_SPECIAL2    			 12	// Linhagem/Maestria 2
#define EF_SPECIAL3    			 13	// Linhagem/Maestria 3
#define EF_SPECIAL4    			 14	// Linhagem/Maestria 4
// Fim do bloco Score
#define EF_SCORE14     			 15	// Slot reservado de Score 14
#define EF_SCORE15     			 16	// Slot reservado de Score 15

////////////////      REQUISITOS         //////////////////////////////////////////
#define EF_POS                   17  // Bitmask de slots onde o item pode ser equipado (padrão 0 = não equipável)
#define EF_CLASS                 18  // Bitmask de classes permitidas (padrão = todas)
#define EF_R1SIDC                19  // Requisito combinado STR/INT/DEX/CON para armas de 1 mão ou armaduras
#define EF_R2SIDC                20  // Requisito combinado STR/INT/DEX/CON para armas de 2 mãos

////////////////   BÔNUS E TIPOS DE ARMA  ////////////////////////////////
#define EF_WTYPE         	     21	 // Categoria/animação da arma:
//   1: Espada curta (1 mão)   |   2: Espada média            |   3: Espada grande (2 mãos)
//  11: Machado pequeno        |  12: Machado médio           |  13: Machado grande
//  21: Lança pequena          |  22: Lança média             |  23: Lança grande
//  31: Cajado pequeno         |  32: Cajado médio            |  33: Cajado grande
//  41: Garra (Foema/BM)       |  51: Escudo
//  61: Clava pequena          |  62: Clava média             |  63: Clava grande
// 101: Arco                   | 102: Dardo / Besta           | 103: Tomahawk / Chakram   | 104: Azagaia
////////////////////////////////////////////////////////////////////////////
#define EF_REQ_STR               22  // Requisito mínimo de Força (adicional ao base SIDC)
#define EF_REQ_INT               23  // Requisito mínimo de Inteligência
#define EF_REQ_DEX               24  // Requisito mínimo de Destreza
#define EF_REQ_CON               25  // Requisito mínimo de Constituição

#define EF_ATTSPEED  	       	 26	 // Velocidade de ataque
#define EF_RANGE                 27  // Alcance de ataque (aplica-se o maior entre os equipados)
////#define EF_PRICE				 28	 // Preço (desativado no servidor)
#define EF_RUNSPEED              29  // Velocidade de movimento
#define EF_SPELL    	    	 30	 // Magia/buff ativado ao usar ou equipar o item
#define EF_DURATION              31  // Duração (consumíveis) ou parâmetro 1 do efeito
#define EF_PARM2     			 32	 // Parâmetro 2 do efeito do item
#define EF_GRID                  33  // Formato/ocupação na grade do inventário (índice em g_pItemGrid)
#define EF_GROUND                34  // Tamanho ocupado no mapa (portões/objetos)
#define EF_CLAN                  35  // Clã / facção associada ao item
#define EF_HWORDCOIN             36  // Palavra alta do valor em ouro (itens de moeda)
#define EF_LWORDCOIN             37  // Palavra baixa do valor em ouro
#define EF_VOLATILE              38  // Tipo de consumo: 0=equipamento, 1=poção, 2=ouro, 3=chave, 4..6=bênçãos, 7=nível+1
#define EF_KEYID                 39  // Identificador da chave/porta vinculada
#define EF_PARRY                 40  // Bônus de esquiva / parry
#define EF_HITRATE               41  // Bônus de taxa de acerto
#define EF_CRITICAL              42  // Chance de acerto crítico
#define EF_SANC                  43  // Grau de refinação (+0..+15, codificado com tintura acima de +9)
#define EF_SAVEMANA              44  // Redução percentual de custo de mana (1 -> 99%)
#define EF_HPADD                 45  // Bônus percentual de HP máximo (1 -> 101%)
#define EF_MPADD                 46  // Bônus percentual de MP máximo (1 -> 101%)
#define EF_REGENHP               47  // Regeneração adicional de HP
#define EF_REGENMP               48  // Regeneração adicional de MP
#define EF_RESIST1               49  // Resistência a Fogo
#define EF_RESIST2               50  // Resistência a Gelo
#define EF_RESIST3               51  // Resistência a Sagrado
#define EF_RESIST4               52  // Resistência a Trovão
#define EF_ACADD                 53  // Bônus adicional de defesa (AC)
#define EF_RESISTALL             54  // Bônus em todas as resistências elementais
#define EF_BONUS                 55  // Bônus genérico
#define EF_HWORDGUILD            56  // Palavra alta de Guild ID / Ataque PvP em itens de guerra
#define EF_LWORDGUILD            57  // Palavra baixa de Guild ID / Defesa PvP em itens de guerra
#define EF_QUEST                 58  // Bitmask/ID de quest vinculada (em geral usado com EF_VOLATILE)
#define EF_UNIQUE                59  // Identificador de classe única do item
#define EF_MAGIC                 60  // Amplificação de dano mágico (1 -> +1%)
#define EF_AMOUNT                61  // Quantidade empilhada (máximo 120 por slot)
#define EF_HWORDINDEX            62  // Palavra alta de índice auxiliar
#define EF_LWORDINDEX            63  // Palavra baixa de índice auxiliar
#define EF_INIT1                 64  // Efeito inicial 1 (gerado ao dropar/criar item)
#define EF_INIT2                 65  // Efeito inicial 2
#define EF_INIT3                 66  // Efeito inicial 3
#define EF_DAMAGEADD             67  // Adicional direto de dano físico (usado em bônus base/único)
#define EF_MAGICADD              68  // Adicional direto de ataque mágico
// Os efeitos abaixo (69..72) têm prioridade na exibição de itens únicos sobre os equivalentes base:
#define EF_HPADD2                69  // Bônus de HP exibido no bloco base do item único
#define EF_MPADD2                70  // Bônus de MP exibido no bloco base do item único
#define EF_CRITICAL2             71  // Crítico adicional (opção única)
#define EF_ACADD2                72  // Defesa adicional (opção única)
#define EF_DAMAGE2               73  // Dano adicional secundário
#define EF_SPECIALALL            74  // Bônus em todas as maestrias (exceto a primeira)

#define	EF_CURKILL				 75  // Não utilizado
#define EF_LTOTKILL				 76  // Não utilizado
#define EF_HTOTKILL				 77  // Não utilizado
#define EF_INCUBATE				 78  // Limiar de incubação do ovo de montaria

// Atributos específicos de montarias e crias (slots 79..83)
#define EF_MOUNTLIFE			 79  // Vitalidade restante da montaria
#define EF_MOUNTHP				 80  // HP atual da montaria
#define EF_MOUNTSANC			 81  // Nível/crescimento da montaria
#define EF_MOUNTFEED			 82  // Nível de saciedade da montaria
#define EF_MOUNTKILL			 83  // Contador auxiliar / abates da montaria

#define EF_INCUDELAY             84  // Tempo restante de incubação
#define EF_SUBGUILD				 85	// Subguilda (0 = guilda principal; 1..3 = subguildas)

#define EF_ITEMLEVEL			 87  // Grau/nível do item para regras de refinação e bloqueio
#define EF_DONATE				 91  // Marcador de item de doação / loja especial

#define EF_GRADE0				 100 // Grau A
#define EF_GRADE1				 101 // Grau B
#define EF_GRADE2				 102 // Grau C
#define EF_GRADE3				 103 // Grau D
#define EF_GRADE4				 104 // Grau E
#define EF_GRADE5				 105 // Grau E Arch

#define EF_WDAY					 106 // Dia da semana (validade/expiração)
#define EF_HOUR					 107 // Hora (validade/expiração)
#define EF_MIN					 108 // Minuto (validade/expiração)
#define EF_YEAR					 109 // Ano (validade/expiração)
#define EF_WMONTH				 110 // Mês (validade/expiração)

#define EF_MOBTYPE				 112 // Restrição de classe de evolução (Mortal/Arch/Celestial)
#define EF_ITEMTYPE				 113 // Categoria auxiliar de tipo de item

#define EF_NOSANC				 126 // Impede refinação do item
#define EF_NOTRADE				 127 // Item inegociável / preso ao personagem
