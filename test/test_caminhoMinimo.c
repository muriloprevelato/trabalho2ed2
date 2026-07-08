#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "caminhoMinimo.h"
#include "grafo.h"
#include "vertice.h"
#include "unity.h"

// Sem estado global neste módulo - cada teste monta seu próprio grafo,
// então setUp/tearDown ficam vazios.
void setUp(void){}
void tearDown(void){}

// Grafo em linha: v0 -> v1 -> v2 -> v3 -> v4 (5 vértices, 4 saltos).
// Origem e destino claramente distintos - usado para confirmar que a
// reconstrução do caminho sai na ordem origem->destino, não invertida.
static Grafo* montarGrafoLinha(void){
    Grafo *g = criarGrafo();
    grafoInserirVertice(g, criarVertice("v0", 0.0,  0.0));
    grafoInserirVertice(g, criarVertice("v1", 10.0, 0.0));
    grafoInserirVertice(g, criarVertice("v2", 20.0, 0.0));
    grafoInserirVertice(g, criarVertice("v3", 30.0, 0.0));
    grafoInserirVertice(g, criarVertice("v4", 40.0, 0.0));

    grafoInserirAresta(g, "v0", "v1", "-", "-", 10.0, 5.0, "Rua_A");
    grafoInserirAresta(g, "v1", "v2", "-", "-", 10.0, 5.0, "Rua_A");
    grafoInserirAresta(g, "v2", "v3", "-", "-", 10.0, 5.0, "Rua_A");
    grafoInserirAresta(g, "v3", "v4", "-", "-", 10.0, 5.0, "Rua_A");

    return g;
}

// Dois vértices, sem nenhuma aresta entre eles - usado para o caso inalcançável.
static Grafo* montarGrafoDesconexo(void){
    Grafo *g = criarGrafo();
    grafoInserirVertice(g, criarVertice("a", 0.0, 0.0));
    grafoInserirVertice(g, criarVertice("b", 10.0, 0.0));
    return g;
}

static Grafo* montarGrafoComVmZero(void){
    Grafo *g = criarGrafo();
    grafoInserirVertice(g, criarVertice("x", 0.0, 0.0));
    grafoInserirVertice(g, criarVertice("y", 10.0, 0.0));
    grafoInserirVertice(g, criarVertice("z", 20.0, 0.0));

    grafoInserirAresta(g, "x", "y", "-", "-", 5.0, 0.0, "Rua_Bloqueada"); // vm=0
    grafoInserirAresta(g, "x", "z", "-", "-", 8.0, 4.0, "Rua_C");
    grafoInserirAresta(g, "z", "y", "-", "-", 8.0, 4.0, "Rua_D");

    return g;
}

// Rota direta a->b é rápida mas comprida (cmp=100, vm=50 -> tempo=2).
// Rota a->c->b é curta mas lenta (cmp=10+10=20, vm=1 -> tempo=10+10=20).
// Os dois modos devem escolher rotas DIFERENTES - prova de que a
// função de peso está realmente sendo usada!
static Grafo* montarGrafoModosDivergentes(void){
    Grafo *g = criarGrafo();
    grafoInserirVertice(g, criarVertice("a", 0.0, 0.0));
    grafoInserirVertice(g, criarVertice("b", 100.0, 0.0));
    grafoInserirVertice(g, criarVertice("c", 50.0, 20.0));

    grafoInserirAresta(g, "a", "b", "-", "-", 100.0, 50.0, "Rodovia"); // longa, rapida
    grafoInserirAresta(g, "a", "c", "-", "-", 10.0, 1.0, "Rua_Lenta");  // curta, lenta
    grafoInserirAresta(g, "c", "b", "-", "-", 10.0, 1.0, "Rua_Lenta");

    return g;
}

static Grafo* montarGrafoRelaxamentoTardio(void){
    Grafo *g = criarGrafo();
    grafoInserirVertice(g, criarVertice("a", 0.0, 0.0));
    grafoInserirVertice(g, criarVertice("b", 10.0, 0.0));
    grafoInserirVertice(g, criarVertice("c", 0.0, 10.0));
    grafoInserirVertice(g, criarVertice("d", 10.0, 10.0));

    grafoInserirAresta(g, "a", "b", "-", "-", 10.0,  1.0, "Rua_Direta");
    grafoInserirAresta(g, "a", "c", "-", "-",  1.0,  1.0, "Rua_1");
    grafoInserirAresta(g, "c", "b", "-", "-",  1.0,  1.0, "Rua_2");
    grafoInserirAresta(g, "c", "d", "-", "-", 100.0, 1.0, "Rua_Longa");
    grafoInserirAresta(g, "b", "d", "-", "-",  1.0,  1.0, "Rua_3");

    return g;
}

