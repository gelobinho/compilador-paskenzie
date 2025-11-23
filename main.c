#include <math.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h> // atof, atoi, malloc

// ============================================================================
// DEFINIÇÕES E ESTRUTURAS (LÉXICO + SEMÂNTICO)
// ============================================================================

// Tabela Hash (Fase 2)
#define PRIME_NUMER 211 

typedef enum{
    ERRO,
    IDENTIFICADOR,
    COMENTARIO,
    DIV, OR, AND, NOT, IF, THEN, ELSE, WHILE, DO, BEGIN, END, READ, WRITE, VAR, PROGRAM, TRUE_TOKEN, FALSE_TOKEN, CHAR, INTEGER, BOOLEAN,
    PONTO_VIRGULA, VIRGULA, DOIS_PONTOS, ASTERISCO, ABRE_PAR, FECHA_PAR, MAIS, MENOS, BARRA, PONTO, IGUAL,
    ATRIBUICAO, // :=
    MENOR, MAIOR, MENOR_IGUAL, MAIOR_IGUAL, NEGACAO,
    CONSTCHAR, CONSTINT,
    EOS
} TAtomo;

// Estrutura para retorno da funcao obter_atomo()
typedef struct{ 
    TAtomo atomo; 
    int linha; 
    union{ 
        int numero; // atributo do átomo constint
        char id[16];  // atributo identifier
        char ch;      // atributo do átomo constchar
    } atributo; 
} TInfoAtomo; 

// Estrutura da Tabela de Símbolos (Fase 2)
typedef struct _TNo {
    char ID[16];
    int endereco;
    struct _TNo *prox;
} TNo;

typedef struct {
    TNo *entradas[PRIME_NUMER];
} TTabelaSimbolos;

// Variáveis Globais
char *buffer;
char lexema[20];
int nLinha;
TInfoAtomo lookahead; 

// Globais para Semântico e Geração de Código
TTabelaSimbolos tabelaSimbolos;
int endereco_global = 0; // Conta endereços (0, 1, 2...)
int cont_rotulo = 1;     // Conta rótulos (L1, L2...)

// ============================================================================
// DECLARAÇÃO DAS FUNÇÕES
// ============================================================================

// Léxico
const char* simbolo_real(TAtomo a);
TInfoAtomo obter_atomo();
void reconhece_numero(TInfoAtomo *infoAtomo);
void reconhece_id(TInfoAtomo *infoAtomo);
void reconhece_constchar(TInfoAtomo *infoAtomo);
void reconhece_qualquer(TInfoAtomo *infoAtomo);
void consome(TAtomo esperado);
void reconhece_comentario(TInfoAtomo *infoAtomo);

// Semântico e Utilitários (Fase 2)
// IMPORTANTE: Esta função vem do arquivo hashMack.o ou hashMack.obj
int hashMack(char *s); 

void init_tabela_simbolos();
void insere_simbolo(char *id, int linha);
int busca_simbolo(char *id);
void imprime_tabela_simbolos();
int proximo_rotulo();
void erro_semantico(int linha, char *msg);

// Sintático + Geração de Código (Fase 2)
void program();
void block();
void variable_declaration_part();
void variable_declaration();
void type();
void statement_part();
void statement();
void assignment_statement();
void read_statement();
void write_statement();
void if_statement();
void while_statement();
void expression();
void simple_expression();
void term();
void factor();
void relational_operator();
void adding_operator();
void multiplying_operator();

