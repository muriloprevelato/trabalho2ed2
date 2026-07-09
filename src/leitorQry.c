#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <float.h>
#include "leitorQry.h"
#include "cidade.h"
#include "quadra.h"
#include "grafo.h"
#include "vertice.h"
#include "registradores.h"
#include "svg.h"
#include "resolveEndereco.h"
#include "caminhoMinimo.h"
#include "componentesConexos.h"
#include "ampliacaoViaria.h"

#define LINHA_MAX 512 // fgets: tamanho máximo de linha do .qry

// Migrado de main.c -> mesma lógica de bbox absoluto (canvas de 0 até o
// extremo, não extensão relativa) já usada desde a primeira fatia
// visível, agora centralizada aqui.

typedef struct {
    double xMin, yMin, xMax, yMax;
} ContextoDimensoes;

static void visitanteDimensoesQuadra(Quadra *q, void *contexto){
    ContextoDimensoes *ctx = (ContextoDimensoes *) contexto;

    double x  = getQuadraX(q);
    double y  = getQuadraY(q);
    double x2 = x + getQuadraW(q);
    double y2 = y + getQuadraH(q);

    if(x  < ctx->xMin) ctx->xMin = x;
    if(y  < ctx->yMin) ctx->yMin = y;
    if(x2 > ctx->xMax) ctx->xMax = x2;
    if(y2 > ctx->yMax) ctx->yMax = y2;
}

static void visitanteDimensoesVertice(Vertice *v, void *contexto){
    ContextoDimensoes *ctx = (ContextoDimensoes *) contexto;

    double x = getVerticeX(v);
    double y = getVerticeY(v);

    if(x < ctx->xMin) ctx->xMin = x;
    if(y < ctx->yMin) ctx->yMin = y;
    if(x > ctx->xMax) ctx->xMax = x;
    if(y > ctx->yMax) ctx->yMax = y;
}

void calcularDimensoesMapa(const Cidade *cidade, const Grafo *grafo, double *larguraSaida, double *alturaSaida){
    assert(cidade != NULL);
    assert(larguraSaida != NULL);
    assert(alturaSaida != NULL);

    ContextoDimensoes ctx = { DBL_MAX, DBL_MAX, -DBL_MAX, -DBL_MAX };
    percorrerQuadrasCidade(cidade, visitanteDimensoesQuadra, &ctx);
    if(grafo != NULL){
        percorrerVertices(grafo, visitanteDimensoesVertice, &ctx);
    }

    if(ctx.xMin == DBL_MAX){
        ctx.xMax = ctx.yMax = 100.0; // cidade e grafo vazios: canvas mínimo
    }

    *larguraSaida = ctx.xMax;
    *alturaSaida  = ctx.yMax;
}

// Também migrado de main.c -> os mesmos visitantes de desenho que
// existiam lá (visitanteDesenho, visitanteDesenharVerticeEArestas etc.),
// agora centralizados aqui para servir tanto o SVG do .geo (main.c)
// quanto a primeira camada do SVG combinado (processarArquivoQry).

#define RAIO_VERTICE 2.0
#define COR_VIA "gray"
#define SW_VIA 1.0

static void visitanteDesenhoQuadra(Quadra *q, void *contexto){
    ArqSvg *svg = (ArqSvg *) contexto;

    svgRetangulo(svg, getQuadraX(q), getQuadraY(q), getQuadraW(q), getQuadraH(q),
                 getQuadraCFill(q), getQuadraCStrk(q), getQuadraSw(q));
    svgTexto(svg, getQuadraX(q) + 2.0, getQuadraY(q) + 10.0, getQuadraCep(q), "black");
}

typedef struct {
    ArqSvg *svg;
    Vertice *origemAtual;
} ContextoDesenhoArestas;

static void visitanteDesenharAresta(Aresta *a, void *contexto){
    ContextoDesenhoArestas *ctx = (ContextoDesenhoArestas *) contexto;
    Vertice *destino = getArestaDestino(a);

    svgLinha(ctx->svg,
             getVerticeX(ctx->origemAtual), getVerticeY(ctx->origemAtual),
             getVerticeX(destino),          getVerticeY(destino),
             COR_VIA, SW_VIA);
}