// Caminho simples: caminho feliz básico

void test_calcularCaminhoMinimo_CaminhoSimples_RetornaOkComAtributosCorretos(void){
    Grafo *g = montarGrafoLinha();
    Caminho *c = NULL;

    int resultado = calcularCaminhoMinimo(g, "v0", "v4", CAMINHO_MAIS_CURTO, &c);

    TEST_ASSERT_EQUAL_INT(CAMINHO_OK, resultado);
    TEST_ASSERT_NOT_NULL(c);
    TEST_ASSERT_EQUAL_INT(5, caminhoNumVertices(c));
    TEST_ASSERT_FLOAT_WITHIN(0.001, 40.0, caminhoCustoTotal(c));

    destruirCaminho(c);
    destruirGrafo(g);
}

void test_calcularCaminhoMinimo_OrdemOrigemParaDestino(void){
    Grafo *g = montarGrafoLinha();
    Caminho *c = NULL;

    calcularCaminhoMinimo(g, "v0", "v4", CAMINHO_MAIS_CURTO, &c);
    TEST_ASSERT_NOT_NULL(c);

    // Índice 0 tem que ser a ORIGEM (v0), não o destino.
    TEST_ASSERT_EQUAL_STRING("v0", caminhoObterVertice(c, 0));
    TEST_ASSERT_EQUAL_STRING("v1", caminhoObterVertice(c, 1));
    TEST_ASSERT_EQUAL_STRING("v2", caminhoObterVertice(c, 2));
    TEST_ASSERT_EQUAL_STRING("v3", caminhoObterVertice(c, 3));
    TEST_ASSERT_EQUAL_STRING("v4", caminhoObterVertice(c, 4));

    destruirCaminho(c);
    destruirGrafo(g);
}

//Destino inalcançável

void test_calcularCaminhoMinimo_DestinoInalcancavel_RetornaInalcancavelComPonteiroNulo(void){
    Grafo *g = montarGrafoDesconexo();
    Caminho *c = NULL;

    int resultado = calcularCaminhoMinimo(g, "a", "b", CAMINHO_MAIS_CURTO, &c);

    TEST_ASSERT_EQUAL_INT(CAMINHO_INALCANCAVEL, resultado);
    TEST_ASSERT_NULL(c); // nada a destruir nesse caminho

    destruirGrafo(g);
}

// Caminho trivial: origem == destino 

void test_calcularCaminhoMinimo_OrigemIgualDestino_CaminhoTrivial(void){
    Grafo *g = montarGrafoLinha();
    Caminho *c = NULL;

    int resultado = calcularCaminhoMinimo(g, "v2", "v2", CAMINHO_MAIS_CURTO, &c);

    TEST_ASSERT_EQUAL_INT(CAMINHO_OK, resultado);
    TEST_ASSERT_NOT_NULL(c);
    TEST_ASSERT_EQUAL_INT(1, caminhoNumVertices(c));
    TEST_ASSERT_EQUAL_STRING("v2", caminhoObterVertice(c, 0));
    TEST_ASSERT_FLOAT_WITHIN(0.001, 0.0, caminhoCustoTotal(c));

    destruirCaminho(c);
    destruirGrafo(g);
}

void test_calcularCaminhoMinimo_ModoMaisRapido_VmZero_DesviaRotaAlternativa(void){
    Grafo *g = montarGrafoComVmZero();
    Caminho *c = NULL;

    int resultado = calcularCaminhoMinimo(g, "x", "y", CAMINHO_MAIS_RAPIDO, &c);

    TEST_ASSERT_EQUAL_INT(CAMINHO_OK, resultado); // não deveria ficar inalcancavel
    TEST_ASSERT_NOT_NULL(c);
    TEST_ASSERT_EQUAL_INT(3, caminhoNumVertices(c)); // x,z,y - não a rota direta bloqueada
    TEST_ASSERT_EQUAL_STRING("x", caminhoObterVertice(c, 0));
    TEST_ASSERT_EQUAL_STRING("z", caminhoObterVertice(c, 1));
    TEST_ASSERT_EQUAL_STRING("y", caminhoObterVertice(c, 2));
    TEST_ASSERT_FLOAT_WITHIN(0.001, 4.0, caminhoCustoTotal(c));

    destruirCaminho(c);
    destruirGrafo(g);
}