// ============================================================================
// FUNÇÃO PRINCIPAL
// ============================================================================
int main(int argc, char *argv[]) {
    char *nomeArquivo;

    // Lógica para facilitar uso no GDB Online ou IDEs sem configuração de argumentos
    if (argc < 2) {
        nomeArquivo = "compilador.txt"; // Nome padrão para debug/testes
        printf(">>> Aviso: Nenhum arquivo passado na linha de comando.\n");
        printf(">>> Tentando abrir '%s' por padrao...\n\n", nomeArquivo);
    } else {
        nomeArquivo = argv[1];
    }

    FILE *f = fopen(nomeArquivo, "r");
    if (!f) {
        fprintf(stderr, "Erro ao abrir o arquivo '%s'.\n", nomeArquivo);
        fprintf(stderr, "Verifique se o arquivo existe no diretorio correto.\n");
        return 1;
    }

    fseek(f, 0, SEEK_END);
    long tamanho = ftell(f);
    fseek(f, 0, SEEK_SET);

    buffer = malloc(tamanho + 1);
    if (!buffer) {
        fprintf(stderr, "Erro ao alocar memória.\n");
        fclose(f);
        return 1;
    }

    long i = 0;
    int c;
    while ((c = fgetc(f)) != EOF) {
        buffer[i++] = (char)c;
    }
    buffer[i] = '\0';
    fclose(f);

    nLinha = 1;
    init_tabela_simbolos(); // Inicializa a tabela hash

    lookahead = obter_atomo();
    program(); // Inicia análise e compilação

    // Se chegou aqui, não houve erro fatal
    imprime_tabela_simbolos();

    return 0;
}

// ============================================================================
// IMPLEMENTAÇÃO: SEMÂNTICO E AUXILIARES (Fase 2)
// ============================================================================

// REMOVIDA: A função hashMack não é mais implementada aqui.
// Ela será linkada externamente do arquivo hashMack.o

void init_tabela_simbolos() {
    for (int i = 0; i < PRIME_NUMER; i++) {
        tabelaSimbolos.entradas[i] = NULL;
    }
}

// Insere variável na tabela. Erro se já existir.
void insere_simbolo(char *id, int linha) {
    int idx = hashMack(id); // Chama a função do professor
    TNo *atual = tabelaSimbolos.entradas[idx];

    // Verifica duplicação
    while (atual != NULL) {
        if (strcmp(atual->ID, id) == 0) {
            char msg[100];
            sprintf(msg, "identificador [%s] ja declarado", id);
            erro_semantico(linha, msg);
        }
        atual = atual->prox;
    }

    // Insere novo nó no início da lista (colisão)
    TNo *novo = (TNo*) malloc(sizeof(TNo));
    strcpy(novo->ID, id);
    novo->endereco = endereco_global++; // Atribui endereço e incrementa global
    novo->prox = tabelaSimbolos.entradas[idx];
    tabelaSimbolos.entradas[idx] = novo;
}

// Busca variável. Retorna endereço ou -1 se não achar.
int busca_simbolo(char *id) {
    int idx = hashMack(id); // Chama a função do professor
    TNo *atual = tabelaSimbolos.entradas[idx];
    while (atual != NULL) {
        if (strcmp(atual->ID, id) == 0) {
            return atual->endereco;
        }
        atual = atual->prox;
    }
    return -1;
}

void erro_semantico(int linha, char *msg) {
    printf("# %d:erro semantico, %s\n", linha, msg);
    exit(1);
}

int proximo_rotulo() {
    return cont_rotulo++;
}

void imprime_tabela_simbolos() {
    printf("TABELA DE SIMBOLOS\n");
    for (int i = 0; i < PRIME_NUMER; i++) {
        TNo *p = tabelaSimbolos.entradas[i];
        while (p != NULL) {
            // Formatação ajustada conforme a imagem do professor
            printf("Entrada Tabela Simbolos: [%d]\n", i);
            printf("=> %s | Endereco: %d\n", p->ID, p->endereco);
            p = p->prox;
        }
    }
}

// ============================================================================
// IMPLEMENTAÇÃO: LÉXICO (Fase 1 - Mantido com ajustes menores)
// ============================================================================

const char* simbolo_real(TAtomo a) {
    // Mantida apenas para fins de debug de erro sintático
    switch(a) {
        case ABRE_PAR: return "(";
        case FECHA_PAR: return ")";
        case MAIS: return "+";
        case MENOS: return "-";
        case ASTERISCO: return "*";
        case BARRA: return "/";
        case DIV: return "div";
        case PONTO_VIRGULA: return ";";
        case VIRGULA: return ",";
        case DOIS_PONTOS: return ":";
        case PONTO: return ".";
        case IGUAL: return "=";
        case ATRIBUICAO: return ":=";
        case MENOR: return "<";
        case MAIOR: return ">";
        case MENOR_IGUAL: return "<=";
        case MAIOR_IGUAL: return ">=";
        case NEGACAO: return "<>";
        case PROGRAM: return "program";
        case VAR: return "var";
        case ERRO: return "erro";
        case IDENTIFICADOR: return "identificador";
        case CONSTINT: return "constint";
        case CONSTCHAR: return "constchar";
        case EOS: return "eos";
        case OR: return "or";
        case AND: return "and";
        case WHILE: return "while";
        case NOT: return "not";
        case IF: return "if";
        case THEN: return "then";
        case ELSE: return "else";
        case DO: return "do";
        case BEGIN: return "begin";
        case END: return "end";
        case READ: return "read";
        case WRITE: return "write";
        case TRUE_TOKEN: return "true";
        case FALSE_TOKEN: return "false";
        default: return "nao identificado"; 
    }
}

