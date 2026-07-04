#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grafo.h"
#include "vertice.h"
#include "unity.h"

// Sem arquivos em disco então setUp e tearDown sem estado global neste módulo.
void setUp(void){}
void tearDown(void){}

// Só para não ficar repetindo a montagem em cada teste! Cria um grafo
// com 3 vértices (v1, v2, v3), sem arestas - cada teste que precisa de
// arestas as adiciona conforme o cenário. 
static Grafo* grafoComTresVertices(void){
    Grafo *g = criarGrafo();
    grafoInserirVertice(g, criarVertice("v1", 0.0,  0.0));
    grafoInserirVertice(g, criarVertice("v2", 10.0, 0.0));
    grafoInserirVertice(g, criarVertice("v3", 10.0, 10.0));
    return g;
}

// Visitante contador simples, reusado por vários testes que só precisam saber quantas arestas foram visitadas, sem distinguir quais.
static void contadorAresta(Aresta *a, void *contexto){
    (void) a;
    int *contador = (int*) contexto;
    (*contador)++;
}

// Criação e destruição 

void test_criarGrafo_DeveRetornarPonteiroValido(void){
    Grafo *g = criarGrafo();
    TEST_ASSERT_NOT_NULL(g);
    destruirGrafo(g);
}

void test_destruirGrafo_GrafoVazio_naoQuebrar(void){
    Grafo *g = criarGrafo();
    TEST_ASSERT_NOT_NULL(g);
    destruirGrafo(g);
}

/*
Ownership: vértices e arestas liberados sem double-free 
Não chamamos destruirVertice em nenhum teste deste bloco - a hash
interna do grafo é a dona dos vértices (ver nota de ownership do
grafo.h). 
*/

void test_destruirGrafo_LiberaVerticesEArestasSemDoubleFree(void){
    Grafo *g = grafoComTresVertices();

    TEST_ASSERT_EQUAL_INT(GRAFO_OK, grafoInserirAresta(g, "v1", "v2", "cep01", "-", 100.0, 10.0, "Rua_A"));
    TEST_ASSERT_EQUAL_INT(GRAFO_OK, grafoInserirAresta(g, "v1", "v2", "-", "cep02", 100.0, 8.0, "Rua_A"));
    TEST_ASSERT_EQUAL_INT(GRAFO_OK, grafoInserirAresta(g, "v2", "v3", "-", "-", 50.0, 5.0, "Rua_B"));
    TEST_ASSERT_EQUAL_INT(GRAFO_OK, grafoInserirAresta(g, "v3", "v1", "-", "-", 70.0, 6.0, "Rua_C"));

    destruirGrafo(g); // se isto vazar ou fizer double-free, Valgrind denuncia
}

// Inserir / buscar / contém vértice 

void test_inserirEBuscarVertice_DevolveOMesmoVertice(void){
    Grafo *g = criarGrafo();
    Vertice *v = criarVertice("v1", 5.0, 7.0);

    grafoInserirVertice(g, v);

    Vertice *encontrado = buscarVertice(g, "v1");
    TEST_ASSERT_EQUAL_PTR(v, encontrado);
    TEST_ASSERT_EQUAL_STRING("v1", getVerticeId(encontrado));

    destruirGrafo(g);
}

void test_buscarVertice_IdAusente_RetornaNull(void){
    Grafo *g = grafoComTresVertices();

    TEST_ASSERT_NULL(buscarVertice(g, "naoexiste"));

    destruirGrafo(g);
}

void test_grafoContemVertice_AntesDeInserir_RetornaErro(void){
    Grafo *g = criarGrafo();

    TEST_ASSERT_EQUAL_INT(GRAFO_ERRO, grafoContemVertice(g, "v1"));

    destruirGrafo(g);
}

void test_grafoContemVertice_DepoisDeInserir_RetornaOk(void){
    Grafo *g = grafoComTresVertices();

    TEST_ASSERT_EQUAL_INT(GRAFO_OK, grafoContemVertice(g, "v1"));
    TEST_ASSERT_EQUAL_INT(GRAFO_OK, grafoContemVertice(g, "v3"));

    destruirGrafo(g);
}

// Número de vértices

void test_grafoNumVertices_GrafoVazio_RetornaZero(void){
    Grafo *g = criarGrafo();

    TEST_ASSERT_EQUAL_INT(0, grafoNumVertices(g));

    destruirGrafo(g);
}

