Compilador PasKenzie - Fase 2:
Análise Semântica e Geração de Código Intermediário

Grupo:
Kaue Henrique Matias Alves | RA: 10417894
Victor Maki Tarcha | RA: 10419861

Visão Geral do Projeto

Este trabalho implementa a segunda etapa do compilador para a linguagem PasKenzie. Baseado na estrutura léxica e sintática validada na Fase 1, adicionamos agora a Análise Semântica (verificação de declaração e unicidade de variáveis) e a Geração de Código para a máquina hipotética MEPA.

O compilador agora não apenas valida a gramática, mas também traduz o código-fonte para instruções de pilha e gerencia uma Tabela de Símbolos utilizando Hashing.

Compilação e Execução

Para a compilação desta fase, é necessário linkar o arquivo objeto fornecido (hashMack.o ou hashMack.obj) que contém a implementação da função de hash.

Comando de Compilação (Linux/WSL):
gcc -Wall -Wno-unused-result -g -Og compilador.c hashMack.o -o compilador

Comando de Compilação (Windows/MinGW):
gcc -Wall -Wno-unused-result -g -Og compilador.c hashMack.obj -o compilador

Como Executar:
./compilador <arquivo_fonte.txt>

Decisões de Design e Implementação

1. Estrutura Incremental (Tradução Dirigida pela Sintaxe): Mantivemos a arquitetura do Analisador Descendente Recursivo da Fase 1. As verificações semânticas e a emissão de código MEPA foram inseridas diretamente nas funções sintáticas, executando de forma entrelaçada à análise gramatical.

2. Tabela de Símbolos (Hash Table): Implementamos uma tabela de dispersão com tratamento de colisões por encadeamento. O endereçamento das variáveis é sequencial (0, 1, 2...), controlado pela variável global 'proximo_endereco'.

3. Tratamento de Erros: Implementamos uma estratégia de "Panic Mode" simplificada. Ao encontrar o primeiro erro — seja ele Léxico (símbolo inválido), Sintático (token inesperado) ou Semântico (variável não declarada ou duplicada) — o compilador exibe uma mensagem detalhada contendo a linha e o tipo do erro, e encerra imediatamente a execução (exit 1), prevenindo a geração de código inválido.

4. Geração de Rótulos: Para comandos de controle de fluxo, criamos a função auxiliar proximo_rotulo(), garantindo unicidade nos desvios condicionais.

Detalhes Técnicos e Justificativas

1. Remoção do Rastreio Léxico: Diferente da Fase 1, removemos a impressão dos tokens ("# linha: token") dentro da função consome(), mantendo a saída limpa conforme a exigência de imprimir apenas as instruções MEPA e a Tabela de Símbolos final.

2. Tratamento de Variáveis:
   - Declaração: Ao encontrar "var", contamos quantas variáveis são declaradas para emitir uma única instrução "AMEM n".
   - Uso: Em comandos como "read" ou atribuições, buscamos o endereço na tabela. Se a variável não existe, o compilador aborta com erro semântico imediato.

3. Tipagem: Como o enunciado simplifica o escopo para tratar apenas inteiros na geração de código, tratamos internamente tipos 'char' como inteiros na emissão das instruções 'CRCT'.

4. Identificadores Longos (Truncamento): Caso um identificador exceda o limite de 15 caracteres, optamos por truncar o nome para os primeiros 15 caracteres válidos em vez de emitir um erro léxico fatal. Isso garante robustez na análise, permitindo que o compilador processe a parte significativa do nome.

5. Abrangência das Estruturas (Factor e If): Notamos que os exemplos do enunciado cobriam apenas um subconjunto da gramática (inteiros e IDs). Para garantir que o compilador continuasse aceitando toda a sintaxe definida na Fase 1 (incluindo 'char', 'true'/'false' e 'not'), expandimos a lógica dessas funções para cobrir todas as possibilidades da gramática EBNF, tratando internamente esses tipos como valores inteiros na MEPA.

Desafios Encontrados e Soluções

1. Linkagem Externa: Tivemos problemas iniciais para compilar devido à dependência do arquivo objeto externo (hashMack). Resolvemos declarando explicitamente o protótipo "extern int hashMack(char * s);" e ajustando a linha de comando do GCC.

2. Ordem das Instruções no IF/ELSE: Foi necessário cuidado redobrado na lógica de geração dos rótulos (Labels) no comando IF, garantindo que o rótulo de saída fosse impresso corretamente tanto no fluxo do THEN quanto no do ELSE para manter a coerência da MEPA.