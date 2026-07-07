#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "svg.h"
#include "unity.h"

// Sem estado global neste módulo — cada teste cria/remove seu próprio arquivo temporário, então setUp/tearDown ficam vazios.
void setUp(void){}
void tearDown(void){}

// Caminho de arquivo temporário usado pelos testes que escrevem em disco.
#define ARQ_TMP "teste_svg_tmp.svg"

// Helper de leitura
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

// Abertura: caminho inválido (o "caminho infeliz" central deste módulo)

void test_abreEscritaSvg_CaminhoInvalido_RetornaNull(void){
    ArqSvg *f = abreEscritaSvg("/diretorio/que/definitivamente/nao/existe/x.svg", 100.0, 100.0);
    TEST_ASSERT_NULL(f);
}

// Abertura/fechamento: caminho válido

void test_abreEscritaSvg_CaminhoValido_RetornaPonteiroValido(void){
    ArqSvg *f = abreEscritaSvg(ARQ_TMP, 100.0, 100.0);
    TEST_ASSERT_NOT_NULL(f);

    fechaSvg(f);
    remove(ARQ_TMP);
}

/*  
    Caso de borda: abrir e fechar sem desenhar nada ainda deve produzir um
    documento XML/SVG bem-formado — <svg ...> de abertura e </svg> de
    fechamento, mesmo vazio por dentro.
*/
void test_arquivoVazio_AindaAssimEhSvgBemFormado(void){
    ArqSvg *f = abreEscritaSvg(ARQ_TMP, 200.0, 150.0);
    TEST_ASSERT_NOT_NULL(f);

    fechaSvg(f);

    char *conteudo = lerArquivoCompleto(ARQ_TMP);
    TEST_ASSERT_NOT_NULL(conteudo);

    TEST_ASSERT_NOT_NULL(strstr(conteudo, "<svg"));
    TEST_ASSERT_NOT_NULL(strstr(conteudo, "</svg>"));

    free(conteudo);
    remove(ARQ_TMP);
}

// Conteúdo: retângulo

void test_svgRetangulo_ConteudoContemTagECoordenadas(void){
    ArqSvg *f = abreEscritaSvg(ARQ_TMP, 200.0, 200.0);
    TEST_ASSERT_NOT_NULL(f);

    svgRetangulo(f, 10.0, 20.0, 100.0, 50.0, "orange", "black", 2.0);

    fechaSvg(f); // escreve tudo -> fecha -> só então lê

    char *conteudo = lerArquivoCompleto(ARQ_TMP);
    TEST_ASSERT_NOT_NULL(conteudo);

    TEST_ASSERT_NOT_NULL(strstr(conteudo, "<rect"));
    TEST_ASSERT_NOT_NULL(strstr(conteudo, "10"));
    TEST_ASSERT_NOT_NULL(strstr(conteudo, "20"));
    TEST_ASSERT_NOT_NULL(strstr(conteudo, "100"));
    TEST_ASSERT_NOT_NULL(strstr(conteudo, "50"));
    TEST_ASSERT_NOT_NULL(strstr(conteudo, "orange"));
    TEST_ASSERT_NOT_NULL(strstr(conteudo, "black"));

    free(conteudo);
    remove(ARQ_TMP);
}

// Conteúdo: texto

void test_svgTexto_ConteudoContemTagETexto(void){
    ArqSvg *f = abreEscritaSvg(ARQ_TMP, 200.0, 200.0);
    TEST_ASSERT_NOT_NULL(f);

    svgTexto(f, 15.0, 25.0, "cep15", "blue");

    fechaSvg(f);

    char *conteudo = lerArquivoCompleto(ARQ_TMP);
    TEST_ASSERT_NOT_NULL(conteudo);

    TEST_ASSERT_NOT_NULL(strstr(conteudo, "<text"));
    TEST_ASSERT_NOT_NULL(strstr(conteudo, "cep15"));
    TEST_ASSERT_NOT_NULL(strstr(conteudo, "blue"));

    free(conteudo);
    remove(ARQ_TMP);
}
/*
    Caracteres especiais de XML devem sair escapados, não crus — senão um
    CEP (ou outro texto vindo de entrada não controlada) contendo & < >
    produziria um .svg malformado.
*/
void test_svgTexto_CaracteresEspeciais_SaoEscapados(void){
    ArqSvg *f = abreEscritaSvg(ARQ_TMP, 200.0, 200.0);
    TEST_ASSERT_NOT_NULL(f);

    svgTexto(f, 0.0, 0.0, "a & b < c > d", "black");

    fechaSvg(f);

    char *conteudo = lerArquivoCompleto(ARQ_TMP);
    TEST_ASSERT_NOT_NULL(conteudo);

    TEST_ASSERT_NOT_NULL(strstr(conteudo, "&amp;"));
    TEST_ASSERT_NOT_NULL(strstr(conteudo, "&lt;"));
    TEST_ASSERT_NOT_NULL(strstr(conteudo, "&gt;"));
    TEST_ASSERT_NULL(strstr(conteudo, "& b"));

    free(conteudo);
    remove(ARQ_TMP);
}

