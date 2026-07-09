#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "leitorVia.h"
#include "grafo.h"
#include "vertice.h"
#include "unity.h"

// Sem estado global neste módulo - cada teste cria/remove seu próprio
// .via temporário, então setUp/tearDown ficam vazios.
void setUp(void){}
void tearDown(void){}

// Caminho de arquivo temporário usado pelos testes. 
#define ARQ_TMP "teste_via_tmp.via"

// Helper de escrita

static void escreverViaTemporario(const char *conteudo){
    FILE *fp = fopen(ARQ_TMP, "w");
    TEST_ASSERT_NOT_NULL(fp);
    fputs(conteudo, fp);
    fclose(fp);
}

// Helper para percorrerArestaSaindo

// Captura a única aresta esperada de um vértice - usado nos testes de
//getters, onde só há uma aresta de saída por vez.
typedef struct {
    Aresta *capturada;
} ContextoCapturaUnica;

static void visitanteCapturaUnica(Aresta *a, void *contexto){
    ContextoCapturaUnica *ctx = (ContextoCapturaUnica*) contexto;
    ctx->capturada = a;
}

// Conta quantas arestas foram visitadas - usado nos testes onde só
// importa "entrou ou não", sem inspecionar atributos.
static void contadorAresta(Aresta *a, void *contexto){
    (void) a;
    int *contador = (int*) contexto;
    (*contador)++;
}

// Arquivo válido

void test_lerArquivoVia_ArquivoValido_RetornaOk(void){
    escreverViaTemporario(
        "3\n"
        "v v1 10.0 10.0\n"
        "v v2 110.0 10.0\n"
        "v v3 110.0 70.0\n"
        "e v1 v2 cep1 - 70.0 3.5 Rua1\n"
    );

    Grafo *g = criarGrafo();
    TEST_ASSERT_NOT_NULL(g);

    int resultado = lerArquivoVia(ARQ_TMP, g);
    TEST_ASSERT_EQUAL_INT(VIA_OK, resultado);
    TEST_ASSERT_EQUAL_INT(3, grafoNumVertices(g));

    destruirGrafo(g);
    remove(ARQ_TMP);
}

void test_lerArquivoVia_VerticesEArestasComAtributosCorretos(void){
    escreverViaTemporario(
        "3\n"
        "v v1 10.0 10.0\n"
        "v v2 110.0 10.0\n"
        "v v3 110.0 70.0\n"
        "e v1 v2 cep1 - 70.0 3.5 Rua1\n"
    );

    Grafo *g = criarGrafo();
    TEST_ASSERT_NOT_NULL(g);
    lerArquivoVia(ARQ_TMP, g);

    // Coordenadas do vértice.
    Vertice *v2 = buscarVertice(g, "v2");
    TEST_ASSERT_NOT_NULL(v2);
    TEST_ASSERT_FLOAT_WITHIN(0.001, 110.0, getVerticeX(v2));
    TEST_ASSERT_FLOAT_WITHIN(0.001, 10.0, getVerticeY(v2));

    // Atributos da aresta.
    ContextoCapturaUnica ctx = { NULL };
    percorrerArestasSaindo(g, "v1", visitanteCapturaUnica, &ctx);
    TEST_ASSERT_NOT_NULL(ctx.capturada);

    TEST_ASSERT_EQUAL_STRING("v2", getVerticeId(getArestaDestino(ctx.capturada)));
    TEST_ASSERT_EQUAL_STRING("cep1", getArestaLdir(ctx.capturada));
    TEST_ASSERT_EQUAL_STRING("-", getArestaLesq(ctx.capturada));
    TEST_ASSERT_FLOAT_WITHIN(0.001, 70.0, getArestaCmp(ctx.capturada));
    TEST_ASSERT_FLOAT_WITHIN(0.001, 3.5, getArestaVm(ctx.capturada));
    TEST_ASSERT_EQUAL_STRING("Rua1", getArestaNome(ctx.capturada));

    destruirGrafo(g);
    remove(ARQ_TMP);
}