TInfoAtomo obter_atomo() {
    TInfoAtomo infoAtomo;
    infoAtomo.atomo = ERRO;

    // Pula espaços
    while (*buffer == ' ' || *buffer == '\t' || *buffer == '\n' || *buffer == '\r' ) {
        if (*buffer == '\n') nLinha++;
        buffer++;
    }

    infoAtomo.linha = nLinha;

    if (*buffer == '\0') {
        infoAtomo.atomo = EOS;
        return infoAtomo;
    }

    // Comentário (* ... *)
    if (*buffer == '(' && *(buffer + 1) == '*') {
        reconhece_comentario(&infoAtomo);
        return infoAtomo;
    }

    if (isdigit(*buffer)) {
        reconhece_numero(&infoAtomo);
        return infoAtomo;
    }

    if (isalpha(*buffer) || *buffer == '_') {
        reconhece_id(&infoAtomo);
        return infoAtomo;
    }

    if (*buffer == '\'') {
        reconhece_constchar(&infoAtomo);
        return infoAtomo;
    }

    reconhece_qualquer(&infoAtomo);
    return infoAtomo;
}

void reconhece_numero(TInfoAtomo *infoAtomo){
    char *ini_lexema = buffer;
    
    // Simplificado para o escopo do projeto (inteiros)
    while (isdigit(*buffer)) buffer++;

    // Verifica notação científica simplificada se necessário, mas o foco é int simples
    // Mantendo a lógica básica
    if (*buffer == 'd') {
        buffer++;
        if (*buffer == '+' || *buffer == '-') buffer++;
        while (isdigit(*buffer)) buffer++;
    }

    int tamanho = buffer - ini_lexema;
    strncpy(lexema, ini_lexema, tamanho);
    lexema[tamanho] = '\0';
    
    // Tratamento para notação científica 'd' -> converter para 'e' para atof se necessário
    // Mas para o trabalho, vamos assumir conversão direta ou int
    for(int k=0; k<tamanho; k++) if(lexema[k]=='d') lexema[k]='e';
    
    infoAtomo->atributo.numero = (int)atof(lexema); // Casting para int conforme especificação
    infoAtomo->atomo = CONSTINT;
}
    
void reconhece_comentario(TInfoAtomo *infoAtomo) {
    buffer += 2; 
    while (*buffer != '\0') {
        if (*buffer == '\n') nLinha++;
        if (*buffer == '*' && *(buffer + 1) == ')') {
            buffer += 2; 
            infoAtomo->atomo = COMENTARIO;
            return;
        }
        buffer++;
    }
}