typedef struct {
    ArqSvg *svg;
    const Grafo *grafo;
} ContextoDesenhoGrafo;

static void visitanteDesenharVerticeEArestas(Vertice *v, void *contexto){
    ContextoDesenhoGrafo *ctx = (ContextoDesenhoGrafo *) contexto;

    svgCirculo(ctx->svg, getVerticeX(v), getVerticeY(v), RAIO_VERTICE,
               COR_VIA, "black", 0.5);

    ContextoDesenhoArestas ctxArestas = { ctx->svg, v };
    percorrerArestasSaindo(ctx->grafo, getVerticeId(v),
                            visitanteDesenharAresta, &ctxArestas);
}

void desenharMapaBase(ArqSvg *svg, const Cidade *cidade, const Grafo *grafo){
    assert(svg    != NULL);
    assert(cidade != NULL);

    percorrerQuadrasCidade(cidade, visitanteDesenhoQuadra, svg);

    if(grafo != NULL){
        ContextoDesenhoGrafo ctxGrafo = { svg, grafo };
        percorrerVertices(grafo, visitanteDesenharVerticeEArestas, &ctxGrafo);
    }
}

// Vértice mais próximo (duplicado de resolveEndereco.c) 
//
// resolveEndereco.c tem essa mesma lógica, mas privada (static) e só
// acessível via resolverEndereco, que devolve coordenada+vértice juntos
// a partir de um endereço. Aqui a necessidade é diferente: o comando p?
// só tem a coordenada já resolvida (guardada no registrador por um @o?
// anterior), sem o id do vértice -> Registradores não guarda o id do
// vértice, só (x, y, texto). Duplicação consciente e pequena (+-10
// linhas)

typedef struct {
    double xAlvo, yAlvo;
    Vertice *maisProximo;
    double menorDistanciaQuadrado;
} ContextoVerticeMaisProximo;

static void visitanteEncontrarMaisProximo(Vertice *v, void *contexto){
    ContextoVerticeMaisProximo *ctx = (ContextoVerticeMaisProximo *) contexto;

    double dx = getVerticeX(v) - ctx->xAlvo;
    double dy = getVerticeY(v) - ctx->yAlvo;
    double distQuadrado = dx * dx + dy * dy;

    if(ctx->maisProximo == NULL || distQuadrado < ctx->menorDistanciaQuadrado){
        ctx->maisProximo = v;
        ctx->menorDistanciaQuadrado = distQuadrado;
    }
}

// Devolve NULL se o grafo não tiver nenhum vértice.
static Vertice* buscarVerticeMaisProximo(const Grafo *grafo, double x, double y){
    ContextoVerticeMaisProximo ctx = { x, y, NULL, 0.0 };
    percorrerVertices(grafo, visitanteEncontrarMaisProximo, &ctx);
    return ctx.maisProximo;
}

static int parseIndiceRegistrador(const char *token){
    if(token[0] != 'R' && token[0] != 'r') return -1;

    int indice;
    if(sscanf(token + 1, "%d", &indice) != 1) return -1;
    if(indice < 0 || indice >= REGISTRADOR_MAX) return -1;

    return indice;
}

// mvm: ambos os extremos dentro da região ─

typedef struct {
    Grafo *grafo;
    double novoVm;
    double xMin, yMin, xMax, yMax; // região já convertida de x,y,w,h
} ContextoMvm;

static int dentroDaRegiao(double px, double py, const ContextoMvm *ctx){
    return px >= ctx->xMin && px <= ctx->xMax &&
           py >= ctx->yMin && py <= ctx->yMax;
}

