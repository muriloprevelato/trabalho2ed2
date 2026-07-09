#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "leitorQry.h"
#include "cidade.h"
#include "quadra.h"
#include "grafo.h"
#include "vertice.h"
#include "registradores.h"
#include "svg.h"
#include "unity.h"

// Sem estado global neste módulo - cada teste monta seus próprios recursos, então setUp/tearDown ficam vazios.
void setUp(void){}
void tearDown(void){}

#define ARQ_QRY_TMP "teste_qry_tmp.qry"
#define ARQ_SVG_TMP "teste_qry_tmp.svg"
#define ARQ_TXT_TMP "teste_qry_tmp.txt"

static void escreverQryTemporario(const char *conteudo){
    FILE *fp = fopen(ARQ_QRY_TMP, "w");
    TEST_ASSERT_NOT_NULL(fp);
    fputs(conteudo, fp);
    fclose(fp);
}

static char* lerArquivoCompleto(const char *caminho){
    FILE *fp = fopen(caminho, "r");
    if(fp == NULL) return NULL;

    fseek(fp, 0, SEEK_END);
    long tamanho = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *buffer = malloc((size_t) tamanho + 1);
    if(buffer == NULL){
        fclose(fp);
        return NULL;
    }

    size_t lidos = fread(buffer, 1, (size_t) tamanho, fp);
    buffer[lidos] = '\0';

    fclose(fp);
    return buffer;
}

// Uma quadra simples, cep15, âncora (0,0), w=100, h=50.
static Cidade* montarCidadeComQuadra(void){
    Cidade *c = criarCidade();
    Quadra *q = criarQuadra("cep15", 0.0, 0.0, 100.0, 50.0, 1.0, "white", "black");
    inserirQuadraCidade(c, q);
    return c;
}

// Três vértices em linha (v1-v2-v3), duas arestas conectando-os, vm=10 em ambas
static Grafo* montarGrafoConectado(void){
    Grafo *g = criarGrafo();
    grafoInserirVertice(g, criarVertice("v1", 0.0, 0.0));
    grafoInserirVertice(g, criarVertice("v2", 50.0, 0.0));
    grafoInserirVertice(g, criarVertice("v3", 100.0, 0.0));
    grafoInserirAresta(g, "v1", "v2", "-", "-", 50.0, 10.0, "Rua_A");
    grafoInserirAresta(g, "v2", "v3", "-", "-", 50.0, 10.0, "Rua_B");
    return g;
}

// Grafo desenhado especificamente pro teste de geometria do mvm:
//   a-b: os dois extremos caem em [0,0]-[50,50] (dentro)
//   a-c: só a origem cai na região; c fica bem fora (parcial)
static Grafo* montarGrafoParaMvm(void){
    Grafo *g = criarGrafo();
    grafoInserirVertice(g, criarVertice("a", 10.0, 10.0));
    grafoInserirVertice(g, criarVertice("b", 20.0, 20.0));
    grafoInserirVertice(g, criarVertice("c", 500.0, 500.0));
    grafoInserirAresta(g, "a", "b", "-", "-", 10.0, 5.0, "Rua_Dentro");
    grafoInserirAresta(g, "a", "c", "-", "-", 10.0, 5.0, "Rua_Parcial");
    return g;
}

// Busca o vm de uma aresta específica (idOrigem->idDestino) percorrendo todas as arestas do grafo 
typedef struct {
    const char *idOrigemProcurado;
    const char *idDestinoProcurado;
    double vmEncontrado;
    int encontrado;
} ContextoBuscarVm;

static void visitanteBuscarVm(const char *idOrigem, Aresta *a, void *contexto){
    ContextoBuscarVm *ctx = (ContextoBuscarVm*) contexto;
    const char *idDestino = getVerticeId(getArestaDestino(a));

    if(strcmp(idOrigem, ctx->idOrigemProcurado) == 0 &&
       strcmp(idDestino, ctx->idDestinoProcurado) == 0){
        ctx->vmEncontrado = getArestaVm(a);
        ctx->encontrado = 1;
    }
}

