#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "filaPrioridade.h"
#include "unity.h"

// Sem estado global neste módulo - cada teste cria/destrói sua própria fila, então setUp/tearDown ficam vazios.
void setUp(void){}
void tearDown(void){}

void test_criarFilaPrioridade_DeveRetornarPonteiroValido(void){
    FilaPrioridade *fp = criarFilaPrioridade();
    TEST_ASSERT_NOT_NULL(fp);
    destruirFilaPrioridade(fp);
}


// Mesma lógica, intenção diferente
void test_destruirFilaPrioridade_FilaVazia_naoQuebrar(void){
    FilaPrioridade *fp = criarFilaPrioridade();
    TEST_ASSERT_NOT_NULL(fp);
    destruirFilaPrioridade(fp);
}

void test_inserirEExtrair_DevolveChaveEPrioridadeCorretas(void){
    FilaPrioridade *fp = criarFilaPrioridade();

    filaPrioridadeInserir(fp, "v1", 5.5);

    char chaveSaida[FILA_CHAVE_MAX];
    double prioridadeSaida;
    int resultado = filaPrioridadeExtrairMin(fp, chaveSaida, &prioridadeSaida);

    TEST_ASSERT_EQUAL_INT(FILA_OK, resultado);
    TEST_ASSERT_EQUAL_STRING("v1", chaveSaida);
    TEST_ASSERT_FLOAT_WITHIN(0.001, 5.5, prioridadeSaida);

    destruirFilaPrioridade(fp);
}

void test_extrairMin_FilaVazia_RetornaFilaVazia(void){
    FilaPrioridade *fp = criarFilaPrioridade();

    char chaveSaida[FILA_CHAVE_MAX];
    double prioridadeSaida;
    int resultado = filaPrioridadeExtrairMin(fp, chaveSaida, &prioridadeSaida);

    TEST_ASSERT_EQUAL_INT(FILA_VAZIA, resultado);

    destruirFilaPrioridade(fp);
}

void test_tamanho_FilaVazia_RetornaZero(void){
    FilaPrioridade *fp = criarFilaPrioridade();

    TEST_ASSERT_EQUAL_INT(0, filaPrioridadeTamanho(fp));

    destruirFilaPrioridade(fp);
}

void test_tamanho_AcompanhaInsercoesEExtracoes(void){
    FilaPrioridade *fp = criarFilaPrioridade();

    filaPrioridadeInserir(fp, "a", 3.0);
    TEST_ASSERT_EQUAL_INT(1, filaPrioridadeTamanho(fp));

    filaPrioridadeInserir(fp, "b", 1.0);
    TEST_ASSERT_EQUAL_INT(2, filaPrioridadeTamanho(fp));

    filaPrioridadeInserir(fp, "c", 2.0);
    TEST_ASSERT_EQUAL_INT(3, filaPrioridadeTamanho(fp));

    char chaveSaida[FILA_CHAVE_MAX];
    double prioridadeSaida;

    filaPrioridadeExtrairMin(fp, chaveSaida, &prioridadeSaida);
    TEST_ASSERT_EQUAL_INT(2, filaPrioridadeTamanho(fp));

    filaPrioridadeExtrairMin(fp, chaveSaida, &prioridadeSaida);
    TEST_ASSERT_EQUAL_INT(1, filaPrioridadeTamanho(fp));

    filaPrioridadeExtrairMin(fp, chaveSaida, &prioridadeSaida);
    TEST_ASSERT_EQUAL_INT(0, filaPrioridadeTamanho(fp));

    destruirFilaPrioridade(fp);
}

