#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include "cidade.h"
#include "quadra.h"
#include "svg.h"
#include "leitorGeo.h"
#include "grafo.h"
#include "vertice.h"
#include "leitorVia.h"

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

// Contexto para cálculo do bounding box.

typedef struct {
    double xMin, yMin;
    double xMax, yMax;
} ContextoBBox;

static void visitanteBBox(Quadra *q, void *contexto){
    ContextoBBox *ctx = (ContextoBBox *) contexto;

    // Âncora SE = canto superior-esquerdo na tela (notação invertida).
    double x  = getQuadraX(q);
    double y  = getQuadraY(q);
    double x2 = x + getQuadraW(q);
    double y2 = y + getQuadraH(q);

    if(x < ctx->xMin) ctx->xMin = x;
    if(y < ctx->yMin) ctx->yMin = y;
    if(x2 > ctx->xMax) ctx->xMax = x2;
    if(y2 > ctx->yMax) ctx->yMax = y2;
}

// Contexto para o callback de desenho.
typedef struct {
    ArqSvg *svg;
} ContextoDesenho;

static void visitanteDesenho(Quadra *q, void *contexto){
    ContextoDesenho *ctx = (ContextoDesenho *) contexto;

    // Âncora SE = (x, y) na notação do projeto.
    // svgRetangulo espera o canto superior-esquerdo - que, na notação
    // invertida do projeto, É a âncora (menor x, menor y na tela).
    // Portanto a conversão é direta: passa x e y sem ajuste.
    svgRetangulo(ctx->svg,
                 getQuadraX(q),
                 getQuadraY(q),
                 getQuadraW(q),
                 getQuadraH(q),
                 getQuadraCFill(q),
                 getQuadraCStrk(q),
                 getQuadraSw(q));

    // Texto do CEP posicionado levemente deslocado do canto superior-esquerdo da quadra, para ficar visível dentro do retângulo.
    svgTexto(ctx->svg,
             getQuadraX(q) + 2.0,
             getQuadraY(q) + 10.0,
             getQuadraCep(q),
             "black");
}
 
// Estende o bounding box (já calculado a partir das quadras) para
// também considerar as coordenadas dos vértices do grafo 
static void visitanteBBoxVertice(Vertice *v, void *contexto){
    ContextoBBox *ctx = (ContextoBBox *) contexto;
 
    double x = getVerticeX(v);
    double y = getVerticeY(v);
 
    if(x < ctx->xMin) ctx->xMin = x;
    if(y < ctx->yMin) ctx->yMin = y;
    if(x > ctx->xMax) ctx->xMax = x;
    if(y > ctx->yMax) ctx->yMax = y;
}
 
#define RAIO_VERTICE 2.0    // raio do círculo que marca cada vértice no SVG
#define COR_VIA "gray" // .via não tem comando de cor. Escolhi uma cor neutra. Fiz testes com amarelo e estava ruim
#define SW_VIA 1.0    
 
// Contexto para desenhar as arestas de saída de UM vértice específico
typedef struct {
    ArqSvg *svg;
    Vertice *origemAtual;
} ContextoDesenhoArestas;
 
static void visitanteDesenharAresta(Aresta *a, void *contexto){
    ContextoDesenhoArestas *ctx = (ContextoDesenhoArestas *) contexto;
    Vertice *destino = getArestaDestino(a);
 
    svgLinha(ctx->svg, getVerticeX(ctx->origemAtual), getVerticeY(ctx->origemAtual), getVerticeX(destino),getVerticeY(destino), COR_VIA, SW_VIA);
}
 
// Contexto para o padrão composto documentado em grafo.h: percorre
// TODOS os vértices e, para cada um, percorre suas arestas de saída
typedef struct {
    ArqSvg *svg;
    Grafo *grafo;
} ContextoDesenhoGrafo;
 
static void visitanteDesenharVerticeEArestas(Vertice *v, void *contexto){
    ContextoDesenhoGrafo *ctx = (ContextoDesenhoGrafo *) contexto;

    svgCirculo(ctx->svg, getVerticeX(v), getVerticeY(v), RAIO_VERTICE, COR_VIA, "black", 0.5);

    ContextoDesenhoArestas ctxArestas = { ctx->svg, v };
    percorrerArestasSaindo(ctx->grafo, getVerticeId(v),
                            visitanteDesenharAresta, &ctxArestas);
}