static double buscarVmDaAresta(Grafo *g, const char *idOrigem, const char *idDestino){
    ContextoBuscarVm ctx = { idOrigem, idDestino, 0.0, 0 };
    percorrerTodasArestas(g, visitanteBuscarVm, &ctx);
    TEST_ASSERT_TRUE(ctx.encontrado);
    return ctx.vmEncontrado;
}

void test_calcularDimensoesMapa_ComQuadras_RetornaDimensoesCorretas(void){
    Cidade *cidade = montarCidadeComQuadra(); // w=100, h=50 -> extremo (100,50)

    double largura, altura;
    calcularDimensoesMapa(cidade, NULL, &largura, &altura);

    TEST_ASSERT_FLOAT_WITHIN(0.001, 100.0, largura);
    TEST_ASSERT_FLOAT_WITHIN(0.001, 50.0, altura);

    destruirCidade(cidade);
}

void test_calcularDimensoesMapa_ComGrafoAlemDasQuadras_ConsideraAmbos(void){
    Cidade *cidade = montarCidadeComQuadra(); // extremo (100,50)

    Grafo *grafo = criarGrafo();
    grafoInserirVertice(grafo, criarVertice("v1", 300.0, 300.0)); // além das quadras

    double largura, altura;
    calcularDimensoesMapa(cidade, grafo, &largura, &altura);

    TEST_ASSERT_FLOAT_WITHIN(0.001, 300.0, largura);
    TEST_ASSERT_FLOAT_WITHIN(0.001, 300.0, altura);

    destruirGrafo(grafo);
    destruirCidade(cidade);
}

void test_calcularDimensoesMapa_CidadeEGrafoVazios_CanvasMinimo(void){
    Cidade *cidade = criarCidade();
    Grafo *grafo = criarGrafo();

    double largura, altura;
    calcularDimensoesMapa(cidade, grafo, &largura, &altura);

    TEST_ASSERT_FLOAT_WITHIN(0.001, 100.0, largura);
    TEST_ASSERT_FLOAT_WITHIN(0.001, 100.0, altura);

    destruirGrafo(grafo);
    destruirCidade(cidade);
}

void test_desenharMapaBase_DesenhaQuadras(void){
    Cidade *cidade = montarCidadeComQuadra();

    ArqSvg *svg = abreEscritaSvg(ARQ_SVG_TMP, 200.0, 200.0);
    TEST_ASSERT_NOT_NULL(svg);
    desenharMapaBase(svg, cidade, NULL);
    fechaSvg(svg);

    char *conteudo = lerArquivoCompleto(ARQ_SVG_TMP);
    TEST_ASSERT_NOT_NULL(conteudo);
    TEST_ASSERT_NOT_NULL(strstr(conteudo, "<rect"));

    free(conteudo);
    remove(ARQ_SVG_TMP);
    destruirCidade(cidade);
}

void test_desenharMapaBase_ComGrafo_DesenhaVerticesEArestas(void){
    Cidade *cidade = montarCidadeComQuadra();
    Grafo *grafo = montarGrafoConectado();

    ArqSvg *svg = abreEscritaSvg(ARQ_SVG_TMP, 200.0, 200.0);
    TEST_ASSERT_NOT_NULL(svg);
    desenharMapaBase(svg, cidade, grafo);
    fechaSvg(svg);

    char *conteudo = lerArquivoCompleto(ARQ_SVG_TMP);
    TEST_ASSERT_NOT_NULL(conteudo);
    TEST_ASSERT_NOT_NULL(strstr(conteudo, "<rect"));
    TEST_ASSERT_NOT_NULL(strstr(conteudo, "<circle")); 
    TEST_ASSERT_NOT_NULL(strstr(conteudo, "<line"));   

    free(conteudo);
    remove(ARQ_SVG_TMP);
    destruirGrafo(grafo);
    destruirCidade(cidade);
}

void test_desenharMapaBase_GrafoNulo_DesenhaSoQuadras(void){
    Cidade *cidade = montarCidadeComQuadra();

    ArqSvg *svg = abreEscritaSvg(ARQ_SVG_TMP, 200.0, 200.0);
    TEST_ASSERT_NOT_NULL(svg);
    desenharMapaBase(svg, cidade, NULL);
    fechaSvg(svg);

    char *conteudo = lerArquivoCompleto(ARQ_SVG_TMP);
    TEST_ASSERT_NOT_NULL(conteudo);
    TEST_ASSERT_NOT_NULL(strstr(conteudo, "<rect"));
    TEST_ASSERT_NULL(strstr(conteudo, "<circle")); // sem grafo, sem vértice desenhado

    free(conteudo);
    remove(ARQ_SVG_TMP);
    destruirCidade(cidade);
}

