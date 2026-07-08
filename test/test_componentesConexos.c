#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "componentesConexos.h"
#include "grafo.h"
#include "vertice.h"
#include "unity.h"

// Sem estado global neste módulo - cada teste monta seu próprio grafo, então setUp/tearDown ficam vazios.
void setUp(void){}
void tearDown(void){}

typedef struct {
    const char *idProcurado;
    int encontrado;
} ContextoProcurarId;

static void visitanteProcurarId(Vertice *v, void *contexto){
    ContextoProcurarId *ctx = (ContextoProcurarId*) contexto;
    if(strcmp(getVerticeId(v), ctx->idProcurado) == 0){
        ctx->encontrado = 1;
    }
}

static int componenteContemId(const Componentes *c, int indice, const char *id){
    ContextoProcurarId ctx = { id, 0 };
    percorrerVerticesDoComponente(c, indice, visitanteProcurarId, &ctx);
    return ctx.encontrado;
}

// Retorna o índice do componente que contém o id dado, ou -1 se
// nenhum componente o contiver (não deveria acontecer para um id
// válido do grafo original).
static int indiceDoComponenteContendo(const Componentes *c, const char *id){
    int n = componentesNumComponentes(c);
    for(int i = 0; i < n; i++){
        if(componenteContemId(c, i, id)) return i;
    }
    return -1;
}

static void contadorVertice(Vertice *v, void *contexto){
    (void) v;
    int *contador = (int*) contexto;
    (*contador)++;
}

static int tamanhoComponente(const Componentes *c, int indice){
    int contador = 0;
    percorrerVerticesDoComponente(c, indice, contadorVertice, &contador);
    return contador;
}

// Helpers de montagem de grafo 

// a -> b -> c, tudo com vm alto - um único componente conectado.
static Grafo* montarGrafoConectado(void){
    Grafo *g = criarGrafo();
    grafoInserirVertice(g, criarVertice("a", 0.0, 0.0));
    grafoInserirVertice(g, criarVertice("b", 10.0, 0.0));
    grafoInserirVertice(g, criarVertice("c", 20.0, 0.0));

    grafoInserirAresta(g, "a", "b", "-", "-", 10.0, 10.0, "Rua_A");
    grafoInserirAresta(g, "b", "c", "-", "-", 10.0, 10.0, "Rua_B");

    return g;
}

// Duas ilhas desconectadas: {a,b} e {c,d}, sem nenhuma aresta entre os dois grupos.
static Grafo* montarGrafoDuasIlhas(void){
    Grafo *g = criarGrafo();
    grafoInserirVertice(g, criarVertice("a", 0.0, 0.0));
    grafoInserirVertice(g, criarVertice("b", 10.0, 0.0));
    grafoInserirVertice(g, criarVertice("c", 100.0, 100.0));
    grafoInserirVertice(g, criarVertice("d", 110.0, 100.0));

    grafoInserirAresta(g, "a", "b", "-", "-", 10.0, 10.0, "Rua_A");
    grafoInserirAresta(g, "c", "d", "-", "-", 10.0, 10.0, "Rua_B");

    return g;
}

// a-b conectados; c totalmente isolado (nenhuma aresta).
static Grafo* montarGrafoComVerticeIsolado(void){
    Grafo *g = criarGrafo();
    grafoInserirVertice(g, criarVertice("a", 0.0, 0.0));
    grafoInserirVertice(g, criarVertice("b", 10.0, 0.0));
    grafoInserirVertice(g, criarVertice("c", 100.0, 100.0));

    grafoInserirAresta(g, "a", "b", "-", "-", 10.0, 10.0, "Rua_A");

    return g;
}

// a-b com vm alto (bom); b-c com vm baixo (insuficiente, dependendo de
// vl). Usado para confirmar que o filtro de vl fragmenta componentes.
static Grafo* montarGrafoParaFiltroVl(void){
    Grafo *g = criarGrafo();
    grafoInserirVertice(g, criarVertice("a", 0.0, 0.0));
    grafoInserirVertice(g, criarVertice("b", 10.0, 0.0));
    grafoInserirVertice(g, criarVertice("c", 20.0, 0.0));

    grafoInserirAresta(g, "a", "b", "-", "-", 10.0, 10.0, "Rua_Boa");
    grafoInserirAresta(g, "b", "c", "-", "-", 10.0,  2.0, "Rua_Ruim");

    return g;
}

