#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cidade.h"
#include "quadra.h"
#include "unity.h"

// Sem arquivos em disco então setUp e tearDown sem estado global neste módulo.
void setUp(void){}
void tearDown(void){}

// Só para não ficar repetindo parâmetros em cada teste!
static Quadra* quadra_com_cep(const char *cep){
    return criarQuadra(cep, 10.0, 20.0, 50.0, 30.0, 2.0, "orange", "black");
}

// Criação e destruição

void test_criarCidade_DeveRetornarPonteiroValido(void){
    Cidade *c = criarCidade();
    TEST_ASSERT_NOT_NULL(c);
    destruirCidade(c);
}

void test_destruirCidade_CidadeVazia_naoQuebrar(void){
    Cidade *c = criarCidade();
    TEST_ASSERT_NOT_NULL(c);
    destruirCidade(c);
}

/* Ownership: a cidade é dona das quadras inseridas
Não chamamos destruirQuadra manualmente em nenhum teste deste bloco —
se a cidade não liberar as quadras corretamente, o Valgrind acusa leak;
se ela tentar liberar algo que não é dela (ou liberar 2x), acusa erro.
*/

void test_destruirCidade_LiberaQuadrasInseridas(void){
    Cidade *c = criarCidade();
    TEST_ASSERT_NOT_NULL(c);

    inserirQuadraCidade(c, quadra_com_cep("cep000"));
    inserirQuadraCidade(c, quadra_com_cep("cep001"));
    inserirQuadraCidade(c, quadra_com_cep("cep002"));

    destruirCidade(c); // se isto não liberar as 3 quadras, Valgrind denuncia
}

// Inserir | buscar | contém 

void test_inserirEBuscar_DevolveAMesmaQuadra(void){
    Cidade *c = criarCidade();
    TEST_ASSERT_NOT_NULL(c);

    Quadra *q = quadra_com_cep("cep15");
    inserirQuadraCidade(c, q);

    Quadra *encontrada = buscarQuadraCidade(c, "cep15");
    TEST_ASSERT_EQUAL_PTR(q, encontrada);
    TEST_ASSERT_EQUAL_STRING("cep15", getQuadraCep(encontrada));

    destruirCidade(c);
}

void test_buscar_CepAusente_RetornaNull(void){
    Cidade *c = criarCidade();
    TEST_ASSERT_NOT_NULL(c);

    TEST_ASSERT_NULL(buscarQuadraCidade(c, "naoexiste"));

    destruirCidade(c);
}

void test_contemCep_AntesDeInserir_RetornaErro(void){
    Cidade *c = criarCidade();
    TEST_ASSERT_NOT_NULL(c);

    TEST_ASSERT_EQUAL_INT(CIDADE_ERRO, cidadeContemCep(c, "cep15"));

    destruirCidade(c);
}

void test_contemCep_DepoisDeInserir_RetornaOk(void){
    Cidade *c = criarCidade();
    TEST_ASSERT_NOT_NULL(c);

    inserirQuadraCidade(c, quadra_com_cep("cep15"));
    TEST_ASSERT_EQUAL_INT(CIDADE_OK, cidadeContemCep(c, "cep15"));

    destruirCidade(c);
}

// Número de quadras

void test_numQuadras_CidadeVazia_RetornaZero(void){
    Cidade *c = criarCidade();
    TEST_ASSERT_NOT_NULL(c);

    TEST_ASSERT_EQUAL_INT(0, cidadeNumQuadras(c));

    destruirCidade(c);
}

void test_numQuadras_AcompanhaInsercoes(void){
    Cidade *c = criarCidade();
    TEST_ASSERT_NOT_NULL(c);

    TEST_ASSERT_EQUAL_INT(0, cidadeNumQuadras(c));

    inserirQuadraCidade(c, quadra_com_cep("cep000"));
    TEST_ASSERT_EQUAL_INT(1, cidadeNumQuadras(c));

    inserirQuadraCidade(c, quadra_com_cep("cep001"));
    TEST_ASSERT_EQUAL_INT(2, cidadeNumQuadras(c));

    inserirQuadraCidade(c, quadra_com_cep("cep002"));
    TEST_ASSERT_EQUAL_INT(3, cidadeNumQuadras(c));

    destruirCidade(c);
}

// Insere quadras suficientes pra forçar rehash interno da hash subjacente, e confirma que cidadeNumQuadras continua correto mesmo atravessando isso.
#define QTD_QUADRAS_REHASH 100