void test_processarArquivoQry_CaminhoInvalido_RetornaErro(void){
    Cidade *cidade = montarCidadeComQuadra();
    Registradores *regs = criarRegistradores();
    ArqSvg *svg = abreEscritaSvg(ARQ_SVG_TMP, 200.0, 200.0);
    FILE *txt = fopen(ARQ_TXT_TMP, "w");
    TEST_ASSERT_NOT_NULL(svg);
    TEST_ASSERT_NOT_NULL(txt);

    int resultado = processarArquivoQry("/diretorio/inexistente/x.qry",
                                         cidade, NULL, regs, svg, txt);
    TEST_ASSERT_EQUAL_INT(QRY_ERRO, resultado);

    fechaSvg(svg);
    fclose(txt);
    remove(ARQ_SVG_TMP);
    remove(ARQ_TXT_TMP);
    destruirRegistradores(regs);
    destruirCidade(cidade);
}

void test_processarArquivoQry_ComandoDesconhecido_NaoAborta(void){
    escreverQryTemporario("comandoDesconhecido x y z\n");

    Cidade *cidade = montarCidadeComQuadra();
    Grafo *grafo = montarGrafoConectado();
    Registradores *regsB = criarRegistradores();
    ArqSvg *svg = abreEscritaSvg(ARQ_SVG_TMP, 200.0, 200.0);
    FILE *txt = fopen(ARQ_TXT_TMP, "w");

    int resultado = processarArquivoQry(ARQ_QRY_TMP, cidade, grafo, regsB, svg, txt);
    TEST_ASSERT_EQUAL_INT(QRY_OK, resultado);

    fechaSvg(svg);
    fclose(txt);
    remove(ARQ_QRY_TMP);
    remove(ARQ_SVG_TMP);
    remove(ARQ_TXT_TMP);
    destruirRegistradores(regsB);
    destruirGrafo(grafo);
    destruirCidade(cidade);
}

void test_processarArquivoQry_GrafoNulo_ComandoGrafoDependente_NaoAborta(void){
    escreverQryTemporario("regs 1.0\n"); // depende do grafo, mas grafo é NULL aqui

    Cidade *cidade = montarCidadeComQuadra();
    Registradores *regsB = criarRegistradores();
    ArqSvg *svg = abreEscritaSvg(ARQ_SVG_TMP, 200.0, 200.0);
    FILE *txt = fopen(ARQ_TXT_TMP, "w");

    int resultado = processarArquivoQry(ARQ_QRY_TMP, cidade, NULL, regsB, svg, txt);
    TEST_ASSERT_EQUAL_INT(QRY_OK, resultado); // não deveria abortar

    fechaSvg(svg);
    fclose(txt);
    remove(ARQ_QRY_TMP);
    remove(ARQ_SVG_TMP);
    remove(ARQ_TXT_TMP);
    destruirRegistradores(regsB);
    destruirCidade(cidade);
}

void test_processarArquivoQry_Mvm_AmbosExtremosNaRegiao_AtualizaVm(void){
    escreverQryTemporario("mvm 20.0 0.0 0.0 50.0 50.0\n");

    Cidade *cidade = montarCidadeComQuadra();
    Grafo *grafo = montarGrafoParaMvm();
    Registradores *regsB = criarRegistradores();
    ArqSvg *svg = abreEscritaSvg(ARQ_SVG_TMP, 600.0, 600.0);
    FILE *txt = fopen(ARQ_TXT_TMP, "w");

    processarArquivoQry(ARQ_QRY_TMP, cidade, grafo, regsB, svg, txt);

    // Rua Dentro (a->b): AMBOS os extremos em [0,0]-[50,50] -> deve mudar.
    TEST_ASSERT_FLOAT_WITHIN(0.001, 20.0, buscarVmDaAresta(grafo, "a", "b"));

    // Rua Parcial (a->c): só 'a' está na região, 'c' está longe -> NÃO muda.
    TEST_ASSERT_FLOAT_WITHIN(0.001, 5.0, buscarVmDaAresta(grafo, "a", "c"));

    fechaSvg(svg);
    fclose(txt);
    remove(ARQ_QRY_TMP);
    remove(ARQ_SVG_TMP);
    remove(ARQ_TXT_TMP);
    destruirRegistradores(regsB);
    destruirGrafo(grafo);
    destruirCidade(cidade);
}

