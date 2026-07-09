#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "resolveEndereco.h"
#include "cidade.h"
#include "quadra.h"
#include "grafo.h"
#include "vertice.h"
#include "unity.h"

// Sem estado global neste módulo — cada teste monta sua própria cidade e grafo, então setUp/tearDown ficam vazios.
void setUp(void){}
void tearDown(void){}


// Quadra âncora SE=(0,0), w=100, h=50.
//   face S: y fixo=0,  x de 0 a 100
//   face N: y fixo=50, x de 0 a 100
//   face L: x fixo=0,  y de 0 a 50
//   face O: x fixo=100, y de 0 a 50
static Cidade* montarCidadeComQuadra(void){
    Cidade *c = criarCidade();
    Quadra *q = criarQuadra("cep15", 0.0, 0.0, 100.0, 50.0, 1.0, "white", "black");
    inserirQuadraCidade(c, q);
    return c;
}

// Três vértices: um exatamente sobre cep15/S/30 (distância 0), os outros dois mais longe.
static Grafo* montarGrafoComVertices(void){
    Grafo *g = criarGrafo();
    grafoInserirVertice(g, criarVertice("v1", 30.0, 0.0));    // exatamente na coordenada
    grafoInserirVertice(g, criarVertice("v2", 100.0, 100.0)); // longe
    grafoInserirVertice(g, criarVertice("v3", 25.0, 5.0));    // perto, mas não tanto
    return g;
}

void test_resolverEndereco_EnderecoValido_RetornaOkComCoordenadaCorreta(void){
    Cidade *cidade = montarCidadeComQuadra();
    Grafo *grafo = montarGrafoComVertices();

    double x, y;
    char idVertice[VERTICE_ID_MAX];
    int resultado = resolverEndereco(cidade, grafo, "cep15", 'S', 30.0, &x, &y, idVertice);

    TEST_ASSERT_EQUAL_INT(RESOLVE_OK, resultado);
    TEST_ASSERT_FLOAT_WITHIN(0.001, 30.0, x);
    TEST_ASSERT_FLOAT_WITHIN(0.001, 0.0, y);
    TEST_ASSERT_EQUAL_STRING("v1", idVertice); // distância 0, o mais próximo possível

    destruirGrafo(grafo);
    destruirCidade(cidade);
}

void test_resolverEndereco_EscolheVerticeMaisProximoPorDistanciaReal(void){
    Cidade *cidade = montarCidadeComQuadra();

    Grafo *grafo = criarGrafo();
    grafoInserirVertice(grafo, criarVertice("longe1", 1000.0, 1000.0));
    grafoInserirVertice(grafo, criarVertice("perto",  32.0, 2.0)); // dist^2 = 4+4 = 8
    grafoInserirVertice(grafo, criarVertice("longe2", -500.0, -500.0));

    double x, y;
    char idVertice[VERTICE_ID_MAX];
    int resultado = resolverEndereco(cidade, grafo, "cep15", 'S', 30.0, &x, &y, idVertice);

    TEST_ASSERT_EQUAL_INT(RESOLVE_OK, resultado);
    TEST_ASSERT_EQUAL_STRING("perto", idVertice);

    destruirGrafo(grafo);
    destruirCidade(cidade);
}

void test_resolverEndereco_FaceLateral_NumValido_RetornaOk(void){
    Cidade *cidade = montarCidadeComQuadra(); // w=100, h=50

    Grafo *grafo = criarGrafo();
    grafoInserirVertice(grafo, criarVertice("vL", 0.0, 25.0)); // face L: x fixo=0

    double x, y;
    char idVertice[VERTICE_ID_MAX];
    int resultado = resolverEndereco(cidade, grafo, "cep15", 'L', 25.0, &x, &y, idVertice);

    TEST_ASSERT_EQUAL_INT(RESOLVE_OK, resultado);
    TEST_ASSERT_FLOAT_WITHIN(0.001, 0.0, x);
    TEST_ASSERT_FLOAT_WITHIN(0.001, 25.0, y);

    destruirGrafo(grafo);
    destruirCidade(cidade);
}

void test_resolverEndereco_CepInexistente_RetornaErro(void){
    Cidade *cidade = montarCidadeComQuadra();
    Grafo *grafo = montarGrafoComVertices();

    double x, y;
    char idVertice[VERTICE_ID_MAX];
    int resultado = resolverEndereco(cidade, grafo, "naoexiste", 'S', 30.0, &x, &y, idVertice);

    TEST_ASSERT_EQUAL_INT(RESOLVE_ERRO, resultado);

    destruirGrafo(grafo);
    destruirCidade(cidade);
}