void test_numQuadras_PermaneceCorretoAposRehashInterno(void){
    Cidade *c = criarCidade();
    TEST_ASSERT_NOT_NULL(c);

    char cep[32];
    for(int i = 0; i < QTD_QUADRAS_REHASH; i++){
        snprintf(cep, sizeof(cep), "cep%03d", i);
        inserirQuadraCidade(c, quadra_com_cep(cep));
    }

    TEST_ASSERT_EQUAL_INT(QTD_QUADRAS_REHASH, cidadeNumQuadras(c));

    // Confere que todas continuam buscáveis, não só a contagem.
    for(int i = 0; i < QTD_QUADRAS_REHASH; i++){
        snprintf(cep, sizeof(cep), "cep%03d", i);
        Quadra *q = buscarQuadraCidade(c, cep);
        TEST_ASSERT_NOT_NULL(q);
        TEST_ASSERT_EQUAL_STRING(cep, getQuadraCep(q));
    }

    destruirCidade(c);
}

// Percorrer: visita cada quadra exatamente uma vez 
typedef struct {
    int flags[QTD_QUADRAS_REHASH];
    int totalVisitas;
} ContextoVarreduraCidade;

static void visitanteMarcaIndice(Quadra *q, void *contexto){
    ContextoVarreduraCidade *ctx = (ContextoVarreduraCidade*) contexto;

    int indice;
    sscanf(getQuadraCep(q), "cep%d", &indice);

    TEST_ASSERT_TRUE(indice >= 0 && indice < QTD_QUADRAS_REHASH);
    ctx->flags[indice]++;
    ctx->totalVisitas++;
}

void test_percorrer_VisitaCadaQuadraExatamenteUmaVez(void){
    Cidade *c = criarCidade();
    TEST_ASSERT_NOT_NULL(c);

    char cep[32];
    for(int i = 0; i < QTD_QUADRAS_REHASH; i++){
        snprintf(cep, sizeof(cep), "cep%03d", i);
        inserirQuadraCidade(c, quadra_com_cep(cep));
    }

    ContextoVarreduraCidade ctx;
    memset(ctx.flags, 0, sizeof(ctx.flags));
    ctx.totalVisitas = 0;

    percorrerQuadrasCidade(c, visitanteMarcaIndice, &ctx);

    TEST_ASSERT_EQUAL_INT(cidadeNumQuadras(c), ctx.totalVisitas);
    for(int i = 0; i < QTD_QUADRAS_REHASH; i++){
        TEST_ASSERT_EQUAL_INT(1, ctx.flags[i]); // nem 0 (faltou), nem 2+ (duplicou)
    }

    destruirCidade(c);
}

void test_percorrer_CidadeVazia_NuncaChamaVisitante(void){
    Cidade *c = criarCidade();
    TEST_ASSERT_NOT_NULL(c);

    ContextoVarreduraCidade ctx;
    memset(ctx.flags, 0, sizeof(ctx.flags));
    ctx.totalVisitas = 0;

    percorrerQuadrasCidade(c, visitanteMarcaIndice, &ctx);

    TEST_ASSERT_EQUAL_INT(0, ctx.totalVisitas);

    destruirCidade(c);
}

int main(void){
    UNITY_BEGIN();

    RUN_TEST(test_criarCidade_DeveRetornarPonteiroValido);
    RUN_TEST(test_destruirCidade_CidadeVazia_naoQuebrar);
    RUN_TEST(test_destruirCidade_LiberaQuadrasInseridas);
    RUN_TEST(test_inserirEBuscar_DevolveAMesmaQuadra);
    RUN_TEST(test_buscar_CepAusente_RetornaNull);
    RUN_TEST(test_contemCep_AntesDeInserir_RetornaErro);
    RUN_TEST(test_contemCep_DepoisDeInserir_RetornaOk);
    RUN_TEST(test_numQuadras_CidadeVazia_RetornaZero);
    RUN_TEST(test_numQuadras_AcompanhaInsercoes);
    RUN_TEST(test_numQuadras_PermaneceCorretoAposRehashInterno);
    RUN_TEST(test_percorrer_VisitaCadaQuadraExatamenteUmaVez);
    RUN_TEST(test_percorrer_CidadeVazia_NuncaChamaVisitante);

    return UNITY_END();
}