void test_processarArquivoQry_ArrobaO_ResolveEGuardaNoRegistrador(void){
    escreverQryTemporario("@o? R0 cep15 S 30\n");

    Cidade *cidade = montarCidadeComQuadra();
    Grafo *grafo = montarGrafoConectado();
    Registradores *regsB = criarRegistradores();
    ArqSvg *svg = abreEscritaSvg(ARQ_SVG_TMP, 200.0, 200.0);
    FILE *txt = fopen(ARQ_TXT_TMP, "w");

    processarArquivoQry(ARQ_QRY_TMP, cidade, grafo, regsB, svg, txt);

    TEST_ASSERT_EQUAL_INT(REGISTRADOR_OK, registradorPreenchido(regsB, 0));
    TEST_ASSERT_FLOAT_WITHIN(0.001, 30.0, getRegistradorX(regsB, 0));
    TEST_ASSERT_FLOAT_WITHIN(0.001, 0.0, getRegistradorY(regsB, 0));

    fechaSvg(svg);
    fclose(txt);
    remove(ARQ_QRY_TMP);
    remove(ARQ_SVG_TMP);
    remove(ARQ_TXT_TMP);
    destruirRegistradores(regsB);
    destruirGrafo(grafo);
    destruirCidade(cidade);
}

void test_processarArquivoQry_ArrobaO_CepInexistente_NaoAborta(void){
    escreverQryTemporario("@o? R0 naoexiste S 30\n");

    Cidade *cidade = montarCidadeComQuadra();
    Grafo *grafo = montarGrafoConectado();
    Registradores *regsB = criarRegistradores();
    ArqSvg *svg = abreEscritaSvg(ARQ_SVG_TMP, 200.0, 200.0);
    FILE *txt = fopen(ARQ_TXT_TMP, "w");

    int resultado = processarArquivoQry(ARQ_QRY_TMP, cidade, grafo, regsB, svg, txt);
    TEST_ASSERT_EQUAL_INT(QRY_OK, resultado); // não aborta

    TEST_ASSERT_EQUAL_INT(REGISTRADOR_ERRO, registradorPreenchido(regsB, 0));

    fechaSvg(svg);
    fclose(txt);
    remove(ARQ_QRY_TMP);
    remove(ARQ_SVG_TMP);
    remove(ARQ_TXT_TMP);
    destruirRegistradores(regsB);
    destruirGrafo(grafo);
    destruirCidade(cidade);
}

// ─── regs: reporta o número de componentes no .txt ────────────────────────────

void test_processarArquivoQry_Regs_ReportaNumeroDeComponentesNoTxt(void){
    escreverQryTemporario("regs 1.0\n"); // vl baixo: as duas arestas (vm=10) contam -> 1 componente

    Cidade *cidade = montarCidadeComQuadra();
    Grafo *grafo = montarGrafoConectado(); // v1-v2-v3, tudo conectado
    Registradores *regsB = criarRegistradores();
    ArqSvg *svg = abreEscritaSvg(ARQ_SVG_TMP, 200.0, 200.0);
    FILE *txt = fopen(ARQ_TXT_TMP, "w");

    processarArquivoQry(ARQ_QRY_TMP, cidade, grafo, regsB, svg, txt);

    fechaSvg(svg);
    fclose(txt);

    char *conteudoTxt = lerArquivoCompleto(ARQ_TXT_TMP);
    TEST_ASSERT_NOT_NULL(conteudoTxt);
    TEST_ASSERT_TRUE(strlen(conteudoTxt) > 0); // algo foi escrito
    TEST_ASSERT_NOT_NULL(strstr(conteudoTxt, "1")); // o número 1 (componente único) aparece

    free(conteudoTxt);
    remove(ARQ_QRY_TMP);
    remove(ARQ_SVG_TMP);
    remove(ARQ_TXT_TMP);
    destruirRegistradores(regsB);
    destruirGrafo(grafo);
    destruirCidade(cidade);
}