void reconhece_id(TInfoAtomo *infoAtomo){
    char *ini_lexema = buffer;
    while(isalnum(*buffer) || *buffer == '_') buffer++;
    
    int tamanho = buffer - ini_lexema;
    if (tamanho > 15) {
        printf("# %d:erro lexico: identificador maior que 15 caracteres\n", nLinha);
        exit(1);
    }
    
    strncpy(infoAtomo->atributo.id, ini_lexema, tamanho);
    infoAtomo->atributo.id[tamanho] = '\0';
    
    // Palavras reservadas
    if (strcmp(infoAtomo->atributo.id, "div") == 0) infoAtomo->atomo = DIV;
    else if (strcmp(infoAtomo->atributo.id, "or") == 0) infoAtomo->atomo = OR;
    else if (strcmp(infoAtomo->atributo.id, "and") == 0) infoAtomo->atomo = AND;
    else if (strcmp(infoAtomo->atributo.id, "not") == 0) infoAtomo->atomo = NOT;
    else if (strcmp(infoAtomo->atributo.id, "if") == 0) infoAtomo->atomo = IF;
    else if (strcmp(infoAtomo->atributo.id, "then") == 0) infoAtomo->atomo = THEN;
    else if (strcmp(infoAtomo->atributo.id, "else") == 0) infoAtomo->atomo = ELSE;
    else if (strcmp(infoAtomo->atributo.id, "while") == 0) infoAtomo->atomo = WHILE;
    else if (strcmp(infoAtomo->atributo.id, "do") == 0) infoAtomo->atomo = DO;
    else if (strcmp(infoAtomo->atributo.id, "begin") == 0) infoAtomo->atomo = BEGIN;
    else if (strcmp(infoAtomo->atributo.id, "end") == 0) infoAtomo->atomo = END;
    else if (strcmp(infoAtomo->atributo.id, "read") == 0) infoAtomo->atomo = READ;
    else if (strcmp(infoAtomo->atributo.id, "write") == 0) infoAtomo->atomo = WRITE;
    else if (strcmp(infoAtomo->atributo.id, "var") == 0) infoAtomo->atomo = VAR;
    else if (strcmp(infoAtomo->atributo.id, "program") == 0) infoAtomo->atomo = PROGRAM;
    else if (strcmp(infoAtomo->atributo.id, "true") == 0) infoAtomo->atomo = TRUE_TOKEN;
    else if (strcmp(infoAtomo->atributo.id, "false") == 0) infoAtomo->atomo = FALSE_TOKEN;
    else if (strcmp(infoAtomo->atributo.id, "char") == 0) infoAtomo->atomo = CHAR;
    else if (strcmp(infoAtomo->atributo.id, "integer") == 0) infoAtomo->atomo = INTEGER;
    else if (strcmp(infoAtomo->atributo.id, "boolean") == 0) infoAtomo->atomo = BOOLEAN;
    else infoAtomo->atomo = IDENTIFICADOR;
}

void reconhece_constchar(TInfoAtomo *infoAtomo){      
    buffer++; 
    if (*buffer == '\0') return;
    infoAtomo->atributo.ch = *buffer; 
    buffer++;
    if (*buffer == '\'') buffer++; // consome fecha aspas
    infoAtomo->atomo = CONSTCHAR;
}

void reconhece_qualquer(TInfoAtomo *infoAtomo){
    // Símbolos simples e duplos
    if (*buffer == '+') { infoAtomo->atomo = MAIS; buffer++; }
    else if (*buffer == '-') { infoAtomo->atomo = MENOS; buffer++; }
    else if (*buffer == '*') { infoAtomo->atomo = ASTERISCO; buffer++; }
    else if (*buffer == '/') { infoAtomo->atomo = BARRA; buffer++; } // Na gramática, div é a palavra chave, mas barra pode aparecer em outro contexto ou erro
    else if (*buffer == ';') { infoAtomo->atomo = PONTO_VIRGULA; buffer++; }
    else if (*buffer == ',') { infoAtomo->atomo = VIRGULA; buffer++; }
    else if (*buffer == '.') { infoAtomo->atomo = PONTO; buffer++; }
    else if (*buffer == '(') { infoAtomo->atomo = ABRE_PAR; buffer++; }
    else if (*buffer == ')') { infoAtomo->atomo = FECHA_PAR; buffer++; }
    else if (*buffer == '=') { infoAtomo->atomo = IGUAL; buffer++; }
    else if (*buffer == ':') {
        if (*(buffer + 1) == '=') { infoAtomo->atomo = ATRIBUICAO; buffer += 2; }
        else { infoAtomo->atomo = DOIS_PONTOS; buffer++; }
    }
    else if (*buffer == '<') {
        if (*(buffer + 1) == '=') { infoAtomo->atomo = MENOR_IGUAL; buffer += 2; }
        else if (*(buffer + 1) == '>') { infoAtomo->atomo = NEGACAO; buffer += 2; }
        else { infoAtomo->atomo = MENOR; buffer++; }
    }
    else if (*buffer == '>') {
        if (*(buffer + 1) == '=') { infoAtomo->atomo = MAIOR_IGUAL; buffer += 2; }
        else { infoAtomo->atomo = MAIOR; buffer++; }
    }
}