void test_lerArquivoVia_ArestaParaVerticeInexistente_DescartaSemAbortar(void){
    escreverViaTemporario(
        "2\n"
        "v v1 0.0 0.0\n"
        "v v2 10.0 0.0\n"
        "e v1 v99 - - 50.0 5.0 Rua_X\n" // v99 nunca declarado
    );

    Grafo *g = criarGrafo();
    TEST_ASSERT_NOT_NULL(g);

    int resultado = lerArquivoVia(ARQ_TMP, g);
    TEST_ASSERT_EQUAL_INT(VIA_OK, resultado); // abrir o arquivo funcionou

    TEST_ASSERT_EQUAL_INT(GRAFO_ERRO, grafoContemVertice(g, "v99")); // nunca criado

    int contador = 0;
    percorrerArestasSaindo(g, "v1", contadorAresta, &contador);
    TEST_ASSERT_EQUAL_INT(0, contador); // aresta não foi inserida

    destruirGrafo(g);
    remove(ARQ_TMP);
}

// Aresta com cmp/vm negativos: descartada, não aborta 

void test_lerArquivoVia_ArestaComCmpNegativo_DescartaSemAbortar(void){
    escreverViaTemporario(
        "2\n"
        "v v1 0.0 0.0\n"
        "v v2 10.0 0.0\n"
        "e v1 v2 - - -5.0 5.0 Rua_Invalida\n" // cmp negativo
    );

    Grafo *g = criarGrafo();
    TEST_ASSERT_NOT_NULL(g);

    int resultado = lerArquivoVia(ARQ_TMP, g);
    TEST_ASSERT_EQUAL_INT(VIA_OK, resultado);

    int contador = 0;
    percorrerArestasSaindo(g, "v1", contadorAresta, &contador);
    TEST_ASSERT_EQUAL_INT(0, contador);

    destruirGrafo(g);
    remove(ARQ_TMP);
}

void test_lerArquivoVia_ArestaComVmNegativo_DescartaSemAbortar(void){
    escreverViaTemporario(
        "2\n"
        "v v1 0.0 0.0\n"
        "v v2 10.0 0.0\n"
        "e v1 v2 - - 50.0 -3.0 Rua_Invalida\n" // vm negativo
    );

    Grafo *g = criarGrafo();
    TEST_ASSERT_NOT_NULL(g);

    int resultado = lerArquivoVia(ARQ_TMP, g);
    TEST_ASSERT_EQUAL_INT(VIA_OK, resultado);

    int contador = 0;
    percorrerArestasSaindo(g, "v1", contadorAresta, &contador);
    TEST_ASSERT_EQUAL_INT(0, contador);

    destruirGrafo(g);
    remove(ARQ_TMP);
}

// Vértice duplicado

void test_lerArquivoVia_VerticeDuplicado_MantemSoOPrimeiro(void){
    escreverViaTemporario(
        "2\n"
        "v v1 0.0 0.0\n"
        "v v1 99.0 99.0\n" // mesmo id, coordenadas diferentes
    );

    Grafo *g = criarGrafo();
    TEST_ASSERT_NOT_NULL(g);

    lerArquivoVia(ARQ_TMP, g);

    TEST_ASSERT_EQUAL_INT(1, grafoNumVertices(g));

    Vertice *v1 = buscarVertice(g, "v1");
    TEST_ASSERT_NOT_NULL(v1);
    // Confirma que ficou a PRIMEIRA versão, não a duplicata descartada.
    TEST_ASSERT_FLOAT_WITHIN(0.001, 0.0, getVerticeX(v1));
    TEST_ASSERT_FLOAT_WITHIN(0.001, 0.0, getVerticeY(v1));

    destruirGrafo(g);
    remove(ARQ_TMP);
}

void test_lerArquivoVia_LinhaMalformadaNoMeio_LinhasBoasEntram(void){
    escreverViaTemporario(
        "3\n"
        "v v1 0.0 0.0\n"
        "v v2 60.0\n"                  // malformada: falta o campo y
        "comandoDesconhecido x y z\n"  // comando inexistente
        "v v3 110.0 0.0\n"
    );

    Grafo *g = criarGrafo();
    TEST_ASSERT_NOT_NULL(g);

    int resultado = lerArquivoVia(ARQ_TMP, g);
    TEST_ASSERT_EQUAL_INT(VIA_OK, resultado);

    TEST_ASSERT_EQUAL_INT(GRAFO_OK, grafoContemVertice(g, "v1"));
    TEST_ASSERT_EQUAL_INT(GRAFO_ERRO, grafoContemVertice(g, "v2")); // malformada, descartada
    TEST_ASSERT_EQUAL_INT(GRAFO_OK, grafoContemVertice(g, "v3")); // leitura continuou

    TEST_ASSERT_EQUAL_INT(2, grafoNumVertices(g));

    destruirGrafo(g);
    remove(ARQ_TMP);
}