void test_processarArquivoQry_Exp_AumentaVmDasArestasSelecionadas(void){
    escreverQryTemporario("exp 20.0\n"); 
    Cidade *cidade = montarCidadeComQuadra();
    Grafo *grafo = montarGrafoConectado(); // v1-v2-v3, vm=10 em ambas
    Registradores *regsB = criarRegistradores();
    ArqSvg *svg = abreEscritaSvg(ARQ_SVG_TMP, 200.0, 200.0);
    FILE *txt = fopen(ARQ_TXT_TMP, "w");

    processarArquivoQry(ARQ_QRY_TMP, cidade, grafo, regsB, svg, txt);

    // 10.0 * 1.5 = 15.0 -> confirma que a mutação realmente aconteceu no grafo.
    TEST_ASSERT_FLOAT_WITHIN(0.001, 15.0, buscarVmDaAresta(grafo, "v1", "v2"));
    TEST_ASSERT_FLOAT_WITHIN(0.001, 15.0, buscarVmDaAresta(grafo, "v2", "v3"));

    fechaSvg(svg);
    fclose(txt);
    remove(ARQ_QRY_TMP);
    remove(ARQ_SVG_TMP);
    remove(ARQ_TXT_TMP);
    destruirRegistradores(regsB);
    destruirGrafo(grafo);
    destruirCidade(cidade);
}


void test_processarArquivoQry_PInterrogacao_CalculaCaminhoEEscreveNoTxt(void){
    escreverQryTemporario("p? R0 R1 red blue\n");

    Cidade *cidade = montarCidadeComQuadra();
    Grafo *grafo = montarGrafoConectado(); // v1-v2-v3
    Registradores *regsB = criarRegistradores();
    
    setRegistrador(regsB, 0, 0.0, 0.0, "cep15/S/0");   // perto de v1
    setRegistrador(regsB, 1, 100.0, 0.0, "cep15/S/100"); // perto de v3

    ArqSvg *svg = abreEscritaSvg(ARQ_SVG_TMP, 200.0, 200.0);
    FILE *txt = fopen(ARQ_TXT_TMP, "w");

    int resultado = processarArquivoQry(ARQ_QRY_TMP, cidade, grafo, regsB, svg, txt);
    TEST_ASSERT_EQUAL_INT(QRY_OK, resultado);

    fechaSvg(svg);
    fclose(txt);

    char *conteudoTxt = lerArquivoCompleto(ARQ_TXT_TMP);
    TEST_ASSERT_NOT_NULL(conteudoTxt);
    TEST_ASSERT_TRUE(strlen(conteudoTxt) > 0);

    free(conteudoTxt);
    remove(ARQ_QRY_TMP);
    remove(ARQ_SVG_TMP);
    remove(ARQ_TXT_TMP);
    destruirRegistradores(regsB);
    destruirGrafo(grafo);
    destruirCidade(cidade);
}

void test_processarArquivoQry_PInterrogacao_RegistradorNaoPreenchido_NaoAborta(void){
    escreverQryTemporario("p? R5 R6 red blue\n"); // nenhum dos dois foi setado

    Cidade *cidade = montarCidadeComQuadra();
    Grafo *grafo = montarGrafoConectado();
    Registradores *regsB = criarRegistradores(); // todos vazios

    ArqSvg *svg = abreEscritaSvg(ARQ_SVG_TMP, 200.0, 200.0);
    FILE *txt = fopen(ARQ_TXT_TMP, "w");

    int resultado = processarArquivoQry(ARQ_QRY_TMP, cidade, grafo, regsB, svg, txt);
    TEST_ASSERT_EQUAL_INT(QRY_OK, resultado); // não deveria abortar

    fechaSvg(svg);
    fclose(txt);
    remove(ARQ_QRY_TMP);
    remove(ARQ_SVG_TMP);
    remove(ARQ_TXT_TMP);
    destruirRegistradores(regsB);
    destruirGrafo(grafo);
    destruirCidade(cidade);
}

