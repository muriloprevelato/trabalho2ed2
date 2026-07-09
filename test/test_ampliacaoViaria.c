#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ampliacaoViaria.h"
#include "grafo.h"
#include "vertice.h"
#include "unity.h"

// Sem estado global neste módulo - cada teste monta seu próprio grafo, então setUp/tearDown ficam vazios.
void setUp(void){}
void tearDown(void){}

typedef struct {
    int totalVisitas;
} ContextoContador;

static void visitanteContador(const char *idOrigem, Aresta *a, void *contexto){
    (void) idOrigem;
    (void) a;
    ContextoContador *ctx = (ContextoContador*) contexto;
    ctx->totalVisitas++;
}

static int contarArestasAmpliadas(const Ampliacao *a){
    ContextoContador ctx = { 0 };
    percorrerArestasAmpliadas(a, visitanteContador, &ctx);
    return ctx.totalVisitas;
}

// Verifica se a Ampliacao contém uma aresta específica (idOrigem -> idDestino) entre as selecionadas.
typedef struct {
    const char *idOrigemProcurado;
    const char *idDestinoProcurado;
    int encontrado;
    double vmEncontrado;
} ContextoProcurarAresta;

static void visitanteProcurarAresta(const char *idOrigem, Aresta *a, void *contexto){
    ContextoProcurarAresta *ctx = (ContextoProcurarAresta*) contexto;
    const char *idDestino = getVerticeId(getArestaDestino(a));

    if(strcmp(idOrigem, ctx->idOrigemProcurado) == 0 &&
       strcmp(idDestino, ctx->idDestinoProcurado) == 0){
        ctx->encontrado = 1;
        ctx->vmEncontrado = getArestaVm(a);
    }
}

static int ampliacaoContemAresta(const Ampliacao *a, const char *idOrigem, const char *idDestino){
    ContextoProcurarAresta ctx = { idOrigem, idDestino, 0, 0.0 };
    percorrerArestasAmpliadas(a, visitanteProcurarAresta, &ctx);
    return ctx.encontrado;
}

static Grafo* montarGrafoTriangulo(void){
    Grafo *g = criarGrafo();
    grafoInserirVertice(g, criarVertice("a", 0.0, 0.0));
    grafoInserirVertice(g, criarVertice("b", 10.0, 0.0));
    grafoInserirVertice(g, criarVertice("c", 10.0, 10.0));

    grafoInserirAresta(g, "a", "b", "-", "-", 10.0, 2.0, "Rua_AB");
    grafoInserirAresta(g, "b", "c", "-", "-", 20.0, 2.0, "Rua_BC");
    grafoInserirAresta(g, "a", "c", "-", "-", 30.0, 2.0, "Rua_AC");

    return g;
}

// Duas ilhas desconectadas: {a,b} (uma aresta) e {c,d} (uma aresta),
// sem nenhuma conexão entre os dois grupos. 
static Grafo* montarGrafoDuasIlhas(void){
    Grafo *g = criarGrafo();
    grafoInserirVertice(g, criarVertice("a", 0.0, 0.0));
    grafoInserirVertice(g, criarVertice("b", 10.0, 0.0));
    grafoInserirVertice(g, criarVertice("c", 100.0, 100.0));
    grafoInserirVertice(g, criarVertice("d", 110.0, 100.0));

    grafoInserirAresta(g, "a", "b", "-", "-", 10.0, 2.0, "Rua_A");
    grafoInserirAresta(g, "c", "d", "-", "-", 10.0, 2.0, "Rua_B");

    return g;
}

// Apenas UMA aresta direcionada a->b (sem a reversa b->a) - confirma
// que o Kruskal trata como conexão não-direcionada.
static Grafo* montarGrafoMaoUnica(void){
    Grafo *g = criarGrafo();
    grafoInserirVertice(g, criarVertice("a", 0.0, 0.0));
    grafoInserirVertice(g, criarVertice("b", 10.0, 0.0));
    grafoInserirAresta(g, "a", "b", "-", "-", 10.0, 2.0, "Rua_MaoUnica");

    return g;
}

// a-b com vm alto (não deveria ser selecionada pro upgrade).
static Grafo* montarGrafoVmAlto(void){
    Grafo *g = criarGrafo();
    grafoInserirVertice(g, criarVertice("a", 0.0, 0.0));
    grafoInserirVertice(g, criarVertice("b", 10.0, 0.0));

    grafoInserirAresta(g, "a", "b", "-", "-", 10.0, 50.0, "Rua_Rapida");

    return g;
}

// MST básica