// Caminho inválido -> Padrão de erro

void test_lerArquivoVia_CaminhoInvalido_RetornaErro(void){
    Grafo *g = criarGrafo();
    TEST_ASSERT_NOT_NULL(g);

    int resultado = lerArquivoVia("/diretorio/que/definitivamente/nao/existe/x.via", g);
    TEST_ASSERT_EQUAL_INT(VIA_ERRO, resultado);
    TEST_ASSERT_EQUAL_INT(0, grafoNumVertices(g)); // nada foi inserido

    destruirGrafo(g);
}

// Hífen nos lados: getArestaLdir/Lesq devolvem "-" literalmente

void test_lerArquivoVia_HifenNosLados_GettersRetornamHifen(void){
    escreverViaTemporario(
        "2\n"
        "v v1 0.0 0.0\n"
        "v v2 10.0 0.0\n"
        "e v1 v2 - - 50.0 5.0 Rua_Sem_Quadra\n"
    );

    Grafo *g = criarGrafo();
    TEST_ASSERT_NOT_NULL(g);
    lerArquivoVia(ARQ_TMP, g);

    ContextoCapturaUnica ctx = { NULL };
    percorrerArestasSaindo(g, "v1", visitanteCapturaUnica, &ctx);
    TEST_ASSERT_NOT_NULL(ctx.capturada);

    TEST_ASSERT_EQUAL_STRING("-", getArestaLdir(ctx.capturada));
    TEST_ASSERT_EQUAL_STRING("-", getArestaLesq(ctx.capturada));

    destruirGrafo(g);
    remove(ARQ_TMP);
}

/*
Primeira linha não é um inteiro válido 
Comportamento documentado no .h: reporta e segue da linha 2, sem
tentar reinterpretar a linha 1 como v/e.
*/

void test_lerArquivoVia_PrimeiraLinhaNaoInteira_ContinuaLendoDaLinha2(void){
    escreverViaTemporario(
        "abc\n" // não é um inteiro
        "v v1 0.0 0.0\n"
        "v v2 10.0 0.0\n"
    );

    Grafo *g = criarGrafo();
    TEST_ASSERT_NOT_NULL(g);

    int resultado = lerArquivoVia(ARQ_TMP, g);
    TEST_ASSERT_EQUAL_INT(VIA_OK, resultado);
    TEST_ASSERT_EQUAL_INT(2, grafoNumVertices(g)); // v1 e v2 processados normalmente

    destruirGrafo(g);
    remove(ARQ_TMP);
}

void test_lerArquivoVia_LinhaEmBranco_EhIgnorada(void){
    escreverViaTemporario(
        "2\n"
        "\n"
        "v v1 0.0 0.0\n"
        "\n"
        "v v2 10.0 0.0\n"
    );

    Grafo *g = criarGrafo();
    TEST_ASSERT_NOT_NULL(g);

    int resultado = lerArquivoVia(ARQ_TMP, g);
    TEST_ASSERT_EQUAL_INT(VIA_OK, resultado);
    TEST_ASSERT_EQUAL_INT(2, grafoNumVertices(g));

    destruirGrafo(g);
    remove(ARQ_TMP);
}

int main(void){
    UNITY_BEGIN();

    RUN_TEST(test_lerArquivoVia_ArquivoValido_RetornaOk);
    RUN_TEST(test_lerArquivoVia_VerticesEArestasComAtributosCorretos);
    RUN_TEST(test_lerArquivoVia_ArestaParaVerticeInexistente_DescartaSemAbortar);
    RUN_TEST(test_lerArquivoVia_ArestaComCmpNegativo_DescartaSemAbortar);
    RUN_TEST(test_lerArquivoVia_ArestaComVmNegativo_DescartaSemAbortar);
    RUN_TEST(test_lerArquivoVia_VerticeDuplicado_MantemSoOPrimeiro);
    RUN_TEST(test_lerArquivoVia_LinhaMalformadaNoMeio_LinhasBoasEntram);
    RUN_TEST(test_lerArquivoVia_CaminhoInvalido_RetornaErro);
    RUN_TEST(test_lerArquivoVia_HifenNosLados_GettersRetornamHifen);
    RUN_TEST(test_lerArquivoVia_PrimeiraLinhaNaoInteira_ContinuaLendoDaLinha2);
    RUN_TEST(test_lerArquivoVia_LinhaEmBranco_EhIgnorada);

    return UNITY_END();
}