void test_grafoNumVertices_AposInsercoes_RetornaContagemCorreta(void){
    Grafo *g = grafoComTresVertices();

    TEST_ASSERT_EQUAL_INT(3, grafoNumVertices(g));

    destruirGrafo(g);
}

// Inserir aresta: caminho feliz e getters 

void test_inserirAresta_ValoresValidos_RetornaOk(void){
    Grafo *g = grafoComTresVertices();

    int resultado = grafoInserirAresta(g, "v1", "v2", "cep01", "-", 100.0, 12.5, "Rua_Belo_Horizonte");
    TEST_ASSERT_EQUAL_INT(GRAFO_OK, resultado);

    destruirGrafo(g);
}

// Contexto simples para capturar a única aresta esperada de um vértice,
// usado nos testes de getters (mais direto que o padrão de flags,
// já que aqui só há uma aresta de saída).
typedef struct {
    Aresta *capturada;
} ContextoCapturaUnica;

static void visitanteCapturaUnica(Aresta *a, void *contexto){
    ContextoCapturaUnica *ctx = (ContextoCapturaUnica*) contexto;
    ctx->capturada = a;
}

void test_getters_RetornamValoresCorretos(void){
    Grafo *g = grafoComTresVertices();

    grafoInserirAresta(g, "v1", "v2", "cep01", "-", 100.0, 12.5, "Rua1");

    ContextoCapturaUnica ctx = { NULL };
    percorrerArestasSaindo(g, "v1", visitanteCapturaUnica, &ctx);
    TEST_ASSERT_NOT_NULL(ctx.capturada);

    TEST_ASSERT_EQUAL_STRING("Rua1", getArestaNome(ctx.capturada));
    TEST_ASSERT_EQUAL_STRING("cep01", getArestaLdir(ctx.capturada));
    TEST_ASSERT_EQUAL_STRING("-", getArestaLesq(ctx.capturada)); // convenção de ausência
    TEST_ASSERT_FLOAT_WITHIN(0.001, 100.0, getArestaCmp(ctx.capturada));
    TEST_ASSERT_FLOAT_WITHIN(0.001, 12.5, getArestaVm(ctx.capturada));

    destruirGrafo(g);
}

// o Vertice* devolvido por getArestaDestino tem que ser o mesmo objeto que está na hash interna
// (ponteiro emprestado, não uma cópia) - senão buscarVertice e
// getArestaDestino veriam vértices "diferentes" para o mesmo id.
void test_getArestaDestino_ApontaParaOMesmoVerticeDaHash(void){
    Grafo *g = grafoComTresVertices();

    grafoInserirAresta(g, "v1", "v2", "-", "-", 50.0, 10.0, "Rua_X");

    ContextoCapturaUnica ctx = { NULL };
    percorrerArestasSaindo(g, "v1", visitanteCapturaUnica, &ctx);
    TEST_ASSERT_NOT_NULL(ctx.capturada);

    Vertice *destinoViaAresta = getArestaDestino(ctx.capturada);
    Vertice *destinoViaHash = buscarVertice(g, "v2");

    TEST_ASSERT_EQUAL_PTR(destinoViaHash, destinoViaAresta);

    destruirGrafo(g);
}

// Inserir aresta: valores inválidos (cmp/vm negativos)

void test_inserirAresta_CmpNegativo_RetornaErroENaoInsere(void){
    Grafo *g = grafoComTresVertices();

    int resultado = grafoInserirAresta(g, "v1", "v2", "-", "-", -5.0, 10.0, "Rua_Invalida");
    TEST_ASSERT_EQUAL_INT(GRAFO_ERRO, resultado);

    // Confirma que nada foi inserido - não basta o retorno estar certo,
    // a lista de adjacência de v1 tem que continuar vazia.
    int contador = 0;
    percorrerArestasSaindo(g, "v1", contadorAresta, &contador);
    TEST_ASSERT_EQUAL_INT(0, contador);

    destruirGrafo(g);
}

void test_inserirAresta_VmNegativo_RetornaErroENaoInsere(void){
    Grafo *g = grafoComTresVertices();

    int resultado = grafoInserirAresta(g, "v1", "v2", "-", "-", 10.0, -3.0, "Rua_Invalida");
    TEST_ASSERT_EQUAL_INT(GRAFO_ERRO, resultado);

    destruirGrafo(g);
}

// Arestas paralelas

