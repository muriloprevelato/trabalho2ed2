#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hashGenerica.h"
#include "unity.h"

// Sem arquivos em disco então setUp e tearDown sem estado global neste módulo.
void setUp(void){}
void tearDown(void){}

// Conta quantas vezes foi chamado. Usado para confirmar que destruirHash
// libera exatamente os valores que estão na tabela.
static int contagemDestruicoes = 0;

static void destrutorEspiao(void *valor){
    contagemDestruicoes++;
    free(valor);
}

// Destrutor "de verdade" usado nos testes que não focam em ownership
static void destrutorPadrao(void *valor){
    free(valor);
}

// Aloca um int* no heap com o valor dado — facilita simular "valores"
// genéricos sem amarrar o teste a Quadra ou outro tipo concreto.
static int* alocarInt(int valor){
    int *p = malloc(sizeof(int));
    *p = valor;
    return p;
}

// Criação e destruição

void test_criarHash_DeveRetornarPonteiroValido(void){
    HashGenerica *h = criarHash(8, destrutorPadrao);
    TEST_ASSERT_NOT_NULL(h);
    destruirHash(h);
}

void test_criarHash_ComCapacidadeZero_DeveRetornarNull(void){
    HashGenerica *h = criarHash(0, destrutorPadrao);
    TEST_ASSERT_NULL(h);
}

void test_criarHash_ComCapacidadeNegativa_DeveRetornarNull(void){
    HashGenerica *h = criarHash(-5, destrutorPadrao);
    TEST_ASSERT_NULL(h);
}

void test_criarHash_ComDestrutorNulo_DeveRetornarNull(void){
    HashGenerica *h = criarHash(8, NULL);
    TEST_ASSERT_NULL(h);
}

void test_destruirHash_TabelaVazia_naoQuebrar(void){
    HashGenerica *h = criarHash(8, destrutorPadrao);
    TEST_ASSERT_NOT_NULL(h);
    destruirHash(h);
}

// Ownership: destrutor chamado exatamente uma vez por valor 

void test_destruirHash_ChamaDestrutorParaCadaValor(void){
    contagemDestruicoes = 0;

    HashGenerica *h = criarHash(4, destrutorEspiao);
    TEST_ASSERT_NOT_NULL(h);

    inserirHash(h, "cep1", alocarInt(1));
    inserirHash(h, "cep2", alocarInt(2));
    inserirHash(h, "cep3", alocarInt(3));

    destruirHash(h);

    TEST_ASSERT_EQUAL_INT(3, contagemDestruicoes);
}

void test_destruirHash_TabelaVazia_naoChamaDestrutor(void){
    contagemDestruicoes = 0;

    HashGenerica *h = criarHash(4, destrutorEspiao);
    TEST_ASSERT_NOT_NULL(h);

    destruirHash(h);

    TEST_ASSERT_EQUAL_INT(0, contagemDestruicoes);
}

// Inserir | buscar | contém 

void test_inserirEBuscar_DevolveOMesmoPonteiro(void){
    HashGenerica *h = criarHash(8, destrutorPadrao);
    TEST_ASSERT_NOT_NULL(h);

    int *valor = alocarInt(42);
    inserirHash(h, "cep15", valor);

    void *encontrado = buscarHash(h, "cep15");
    TEST_ASSERT_EQUAL_PTR(valor, encontrado);
    TEST_ASSERT_EQUAL_INT(42, *(int*)encontrado);

    destruirHash(h);
}

void test_buscar_ChaveAusente_RetornaNull(void){
    HashGenerica *h = criarHash(8, destrutorPadrao);
    TEST_ASSERT_NOT_NULL(h);

    TEST_ASSERT_NULL(buscarHash(h, "naoexiste"));

    destruirHash(h);
}

void test_contemChave_AntesDeInserir_RetornaErro(void){
    HashGenerica *h = criarHash(8, destrutorPadrao);
    TEST_ASSERT_NOT_NULL(h);

    TEST_ASSERT_EQUAL_INT(HASH_ERRO, contemChaveHash(h, "cep15"));

    destruirHash(h);
}

void test_contemChave_DepoisDeInserir_RetornaOk(void){
    HashGenerica *h = criarHash(8, destrutorPadrao);
    TEST_ASSERT_NOT_NULL(h);

    inserirHash(h, "cep15", alocarInt(1));
    TEST_ASSERT_EQUAL_INT(HASH_OK, contemChaveHash(h, "cep15"));

    destruirHash(h);
}

// Tam

void test_tamanho_TabelaVazia_RetornaZero(void){
    HashGenerica *h = criarHash(8, destrutorPadrao);
    TEST_ASSERT_NOT_NULL(h);

    TEST_ASSERT_EQUAL_INT(0, tamanhoHash(h));

    destruirHash(h);
}

void test_tamanho_AposInsercoes_RetornaContagemCorreta(void){
    HashGenerica *h = criarHash(8, destrutorPadrao);
    TEST_ASSERT_NOT_NULL(h);

    inserirHash(h, "cep1", alocarInt(1));
    inserirHash(h, "cep2", alocarInt(2));
    inserirHash(h, "cep3", alocarInt(3));

    TEST_ASSERT_EQUAL_INT(3, tamanhoHash(h));

    destruirHash(h);
}

