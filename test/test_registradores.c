#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "registradores.h"
#include "unity.h"

// Sem estado global neste módulo - cada teste cria seu próprio banco, então setUp/tearDown ficam vazios.
void setUp(void){}
void tearDown(void){}

void test_criarRegistradores_DeveRetornarPonteiroValido(void){
    Registradores *r = criarRegistradores();
    TEST_ASSERT_NOT_NULL(r);
    destruirRegistradores(r);
}

void test_destruirRegistradores_TodosVazios_naoQuebrar(void){
    Registradores *r = criarRegistradores();
    TEST_ASSERT_NOT_NULL(r);
    destruirRegistradores(r);
}

void test_registradorPreenchido_TodosVaziosNaCriacao(void){
    Registradores *r = criarRegistradores();

    for(int i = 0; i < REGISTRADOR_MAX; i++){
        TEST_ASSERT_EQUAL_INT(REGISTRADOR_ERRO, registradorPreenchido(r, i));
    }

    destruirRegistradores(r);
}

void test_setRegistrador_PreencheERetornaValoresCorretos(void){
    Registradores *r = criarRegistradores();

    setRegistrador(r, 3, 15.5, 25.5, "cep15/S/45");

    TEST_ASSERT_EQUAL_INT(REGISTRADOR_OK, registradorPreenchido(r, 3));
    TEST_ASSERT_FLOAT_WITHIN(0.001, 15.5, getRegistradorX(r, 3));
    TEST_ASSERT_FLOAT_WITHIN(0.001, 25.5, getRegistradorY(r, 3));
    TEST_ASSERT_EQUAL_STRING("cep15/S/45", getRegistradorTexto(r, 3));

    destruirRegistradores(r);
}


void test_setRegistrador_IndiceZero_FuncionaCorretamente(void){
    Registradores *r = criarRegistradores();

    setRegistrador(r, 0, 1.0, 2.0, "cep01/N/10");

    TEST_ASSERT_EQUAL_INT(REGISTRADOR_OK, registradorPreenchido(r, 0));
    TEST_ASSERT_FLOAT_WITHIN(0.001, 1.0, getRegistradorX(r, 0));

    destruirRegistradores(r);
}

void test_setRegistrador_UltimoIndiceValido_FuncionaCorretamente(void){
    Registradores *r = criarRegistradores();

    setRegistrador(r, REGISTRADOR_MAX - 1, 99.0, 88.0, "cep10/O/5");

    TEST_ASSERT_EQUAL_INT(REGISTRADOR_OK, registradorPreenchido(r, REGISTRADOR_MAX - 1));
    TEST_ASSERT_FLOAT_WITHIN(0.001, 99.0, getRegistradorX(r, REGISTRADOR_MAX - 1));
    TEST_ASSERT_FLOAT_WITHIN(0.001, 88.0, getRegistradorY(r, REGISTRADOR_MAX - 1));

    destruirRegistradores(r);
}

void test_setRegistrador_Sobrescrita_SegundoValorPrevalece(void){
    Registradores *r = criarRegistradores();

    setRegistrador(r, 5, 10.0, 20.0, "cep05/N/1");
    setRegistrador(r, 5, 30.0, 40.0, "cep06/S/2"); // sobrescreve

    TEST_ASSERT_EQUAL_INT(REGISTRADOR_OK, registradorPreenchido(r, 5));
    TEST_ASSERT_FLOAT_WITHIN(0.001, 30.0, getRegistradorX(r, 5));
    TEST_ASSERT_FLOAT_WITHIN(0.001, 40.0, getRegistradorY(r, 5));
    TEST_ASSERT_EQUAL_STRING("cep06/S/2", getRegistradorTexto(r, 5));

    destruirRegistradores(r);
}

void test_setRegistrador_NaoAfetaOutrosRegistradores(void){
    Registradores *r = criarRegistradores();

    setRegistrador(r, 0, 1.0, 1.0, "cep01/N/1");
    setRegistrador(r, 5, 5.0, 5.0, "cep05/N/5");

    // R0 continua com seu próprio valor, não foi sobrescrito por R5.
    TEST_ASSERT_FLOAT_WITHIN(0.001, 1.0, getRegistradorX(r, 0));
    TEST_ASSERT_EQUAL_STRING("cep01/N/1", getRegistradorTexto(r, 0));

    // R5 tem seu próprio valor.
    TEST_ASSERT_FLOAT_WITHIN(0.001, 5.0, getRegistradorX(r, 5));

    // Os registradores nunca setados continuam vazios.
    TEST_ASSERT_EQUAL_INT(REGISTRADOR_ERRO, registradorPreenchido(r, 1));
    TEST_ASSERT_EQUAL_INT(REGISTRADOR_ERRO, registradorPreenchido(r, 10));

    destruirRegistradores(r);
}


void test_setRegistrador_CopiaTextoDefensivamente(void){
    char buffer[REGISTRADOR_TEXTO_MAX];
    strcpy(buffer, "cep20/L/30");

    Registradores *r = criarRegistradores();
    setRegistrador(r, 7, 1.0, 2.0, buffer);

    // Suja o buffer original depois de setar.
    strcpy(buffer, "XXXXXXXXXX");

    TEST_ASSERT_EQUAL_STRING("cep20/L/30", getRegistradorTexto(r, 7));

    destruirRegistradores(r);
}

void test_setRegistrador_TextoNoLimiteMaximo(void){
    // REGISTRADOR_TEXTO_MAX = 64; usamos 63 caracteres visíveis + '\0'.
    char texto_longo[REGISTRADOR_TEXTO_MAX];
    memset(texto_longo, 'a', REGISTRADOR_TEXTO_MAX - 1);
    texto_longo[REGISTRADOR_TEXTO_MAX - 1] = '\0';

    Registradores *r = criarRegistradores();
    setRegistrador(r, 2, 1.0, 2.0, texto_longo);

    TEST_ASSERT_EQUAL_STRING(texto_longo, getRegistradorTexto(r, 2));

    destruirRegistradores(r);
}

int main(void){
    UNITY_BEGIN();

    RUN_TEST(test_criarRegistradores_DeveRetornarPonteiroValido);
    RUN_TEST(test_destruirRegistradores_TodosVazios_naoQuebrar);
    RUN_TEST(test_registradorPreenchido_TodosVaziosNaCriacao);
    RUN_TEST(test_setRegistrador_PreencheERetornaValoresCorretos);
    RUN_TEST(test_setRegistrador_IndiceZero_FuncionaCorretamente);
    RUN_TEST(test_setRegistrador_UltimoIndiceValido_FuncionaCorretamente);
    RUN_TEST(test_setRegistrador_Sobrescrita_SegundoValorPrevalece);
    RUN_TEST(test_setRegistrador_NaoAfetaOutrosRegistradores);
    RUN_TEST(test_setRegistrador_CopiaTextoDefensivamente);
    RUN_TEST(test_setRegistrador_TextoNoLimiteMaximo);

    return UNITY_END();
}