void test_resolverEndereco_FaceInvalida_RetornaErro(void){
    Cidade *cidade = montarCidadeComQuadra();
    Grafo *grafo = montarGrafoComVertices();

    double x, y;
    char idVertice[VERTICE_ID_MAX];
    int resultado = resolverEndereco(cidade, grafo, "cep15", 'X', 30.0, &x, &y, idVertice);

    TEST_ASSERT_EQUAL_INT(RESOLVE_ERRO, resultado);

    destruirGrafo(grafo);
    destruirCidade(cidade);
}

void test_resolverEndereco_NumNegativo_RetornaErroSemAbortar(void){
    Cidade *cidade = montarCidadeComQuadra();
    Grafo *grafo = montarGrafoComVertices();

    double x, y;
    char idVertice[VERTICE_ID_MAX];
    int resultado = resolverEndereco(cidade, grafo, "cep15", 'S', -5.0, &x, &y, idVertice);

    TEST_ASSERT_EQUAL_INT(RESOLVE_ERRO, resultado);

    destruirGrafo(grafo);
    destruirCidade(cidade);
}

void test_resolverEndereco_NumAlemDaLargura_RetornaErroSemAbortar(void){
    Cidade *cidade = montarCidadeComQuadra(); // largura=100 (face S)
    Grafo *grafo = montarGrafoComVertices();

    double x, y;
    char idVertice[VERTICE_ID_MAX];
    int resultado = resolverEndereco(cidade, grafo, "cep15", 'S', 150.0, &x, &y, idVertice);

    TEST_ASSERT_EQUAL_INT(RESOLVE_ERRO, resultado);

    destruirGrafo(grafo);
    destruirCidade(cidade);
}

void test_resolverEndereco_NumAlemDaAltura_FaceLateral_RetornaErroSemAbortar(void){
    Cidade *cidade = montarCidadeComQuadra(); // altura=50 (faces L/O)
    Grafo *grafo = montarGrafoComVertices();

    double x, y;
    char idVertice[VERTICE_ID_MAX];
    int resultado = resolverEndereco(cidade, grafo, "cep15", 'L', 999.0, &x, &y, idVertice);

    TEST_ASSERT_EQUAL_INT(RESOLVE_ERRO, resultado);

    destruirGrafo(grafo);
    destruirCidade(cidade);
}

void test_resolverEndereco_NumNosLimitesExatos_RetornaOk(void){
    Cidade *cidade = montarCidadeComQuadra(); // largura=100

    Grafo *grafo = criarGrafo();
    grafoInserirVertice(grafo, criarVertice("v0",   0.0, 0.0));
    grafoInserirVertice(grafo, criarVertice("v100", 100.0, 0.0));

    double x, y;
    char idVertice[VERTICE_ID_MAX];

    int r1 = resolverEndereco(cidade, grafo, "cep15", 'S', 0.0, &x, &y, idVertice);
    TEST_ASSERT_EQUAL_INT(RESOLVE_OK, r1);

    int r2 = resolverEndereco(cidade, grafo, "cep15", 'S', 100.0, &x, &y, idVertice); // == largura exata
    TEST_ASSERT_EQUAL_INT(RESOLVE_OK, r2);

    destruirGrafo(grafo);
    destruirCidade(cidade);
}

// Grafo vazio: caso de borda, sem "mais próximo" possível

void test_resolverEndereco_GrafoVazio_RetornaErro(void){
    Cidade *cidade = montarCidadeComQuadra();
    Grafo *grafo = criarGrafo(); // sem nenhum vértice

    double x, y;
    char idVertice[VERTICE_ID_MAX];
    int resultado = resolverEndereco(cidade, grafo, "cep15", 'S', 30.0, &x, &y, idVertice);

    TEST_ASSERT_EQUAL_INT(RESOLVE_ERRO, resultado);

    destruirGrafo(grafo);
    destruirCidade(cidade);
}

int main(void){
    UNITY_BEGIN();

    RUN_TEST(test_resolverEndereco_EnderecoValido_RetornaOkComCoordenadaCorreta);
    RUN_TEST(test_resolverEndereco_EscolheVerticeMaisProximoPorDistanciaReal);
    RUN_TEST(test_resolverEndereco_FaceLateral_NumValido_RetornaOk);
    RUN_TEST(test_resolverEndereco_CepInexistente_RetornaErro);
    RUN_TEST(test_resolverEndereco_FaceInvalida_RetornaErro);
    RUN_TEST(test_resolverEndereco_NumNegativo_RetornaErroSemAbortar);
    RUN_TEST(test_resolverEndereco_NumAlemDaLargura_RetornaErroSemAbortar);
    RUN_TEST(test_resolverEndereco_NumAlemDaAltura_FaceLateral_RetornaErroSemAbortar);
    RUN_TEST(test_resolverEndereco_NumNosLimitesExatos_RetornaOk);
    RUN_TEST(test_resolverEndereco_GrafoVazio_RetornaErro);

    return UNITY_END();
}