// APENAS uma aresta direcionada a->b (sem a reversa b->a) - o teste-
// chave da decisão de tratar o grafo como não-direcionado.
static Grafo* montarGrafoMaoUnica(void){
    Grafo *g = criarGrafo();
    grafoInserirVertice(g, criarVertice("a", 0.0, 0.0));
    grafoInserirVertice(g, criarVertice("b", 10.0, 0.0));

    grafoInserirAresta(g, "a", "b", "-", "-", 10.0, 10.0, "Rua_MaoUnica");

    return g;
}

void test_calcularComponentesConexos_GrafoConectado_UmComponente(void){
    Grafo *g = montarGrafoConectado();

    Componentes *c = calcularComponentesConexos(g, 0.0);
    TEST_ASSERT_NOT_NULL(c);

    TEST_ASSERT_EQUAL_INT(1, componentesNumComponentes(c));

    int indice = indiceDoComponenteContendo(c, "a");
    TEST_ASSERT_TRUE(indice >= 0);
    TEST_ASSERT_TRUE(componenteContemId(c, indice, "b"));
    TEST_ASSERT_TRUE(componenteContemId(c, indice, "c"));
    TEST_ASSERT_EQUAL_INT(3, tamanhoComponente(c, indice));

    destruirComponentes(c);
    destruirGrafo(g);
}

void test_calcularComponentesConexos_DuasIlhas_DoisComponentes(void){
    Grafo *g = montarGrafoDuasIlhas();

    Componentes *c = calcularComponentesConexos(g, 0.0);
    TEST_ASSERT_NOT_NULL(c);

    TEST_ASSERT_EQUAL_INT(2, componentesNumComponentes(c));

    int indiceA = indiceDoComponenteContendo(c, "a");
    int indiceC = indiceDoComponenteContendo(c, "c");

    TEST_ASSERT_TRUE(indiceA >= 0);
    TEST_ASSERT_TRUE(indiceC >= 0);
    TEST_ASSERT_NOT_EQUAL(indiceA, indiceC); // componentes DIFERENTES

    TEST_ASSERT_TRUE(componenteContemId(c, indiceA, "b")); // a,b juntos
    TEST_ASSERT_FALSE(componenteContemId(c, indiceA, "c")); // c não está com a
    TEST_ASSERT_FALSE(componenteContemId(c, indiceA, "d")); // d não está com a

    TEST_ASSERT_TRUE(componenteContemId(c, indiceC, "d")); // c,d juntos

    destruirComponentes(c);
    destruirGrafo(g);
}

void test_calcularComponentesConexos_GrafoVazio_ZeroComponentes(void){
    Grafo *g = criarGrafo(); // nenhum vértice

    Componentes *c = calcularComponentesConexos(g, 0.0);
    TEST_ASSERT_NOT_NULL(c);

    TEST_ASSERT_EQUAL_INT(0, componentesNumComponentes(c));

    destruirComponentes(c);
    destruirGrafo(g);
}

void test_calcularComponentesConexos_VerticeIsolado_FormaComponenteProprio(void){
    Grafo *g = montarGrafoComVerticeIsolado();

    Componentes *c = calcularComponentesConexos(g, 0.0);
    TEST_ASSERT_NOT_NULL(c);

    TEST_ASSERT_EQUAL_INT(2, componentesNumComponentes(c));

    int indiceA = indiceDoComponenteContendo(c, "a");
    int indiceIsolado = indiceDoComponenteContendo(c, "c");

    TEST_ASSERT_NOT_EQUAL(indiceA, indiceIsolado);
    TEST_ASSERT_EQUAL_INT(2, tamanhoComponente(c, indiceA)); // {a,b}
    TEST_ASSERT_EQUAL_INT(1, tamanhoComponente(c, indiceIsolado)); // {c}, sozinho

    destruirComponentes(c);
    destruirGrafo(g);
}