// Conteúdo: linha

void test_svgLinha_ConteudoContemTagECoordenadas(void){
    ArqSvg *f = abreEscritaSvg(ARQ_TMP, 200.0, 200.0);
    TEST_ASSERT_NOT_NULL(f);

    svgLinha(f, 5.0, 5.0, 95.0, 95.0, "red", 1.5);

    fechaSvg(f);

    char *conteudo = lerArquivoCompleto(ARQ_TMP);
    TEST_ASSERT_NOT_NULL(conteudo);

    TEST_ASSERT_NOT_NULL(strstr(conteudo, "<line"));
    TEST_ASSERT_NOT_NULL(strstr(conteudo, "5"));
    TEST_ASSERT_NOT_NULL(strstr(conteudo, "95"));
    TEST_ASSERT_NOT_NULL(strstr(conteudo, "red"));

    free(conteudo);
    remove(ARQ_TMP);
}

void test_svgCirculo_ConteudoContemTagECoordenadas(void){
    ArqSvg *f = abreEscritaSvg(ARQ_TMP, 200.0, 200.0);
    TEST_ASSERT_NOT_NULL(f);
 
    svgCirculo(f, 30.0, 40.0, 5.0, "red", "black", 1.0);
 
    fechaSvg(f);
 
    char *conteudo = lerArquivoCompleto(ARQ_TMP);
    TEST_ASSERT_NOT_NULL(conteudo);
 
    TEST_ASSERT_NOT_NULL(strstr(conteudo, "<circle"));
    TEST_ASSERT_NOT_NULL(strstr(conteudo, "30"));
    TEST_ASSERT_NOT_NULL(strstr(conteudo, "40"));
    TEST_ASSERT_NOT_NULL(strstr(conteudo, "red"));
    TEST_ASSERT_NOT_NULL(strstr(conteudo, "black"));
 
    free(conteudo);
    remove(ARQ_TMP);
}

// Múltiplos elementos no mesmo arquivo

// *Obs: Confirma que chamadas sucessivas se acumulam no arquivo (não se sobrescrevem) e que o documento continua bem-formado ao final.
void test_multiplosElementos_TodosPresentesNoArquivoFinal(void){
    ArqSvg *f = abreEscritaSvg(ARQ_TMP, 200.0, 200.0);
    TEST_ASSERT_NOT_NULL(f);

    svgRetangulo(f, 10.0, 10.0, 50.0, 50.0, "orange", "black", 2.0);
    svgTexto(f, 15.0, 15.0, "cep01", "blue");
    svgLinha(f, 0.0, 0.0, 200.0, 200.0, "red", 1.0);

    fechaSvg(f);

    char *conteudo = lerArquivoCompleto(ARQ_TMP);
    TEST_ASSERT_NOT_NULL(conteudo);

    TEST_ASSERT_NOT_NULL(strstr(conteudo, "<rect"));
    TEST_ASSERT_NOT_NULL(strstr(conteudo, "<text"));
    TEST_ASSERT_NOT_NULL(strstr(conteudo, "<line"));
    TEST_ASSERT_NOT_NULL(strstr(conteudo, "</svg>"));

    free(conteudo);
    remove(ARQ_TMP);
}

int main(void){
    UNITY_BEGIN();

    RUN_TEST(test_abreEscritaSvg_CaminhoInvalido_RetornaNull);
    RUN_TEST(test_abreEscritaSvg_CaminhoValido_RetornaPonteiroValido);
    RUN_TEST(test_arquivoVazio_AindaAssimEhSvgBemFormado);
    RUN_TEST(test_svgRetangulo_ConteudoContemTagECoordenadas);
    RUN_TEST(test_svgTexto_ConteudoContemTagETexto);
    RUN_TEST(test_svgTexto_CaracteresEspeciais_SaoEscapados);
    RUN_TEST(test_svgLinha_ConteudoContemTagECoordenadas);
    RUN_TEST(test_svgCirculo_ConteudoContemTagECoordenadas);
    RUN_TEST(test_multiplosElementos_TodosPresentesNoArquivoFinal);

    return UNITY_END();
}