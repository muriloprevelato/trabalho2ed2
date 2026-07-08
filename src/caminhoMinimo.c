#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "caminhoMinimo.h"
#include "grafo.h"
#include "vertice.h"
#include "hashGenerica.h"
#include "filaPrioridade.h"

#define CAMINHO_CAPACIDADE_INICIAL_AUX 16 // capacidade inicial de dist/anterior
#define PESO_INTRANSITAVEL -1.0

struct Caminho {
    char (*ids)[VERTICE_ID_MAX]; // array alocado, ids na ordem origem->destino
    int tamanho;
    double custoTotal;
};

static double calcularPeso(const Aresta *a, ModoCaminho modo){
    if(modo == CAMINHO_MAIS_CURTO){
        return getArestaCmp(a);
    }
    double vm = getArestaVm(a);
    if(vm <= 0.0){
        return PESO_INTRANSITAVEL;
    }
    return getArestaCmp(a) / vm;
}

typedef struct {
    HashGenerica *dist; // id -> double* (menor distância conhecida)
    HashGenerica *anterior;  // id -> char* (id do predecessor no caminho)
    FilaPrioridade *fila;
    const char *idOrigemAtual; // vértice cujas arestas de saída estão sendo relaxadas
    ModoCaminho modo;
} ContextoRelaxamento;

static void visitanteRelaxar(Aresta *a, void *contexto){
    ContextoRelaxamento *ctx = (ContextoRelaxamento*) contexto;

    double peso = calcularPeso(a, ctx->modo);
    if(peso < 0.0) return; // aresta intransitável neste modo - não relaxa por ela

    double *distOrigemPtr = (double*) buscarHash(ctx->dist, ctx->idOrigemAtual);
    double novaDist = *distOrigemPtr + peso;

    Vertice *destino = getArestaDestino(a);
    const char *idDestino = getVerticeId(destino);

    double *distDestinoPtr = (double*) buscarHash(ctx->dist, idDestino);

    if(distDestinoPtr == NULL){
        // Primeira vez alcançando este vértice - insere pela primeira e
        // única vez (HashGenerica não permite re-inserir a mesma chave).
        double *novoDist = malloc(sizeof(double));
        assert(novoDist != NULL);
        *novoDist = novaDist;
        inserirHash(ctx->dist, idDestino, novoDist);

        char *novoPred = malloc(VERTICE_ID_MAX);
        assert(novoPred != NULL);
        strncpy(novoPred, ctx->idOrigemAtual, VERTICE_ID_MAX - 1);
        novoPred[VERTICE_ID_MAX - 1] = '\0';
        inserirHash(ctx->anterior, idDestino, novoPred);

        filaPrioridadeInserir(ctx->fila, idDestino, novaDist);

    } else if(novaDist < *distDestinoPtr){
        *distDestinoPtr = novaDist;

        char *predExistente = (char*) buscarHash(ctx->anterior, idDestino);
        strncpy(predExistente, ctx->idOrigemAtual, VERTICE_ID_MAX - 1);
        predExistente[VERTICE_ID_MAX - 1] = '\0';

        // Nova entrada na fila com a prioridade menor. A entrada antiga
        // (se ainda não foi extraída) fica obsoleta - é o próprio laço
        // principal que a detecta e descarta (lazy deletion).
        filaPrioridadeInserir(ctx->fila, idDestino, novaDist);
    }
}