// Filtro por vl: fragmenta componentes com trechos "insuficientes"

void test_calcularComponentesConexos_FiltroVl_FragmentaComponentesComVmBaixo(void){
    Grafo *g = montarGrafoParaFiltroVl(); // a-b vm=10, b-c vm=2

    // vl baixo: os dois trechos são suficientes -> 1 componente {a,b,c}.
    Componentes *cBaixo = calcularComponentesConexos(g, 1.0);
    TEST_ASSERT_NOT_NULL(cBaixo);
    TEST_ASSERT_EQUAL_INT(1, componentesNumComponentes(cBaixo));
    destruirComponentes(cBaixo);

    // vl alto: só a-b é suficiente (vm=10>=5), b-c não (vm=2<5) -> 2 componentes: {a,b} e {c}.
    Componentes *cAlto = calcularComponentesConexos(g, 5.0);
    TEST_ASSERT_NOT_NULL(cAlto);
    TEST_ASSERT_EQUAL_INT(2, componentesNumComponentes(cAlto));

    int indiceA = indiceDoComponenteContendo(cAlto, "a");
    int indiceC = indiceDoComponenteContendo(cAlto, "c");
    TEST_ASSERT_NOT_EQUAL(indiceA, indiceC);
    TEST_ASSERT_TRUE(componenteContemId(cAlto, indiceA, "b"));

    destruirComponentes(cAlto);
    destruirGrafo(g);
}

void test_calcularComponentesConexos_ArestaMaoUnica_AindaConectaOsDoisVertices(void){
    Grafo *g = montarGrafoMaoUnica();

    Componentes *c = calcularComponentesConexos(g, 0.0);
    TEST_ASSERT_NOT_NULL(c);

    TEST_ASSERT_EQUAL_INT(1, componentesNumComponentes(c)); // não 2!

    int indice = indiceDoComponenteContendo(c, "a");
    TEST_ASSERT_TRUE(componenteContemId(c, indice, "b"));

    destruirComponentes(c);
    destruirGrafo(g);
}

void test_calcularComponentesConexos_TodosOsVerticesPertencemAUmComponente(void){
    Grafo *g = montarGrafoDuasIlhas(); // 4 vértices, 2 componentes

    Componentes *c = calcularComponentesConexos(g, 0.0);
    TEST_ASSERT_NOT_NULL(c);

    int somaTamanhos = 0;
    int n = componentesNumComponentes(c);
    for(int i = 0; i < n; i++){
        somaTamanhos += tamanhoComponente(c, i);
    }

    TEST_ASSERT_EQUAL_INT(grafoNumVertices(g), somaTamanhos);

    destruirComponentes(c);
    destruirGrafo(g);
}

void test_destruirComponentes_naoTocaNosVerticesDoGrafo(void){
    Grafo *g = montarGrafoConectado();

    Componentes *c = calcularComponentesConexos(g, 0.0);
    TEST_ASSERT_NOT_NULL(c);

    destruirComponentes(c); // se isso tocasse nos Vertice*, o destruirGrafo abaixo faria double-free
    destruirGrafo(g);
}

int main(void){
    UNITY_BEGIN();

    RUN_TEST(test_calcularComponentesConexos_GrafoConectado_UmComponente);
    RUN_TEST(test_calcularComponentesConexos_DuasIlhas_DoisComponentes);
    RUN_TEST(test_calcularComponentesConexos_GrafoVazio_ZeroComponentes);
    RUN_TEST(test_calcularComponentesConexos_VerticeIsolado_FormaComponenteProprio);
    RUN_TEST(test_calcularComponentesConexos_FiltroVl_FragmentaComponentesComVmBaixo);
    RUN_TEST(test_calcularComponentesConexos_ArestaMaoUnica_AindaConectaOsDoisVertices);
    RUN_TEST(test_calcularComponentesConexos_TodosOsVerticesPertencemAUmComponente);
    RUN_TEST(test_destruirComponentes_naoTocaNosVerticesDoGrafo);

    return UNITY_END();
}