// Os dois modos podem escolher rotas diferentes 

void test_calcularCaminhoMinimo_ModosDiferentes_PodemEscolherRotasDiferentes(void){
    Grafo *g = montarGrafoModosDivergentes();

    Caminho *curto = NULL;
    calcularCaminhoMinimo(g, "a", "b", CAMINHO_MAIS_CURTO, &curto);
    TEST_ASSERT_NOT_NULL(curto);

    Caminho *rapido = NULL;
    calcularCaminhoMinimo(g, "a", "b", CAMINHO_MAIS_RAPIDO, &rapido);
    TEST_ASSERT_NOT_NULL(rapido);

    // Mais curto: via c (a,c,b), custo 20 (distância).
    TEST_ASSERT_EQUAL_INT(3, caminhoNumVertices(curto));
    TEST_ASSERT_EQUAL_STRING("c", caminhoObterVertice(curto, 1));
    TEST_ASSERT_FLOAT_WITHIN(0.001, 20.0, caminhoCustoTotal(curto));

    // Mais rápido: direto (a,b), custo 2.0 (tempo) - rota DIFERENTE.
    TEST_ASSERT_EQUAL_INT(2, caminhoNumVertices(rapido));
    TEST_ASSERT_EQUAL_STRING("a", caminhoObterVertice(rapido, 0));
    TEST_ASSERT_EQUAL_STRING("b", caminhoObterVertice(rapido, 1));
    TEST_ASSERT_FLOAT_WITHIN(0.001, 2.0, caminhoCustoTotal(rapido));

    destruirCaminho(curto);
    destruirCaminho(rapido);
    destruirGrafo(g);
}

void test_calcularCaminhoMinimo_EncontraRotaGlobalmenteMinima_ApesarDeRelaxamentoTardio(void){
    Grafo *g = montarGrafoRelaxamentoTardio();
    Caminho *c = NULL;

    int resultado = calcularCaminhoMinimo(g, "a", "d", CAMINHO_MAIS_CURTO, &c);

    TEST_ASSERT_EQUAL_INT(CAMINHO_OK, resultado);
    TEST_ASSERT_NOT_NULL(c);

    // Rota correta: a,c,b,d (custo 3) - não a,b,d (11) nem a,c,d (101).
    TEST_ASSERT_EQUAL_INT(4, caminhoNumVertices(c));
    TEST_ASSERT_EQUAL_STRING("a", caminhoObterVertice(c, 0));
    TEST_ASSERT_EQUAL_STRING("c", caminhoObterVertice(c, 1));
    TEST_ASSERT_EQUAL_STRING("b", caminhoObterVertice(c, 2));
    TEST_ASSERT_EQUAL_STRING("d", caminhoObterVertice(c, 3));
    TEST_ASSERT_FLOAT_WITHIN(0.001, 3.0, caminhoCustoTotal(c));

    destruirCaminho(c);
    destruirGrafo(g);
}

void test_destruirCaminho_naoQuebra(void){
    Grafo *g = montarGrafoLinha();
    Caminho *c = NULL;

    calcularCaminhoMinimo(g, "v0", "v1", CAMINHO_MAIS_CURTO, &c);
    TEST_ASSERT_NOT_NULL(c);

    destruirCaminho(c); // se isto quebrar ou vazar, Valgrind denuncia
    destruirGrafo(g);
}

int main(void){
    UNITY_BEGIN();

    RUN_TEST(test_calcularCaminhoMinimo_CaminhoSimples_RetornaOkComAtributosCorretos);
    RUN_TEST(test_calcularCaminhoMinimo_OrdemOrigemParaDestino);
    RUN_TEST(test_calcularCaminhoMinimo_DestinoInalcancavel_RetornaInalcancavelComPonteiroNulo);
    RUN_TEST(test_calcularCaminhoMinimo_OrigemIgualDestino_CaminhoTrivial);
    RUN_TEST(test_calcularCaminhoMinimo_ModoMaisRapido_VmZero_DesviaRotaAlternativa);
    RUN_TEST(test_calcularCaminhoMinimo_ModosDiferentes_PodemEscolherRotasDiferentes);
    RUN_TEST(test_calcularCaminhoMinimo_EncontraRotaGlobalmenteMinima_ApesarDeRelaxamentoTardio);
    RUN_TEST(test_destruirCaminho_naoQuebra);

    return UNITY_END();
}