// Insertion sort simples - suficiente para os tamanhos usados em teste.
static void ordenarDoubles(double *arr, int n){
    for(int i = 1; i < n; i++){
        double chave = arr[i];
        int j = i - 1;
        while(j >= 0 && arr[j] > chave){
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = chave;
    }
}

#define QTD_TESTE_ORDEM 30

void test_extrairMin_OrdemCrescenteDePrioridade(void){
    double prioridadesInseridas[QTD_TESTE_ORDEM] = {
        15.0, 3.0, 42.0, 8.0, 23.0, 1.0, 99.0, 17.0, 5.0, 31.0,
        7.0,12.0, 88.0, 2.0, 45.0,19.0, 6.0, 77.0, 33.0, 4.0,
        21.0, 9.0, 55.0,13.0, 67.67,11.0, 27.0, 18.0, 40.0,10.0
    };

    FilaPrioridade *fp = criarFilaPrioridade();

    char chave[FILA_CHAVE_MAX];
    for(int i = 0; i < QTD_TESTE_ORDEM; i++){
        snprintf(chave, sizeof(chave), "k%d", i);
        filaPrioridadeInserir(fp, chave, prioridadesInseridas[i]);
    }

    TEST_ASSERT_EQUAL_INT(QTD_TESTE_ORDEM, filaPrioridadeTamanho(fp));

    double prioridadesExtraidas[QTD_TESTE_ORDEM];
    char chaveSaida[FILA_CHAVE_MAX];
    for(int i = 0; i < QTD_TESTE_ORDEM; i++){
        double prioridadeSaida;
        int resultado = filaPrioridadeExtrairMin(fp, chaveSaida, &prioridadeSaida);
        TEST_ASSERT_EQUAL_INT(FILA_OK, resultado);
        prioridadesExtraidas[i] = prioridadeSaida;
    }

    TEST_ASSERT_EQUAL_INT(0, filaPrioridadeTamanho(fp)); // esvaziou por completo

    // 1 Verifica monotonicidade não-decrescente da extração.
    for(int i = 1; i < QTD_TESTE_ORDEM; i++){
        TEST_ASSERT_TRUE(prioridadesExtraidas[i] >= prioridadesExtraidas[i - 1]);
    }

    // 2 Verifica que o multiset extraído é igual ao inserido -
    // ordena as duas cópias e compara elemento a elemento.
    double copiaInseridas[QTD_TESTE_ORDEM];
    memcpy(copiaInseridas, prioridadesInseridas, sizeof(copiaInseridas));
    ordenarDoubles(copiaInseridas, QTD_TESTE_ORDEM);
    // prioridadesExtraidas já está ordenada por construção (passo 1),
    // mas reordena por clareza/robustez caso o teste mude no futuro.
    ordenarDoubles(prioridadesExtraidas, QTD_TESTE_ORDEM);

    for(int i = 0; i < QTD_TESTE_ORDEM; i++){
        TEST_ASSERT_FLOAT_WITHIN(0.001, copiaInseridas[i], prioridadesExtraidas[i]);
    }

    destruirFilaPrioridade(fp);
}



void test_chavesRepetidas_AmbasSaoExtraidasNaOrdemCorreta(void){
    FilaPrioridade *fp = criarFilaPrioridade();

    filaPrioridadeInserir(fp, "v1", 10.0);
    filaPrioridadeInserir(fp, "v1", 3.0); // "caminho melhor" encontrado depois

    TEST_ASSERT_EQUAL_INT(2, filaPrioridadeTamanho(fp)); // NÃO duplicado

    char chaveSaida[FILA_CHAVE_MAX];
    double prioridadeSaida;

    // A menor prioridade (3.0) sai primeiro, mesmo tendo sido inserida
    // por último - confirma que a fila ordena por prioridade, não por
    // ordem de inserção.
    filaPrioridadeExtrairMin(fp, chaveSaida, &prioridadeSaida);
    TEST_ASSERT_EQUAL_STRING("v1", chaveSaida);
    TEST_ASSERT_FLOAT_WITHIN(0.001, 3.0, prioridadeSaida);

    filaPrioridadeExtrairMin(fp, chaveSaida, &prioridadeSaida);
    TEST_ASSERT_EQUAL_STRING("v1", chaveSaida);
    TEST_ASSERT_FLOAT_WITHIN(0.001, 10.0, prioridadeSaida);

    TEST_ASSERT_EQUAL_INT(0, filaPrioridadeTamanho(fp));

    destruirFilaPrioridade(fp);
}

void test_prioridadesNegativas_SaoAceitasEOrdenadasCorretamente(void){
    FilaPrioridade *fp = criarFilaPrioridade();

    filaPrioridadeInserir(fp, "positivo", 5.0);
    filaPrioridadeInserir(fp, "negativo", -3.0);
    filaPrioridadeInserir(fp, "zero", 0.0);

    char chaveSaida[FILA_CHAVE_MAX];
    double prioridadeSaida;

    filaPrioridadeExtrairMin(fp, chaveSaida, &prioridadeSaida);
    TEST_ASSERT_EQUAL_STRING("negativo", chaveSaida);
    TEST_ASSERT_FLOAT_WITHIN(0.001, -3.0, prioridadeSaida);

    filaPrioridadeExtrairMin(fp, chaveSaida, &prioridadeSaida);
    TEST_ASSERT_EQUAL_STRING("zero", chaveSaida);

    filaPrioridadeExtrairMin(fp, chaveSaida, &prioridadeSaida);
    TEST_ASSERT_EQUAL_STRING("positivo", chaveSaida);

    destruirFilaPrioridade(fp);
}

void test_prioridadesEmpatadas_AmbasSaoExtraidas(void){
    FilaPrioridade *fp = criarFilaPrioridade();

    filaPrioridadeInserir(fp, "x", 7.0);
    filaPrioridadeInserir(fp, "y", 7.0);

    char chaveSaida[FILA_CHAVE_MAX];
    double prioridadeSaida;

    int vistoX = 0, vistoY = 0;

    filaPrioridadeExtrairMin(fp, chaveSaida, &prioridadeSaida);
    TEST_ASSERT_FLOAT_WITHIN(0.001, 7.0, prioridadeSaida);
    if(strcmp(chaveSaida, "x") == 0) vistoX = 1;
    if(strcmp(chaveSaida, "y") == 0) vistoY = 1;

    filaPrioridadeExtrairMin(fp, chaveSaida, &prioridadeSaida);
    TEST_ASSERT_FLOAT_WITHIN(0.001, 7.0, prioridadeSaida);
    if(strcmp(chaveSaida, "x") == 0) vistoX = 1;
    if(strcmp(chaveSaida, "y") == 0) vistoY = 1;

    TEST_ASSERT_TRUE(vistoX && vistoY); // ambas saíram, não importa a ordem

    destruirFilaPrioridade(fp);
}

void test_inserir_CopiaChaveDefensivamente(void){
    char buffer[FILA_CHAVE_MAX];
    strcpy(buffer, "v42");

    FilaPrioridade *fp = criarFilaPrioridade();
    filaPrioridadeInserir(fp, buffer, 5.0);

    // Suja o buffer original DEPOIS de inserir.
    strcpy(buffer, "XXXXXXX");

    char chaveSaida[FILA_CHAVE_MAX];
    double prioridadeSaida;
    filaPrioridadeExtrairMin(fp, chaveSaida, &prioridadeSaida);

    TEST_ASSERT_EQUAL_STRING("v42", chaveSaida); // não "XXXXXXX"

    destruirFilaPrioridade(fp);
}

// chave no limite max

void test_chaveNoLimiteMaximo(void){
    // FILA_CHAVE_MAX = 20; usamos 19 caracteres visíveis + '\0'.
    const char *chave_longa = "111111111111111111111111111111111111111111111111111111111111111"; // 19 chars

    FilaPrioridade *fp = criarFilaPrioridade();
    filaPrioridadeInserir(fp, chave_longa, 1.0);

    char chaveSaida[FILA_CHAVE_MAX];
    double prioridadeSaida;
    filaPrioridadeExtrairMin(fp, chaveSaida, &prioridadeSaida);

    TEST_ASSERT_EQUAL_STRING(chave_longa, chaveSaida);

    destruirFilaPrioridade(fp);
}

int main(void){
    UNITY_BEGIN();

    RUN_TEST(test_criarFilaPrioridade_DeveRetornarPonteiroValido);
    RUN_TEST(test_destruirFilaPrioridade_FilaVazia_naoQuebrar);
    RUN_TEST(test_inserirEExtrair_DevolveChaveEPrioridadeCorretas);
    RUN_TEST(test_extrairMin_FilaVazia_RetornaFilaVazia);
    RUN_TEST(test_tamanho_FilaVazia_RetornaZero);
    RUN_TEST(test_tamanho_AcompanhaInsercoesEExtracoes);
    RUN_TEST(test_extrairMin_OrdemCrescenteDePrioridade);
    RUN_TEST(test_chavesRepetidas_AmbasSaoExtraidasNaOrdemCorreta);
    RUN_TEST(test_prioridadesNegativas_SaoAceitasEOrdenadasCorretamente);
    RUN_TEST(test_prioridadesEmpatadas_AmbasSaoExtraidas);
    RUN_TEST(test_inserir_CopiaChaveDefensivamente);
    RUN_TEST(test_chaveNoLimiteMaximo);

    return UNITY_END();
}