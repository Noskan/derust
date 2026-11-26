#ifndef  _ITEMEFFECT_H_
#define  _ITEMEFFECT_H_			  1

/// @file ItemEffect.h
/// @brief Constantes de efeitos (EF_*) usadas pelo cliente TMProject para leitura de ItemList e exibição de tooltips.

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
#define EF_CITIZEN				 28	 // Cidadania / reino exigido
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
#define EF_HWORDGUILD            56  // Guilda (poder de ataque PvP)
#define EF_LWORDGUILD            57  // Guilda (poder de defesa PvP)
#define EF_QUEST                 58  // Bitmask/ID de quest vinculada (em geral usado com EF_VOLATILE)
#define EF_UNIQUE                59  // Identificador de classe única do item
#define EF_MAGIC                 60  // Amplificação de dano mágico (1 -> +1%)
#define EF_AMOUNT                61  // Quantidade empilhada
#define EF_HWORDINDEX            62  // Palavra alta de índice auxiliar
#define EF_LWORDINDEX            63  // Palavra baixa de índice auxiliar
#define EF_INIT1                 64  // Efeito inicial 1
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
#define EF_PREVBONUS			 86	// Bônus de nível do personagem anterior

#define EF_REFLEVEL              87 // Nível de referência para chance de refinação
#define EF_GAMEROOM	             88 // Restrição de sala / lan house

#define EF_ABSDAM                89 // Dano absoluto (ignora defesa)
#define EF_ABSAC                 90 // Defesa absoluta

#define EF_GRADE0				 100 // Grau 0
#define EF_GRADE1				 101 // Grau 1
#define EF_GRADE2				 102 // Grau 2
#define EF_GRADE3				 103 // Grau 3
#define EF_GRADE4				 104 // Grau 4
#define EF_GRADE5				 105 // Grau 5

#define EF_DATE                  106 // Dia (validade/expiração)
#define EF_HOUR                  107 // Hora (validade/expiração)
#define EF_MIN                   108 // Minuto (validade/expiração)
#define EF_YEAR                  109 // Ano (validade/expiração)
#define EF_MONTH                 110 // Mês (validade/expiração)
#define EF_NOTRADE               111 // Item inegociável / preso ao personagem
#define EF_TRANS				 112 // Transformação/evolução permitida (0:Todos, 1:Trans, 2:Normal, 3:Evoluído)

#define EF_FAME					 113 // Fama exigida ou concedida

#define EF_STARTCOL              115 // Início da faixa de efeitos de tintura/cor
#define EF_COLOR0                116 // Cor 0
#define EF_COLOR1                117 // Cor 1
#define EF_COLOR2                118 // Cor 2
#define EF_COLOR3                119 // Cor 3
#define EF_COLOR4                120 // Cor 4
#define EF_COLOR5                121 // Cor 5
#define EF_COLOR6                122 // Cor 6
#define EF_COLOR7                123 // Cor 7
#define EF_COLOR8                124 // Cor 8
#define EF_COLOR9                125 // Cor 9
#define EF_MAXCOL                126 // Fim da faixa de cores

#define EF_STARTCOS				 130	// Início da faixa de trajes/costumes
#define EF_T_COS				 131	// Código de traje não utilizado
#define EF_F_COS				 140	// Código de traje não utilizado
#define EF_B_COS				 150	// Código de traje não utilizado
#define EF_H_COS				 160	// Código de traje não utilizado
#define EF_MAXCOS				 170	// Fim da faixa de trajes


#endif  _ITEMEFFECT_H_
