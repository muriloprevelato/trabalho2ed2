#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cidade.h"
#include "quadra.h"
#include "svg.h"
#include "leitorGeo.h"
#include "grafo.h"
#include "vertice.h"
#include "leitorVia.h"
#include "registradores.h"
#include "leitorQry.h"

#define PATH_MAX_TAM  1024 // tamanho de dir_entrada, dir_saida e nomes de arquivo
#define NOME_MAX_TAM   256
#define CAMINHO_MAX_TAM (PATH_MAX_TAM * 3)
// Defini essa última constante para tentar evitar os vários warnings que aparecem na hora de compilar.

// Helpers

// Garante que o caminho não termina com '/'.
static void trataPath(char *dest, int tamMax, const char *src){
    int tam = (int) strlen(src);
    if(tam >= tamMax) tam = tamMax - 1;
    strncpy(dest, src, (size_t) tam);
    dest[tam] = '\0';
    if(tam > 0 && dest[tam - 1] == '/')
        dest[tam - 1] = '\0';
}

// Extrai o nome base de um caminho, sem diretório e sem extensão.
static void nomeBase(char *dest, int tamMax, const char *caminho){
    const char *inicio = strrchr(caminho, '/');
    inicio = inicio ? inicio + 1 : caminho;

    const char *ponto = strrchr(inicio, '.');
    int tam = ponto ? (int)(ponto - inicio) : (int) strlen(inicio);
    if(tam >= tamMax) tam = tamMax - 1;

    strncpy(dest, inicio, (size_t) tam);
    dest[tam] = '\0';
}

// Extrai só o nome do arquivo de um caminho (sem diretório).
static const char* nomeArquivo(const char *caminho){
    const char *p = strrchr(caminho, '/');
    return p ? p + 1 : caminho;
}