static void visitanteAplicarMvm(const char *idOrigem, Aresta *a, void *contexto){
    ContextoMvm *ctx = (ContextoMvm *) contexto;

    Vertice *origem  = buscarVertice(ctx->grafo, idOrigem);
    Vertice *destino = getArestaDestino(a);

    int origemDentro  = dentroDaRegiao(getVerticeX(origem),  getVerticeY(origem),  ctx);
    int destinoDentro = dentroDaRegiao(getVerticeX(destino), getVerticeY(destino), ctx);

    if(origemDentro && destinoDentro){
        setArestaVm(a, ctx->novoVm);
    }
}

static void processarMvm(const char *linha, int numLinha, Grafo *grafo, FILE *txtSaida){
    double v, x, y, w, h;
    int campos = sscanf(linha, "mvm %lf %lf %lf %lf %lf", &v, &x, &y, &w, &h);
    if(campos != 5){
        fprintf(stderr, "[leitorQry] linha %d: comando 'mvm' malformado"
                        " (esperado 5 campos, lidos %d) -> ignorado\n", numLinha, campos);
        return;
    }

    fprintf(txtSaida, "[*] mvm %.2f %.2f %.2f %.2f %.2f\n", v, x, y, w, h);

    if(grafo == NULL){
        fprintf(txtSaida, "Sistema viario indisponivel.\n\n");
        return;
    }

    ContextoMvm ctx = { grafo, v, x, y, x + w, y + h };
    percorrerTodasArestas(grafo, visitanteAplicarMvm, &ctx);

    fprintf(txtSaida, "Velocidades atualizadas na regiao.\n\n");
}

// @o?: resolve endereço, guarda no registrador, marca no SVG 

static void processarArrobaO(const char *linha, int numLinha, const Cidade *cidade,
                              const Grafo *grafo, Registradores *registradores,
                              ArqSvg *svgCombinado, FILE *txtSaida){
    char regToken[16];
    char cep[QUADRA_CEP_MAX];
    char faceStr[4];
    double num;
 
    int campos = sscanf(linha, "@o? %15s %19s %3s %lf", regToken, cep, faceStr, &num);
    if(campos != 4){
        fprintf(stderr, "[leitorQry] linha %d: comando '@o?' malformado"
                        " (esperado 4 campos, lidos %d) — ignorado\n", numLinha, campos);
        return;
    }
 
    int indiceReg = parseIndiceRegistrador(regToken);
    if(indiceReg < 0){
        fprintf(stderr, "[leitorQry] linha %d: registrador '%s' invalido — ignorado\n",
                numLinha, regToken);
        return;
    }
 
    char face = faceStr[0];
 
    fprintf(txtSaida, "[*] @o? %s %s %c %.2f\n", regToken, cep, face, num);
 
    if(grafo == NULL){
        fprintf(txtSaida, "Sistema viario indisponivel.\n\n");
        return;
    }
 
    double x, y;
    char idVertice[VERTICE_ID_MAX];
    int resultado = resolverEndereco(cidade, grafo, cep, face, num, &x, &y, idVertice);
 
    if(resultado != RESOLVE_OK){
        fprintf(txtSaida, "Endereco invalido: %s/%c/%.2f\n\n", cep, face, num);
        return;
    }
 
    char textoEndereco[REGISTRADOR_TEXTO_MAX];
    snprintf(textoEndereco, REGISTRADOR_TEXTO_MAX, "%s/%c/%.2f", cep, face, num);
    setRegistrador(registradores, indiceReg, x, y, textoEndereco);
 
    fprintf(txtSaida, "Coordenada: (%.2f, %.2f)\n\n", x, y);
 

    svgLinhaTracejada(svgCombinado, x, 0.0, x, 20.0, "red", 1.0);
    char rotulo[16];
    snprintf(rotulo, sizeof(rotulo), "R%d", indiceReg);
    svgTexto(svgCombinado, x + 2.0, 15.0, rotulo, "red");
}

// regs: componentes conexos, bounding boxes coloridos e transparentes

// SVG aceita rgba() diretamente como valor de cor -> nenhuma mudança em svg.h é necessária. 
#define NUM_CORES_PALETA 6
static const char *PALETA_CORES[NUM_CORES_PALETA] = {
    "255,0,0", "0,255,0", "0,0,255", "255,165,0", "128,0,128", "0,200,200"
};

