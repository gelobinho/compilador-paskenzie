1. Objetivo da Fase 2

Nesta fase incluimos:

- Analisador Semântico completo, com:

- Tabela de símbolos (hash)

- Detecção de variáveis não declaradas

- Detecção de variáveis duplicadas

- Endereçamento de variáveis (endereços inteiros)

- Geração de Código MEPA, incluindo:

- INPP, PARA

- AMEM, ARMZ, CRVL, CRCT

- Operações aritméticas e relacionais

- LEIT, IMPR

- DESVIO condicional: DSVF

- DESVIO incondicional: DSVS

- Controle de rótulos (L1, L2…)

- Ajustes no parser para acoplamento com o código MEPA

2. O que foi implementado nesta fase
2.1 Tabela de Símbolos com Hash

Estrutura hash baseada no código fornecido pelo professor (hashMack()).

Cada variável declarada é inserida com:

Nome

Endereço (inteiro crescente)

Detecção de variáveis duplicadas → erro semântico.

Detecção de variáveis não declaradas no uso → erro semântico.

2.2 Controle de Endereços

Cada nova variável recebe um endereço sequencial:

x → 0
y → 1
z → 2
...


Após terminar as declarações, gera-se:

AMEM <quantidade_de_variaveis>

2.3 Geração de Código MEPA para expressões

Inclui:

CRCT n

CRVL endereco

Operadores:

SOMA → SOMA
SUBT → SUBT
MULT → MULT
DIV  → DIVI / DIV


Operadores relacionais:

=   → CMIG
<>  → CMDG
<   → CMME
>   → CMMA
<=  → CMEG
>=  → CMAG
OR  → DISJ
AND → CONJ

2.4 Geração de Código para comandos

Atribuição

CRVL x
CRCT 1
SOMA
ARMZ x


Read

LEIT
ARMZ endereco


Write

CRVL endereco
IMPR


If-Then-Else

<expr>
DSVF L1
<comando>
DSVS L2


L1: NADA
<else>
L2: NADA

- While


L1: NADA
<expr>
DSVF L2
<statement>
DSVS L1
L2: NADA


2.5 Controle de Rótulos
- Rotulador incremental (L1, L2, L3…)
- Utilizado em if/else/while

---

3. O que foi necessário adaptar da Fase 1 para a Fase 2

Mudança 1 — O parser agora não imprime tokens
Na fase 1, cada `consome()` imprimia:

5:identifier : x
8:integer
Na fase 2 o `consome()` foi reescrito para:
- não imprimir tokens
- coordenar a leitura de átomos
- permitir encaixar geração de código imediatamente após reconhecer estruturas.

---

Mudança 2 — Inclusão da Tabela de Símbolos
O parser foi modificado para:

- Inserir variáveis na declaração:


insere_simbolo(...)

- Verificar variáveis no uso:


busca_simbolo(...)


Ou seja, o analisador sintático agora conversa com o analisador semântico.

---

Mudança 3 — Código MEPA acoplado ao parser
Funções como:

- `assignment_statement()`
- `expression()`
- `if_statement()`
- `while_statement()`

foram reescritas para:

- analisar sintaticamente  
- verificar semântica  
- produzir MEPA na mesma passagem  

---

Mudança 4 — Correções na gramática
A fase 2 exige que:

- `write` aceite identificadores, não expressões
- `variable_declaration_part()` feche corretamente com `;`
- `expression()` gere MEPA

---

Mudança 5 — Inicialização do compilador





Como Compilar:

gcc -Wall -Wno-unused-result -g -Og main.c hashMack.o -o compilador -lm
./compilador 