void test_arestasParalelas_AmbasSaoInseridasEVisitadas(void){
    Grafo *g = grafoComTresVertices();

    grafoInserirAresta(g, "v1", "v2", "-", "-", 100.0, 10.0, "Rua_A");
    grafoInserirAresta(g, "v1", "v2", "-", "-", 100.0, 8.0, "Rua_A_contramao");

    int contador = 0;
    percorrerArestasSaindo(g, "v1", contadorAresta, &contador);

    TEST_ASSERT_EQUAL_INT(2, contador); // ambas contam, nenhuma é descartada

    destruirGrafo(g);
}

// percorrerVertices: visita cada vértice exatamente uma vez

#define QTD_VERTICES_TESTE 20

typedef struct {
    int flags[QTD_VERTICES_TESTE];
    int totalVisitas;
} ContextoVarreduraVertices;

static void visitanteMarcaIndiceVertice(Vertice *v, void *contexto){
    ContextoVarreduraVertices *ctx = (ContextoVarreduraVertices*) contexto;

    int indice;
    sscanf(getVerticeId(v), "v%d", &indice);

    TEST_ASSERT_TRUE(indice >= 0 && indice < QTD_VERTICES_TESTE);
    ctx->flags[indice]++;
    ctx->totalVisitas++;
}

void test_percorrerVertices_VisitaCadaVerticeExatamenteUmaVez(void){
    Grafo *g = criarGrafo();

    char id[16];
    for(int i = 0; i < QTD_VERTICES_TESTE; i++){
        snprintf(id, sizeof(id), "v%d", i);
        grafoInserirVertice(g, criarVertice(id, (double) i, (double) i));
    }

    ContextoVarreduraVertices ctx;
    memset(ctx.flags, 0, sizeof(ctx.flags));
    ctx.totalVisitas = 0;

    percorrerVertices(g, visitanteMarcaIndiceVertice, &ctx);

    TEST_ASSERT_EQUAL_INT(grafoNumVertices(g), ctx.totalVisitas);
    for(int i = 0; i < QTD_VERTICES_TESTE; i++){
        TEST_ASSERT_EQUAL_INT(1, ctx.flags[i]); // nem 0 (faltou), nem 2+ (duplicou)
    }

    destruirGrafo(g);
}

static void contadorVertice(Vertice *v, void *contexto){
    (void) v;
    int *contador = (int*) contexto;
    (*contador)++;
}

void test_percorrerVertices_GrafoVazio_NuncaChamaVisitante(void){
    Grafo *g = criarGrafo();

    int contador = 0;
    percorrerVertices(g, contadorVertice, &contador);

    TEST_ASSERT_EQUAL_INT(0, contador);

    destruirGrafo(g);
}

// percorrerArestasSaindo: visita todas de um vértice, nenhuma de outro

#define QTD_ARESTAS_TESTE 5

typedef struct {
    int flags[QTD_ARESTAS_TESTE];
    int totalVisitas;
} ContextoVarreduraArestas;

static void visitanteMarcaIndiceDestino(Aresta *a, void *contexto){
    ContextoVarreduraArestas *ctx = (ContextoVarreduraArestas*) contexto;

    Vertice *destino = getArestaDestino(a);
    int indice;
    sscanf(getVerticeId(destino), "d%d", &indice);

    TEST_ASSERT_TRUE(indice >= 0 && indice < QTD_ARESTAS_TESTE);
    ctx->flags[indice]++;
    ctx->totalVisitas++;
}

void test_percorrerArestasSaindo_VisitaCadaArestaExatamenteUmaVez(void){
    Grafo *g = criarGrafo();
    grafoInserirVertice(g, criarVertice("origem", 0.0, 0.0));

    char idDestino[16];
    for(int i = 0; i < QTD_ARESTAS_TESTE; i++){
        snprintf(idDestino, sizeof(idDestino), "d%d", i);
        grafoInserirVertice(g, criarVertice(idDestino, (double) i, 0.0));
        grafoInserirAresta(g, "origem", idDestino, "-", "-", 10.0, 5.0, "Rua_Teste");
    }

    ContextoVarreduraArestas ctx;
    memset(ctx.flags, 0, sizeof(ctx.flags));
    ctx.totalVisitas = 0;

    percorrerArestasSaindo(g, "origem", visitanteMarcaIndiceDestino, &ctx);

    TEST_ASSERT_EQUAL_INT(QTD_ARESTAS_TESTE, ctx.totalVisitas);
    for(int i = 0; i < QTD_ARESTAS_TESTE; i++){
        TEST_ASSERT_EQUAL_INT(1, ctx.flags[i]);
    }

    destruirGrafo(g);
}