// ============================================================================
// ANALISADOR SINTÁTICO + GERAÇÃO DE CÓDIGO (Fase 2)
// ============================================================================

void consome(TAtomo esperado) {
    while(lookahead.atomo == COMENTARIO){
        lookahead = obter_atomo();
    }
    if (lookahead.atomo == esperado) {
        lookahead = obter_atomo();
    } else {
        printf("# %d:erro sintatico, esperado [%s] encontrado [%s]\n", 
            lookahead.linha, simbolo_real(esperado), simbolo_real(lookahead.atomo));
        exit(1);
    }
}

// <program> ::= program identifier ‘;‘ <block> ‘.’ 
void program() {
    printf("INPP\n"); // Código MEPA: Início Programa

    while(lookahead.atomo == COMENTARIO) lookahead = obter_atomo();
    
    consome(PROGRAM);
    consome(IDENTIFICADOR);
    consome(PONTO_VIRGULA);
    
    block();
    
    consome(PONTO);
    
    printf("PARA\n"); // Código MEPA: Fim Programa
}

// <block> ::= <variable_declaration_part> <statement_part> 
void block(){
    variable_declaration_part();
    
    // Geração de Código: Reserva memória para variáveis
    // endereco_global contém a quantidade total de variáveis declaradas
    if (endereco_global > 0) {
        printf("\tAMEM %d\n", endereco_global);
    }

    statement_part();
}

// <variable_declaration_part> ::= [ var <variable_declaration> ‘;’ { <variable_declaration> ‘;’ } ] 
void variable_declaration_part() {
    if (lookahead.atomo == VAR) {
        consome(VAR);
        variable_declaration();
        consome(PONTO_VIRGULA);

        while (lookahead.atomo == IDENTIFICADOR) {  
            variable_declaration();
            consome(PONTO_VIRGULA);
        }
    }
}

// <variable_declaration> ::= identifier { ‘,’ identifier } ‘:’ <type>
void variable_declaration() {
    // Declaração Semântica: Inserir na tabela
    if (lookahead.atomo == IDENTIFICADOR) {
        insere_simbolo(lookahead.atributo.id, lookahead.linha);
        consome(IDENTIFICADOR);
    } else {
        printf("# %d:erro sintatico, esperado identificador\n", lookahead.linha);
        exit(1);
    }

    while (lookahead.atomo == VIRGULA) {
        consome(VIRGULA);
        if (lookahead.atomo == IDENTIFICADOR) {
            insere_simbolo(lookahead.atributo.id, lookahead.linha);
            consome(IDENTIFICADOR);
        } else {
            printf("# %d:erro sintatico, esperado identificador\n", lookahead.linha);
            exit(1);
        }
    }

    consome(DOIS_PONTOS);
    type();
}

void type() {
    if (lookahead.atomo == CHAR) consome(CHAR);
    else if (lookahead.atomo == INTEGER) consome(INTEGER);
    else if (lookahead.atomo == BOOLEAN) consome(BOOLEAN);
    else {
        printf("# %d:erro sintatico, esperado tipo\n", lookahead.linha);
        exit(1);
    }
}

void statement_part() {
    consome(BEGIN);
    statement();
    while (lookahead.atomo == PONTO_VIRGULA) {
        consome(PONTO_VIRGULA);
        statement();
    }
    consome(END);
}

void statement() {
    if (lookahead.atomo == IDENTIFICADOR) assignment_statement();
    else if (lookahead.atomo == READ) read_statement();
    else if (lookahead.atomo == WRITE) write_statement();
    else if (lookahead.atomo == IF) if_statement();
    else if (lookahead.atomo == WHILE) while_statement();
    else if (lookahead.atomo == BEGIN) statement_part();
}

