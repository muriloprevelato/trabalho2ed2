#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vertice.h"
#include "unity.h"

// Sem arquivos em disco então setUp e tearDown sem estado global neste módulo.
void setUp(void){}
void tearDown(void){}

// Criação e destruição 
void test_criarVertice_DeveRetornarPonteiroValido(void){
    Vertice *v = criarVertice("v1", 10.0, 20.0);
    TEST_ASSERT_NOT_NULL(v);
    destruirVertice(v);
}

void test_criar_ComIdNulo_DeveRetornarNull(void){
    Vertice *v = criarVertice(NULL, 10.0, 20.0);
    TEST_ASSERT_NULL(v);
}

// Getters 
void test_getters_RetornarValoresCorretos(void){
    Vertice *v = criarVertice("cepij", 15.5, 25.5);
    TEST_ASSERT_NOT_NULL(v);

    TEST_ASSERT_EQUAL_STRING("cepij", getVerticeId(v));
    TEST_ASSERT_FLOAT_WITHIN(0.001, 15.5, getVerticeX(v));
    TEST_ASSERT_FLOAT_WITHIN(0.001, 25.5, getVerticeY(v));

    destruirVertice(v);
}

// Coordenadas negativas não são erro - vértice é só uma posição
void test_getters_CoordenadasNegativas_SaoAceitas(void){
    Vertice *v = criarVertice("v99", -50.0, -30.0);
    TEST_ASSERT_NOT_NULL(v);

    TEST_ASSERT_FLOAT_WITHIN(0.001, -50.0, getVerticeX(v));
    TEST_ASSERT_FLOAT_WITHIN(0.001, -30.0, getVerticeY(v));

    destruirVertice(v);
}

void test_criarVertice_CopiaIdDefensivamente(void){
    char buffer[VERTICE_ID_MAX];
    strcpy(buffer, "v42");

    Vertice *v = criarVertice(buffer, 5.0, 5.0);
    TEST_ASSERT_NOT_NULL(v);

    
    // simula o leitorVia reusando o buffer de fgets na próxima linha do arquivo.
    strcpy(buffer, "XXXXXXX");

    // Se criarVertice guardou por referência, isto agora leria "XXXXXXX".
    TEST_ASSERT_EQUAL_STRING("v42", getVerticeId(v));

    destruirVertice(v);
}

// id no limite max

void test_criarVertice_IdNoLimiteMaximo(void){
    // VERTICE_ID_MAX = 20; usamos 19 caracteres visíveis + '\0'.
    const char *id_longo = "1234567890123456789"; // 19 chars
    Vertice *v = criarVertice(id_longo, 0.0, 0.0);
    TEST_ASSERT_NOT_NULL(v);

    TEST_ASSERT_EQUAL_STRING(id_longo, getVerticeId(v));

    destruirVertice(v);
}

int main(void){
    UNITY_BEGIN();

    RUN_TEST(test_criarVertice_DeveRetornarPonteiroValido);
    RUN_TEST(test_criar_ComIdNulo_DeveRetornarNull);
    RUN_TEST(test_getters_RetornarValoresCorretos);
    RUN_TEST(test_getters_CoordenadasNegativas_SaoAceitas);
    RUN_TEST(test_criarVertice_CopiaIdDefensivamente);
    RUN_TEST(test_criarVertice_IdNoLimiteMaximo);

    return UNITY_END();
}