typedef struct {
    double xMin, yMin, xMax, yMax;
} ContextoBBoxComponente;

static void visitanteBBoxComponente(Vertice *v, void *contexto){
    ContextoBBoxComponente *ctx = (ContextoBBoxComponente *) contexto;

    double x = getVerticeX(v);
    double y = getVerticeY(v);

    if(x < ctx->xMin) ctx->xMin = x;
    if(y < ctx->yMin) ctx->yMin = y;
    if(x > ctx->xMax) ctx->xMax = x;
    if(y > ctx->yMax) ctx->yMax = y;
}

static void processarRegs(const char *linha, int numLinha, Grafo *grafo,
                           ArqSvg *svgCombinado, FILE *txtSaida){
    double vl;
    int campos = sscanf(linha, "regs %lf", &vl);
    if(campos != 1){
        fprintf(stderr, "[leitorQry] linha %d: comando 'regs' malformado"
                        " (esperado 1 campo, lidos %d) -> ignorado\n", numLinha, campos);
        return;
    }

    fprintf(txtSaida, "[*] regs %.2f\n", vl);

    if(grafo == NULL){
        fprintf(txtSaida, "Sistema viario indisponivel.\n\n");
        return;
    }

    Componentes *comp = calcularComponentesConexos(grafo, vl);
    if(comp == NULL){
        fprintf(stderr, "[leitorQry] linha %d: falha ao calcular componentes"
                        " (alocacao)\n", numLinha);
        return;
    }

    int n = componentesNumComponentes(comp);
    fprintf(txtSaida, "Numero de componentes conexos: %d\n\n", n);

    for(int i = 0; i < n; i++){
        ContextoBBoxComponente ctx = { DBL_MAX, DBL_MAX, -DBL_MAX, -DBL_MAX };
        percorrerVerticesDoComponente(comp, i, visitanteBBoxComponente, &ctx);

        if(ctx.xMin == DBL_MAX) continue; // componente sem vértices, não deveria acontecer

        char cor[48];
        snprintf(cor, sizeof(cor), "rgba(%s,0.5)", PALETA_CORES[i % NUM_CORES_PALETA]);

        svgRetangulo(svgCombinado, ctx.xMin, ctx.yMin,
                     ctx.xMax - ctx.xMin, ctx.yMax - ctx.yMin,
                     cor, "black", 1.0);
    }

    destruirComponentes(comp);
}

//exp: ampliação viária, arestas selecionadas em vermelho grosso 

typedef struct {
    ArqSvg *svg;
    const Grafo *grafo;
} ContextoDesenharAmpliacao;

static void visitanteDesenharAmpliacao(const char *idOrigem, Aresta *a, void *contexto){
    ContextoDesenharAmpliacao *ctx = (ContextoDesenharAmpliacao *) contexto;

    Vertice *origem  = buscarVertice(ctx->grafo, idOrigem);
    Vertice *destino = getArestaDestino(a);

    svgLinha(ctx->svg,
             getVerticeX(origem),  getVerticeY(origem),
             getVerticeX(destino), getVerticeY(destino),
             "red", 4.0);
}

static void processarExp(const char *linha, int numLinha, Grafo *grafo,
                          ArqSvg *svgCombinado, FILE *txtSaida){
    double vl;
    int campos = sscanf(linha, "exp %lf", &vl);
    if(campos != 1){
        fprintf(stderr, "[leitorQry] linha %d: comando 'exp' malformado"
                        " (esperado 1 campo, lidos %d) -> ignorado\n", numLinha, campos);
        return;
    }

    fprintf(txtSaida, "[*] exp %.2f\n", vl);

    if(grafo == NULL){
        fprintf(txtSaida, "Sistema viario indisponivel.\n\n");
        return;
    }

    Ampliacao *amp = calcularAmpliacaoViaria(grafo, vl);
    if(amp == NULL){
        fprintf(stderr, "[leitorQry] linha %d: falha ao calcular ampliacao"
                        " (alocacao)\n", numLinha);
        return;
    }

    fprintf(txtSaida, "Arestas ampliadas: %d\n\n", ampliacaoNumArestas(amp));

    ContextoDesenharAmpliacao ctxDesenho = { svgCombinado, grafo };
    percorrerArestasAmpliadas(amp, visitanteDesenharAmpliacao, &ctxDesenho);

    destruirAmpliacao(amp);
}

