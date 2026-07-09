#include <string.h>
#include <assert.h>
#include "resolveEndereco.h"
#include "cidade.h"
#include "quadra.h"
#include "grafo.h"
#include "vertice.h"

typedef struct {
    double xAlvo, yAlvo;
    Vertice *maisProximo;        // ponteiro emprestado do Grafo
    double menorDistanciaQuadrado;
} ContextoVerticeMaisProximo;

// Compara por distância ao quadrado 
static void visitanteEncontrarMaisProximo(Vertice *v, void *contexto){
    ContextoVerticeMaisProximo *ctx = (ContextoVerticeMaisProximo*) contexto;

    double dx = getVerticeX(v) - ctx->xAlvo;
    double dy = getVerticeY(v) - ctx->yAlvo;
    double distQuadrado = dx * dx + dy * dy;

    if(ctx->maisProximo == NULL || distQuadrado < ctx->menorDistanciaQuadrado){
        ctx->maisProximo = v;
        ctx->menorDistanciaQuadrado = distQuadrado;
    }
}


int resolverEndereco(const Cidade *cidade, const Grafo *grafo, const char *cep, char face, double num, double *xSaida, double *ySaida, char *idVerticeSaida){
    assert(cidade != NULL);
    assert(grafo != NULL);
    assert(cep != NULL);
    assert(xSaida != NULL);
    assert(ySaida != NULL);
    assert(idVerticeSaida != NULL);

    // 1. Busca a quadra pelo cep. 
    Quadra *q = buscarQuadraCidade(cidade, cep);
    if(q == NULL) return RESOLVE_ERRO;

    // 2. Valida a face.
    if(faceValida(face) != QUADRA_OK) return RESOLVE_ERRO;

    FaceQuadra faceEnum = charParaFaceQuadra(face);

    // 3. Valida 'num' contra o comprimento da face ANTES de chamar
    // obterCoordenadasEndereco, que trata num fora do intervalo como
    // assert 
    double limite;
    if(faceEnum == FACE_S || faceEnum == FACE_N){
        limite = getQuadraW(q);
    } else {
        limite = getQuadraH(q);
    }
    if(num < 0.0 || num > limite) return RESOLVE_ERRO;

    // 4. Endereço -> coordenada. Agora seguro: num já validado.
    double x, y;
    obterCoordenadasEndereco(q, faceEnum, num, &x, &y);

    // 5. Coordenada -> vértice mais próximo.
    ContextoVerticeMaisProximo ctx = { x, y, NULL, 0.0 };
    percorrerVertices(grafo, visitanteEncontrarMaisProximo, &ctx);

    if(ctx.maisProximo == NULL) return RESOLVE_ERRO; // grafo sem vértices

    // 6. Preenche as saídas.
    *xSaida = x;
    *ySaida = y;
    strncpy(idVerticeSaida, getVerticeId(ctx.maisProximo), VERTICE_ID_MAX - 1);
    idVerticeSaida[VERTICE_ID_MAX - 1] = '\0';

    return RESOLVE_OK;
}