// main 

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

    // Validação dos obrigatórios

    if(arq_geo[0] == '\0'){
        fprintf(stderr, "[main] erro: parametro -f (arq.geo) eh obrigatorio\n");
        return 1;
    }
    if(dir_saida[0] == '\0'){
        fprintf(stderr, "[main] erro: parametro -o (diretorio de saida) eh obrigatorio\n");
        return 1;
    }

    // Montagem dos caminhos de entrada

    char caminho_geo[CAMINHO_MAX_TAM];
    snprintf(caminho_geo, CAMINHO_MAX_TAM, "%s/%s", dir_entrada, arq_geo);

    // Montagem dos caminhos de saída

    char base_geo[NOME_MAX_TAM];
    nomeBase(base_geo, NOME_MAX_TAM, arq_geo);

    // SVG produzido após a leitura do .geo
    char caminho_svg_geo[CAMINHO_MAX_TAM];
    snprintf(caminho_svg_geo, CAMINHO_MAX_TAM, "%s/%s.svg", dir_saida, base_geo);

    // SVG e TXT produzidos após o .qry
    char caminho_svg_qry[CAMINHO_MAX_TAM];
    char caminho_txt_qry[CAMINHO_MAX_TAM];
    if(arq_qry[0] != '\0'){
        char base_qry[NOME_MAX_TAM];
        nomeBase(base_qry, NOME_MAX_TAM, nomeArquivo(arq_qry));
        snprintf(caminho_svg_qry, CAMINHO_MAX_TAM, "%s/%s-%s.svg",
                 dir_saida, base_geo, base_qry);
        snprintf(caminho_txt_qry, CAMINHO_MAX_TAM, "%s/%s-%s.txt",
                 dir_saida, base_geo, base_qry);
    }

    // Leitura do .geo

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

    // Leitura do .via -> Opcional

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

    // SVG inicial (após leitura do .geo)

    // Passo 1: calcula o bounding box de todas as quadras para dimensionar
    // o canvas - assim o SVG se ajusta ao tamanho real do mapa.
    ContextoBBox bbox = { DBL_MAX, DBL_MAX, -DBL_MAX, -DBL_MAX };
    percorrerQuadrasCidade(cidade, visitanteBBox, &bbox);
    if(grafo != NULL){
        percorrerVertices(grafo, visitanteBBoxVertice, &bbox);
    }
 
    // Cidade vazia e sem grafo (ou grafo tambem vazio): canvas minimo.
    if(bbox.xMin == DBL_MAX){
        bbox.xMax = bbox.yMax = 100.0;
    }
 
    ArqSvg *svgGeo = abreEscritaSvg(caminho_svg_geo, bbox.xMax, bbox.yMax);
    if(svgGeo == NULL){
        fprintf(stderr, "[main] erro: nao foi possivel criar o SVG: %s\n",
                caminho_svg_geo);
        if(grafo != NULL) destruirGrafo(grafo);
        destruirCidade(cidade);
        return 1;
    }

    // Passo 2: desenha todas as quadras.
    ContextoDesenho ctxDesenho = { svgGeo };
    percorrerQuadrasCidade(cidade, visitanteDesenho, &ctxDesenho);

    // Desenha o mapa viário (vértices + arestas) por cima das quadras,
    // se um .via foi fornecido e lido com sucesso.
    if(grafo != NULL){
        ContextoDesenhoGrafo ctxGrafo = { svgGeo, grafo };
        percorrerVertices(grafo, visitanteDesenharVerticeEArestas, &ctxGrafo);
    }
    
    fechaSvg(svgGeo);

    

    // *** Não implementando ainda .qry 

    if(arq_qry[0] != '\0'){
        fprintf(stderr, "[main] aviso: -q fornecido mas leitor de consultas ainda"
                        " nao implementado, ignorando %s\n", arq_qry);
        (void) caminho_svg_qry;
        (void) caminho_txt_qry;
    }

    // clean

    destruirCidade(cidade);
    return 0;
}