int main(int argc, char const *argv[]){
    char dir_entrada[PATH_MAX_TAM] = "."; // default: diretório corrente
    char dir_saida  [PATH_MAX_TAM] = "";
    char arq_geo    [NOME_MAX_TAM] = "";
    char arq_qry    [NOME_MAX_TAM] = "";
    char arq_via    [NOME_MAX_TAM] = "";

    int i = 1;
    while(i < argc){
        if(strcmp(argv[i], "-e") == 0 && i + 1 < argc){
            trataPath(dir_entrada, PATH_MAX_TAM, argv[++i]);
        } else if(strcmp(argv[i], "-f") == 0 && i + 1 < argc){
            strncpy(arq_geo, argv[++i], NOME_MAX_TAM - 1);
            arq_geo[NOME_MAX_TAM - 1] = '\0';
        } else if(strcmp(argv[i], "-o") == 0 && i + 1 < argc){
            trataPath(dir_saida, PATH_MAX_TAM, argv[++i]);
        } else if(strcmp(argv[i], "-q") == 0 && i + 1 < argc){
            strncpy(arq_qry, argv[++i], NOME_MAX_TAM - 1);
            arq_qry[NOME_MAX_TAM - 1] = '\0';
        } else if(strcmp(argv[i], "-v") == 0 && i + 1 < argc){
            strncpy(arq_via, argv[++i], NOME_MAX_TAM - 1);
            arq_via[NOME_MAX_TAM - 1] = '\0';
        } else {
            fprintf(stderr, "[main] aviso: parametro desconhecido '%s' ignorado\n",
                    argv[i]);
        }
        i++;
    }

    if(arq_geo[0] == '\0'){
        fprintf(stderr, "[main] erro: parametro -f (arq.geo) eh obrigatorio\n");
        return 1;
    }
    if(dir_saida[0] == '\0'){
        fprintf(stderr, "[main] erro: parametro -o (diretorio de saida) eh obrigatorio\n");
        return 1;
    }

    char caminho_geo[CAMINHO_MAX_TAM];
    snprintf(caminho_geo, CAMINHO_MAX_TAM, "%s/%s", dir_entrada, arq_geo);

    char base_geo[NOME_MAX_TAM];
    nomeBase(base_geo, NOME_MAX_TAM, arq_geo);

    // SVG produzido após a leitura do .geo: <base_geo>.svg
    char caminho_svg_geo[CAMINHO_MAX_TAM];
    snprintf(caminho_svg_geo, CAMINHO_MAX_TAM, "%s/%s.svg", dir_saida, base_geo);

    // Caminho de entrada do .qry 
    char caminho_qry[CAMINHO_MAX_TAM];
    char caminho_svg_qry[CAMINHO_MAX_TAM];
    char caminho_txt_qry[CAMINHO_MAX_TAM];
    if(arq_qry[0] != '\0'){
        snprintf(caminho_qry, CAMINHO_MAX_TAM, "%s/%s", dir_entrada, arq_qry);

        char base_qry[NOME_MAX_TAM];
        nomeBase(base_qry, NOME_MAX_TAM, nomeArquivo(arq_qry));
        snprintf(caminho_svg_qry, CAMINHO_MAX_TAM, "%s/%s-%s.svg",
                 dir_saida, base_geo, base_qry);
        snprintf(caminho_txt_qry, CAMINHO_MAX_TAM, "%s/%s-%s.txt",
                 dir_saida, base_geo, base_qry);
    }

    Cidade *cidade = criarCidade();
    if(cidade == NULL){
        fprintf(stderr, "[main] erro: falha ao criar a cidade\n");
        return 1;
    }

    if(lerArquivoGeo(caminho_geo, cidade) != GEO_OK){
        fprintf(stderr, "[main] erro: nao foi possivel abrir o arquivo .geo: %s\n",
                caminho_geo);
        destruirCidade(cidade);
        return 1;
    }

    /*
    Leitura do .via (opcional) 
    Diferente de -f (obrigatório, falha aborta o programa), -v é
    opcional.
    */
    Grafo *grafo = NULL;
    if(arq_via[0] != '\0'){
        char caminho_via[CAMINHO_MAX_TAM];
        snprintf(caminho_via, CAMINHO_MAX_TAM, "%s/%s", dir_entrada, arq_via);

        grafo = criarGrafo();
        if(grafo == NULL){
            fprintf(stderr, "[main] aviso: falha ao criar o grafo,"
                            " viario nao sera desenhado\n");
        } else if(lerArquivoVia(caminho_via, grafo) != VIA_OK){
            fprintf(stderr, "[main] aviso: nao foi possivel abrir o arquivo"
                            " .via: %s -- viario nao sera desenhado\n",
                    caminho_via);
            destruirGrafo(grafo);
            grafo = NULL;
        }
    }

    //  Dimensões do mapa (quadras + grafo, se houver)
    // Centralizado em leitorQry.h -> usado tanto para o SVG do .geo
    // quanto, mais abaixo, para o SVG combinado do .qry (mesma cidade e
    // grafo, mesmas dimensões, calculadas uma única vez).

    double larguraMapa, alturaMapa;
    calcularDimensoesMapa(cidade, grafo, &larguraMapa, &alturaMapa);

    ArqSvg *svgGeo = abreEscritaSvg(caminho_svg_geo, larguraMapa, alturaMapa);
    if(svgGeo == NULL){
        fprintf(stderr, "[main] erro: nao foi possivel criar o SVG: %s\n",
                caminho_svg_geo);
        if(grafo != NULL) destruirGrafo(grafo);
        destruirCidade(cidade);
        return 1;
    }

    desenharMapaBase(svgGeo, cidade, grafo);
    fechaSvg(svgGeo);

    //.qry (opcional) 
    if(arq_qry[0] != '\0'){
        Registradores *registradores = criarRegistradores();
        if(registradores == NULL){
            fprintf(stderr, "[main] aviso: falha ao criar registradores,"
                            " .qry nao sera processado\n");
        } else {
            ArqSvg *svgQry = abreEscritaSvg(caminho_svg_qry, larguraMapa, alturaMapa);
            FILE *txtQry = fopen(caminho_txt_qry, "w");

            if(svgQry == NULL || txtQry == NULL){
                fprintf(stderr, "[main] aviso: nao foi possivel criar os arquivos"
                                " de saida do .qry (%s / %s)\n",
                        caminho_svg_qry, caminho_txt_qry);
                if(svgQry != NULL) fechaSvg(svgQry);
                if(txtQry != NULL) fclose(txtQry);
            } else {
                if(processarArquivoQry(caminho_qry, cidade, grafo, registradores,
                                        svgQry, txtQry) != QRY_OK){
                    fprintf(stderr, "[main] aviso: nao foi possivel abrir o"
                                    " arquivo .qry: %s\n", caminho_qry);
                }
                fechaSvg(svgQry);
                fclose(txtQry);
            }

            destruirRegistradores(registradores);
        }
    }

    // clean

    if(grafo != NULL) destruirGrafo(grafo);
    destruirCidade(cidade);
    return 0;
}