// <assignment_statement> ::= <variable> ‘:=’ <expression>
void assignment_statement(){
    char id[16];
    strcpy(id, lookahead.atributo.id); // Guarda o ID para buscar depois
    
    // Semântico: Verifica se existe
    int end = busca_simbolo(id);
    if (end == -1) {
        char msg[100];
        sprintf(msg, "variavel [%s] nao declarada", id);
        erro_semantico(lookahead.linha, msg);
    }

    consome(IDENTIFICADOR);
    consome(ATRIBUICAO);
    expression();
    
    // Geração de Código: Armazena topo da pilha no endereço
    printf("\tARMZ %d\n", end);
}

// <read_statement> ::= read ‘(’ <variable> { ‘,’ <variable> } ‘)’
void read_statement() {
    consome(READ);
    consome(ABRE_PAR);
    
    // Primeiro ID
    if (lookahead.atomo == IDENTIFICADOR) {
        int end = busca_simbolo(lookahead.atributo.id);
        if (end == -1) {
            char msg[100];
            sprintf(msg, "variavel [%s] nao declarada", lookahead.atributo.id);
            erro_semantico(lookahead.linha, msg);
        }
        printf("\tLEIT\n");
        printf("\tARMZ %d\n", end);
        consome(IDENTIFICADOR);
    }

    while (lookahead.atomo == VIRGULA) {
        consome(VIRGULA);
        if (lookahead.atomo == IDENTIFICADOR) {
            int end = busca_simbolo(lookahead.atributo.id);
            if (end == -1) {
                char msg[100];
                sprintf(msg, "variavel [%s] nao declarada", lookahead.atributo.id);
                erro_semantico(lookahead.linha, msg);
            }
            printf("\tLEIT\n");
            printf("\tARMZ %d\n", end);
            consome(IDENTIFICADOR);
        }
    }
    consome(FECHA_PAR);
}

// <write_statement> ::= write '(' identifier { ',' identifier } ')'
// Nota: A gramática original do PDF 2 pede identifier, mas geralmente write aceita expressões. 
// O PDF 1 diz "write '(' identifier ...". Vamos seguir estritamente o PDF 2 (Identifier).
void write_statement() {
    consome(WRITE);
    consome(ABRE_PAR);

    if (lookahead.atomo == IDENTIFICADOR) {
         int end = busca_simbolo(lookahead.atributo.id);
        if (end == -1) {
            char msg[100];
            sprintf(msg, "variavel [%s] nao declarada", lookahead.atributo.id);
            erro_semantico(lookahead.linha, msg);
        }
        printf("\tCRVL %d\n", end); // Carrega valor
        printf("\tIMPR\n");         // Imprime
        consome(IDENTIFICADOR);
    }

    while (lookahead.atomo == VIRGULA) {
        consome(VIRGULA);
        if (lookahead.atomo == IDENTIFICADOR) {
             int end = busca_simbolo(lookahead.atributo.id);
            if (end == -1) {
                char msg[100];
                sprintf(msg, "variavel [%s] nao declarada", lookahead.atributo.id);
                erro_semantico(lookahead.linha, msg);
            }
            printf("\tCRVL %d\n", end);
            printf("\tIMPR\n");
            consome(IDENTIFICADOR);
        }
    }
    consome(FECHA_PAR);
}

// <if_statement> ::= if <expression> then <statement> [ else <statement> ]
void if_statement() {
    int L1 = proximo_rotulo();
    int L2 = proximo_rotulo();

    consome(IF);
    expression();
    consome(THEN);
    
    printf("\tDSVF L%d\n", L1); // Desvio se Falso
    
    statement();
    
    printf("\tDSVS L%d\n", L2); // Desvio Incondicional (para pular o else)
    printf("L%d:\tNADA\n", L1);
    
    if (lookahead.atomo == ELSE) { 
        consome(ELSE);
        statement();
    }
    printf("L%d:\tNADA\n", L2);
}

// <while_statement> ::= while <expression> do <statement>
void while_statement() {
    int L1 = proximo_rotulo();
    int L2 = proximo_rotulo();

    printf("L%d:\tNADA\n", L1); // Rótulo de início do laço
    
    consome(WHILE);
    expression();
    
    printf("\tDSVF L%d\n", L2); // Sai do laço se expressão for falsa
    
    consome(DO);
    statement();
    
    printf("\tDSVS L%d\n", L1); // Volta para avaliar expressão
    printf("L%d:\tNADA\n", L2); // Rótulo de saída
}

