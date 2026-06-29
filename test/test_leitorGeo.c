#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "leitorGeo.h"
#include "cidade.h"
#include "quadra.h"
#include "unity.h"

// Sem estado global neste módulo - cada teste cria/remove seu próprio
// .geo temporário, então setUp/tearDown ficam vazios.
void setUp(void){}
void tearDown(void){}

// Caminho de arquivo temporário usado pelos testes. 
#define ARQ_TMP "teste_geo_tmp.geo"

// Helper de escrita do .geo temporário 
static void escreverGeoTemporario(const char *conteudo){
    FILE *fp = fopen(ARQ_TMP, "w");
    TEST_ASSERT_NOT_NULL(fp); // falha aqui seria erro do ambiente de teste, não do código sob teste
    fputs(conteudo, fp);
    fclose(fp);
}

// Arquivo válido, com q e cq

void test_lerArquivoGeo_ArquivoValido_RetornaOk(void){
    escreverGeoTemporario(
        "cq 2.0 orange black\n"
        "q cep01 10.0 20.0 50.0 30.0\n"
    );

    Cidade *c = criarCidade();
    TEST_ASSERT_NOT_NULL(c);

    int resultado = lerArquivoGeo(ARQ_TMP, c);
    TEST_ASSERT_EQUAL_INT(GEO_OK, resultado);

    destruirCidade(c);
    remove(ARQ_TMP);
}

void test_lerArquivoGeo_QuadraPegaAtributosCorretos(void){
    escreverGeoTemporario(
        "cq 2.0 orange black\n"
        "q cep01 10.0 20.0 50.0 30.0\n"
    );

    Cidade *c = criarCidade();
    TEST_ASSERT_NOT_NULL(c);

    lerArquivoGeo(ARQ_TMP, c);

    Quadra *q = buscarQuadraCidade(c, "cep01");
    TEST_ASSERT_NOT_NULL(q);

    TEST_ASSERT_EQUAL_STRING("cep01", getQuadraCep(q));
    TEST_ASSERT_FLOAT_WITHIN(0.001, 10.0, getQuadraX(q));
    TEST_ASSERT_FLOAT_WITHIN(0.001, 20.0, getQuadraY(q));
    TEST_ASSERT_FLOAT_WITHIN(0.001, 50.0, getQuadraW(q));
    TEST_ASSERT_FLOAT_WITHIN(0.001, 30.0, getQuadraH(q));
    TEST_ASSERT_FLOAT_WITHIN(0.001,  2.0, getQuadraSw(q));
    TEST_ASSERT_EQUAL_STRING("orange", getQuadraCFill(q));
    TEST_ASSERT_EQUAL_STRING("black", getQuadraCStrk(q));

    destruirCidade(c);
    remove(ARQ_TMP);
}

// q antes de qualquer cq: usa o default white/black/1.0
void test_lerArquivoGeo_QAntesDeCq_UsaCorPadrao(void){
    escreverGeoTemporario(
        "q cep01 10.0 20.0 50.0 30.0\n"
    );

    Cidade *c = criarCidade();
    TEST_ASSERT_NOT_NULL(c);

    lerArquivoGeo(ARQ_TMP, c);

    Quadra *q = buscarQuadraCidade(c, "cep01");
    TEST_ASSERT_NOT_NULL(q);

    TEST_ASSERT_EQUAL_STRING("white", getQuadraCFill(q));
    TEST_ASSERT_EQUAL_STRING("black", getQuadraCStrk(q));
    TEST_ASSERT_FLOAT_WITHIN(0.001, 1.0, getQuadraSw(q));

    destruirCidade(c);
    remove(ARQ_TMP);
}

// cq no meio: não é retroativo -> quadras antes e depois têm cores diferentes
void test_lerArquivoGeo_CqNoMeio_NaoEhRetroativo(void){
    escreverGeoTemporario(
        "q cep01 10.0 20.0 50.0 30.0\n"
        "cq 3.0 green yellow\n"
        "q cep02 60.0 20.0 50.0 30.0\n"
    );

    Cidade *c = criarCidade();
    TEST_ASSERT_NOT_NULL(c);

    lerArquivoGeo(ARQ_TMP, c);

    Quadra *q1 = buscarQuadraCidade(c, "cep01");
    Quadra *q2 = buscarQuadraCidade(c, "cep02");
    TEST_ASSERT_NOT_NULL(q1);
    TEST_ASSERT_NOT_NULL(q2);

    // cep01: criada antes do cq, deve ter ficado com o default.
    TEST_ASSERT_EQUAL_STRING("white", getQuadraCFill(q1));
    TEST_ASSERT_EQUAL_STRING("black", getQuadraCStrk(q1));
    TEST_ASSERT_FLOAT_WITHIN(0.001, 1.0, getQuadraSw(q1));

    // cep02: criada depois do cq, deve ter a cor nova.
    TEST_ASSERT_EQUAL_STRING("green", getQuadraCFill(q2));
    TEST_ASSERT_EQUAL_STRING("yellow", getQuadraCStrk(q2));
    TEST_ASSERT_FLOAT_WITHIN(0.001, 3.0, getQuadraSw(q2));

    destruirCidade(c);
    remove(ARQ_TMP);
}

// CEP duplicado -> só a primeira ocorrência entra