// p?: caminho mínimo (curto + rápido), percursos animados

static int contadorPathId = 0;
 
#define DURACAO_ANIMACAO_SEGUNDOS 6.0 // mesmo valor do exemplo dp projeto

static void desenharPercurso(ArqSvg *svg, const Grafo *grafo, Caminho *caminho, const char *cor){
    int n = caminhoNumVertices(caminho);
 
    // Caminho trivial (origem == destino, n==1): não há segmento pra
    // desenhar nem animar -> svgPath exige pelo menos 2 pontos. Só os
    // marcadores I/F fazem sentido nesse caso (os dois na mesma posição).
    if(n >= 2){
        double *pontosX = malloc((size_t) n * sizeof(double));
        double *pontosY = malloc((size_t) n * sizeof(double));
        assert(pontosX != NULL && pontosY != NULL);
 
        for(int i = 0; i < n; i++){
            Vertice *v = buscarVertice(grafo, caminhoObterVertice(caminho, i));
            pontosX[i] = getVerticeX(v);
            pontosY[i] = getVerticeY(v);
        }
 
        char idPath[32];
        snprintf(idPath, sizeof(idPath), "percurso_%d", contadorPathId++);
 
        svgPath(svg, pontosX, pontosY, n, idPath, cor, 3.0);
        svgCirculoAnimado(svg, idPath, 4.0, cor, DURACAO_ANIMACAO_SEGUNDOS);
 
        free(pontosX);
        free(pontosY);
    }
 
    Vertice *inicio = buscarVertice(grafo, caminhoObterVertice(caminho, 0));
    Vertice *fim    = buscarVertice(grafo, caminhoObterVertice(caminho, n - 1));
 
    svgCirculo(svg, getVerticeX(inicio), getVerticeY(inicio), 4.0, "orange", "black", 1.0);
    svgTexto(svg, getVerticeX(inicio) + 5.0, getVerticeY(inicio), "I", "black");
 
    svgCirculo(svg, getVerticeX(fim), getVerticeY(fim), 4.0, "red", "black", 1.0);
    svgTexto(svg, getVerticeX(fim) + 5.0, getVerticeY(fim), "F", "black");
}

static void escreverPercursoNoTxt(FILE *txtSaida, const char *rotulo, Caminho *caminho){
    fprintf(txtSaida, "%s (custo %.2f): ", rotulo, caminhoCustoTotal(caminho));
    for(int i = 0; i < caminhoNumVertices(caminho); i++){
        fprintf(txtSaida, "%s ", caminhoObterVertice(caminho, i));
    }
    fprintf(txtSaida, "\n");
}