// Vértice existe no grafo mas não tem nenhuma aresta de saída - visitante nunca é chamado
void test_percorrerArestasSaindo_VerticeSemArestas_NuncaChamaVisitante(void){
    Grafo *g = grafoComTresVertices(); // v3 não tem nenhuma aresta de saída aqui

    int contador = 0;
    percorrerArestasSaindo(g, "v3", contadorAresta, &contador);

    TEST_ASSERT_EQUAL_INT(0, contador);

    destruirGrafo(g);
}

/* 
Vértice não existe no grafo - comportamento documentado como
equivalente a "existe mas sem arestas": visitante nunca é chamado,
sem abortar o programa.
*/

void test_percorrerArestasSaindo_VerticeInexistente_NuncaChamaVisitante(void){
    Grafo *g = grafoComTresVertices();

    int contador = 0;
    percorrerArestasSaindo(g, "naoexiste", contadorAresta, &contador);

    TEST_ASSERT_EQUAL_INT(0, contador);

    destruirGrafo(g);
}

// Arestas de um vértice não vazam para a varredura de outro.
void test_percorrerArestasSaindo_NaoVazaArestasDeOutroVertice(void){
    Grafo *g = grafoComTresVertices();

    grafoInserirAresta(g, "v1", "v2", "-", "-", 10.0, 5.0, "Rua_A");
    grafoInserirAresta(g, "v1", "v3", "-", "-", 10.0, 5.0, "Rua_B");
    grafoInserirAresta(g, "v2", "v3", "-", "-", 10.0, 5.0, "Rua_C"); // pertence só a v2

    int contadorV1 = 0;
    percorrerArestasSaindo(g, "v1", contadorAresta, &contadorV1);
    TEST_ASSERT_EQUAL_INT(2, contadorV1); // só as duas de v1, não a de v2

    int contadorV2 = 0;
    percorrerArestasSaindo(g, "v2", contadorAresta, &contadorV2);
    TEST_ASSERT_EQUAL_INT(1, contadorV2);

    destruirGrafo(g);
}

int main(void){
    UNITY_BEGIN();

    RUN_TEST(test_criarGrafo_DeveRetornarPonteiroValido);
    RUN_TEST(test_destruirGrafo_GrafoVazio_naoQuebrar);
    RUN_TEST(test_destruirGrafo_LiberaVerticesEArestasSemDoubleFree);
    RUN_TEST(test_inserirEBuscarVertice_DevolveOMesmoVertice);
    RUN_TEST(test_buscarVertice_IdAusente_RetornaNull);
    RUN_TEST(test_grafoContemVertice_AntesDeInserir_RetornaErro);
    RUN_TEST(test_grafoContemVertice_DepoisDeInserir_RetornaOk);
    RUN_TEST(test_grafoNumVertices_GrafoVazio_RetornaZero);
    RUN_TEST(test_grafoNumVertices_AposInsercoes_RetornaContagemCorreta);
    RUN_TEST(test_inserirAresta_ValoresValidos_RetornaOk);
    RUN_TEST(test_getters_RetornamValoresCorretos);
    RUN_TEST(test_getArestaDestino_ApontaParaOMesmoVerticeDaHash);
    RUN_TEST(test_inserirAresta_CmpNegativo_RetornaErroENaoInsere);
    RUN_TEST(test_inserirAresta_VmNegativo_RetornaErroENaoInsere);
    RUN_TEST(test_arestasParalelas_AmbasSaoInseridasEVisitadas);
    RUN_TEST(test_percorrerVertices_VisitaCadaVerticeExatamenteUmaVez);
    RUN_TEST(test_percorrerVertices_GrafoVazio_NuncaChamaVisitante);
    RUN_TEST(test_percorrerArestasSaindo_VisitaCadaArestaExatamenteUmaVez);
    RUN_TEST(test_percorrerArestasSaindo_VerticeSemArestas_NuncaChamaVisitante);
    RUN_TEST(test_percorrerArestasSaindo_VerticeInexistente_NuncaChamaVisitante);
    RUN_TEST(test_percorrerArestasSaindo_NaoVazaArestasDeOutroVertice);

    return UNITY_END();
}