// <expression>
void expression() {
    simple_expression();

    if (lookahead.atomo == MENOR || lookahead.atomo == MAIOR || lookahead.atomo == MENOR_IGUAL ||
        lookahead.atomo == MAIOR_IGUAL || lookahead.atomo == NEGACAO || lookahead.atomo == OR ||
        lookahead.atomo == AND || lookahead.atomo == IGUAL ) {
            
        TAtomo op = lookahead.atomo;
        relational_operator();
        simple_expression();
        
        // Geração de código Relacional
        switch(op) {
            case IGUAL: printf("\tCMIG\n"); break;
            case MENOR: printf("\tCMME\n"); break;
            case MAIOR: printf("\tCMMA\n"); break;
            case MENOR_IGUAL: printf("\tCMEG\n"); break;
            case MAIOR_IGUAL: printf("\tCMAG\n"); break;
            case NEGACAO: printf("\tCMDG\n"); break;
            case OR: printf("\tDISJ\n"); break; // MEPA Or
            case AND: printf("\tCONJ\n"); break; // MEPA And
            default: break;
        }
    }
}

void relational_operator() {
    // Apenas consome, a lógica de geração está em expression()
    if (lookahead.atomo == MENOR || lookahead.atomo == MAIOR || lookahead.atomo == MENOR_IGUAL ||
        lookahead.atomo == MAIOR_IGUAL || lookahead.atomo == NEGACAO || lookahead.atomo == IGUAL ||
        lookahead.atomo == OR || lookahead.atomo == AND) {
        consome(lookahead.atomo);
    } else {
         printf("# %d:erro sintatico, esperado operador relacional\n", lookahead.linha);
         exit(1);
    }
}

// <simple_expression> ::= <term> { <adding_operator> <term> }
void simple_expression() {
    term();
    while (lookahead.atomo == MAIS || lookahead.atomo == MENOS) {
        TAtomo op = lookahead.atomo;
        adding_operator();
        term();
        
        if (op == MAIS) printf("\tSOMA\n");
        else printf("\tSUBT\n");
    }
}

void adding_operator() {
    if (lookahead.atomo == MAIS) consome(MAIS);
    else consome(MENOS);
}

// <term> ::= <factor> { <multiplying_operator> <factor> }
void term() {
    factor(); 
    while (lookahead.atomo == ASTERISCO || lookahead.atomo == DIV) {
        TAtomo op = lookahead.atomo;
        multiplying_operator();
        factor();
        
        if (op == ASTERISCO) printf("\tMULT\n");
        else printf("\tDIVI\n");
    }
}

void multiplying_operator() {
    if (lookahead.atomo == ASTERISCO) consome(ASTERISCO);
    else consome(DIV);
}

void factor() {
    if (lookahead.atomo == IDENTIFICADOR) {
        int end = busca_simbolo(lookahead.atributo.id);
        if (end == -1) {
            char msg[100];
            sprintf(msg, "variavel [%s] nao declarada", lookahead.atributo.id);
            erro_semantico(lookahead.linha, msg);
        }
        printf("\tCRVL %d\n", end); // Carrega valor da variável
        consome(IDENTIFICADOR);
    }
    else if (lookahead.atomo == CONSTINT) {
        printf("\tCRCT %d\n", lookahead.atributo.numero); // Carrega constante
        consome(CONSTINT);
    }
    else if (lookahead.atomo == CONSTCHAR) {
        // MEPA trabalha com inteiros, char é tratado como int ASCII
        printf("\tCRCT %d\n", (int)lookahead.atributo.ch);
        consome(CONSTCHAR);
    }
    else if (lookahead.atomo == ABRE_PAR) {
        consome(ABRE_PAR);
        expression();
        consome(FECHA_PAR);
    }
    else if (lookahead.atomo == NOT) {
        consome(NOT);
        factor();
        printf("\tNEGA\n");
    }
    else if (lookahead.atomo == TRUE_TOKEN) {
        printf("\tCRCT 1\n");
        consome(TRUE_TOKEN);
    }
    else if (lookahead.atomo == FALSE_TOKEN) {
        printf("\tCRCT 0\n");
        consome(FALSE_TOKEN);
    }
}