void test_lerArquivoGeo_CepDuplicado_MantemSoAPrimeira(void){
    escreverGeoTemporario(
        "cq 2.0 orange black\n"
        "q cep01 10.0 20.0 50.0 30.0\n"
        "q cep01 99.0 99.0 99.0 99.0\n" // mesmo CEP, atributos diferentes
    );

    Cidade *c = criarCidade();
    TEST_ASSERT_NOT_NULL(c);

    lerArquivoGeo(ARQ_TMP, c);

    TEST_ASSERT_EQUAL_INT(1, cidadeNumQuadras(c));

    Quadra *q = buscarQuadraCidade(c, "cep01");
    TEST_ASSERT_NOT_NULL(q);
    TEST_ASSERT_FLOAT_WITHIN(0.001, 10.0, getQuadraX(q));
    TEST_ASSERT_FLOAT_WITHIN(0.001, 20.0, getQuadraY(q));

    destruirCidade(c);
    remove(ARQ_TMP);
}

// q com dimensão inválida: descartada, não fecha programa.
void test_lerArquivoGeo_QComDimensaoInvalida_DescartaSemAbortar(void){
    escreverGeoTemporario(
        "cq 2.0 orange black\n"
        "q cep01 10.0 20.0 -5.0 30.0\n" // w negativo: criarQuadra rejeita
        "q cep02 60.0 20.0 50.0 30.0\n" // linha boa, deve entrar normalmente
    );

    Cidade *c = criarCidade();
    TEST_ASSERT_NOT_NULL(c);

    int resultado = lerArquivoGeo(ARQ_TMP, c);
    TEST_ASSERT_EQUAL_INT(GEO_OK, resultado); // abrir o arquivo funcionou

    TEST_ASSERT_EQUAL_INT(CIDADE_ERRO, cidadeContemCep(c, "cep01")); // descartada
    TEST_ASSERT_EQUAL_INT(CIDADE_OK, cidadeContemCep(c, "cep02")); // entrou normalmente
    TEST_ASSERT_EQUAL_INT(1, cidadeNumQuadras(c));

    destruirCidade(c);
    remove(ARQ_TMP);
}

// Caminho inválido: GEO_ERRO

void test_lerArquivoGeo_CaminhoInvalido_RetornaErro(void){
    Cidade *c = criarCidade();
    TEST_ASSERT_NOT_NULL(c);

    int resultado = lerArquivoGeo("/diretorio/que/definitivamente/nao/existe/x.geo", c);
    TEST_ASSERT_EQUAL_INT(GEO_ERRO, resultado);

    TEST_ASSERT_EQUAL_INT(0, cidadeNumQuadras(c)); // nada foi inserido

    destruirCidade(c);
}

// Linha malformada no meio: linhas boas ao redor ainda entram

void test_lerArquivoGeo_LinhaMalformadaNoMeio_LinhasBoasEntram(void){
    escreverGeoTemporario(
        "cq 2.0 orange black\n"
        "q cep01 10.0 20.0 50.0 30.0\n"
        "q cep02 60.0\n" // malformada: faltam campos (w, h)
        "comandoDesconhecido x y z\n"   // comando inexistente
        "q cep03 110.0 20.0 50.0 30.0\n"
    );

    Cidade *c = criarCidade();
    TEST_ASSERT_NOT_NULL(c);

    int resultado = lerArquivoGeo(ARQ_TMP, c);
    TEST_ASSERT_EQUAL_INT(GEO_OK, resultado); // abertura do arquivo continua OK

    TEST_ASSERT_EQUAL_INT(CIDADE_OK,   cidadeContemCep(c, "cep01"));
    TEST_ASSERT_EQUAL_INT(CIDADE_ERRO, cidadeContemCep(c, "cep02")); // linha malformada, descartada
    TEST_ASSERT_EQUAL_INT(CIDADE_OK,   cidadeContemCep(c, "cep03")); // leitura continuou após o problema

    TEST_ASSERT_EQUAL_INT(2, cidadeNumQuadras(c));

    destruirCidade(c);
    remove(ARQ_TMP);
}

// Linha em branco: ignorada silenciosamente, sem afetar a leitura

void test_lerArquivoGeo_LinhaEmBranco_EhIgnorada(void){
    escreverGeoTemporario(
        "cq 2.0 orange black\n"
        "\n"
        "q cep01 10.0 20.0 50.0 30.0\n"
        "\n"
        "q cep02 60.0 20.0 50.0 30.0\n"
    );

    Cidade *c = criarCidade();
    TEST_ASSERT_NOT_NULL(c);

    int resultado = lerArquivoGeo(ARQ_TMP, c);
    TEST_ASSERT_EQUAL_INT(GEO_OK, resultado);
    TEST_ASSERT_EQUAL_INT(2, cidadeNumQuadras(c));

    destruirCidade(c);
    remove(ARQ_TMP);
}

int main(void){
    UNITY_BEGIN();

    RUN_TEST(test_lerArquivoGeo_ArquivoValido_RetornaOk);
    RUN_TEST(test_lerArquivoGeo_QuadraPegaAtributosCorretos);
    RUN_TEST(test_lerArquivoGeo_QAntesDeCq_UsaCorPadrao);
    RUN_TEST(test_lerArquivoGeo_CqNoMeio_NaoEhRetroativo);
    RUN_TEST(test_lerArquivoGeo_CepDuplicado_MantemSoAPrimeira);
    RUN_TEST(test_lerArquivoGeo_QComDimensaoInvalida_DescartaSemAbortar);
    RUN_TEST(test_lerArquivoGeo_CaminhoInvalido_RetornaErro);
    RUN_TEST(test_lerArquivoGeo_LinhaMalformadaNoMeio_LinhasBoasEntram);
    RUN_TEST(test_lerArquivoGeo_LinhaEmBranco_EhIgnorada);

    return UNITY_END();
}