int calcularCaminhoMinimo(const Grafo *g, const char *idOrigem, const char *idDestino,
                           ModoCaminho modo, Caminho **caminhoSaida){
    assert(g != NULL);
    assert(idOrigem != NULL);
    assert(idDestino != NULL);
    assert(caminhoSaida != NULL);
    assert(grafoContemVertice(g, idOrigem) == GRAFO_OK);
    assert(grafoContemVertice(g, idDestino) == GRAFO_OK);

    HashGenerica *dist = criarHash(CAMINHO_CAPACIDADE_INICIAL_AUX, free);
    HashGenerica *anterior = criarHash(CAMINHO_CAPACIDADE_INICIAL_AUX, free);
    FilaPrioridade *fila = criarFilaPrioridade();
    assert(dist != NULL && anterior != NULL && fila != NULL);

    double *distOrigem = malloc(sizeof(double));
    assert(distOrigem != NULL);
    *distOrigem = 0.0;
    inserirHash(dist, idOrigem, distOrigem);
    filaPrioridadeInserir(fila, idOrigem, 0.0);

    char idAtual[VERTICE_ID_MAX];
    double prioridadeAtual;

    while(filaPrioridadeExtrairMin(fila, idAtual, &prioridadeAtual) == FILA_OK){
        double *distConhecida = (double*) buscarHash(dist, idAtual);

        if(prioridadeAtual > *distConhecida){
            continue; // entrada obsoleta 
        }

        ContextoRelaxamento ctx = { dist, anterior, fila, idAtual, modo };
        percorrerArestasSaindo(g, idAtual, visitanteRelaxar, &ctx);
    }

    double *distDestinoFinal = (double*) buscarHash(dist, idDestino);

    if(distDestinoFinal == NULL){
        destruirHash(dist);
        destruirHash(anterior);
        destruirFilaPrioridade(fila);
        *caminhoSaida = NULL;
        return CAMINHO_INALCANCAVEL;
    }

    int capacidadeMax = grafoNumVertices(g);
    char (*bufferTemp)[VERTICE_ID_MAX] = malloc((size_t) capacidadeMax * sizeof(*bufferTemp));
    assert(bufferTemp != NULL);

    int indice = capacidadeMax - 1;
    strncpy(bufferTemp[indice], idDestino, VERTICE_ID_MAX - 1);
    bufferTemp[indice][VERTICE_ID_MAX - 1] = '\0';

    char atual[VERTICE_ID_MAX];
    strncpy(atual, idDestino, VERTICE_ID_MAX - 1);
    atual[VERTICE_ID_MAX - 1] = '\0';

    while(strcmp(atual, idOrigem) != 0){
        char *pred = (char*) buscarHash(anterior, atual);
        assert(pred != NULL); //  se dist[destino] existe, a
                               // cadeia de predecessores até a origem
                               // está sempre completa

        indice--;
        assert(indice >= 0); // nunca deveria estourar capacidadeMax 

        strncpy(bufferTemp[indice], pred, VERTICE_ID_MAX - 1);
        bufferTemp[indice][VERTICE_ID_MAX - 1] = '\0';

        strncpy(atual, pred, VERTICE_ID_MAX - 1);
        atual[VERTICE_ID_MAX - 1] = '\0';
    }

    int tamanhoCaminho = capacidadeMax - indice;

    Caminho *c = malloc(sizeof(Caminho));
    assert(c != NULL);
    c->ids = malloc((size_t) tamanhoCaminho * sizeof(*c->ids));
    assert(c->ids != NULL);
    memcpy(c->ids, &bufferTemp[indice], (size_t) tamanhoCaminho * sizeof(*c->ids));
    c->tamanho    = tamanhoCaminho;
    c->custoTotal = *distDestinoFinal;

    free(bufferTemp);
    destruirHash(dist);
    destruirHash(anterior);
    destruirFilaPrioridade(fila);

    *caminhoSaida = c;
    return CAMINHO_OK;
}

void destruirCaminho(Caminho *c){
    assert(c != NULL);
    free(c->ids);
    free(c);
}

int caminhoNumVertices(const Caminho *c){
    assert(c != NULL);
    return c->tamanho;
}

const char* caminhoObterVertice(const Caminho *c, int indice){
    assert(c != NULL);
    assert(indice >= 0 && indice < c->tamanho);
    return c->ids[indice];
}

double caminhoCustoTotal(const Caminho *c){
    assert(c != NULL);
    return c->custoTotal;
}