static void processarPInterrogacao(const char *linha, int numLinha, Grafo *grafo,
                                    Registradores *registradores,
                                    ArqSvg *svgCombinado, FILE *txtSaida){
    char reg1Token[16], reg2Token[16], cc[32], cr[32];
    int campos = sscanf(linha, "p? %15s %15s %31s %31s", reg1Token, reg2Token, cc, cr);
    if(campos != 4){
        fprintf(stderr, "[leitorQry] linha %d: comando 'p?' malformado"
                        " (esperado 4 campos, lidos %d) -> ignorado\n", numLinha, campos);
        return;
    }

    fprintf(txtSaida, "[*] p? %s %s %s %s\n", reg1Token, reg2Token, cc, cr);

    if(grafo == NULL){
        fprintf(txtSaida, "Sistema viario indisponivel.\n\n");
        return;
    }

    int indiceReg1 = parseIndiceRegistrador(reg1Token);
    int indiceReg2 = parseIndiceRegistrador(reg2Token);
    if(indiceReg1 < 0 || indiceReg2 < 0){
        fprintf(txtSaida, "Registrador invalido.\n\n");
        return;
    }

    if(registradorPreenchido(registradores, indiceReg1) != REGISTRADOR_OK ||
       registradorPreenchido(registradores, indiceReg2) != REGISTRADOR_OK){
        fprintf(txtSaida, "Registrador nao preenchido.\n\n");
        return;
    }

    double x1 = getRegistradorX(registradores, indiceReg1);
    double y1 = getRegistradorY(registradores, indiceReg1);
    double x2 = getRegistradorX(registradores, indiceReg2);
    double y2 = getRegistradorY(registradores, indiceReg2);

    Vertice *vOrigem  = buscarVerticeMaisProximo(grafo, x1, y1);
    Vertice *vDestino = buscarVerticeMaisProximo(grafo, x2, y2);

    if(vOrigem == NULL || vDestino == NULL){
        fprintf(txtSaida, "Grafo sem vertices.\n\n");
        return;
    }

    const char *idOrigem  = getVerticeId(vOrigem);
    const char *idDestino = getVerticeId(vDestino);

    Caminho *curto  = NULL;
    Caminho *rapido = NULL;
    int resCurto  = calcularCaminhoMinimo(grafo, idOrigem, idDestino, CAMINHO_MAIS_CURTO,  &curto);
    int resRapido = calcularCaminhoMinimo(grafo, idOrigem, idDestino, CAMINHO_MAIS_RAPIDO, &rapido);

    if(resCurto != CAMINHO_OK || resRapido != CAMINHO_OK){
        fprintf(txtSaida, "Destino inalcancavel.\n\n");
        if(curto  != NULL) destruirCaminho(curto);
        if(rapido != NULL) destruirCaminho(rapido);
        return;
    }

    escreverPercursoNoTxt(txtSaida, "Percurso mais curto",  curto);
    escreverPercursoNoTxt(txtSaida, "Percurso mais rapido", rapido);
    fprintf(txtSaida, "\n");

    desenharPercurso(svgCombinado, grafo, curto,  cc);
    desenharPercurso(svgCombinado, grafo, rapido, cr);

    destruirCaminho(curto);
    destruirCaminho(rapido);
}

// processarArquivoQry: o despachante

int processarArquivoQry(const char *caminho, const Cidade *cidade, Grafo *grafo,
                         Registradores *registradores,
                         ArqSvg *svgCombinado, FILE *txtSaida){
    assert(caminho != NULL);
    assert(cidade != NULL);
    assert(registradores != NULL);
    assert(svgCombinado != NULL);
    assert(txtSaida != NULL);

    FILE *fp = fopen(caminho, "r");
    if(fp == NULL) return QRY_ERRO;

    desenharMapaBase(svgCombinado, cidade, grafo);

    char linha[LINHA_MAX];
    int numLinha = 0;

    while(fgets(linha, sizeof(linha), fp) != NULL){
        numLinha++;

        linha[strcspn(linha, "\n")] = '\0';

        if(linha[0] == '\0') continue;

        char cmd[16];
        if(sscanf(linha, "%15s", cmd) != 1) continue;

        if(strcmp(cmd, "@o?") == 0){
            processarArrobaO(linha, numLinha, cidade, grafo, registradores, svgCombinado, txtSaida);
        } else if(strcmp(cmd, "mvm") == 0){
            processarMvm(linha, numLinha, grafo, txtSaida);
        } else if(strcmp(cmd, "regs") == 0){
            processarRegs(linha, numLinha, grafo, svgCombinado, txtSaida);
        } else if(strcmp(cmd, "exp") == 0){
            processarExp(linha, numLinha, grafo, svgCombinado, txtSaida);
        } else if(strcmp(cmd, "p?") == 0){
            processarPInterrogacao(linha, numLinha, grafo, registradores, svgCombinado, txtSaida);
        } else {
            fprintf(stderr, "[leitorQry] linha %d: comando desconhecido '%s' -> ignorado\n",
                    numLinha, cmd);
        }
    }

    fclose(fp);
    return QRY_OK;
}