void test_calcularAmpliacaoViaria_Triangulo_RejeitaArestaMaisCaraDoCiclo(void){
    Grafo *g = montarGrafoTriangulo();

    // vl alto o suficiente pra selecionar qualquer aresta da MST (todas têm vm=2.0).
    Ampliacao *a = calcularAmpliacaoViaria(g, 100.0);
    TEST_ASSERT_NOT_NULL(a);

    TEST_ASSERT_EQUAL_INT(2, ampliacaoNumArestas(a));
    TEST_ASSERT_TRUE(ampliacaoContemAresta(a, "a", "b"));
    TEST_ASSERT_TRUE(ampliacaoContemAresta(a, "b", "c"));
    TEST_ASSERT_FALSE(ampliacaoContemAresta(a, "a", "c")); // rejeitada

    destruirAmpliacao(a);
    destruirGrafo(g);
}

void test_calcularAmpliacaoViaria_GrafoDesconexo_ProduzFlorestaSemQuebrar(void){
    Grafo *g = montarGrafoDuasIlhas(); // V=4, C=2 (duas ilhas)

    Ampliacao *a = calcularAmpliacaoViaria(g, 100.0);
    TEST_ASSERT_NOT_NULL(a); // não deveria travar nem abortar

    TEST_ASSERT_EQUAL_INT(2, ampliacaoNumArestas(a));
    TEST_ASSERT_TRUE(ampliacaoContemAresta(a, "a", "b"));
    TEST_ASSERT_TRUE(ampliacaoContemAresta(a, "c", "d"));

    destruirAmpliacao(a);
    destruirGrafo(g);
}


void test_calcularAmpliacaoViaria_ArestaMaoUnica_ParticipaDaArvore(void){
    Grafo *g = montarGrafoMaoUnica(); // só a->b, sem b->a

    Ampliacao *a = calcularAmpliacaoViaria(g, 100.0);
    TEST_ASSERT_NOT_NULL(a);

    TEST_ASSERT_EQUAL_INT(1, ampliacaoNumArestas(a)); // V=2, C=1 -> V-C=1
    TEST_ASSERT_TRUE(ampliacaoContemAresta(a, "a", "b"));

    destruirAmpliacao(a);
    destruirGrafo(g);
}

void test_calcularAmpliacaoViaria_ArestaSelecionada_VmAumentaEmCinquentaPorCento(void){
    Grafo *g = montarGrafoMaoUnica(); // a->b, vm=2.0

    Ampliacao *a = calcularAmpliacaoViaria(g, 100.0); // vl alto, seleciona
    TEST_ASSERT_NOT_NULL(a);
    TEST_ASSERT_EQUAL_INT(1, ampliacaoNumArestas(a));

    ContextoProcurarAresta ctxBusca = { "a", "b", 0, 0.0 };
    percorrerArestasAmpliadas(a, visitanteProcurarAresta, &ctxBusca);

    TEST_ASSERT_TRUE(ctxBusca.encontrado);
    TEST_ASSERT_FLOAT_WITHIN(0.001, 3.0, ctxBusca.vmEncontrado);

    destruirAmpliacao(a);
    destruirGrafo(g);
}

void test_calcularAmpliacaoViaria_VlBaixo_NenhumaArestaSelecionada(void){
    Grafo *g = montarGrafoVmAlto(); // a-b, vm=50.0

    Ampliacao *a = calcularAmpliacaoViaria(g, 1.0); // vl muito baixo
    TEST_ASSERT_NOT_NULL(a);

    TEST_ASSERT_EQUAL_INT(0, ampliacaoNumArestas(a)); // resultado válido, vazio
    TEST_ASSERT_EQUAL_INT(0, contarArestasAmpliadas(a));

    // vm não deveria ter sido tocado, já que a aresta nem foi selecionada.
    ContextoProcurarAresta ctx = { "a", "b", 0, 0.0 };
    percorrerArestasAmpliadas(a, visitanteProcurarAresta, &ctx);
    TEST_ASSERT_FALSE(ctx.encontrado); // nem deveria aparecer no resultado

    destruirAmpliacao(a);
    destruirGrafo(g);
}

void test_calcularAmpliacaoViaria_GrafoVazio_ResultadoValidoSemArestas(void){
    Grafo *g = criarGrafo();

    Ampliacao *a = calcularAmpliacaoViaria(g, 10.0);
    TEST_ASSERT_NOT_NULL(a);
    TEST_ASSERT_EQUAL_INT(0, ampliacaoNumArestas(a));

    destruirAmpliacao(a);
    destruirGrafo(g);
}

int main(void){
    UNITY_BEGIN();

    RUN_TEST(test_calcularAmpliacaoViaria_Triangulo_RejeitaArestaMaisCaraDoCiclo);
    RUN_TEST(test_calcularAmpliacaoViaria_GrafoDesconexo_ProduzFlorestaSemQuebrar);
    RUN_TEST(test_calcularAmpliacaoViaria_ArestaMaoUnica_ParticipaDaArvore);
    RUN_TEST(test_calcularAmpliacaoViaria_ArestaSelecionada_VmAumentaEmCinquentaPorCento);
    RUN_TEST(test_calcularAmpliacaoViaria_VlBaixo_NenhumaArestaSelecionada);
    RUN_TEST(test_calcularAmpliacaoViaria_GrafoVazio_ResultadoValidoSemArestas);

    return UNITY_END();
}