// Rehash
//
// Capacidade inicial bem pequena (1 bucket) e muito mais chaves do que
// ela suporta sem crescer. Se o rehash perder ou corromper entradas
// durante a redistribuição pelos novos buckets, este teste pega.
#define QTD_CHAVES_REHASH 200

void test_rehash_TodasAsChavesPermanecemBuscaveisAposCrescimento(void){
    HashGenerica *h = criarHash(1, destrutorPadrao);
    TEST_ASSERT_NOT_NULL(h);

    char buffer[32];
    for(int i = 0; i < QTD_CHAVES_REHASH; i++){
        snprintf(buffer, sizeof(buffer), "cep%03d", i);
        inserirHash(h, buffer, alocarInt(i));
    }

    TEST_ASSERT_EQUAL_INT(QTD_CHAVES_REHASH, tamanhoHash(h));

    // Confere cada chave individualmente -> pega tanto perda de entrada
    // quanto corrupção de valor (rehash que copia o ponteiro errado).
    for(int i = 0; i < QTD_CHAVES_REHASH; i++){
        snprintf(buffer, sizeof(buffer), "cep%03d", i);
        void *v = buscarHash(h, buffer);
        TEST_ASSERT_NOT_NULL(v);
        TEST_ASSERT_EQUAL_INT(i, *(int*)v);
    }

    destruirHash(h);
}

// Percorrer: visita cada entrada exatamente uma vez 

// Contexto do visitante: marca em um array de flags quais índices (0..N-1,
// extraídos da chave "cepNNN") foram visitados. Detecta tanto visitas
// faltantes quanto duplicadas — um contador simples não distinguiria
// "uma chave visitada duas vezes" de "duas chaves visitadas uma vez cada".
typedef struct {
    int flags[QTD_CHAVES_REHASH];
    int totalVisitas;
} ContextoVarredura;

static void visitanteMarcaIndice(const char *chave, void *valor, void *contexto){
    (void) valor; // não precisamos do valor para esta verificação
    ContextoVarredura *ctx = (ContextoVarredura*) contexto;

    int indice;
    sscanf(chave, "cep%d", &indice);

    TEST_ASSERT_TRUE(indice >= 0 && indice < QTD_CHAVES_REHASH);
    ctx->flags[indice]++;
    ctx->totalVisitas++;
}

void test_percorrer_VisitaCadaEntradaExatamenteUmaVez(void){
    HashGenerica *h = criarHash(4, destrutorPadrao);
    TEST_ASSERT_NOT_NULL(h);

    char buffer[32];
    for(int i = 0; i < QTD_CHAVES_REHASH; i++){
        snprintf(buffer, sizeof(buffer), "cep%03d", i);
        inserirHash(h, buffer, alocarInt(i));
    }

    ContextoVarredura ctx;
    memset(ctx.flags, 0, sizeof(ctx.flags));
    ctx.totalVisitas = 0;

    percorrerHash(h, visitanteMarcaIndice, &ctx);

    TEST_ASSERT_EQUAL_INT(tamanhoHash(h), ctx.totalVisitas);
    for(int i = 0; i < QTD_CHAVES_REHASH; i++){
        TEST_ASSERT_EQUAL_INT(1, ctx.flags[i]); // nem 0 (faltou), nem 2+ (duplicou)
    }

    destruirHash(h);
}

void test_percorrer_TabelaVazia_NuncaChamaVisitante(void){
    HashGenerica *h = criarHash(4, destrutorPadrao);
    TEST_ASSERT_NOT_NULL(h);

    ContextoVarredura ctx;
    memset(ctx.flags, 0, sizeof(ctx.flags));
    ctx.totalVisitas = 0;

    percorrerHash(h, visitanteMarcaIndice, &ctx);

    TEST_ASSERT_EQUAL_INT(0, ctx.totalVisitas);

    destruirHash(h);
}

int main(void){
    UNITY_BEGIN();

    RUN_TEST(test_criarHash_DeveRetornarPonteiroValido);
    RUN_TEST(test_criarHash_ComCapacidadeZero_DeveRetornarNull);
    RUN_TEST(test_criarHash_ComCapacidadeNegativa_DeveRetornarNull);
    RUN_TEST(test_criarHash_ComDestrutorNulo_DeveRetornarNull);
    RUN_TEST(test_destruirHash_TabelaVazia_naoQuebrar);
    RUN_TEST(test_destruirHash_ChamaDestrutorParaCadaValor);
    RUN_TEST(test_destruirHash_TabelaVazia_naoChamaDestrutor);
    RUN_TEST(test_inserirEBuscar_DevolveOMesmoPonteiro);
    RUN_TEST(test_buscar_ChaveAusente_RetornaNull);
    RUN_TEST(test_contemChave_AntesDeInserir_RetornaErro);
    RUN_TEST(test_contemChave_DepoisDeInserir_RetornaOk);
    RUN_TEST(test_tamanho_TabelaVazia_RetornaZero);
    RUN_TEST(test_tamanho_AposInsercoes_RetornaContagemCorreta);
    RUN_TEST(test_rehash_TodasAsChavesPermanecemBuscaveisAposCrescimento);
    RUN_TEST(test_percorrer_VisitaCadaEntradaExatamenteUmaVez);
    RUN_TEST(test_percorrer_TabelaVazia_NuncaChamaVisitante);

    return UNITY_END();
}