void test_processarArquivoQry_MvmAntesDeRegs_MutacaoPersisteEntreComandos(void){
    escreverQryTemporario(
        "mvm 0.0 40.0 -10.0 70.0 20.0\n" // regiao [40,110]x[-10,10]: contem v2(50,0) e v3(100,0), nao v1(0,0)
        "regs 5.0\n" // vl=5: a aresta com vm=0 não conta mais
    );
 
    Cidade *cidade = montarCidadeComQuadra();
    Grafo *grafo = criarGrafo();
    grafoInserirVertice(grafo, criarVertice("v1", 0.0, 0.0));
    grafoInserirVertice(grafo, criarVertice("v2", 50.0, 0.0));
    grafoInserirVertice(grafo, criarVertice("v3", 100.0, 0.0));
    grafoInserirAresta(grafo, "v1", "v2", "-", "-", 50.0, 10.0, "Rua_A");
    grafoInserirAresta(grafo, "v2", "v3", "-", "-", 50.0, 10.0, "Rua_B");
 
    Registradores *regsB = criarRegistradores();
    ArqSvg *svg = abreEscritaSvg(ARQ_SVG_TMP, 200.0, 200.0);
    FILE *txt = fopen(ARQ_TXT_TMP, "w");
 
    processarArquivoQry(ARQ_QRY_TMP, cidade, grafo, regsB, svg, txt);
 
    fechaSvg(svg);
    fclose(txt);
 
    // Confirma que a mutação do mvm realmente aconteceu no grafo.
    TEST_ASSERT_FLOAT_WITHIN(0.001, 10.0, buscarVmDaAresta(grafo, "v1", "v2")); // não mexida
    TEST_ASSERT_FLOAT_WITHIN(0.001, 0.0, buscarVmDaAresta(grafo, "v2", "v3"));  // derrubada
 
    char *conteudoTxt = lerArquivoCompleto(ARQ_TXT_TMP);
    TEST_ASSERT_NOT_NULL(conteudoTxt);
    TEST_ASSERT_NOT_NULL(strstr(conteudoTxt, "2")); // regs deveria reportar 2 componentes
 
    free(conteudoTxt);
    remove(ARQ_QRY_TMP);
    remove(ARQ_SVG_TMP);
    remove(ARQ_TXT_TMP);
    destruirRegistradores(regsB);
    destruirGrafo(grafo);
    destruirCidade(cidade);
}

int main(void){
    UNITY_BEGIN();

    RUN_TEST(test_calcularDimensoesMapa_ComQuadras_RetornaDimensoesCorretas);
    RUN_TEST(test_calcularDimensoesMapa_ComGrafoAlemDasQuadras_ConsideraAmbos);
    RUN_TEST(test_calcularDimensoesMapa_CidadeEGrafoVazios_CanvasMinimo);
    RUN_TEST(test_desenharMapaBase_DesenhaQuadras);
    RUN_TEST(test_desenharMapaBase_ComGrafo_DesenhaVerticesEArestas);
    RUN_TEST(test_desenharMapaBase_GrafoNulo_DesenhaSoQuadras);
    RUN_TEST(test_processarArquivoQry_CaminhoInvalido_RetornaErro);
    RUN_TEST(test_processarArquivoQry_ComandoDesconhecido_NaoAborta);
    RUN_TEST(test_processarArquivoQry_GrafoNulo_ComandoGrafoDependente_NaoAborta);
    RUN_TEST(test_processarArquivoQry_Mvm_AmbosExtremosNaRegiao_AtualizaVm);
    RUN_TEST(test_processarArquivoQry_ArrobaO_ResolveEGuardaNoRegistrador);
    RUN_TEST(test_processarArquivoQry_ArrobaO_CepInexistente_NaoAborta);
    RUN_TEST(test_processarArquivoQry_Regs_ReportaNumeroDeComponentesNoTxt);
    RUN_TEST(test_processarArquivoQry_Exp_AumentaVmDasArestasSelecionadas);
    RUN_TEST(test_processarArquivoQry_PInterrogacao_CalculaCaminhoEEscreveNoTxt);
    RUN_TEST(test_processarArquivoQry_PInterrogacao_RegistradorNaoPreenchido_NaoAborta);
    RUN_TEST(test_processarArquivoQry_MvmAntesDeRegs_MutacaoPersisteEntreComandos);

    return UNITY_END();
}