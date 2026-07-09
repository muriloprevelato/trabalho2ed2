#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "ampliacaoViaria.h"
#include "grafo.h"
#include "vertice.h"
#include "hashGenerica.h"

#define AMPLIACAO_CAPACIDADE_INICIAL_AUX 16 // capacidade inicial da hash 'pai'
#define AMPLIACAO_FATOR_UPGRADE 1.5         // aumento de 50% no vm selecionado


typedef struct {
    char idOrigem[VERTICE_ID_MAX];
    Aresta *aresta; // ponteiro emprestado do Grafo
} ArestaAmpliada;

struct Ampliacao {
    ArestaAmpliada *arestas; // array dinâmico, cresce via realloc (dobra)
    int numArestas;
    int capacidade;
};

// Union-Find (duplicado) -> Mesmo desenho de componentesConexos.c

static void find(HashGenerica *pai, const char *id, char *raizSaida){
    char atual[VERTICE_ID_MAX];
    strncpy(atual, id, VERTICE_ID_MAX - 1);
    atual[VERTICE_ID_MAX - 1] = '\0';

    while(1){
        char *paiAtual = (char*) buscarHash(pai, atual);
        assert(paiAtual != NULL);
        if(strcmp(paiAtual, atual) == 0) break;
        strncpy(atual, paiAtual, VERTICE_ID_MAX - 1);
        atual[VERTICE_ID_MAX - 1] = '\0';
    }
    strncpy(raizSaida, atual, VERTICE_ID_MAX - 1);
    raizSaida[VERTICE_ID_MAX - 1] = '\0';

    strncpy(atual, id, VERTICE_ID_MAX - 1);
    atual[VERTICE_ID_MAX - 1] = '\0';

    while(strcmp(atual, raizSaida) != 0){
        char *paiAtualBuf = (char*) buscarHash(pai, atual);

        char proximo[VERTICE_ID_MAX];
        strncpy(proximo, paiAtualBuf, VERTICE_ID_MAX - 1);
        proximo[VERTICE_ID_MAX - 1] = '\0';

        strncpy(paiAtualBuf, raizSaida, VERTICE_ID_MAX - 1);
        paiAtualBuf[VERTICE_ID_MAX - 1] = '\0';

        strncpy(atual, proximo, VERTICE_ID_MAX - 1);
        atual[VERTICE_ID_MAX - 1] = '\0';
    }
}

static void unir(HashGenerica *pai, const char *idA, const char *idB){
    char raizA[VERTICE_ID_MAX];
    char raizB[VERTICE_ID_MAX];
    find(pai, idA, raizA);
    find(pai, idB, raizB);

    if(strcmp(raizA, raizB) == 0) return; // já no mesmo conjunto

    char *paiRaizA = (char*) buscarHash(pai, raizA);
    strncpy(paiRaizA, raizB, VERTICE_ID_MAX - 1);
    paiRaizA[VERTICE_ID_MAX - 1] = '\0';
}

static void visitanteInicializarPai(Vertice *v, void *contexto){
    HashGenerica *pai = (HashGenerica*) contexto;
    const char *id = getVerticeId(v);

    char *paiId = malloc(VERTICE_ID_MAX);
    assert(paiId != NULL);
    strncpy(paiId, id, VERTICE_ID_MAX - 1);
    paiId[VERTICE_ID_MAX - 1] = '\0';

    inserirHash(pai, id, paiId);
}

typedef struct {
    char idOrigem[VERTICE_ID_MAX];
    Aresta *aresta;
    double peso; // cmp, capturado no momento da coleta
} CandidatoAresta;

typedef struct {
    CandidatoAresta *candidatos; // array dinâmico
    int numCandidatos;
    int capacidade;
} ContextoColetarCandidatos;

static void visitanteColetarCandidato(const char *idOrigem, Aresta *a, void *contexto){
    ContextoColetarCandidatos *ctx = (ContextoColetarCandidatos*) contexto;

    if(ctx->numCandidatos == ctx->capacidade){
        int novaCapacidade = (ctx->capacidade == 0) ? 16 : ctx->capacidade * 2;
        CandidatoAresta *novos = realloc(ctx->candidatos, (size_t) novaCapacidade * sizeof(CandidatoAresta));
        assert(novos != NULL);
        ctx->candidatos = novos;
        ctx->capacidade = novaCapacidade;
    }

    CandidatoAresta *cand = &ctx->candidatos[ctx->numCandidatos];
    strncpy(cand->idOrigem, idOrigem, VERTICE_ID_MAX - 1);
    cand->idOrigem[VERTICE_ID_MAX - 1] = '\0';
    cand->aresta = a;
    cand->peso = getArestaCmp(a);

    ctx->numCandidatos++;
}

static int compararCandidatos(const void *pa, const void *pb){
    const CandidatoAresta *a = (const CandidatoAresta*) pa;
    const CandidatoAresta *b = (const CandidatoAresta*) pb;
    if(a->peso < b->peso) return -1;
    if(a->peso > b->peso) return 1;
    return 0;
}

Ampliacao* calcularAmpliacaoViaria(Grafo *g, double vl){
    assert(g != NULL);

    // 1. Coleta todas as arestas do grafo como candidatas.
    ContextoColetarCandidatos ctxColeta = { NULL, 0, 0 };
    percorrerTodasArestas(g, visitanteColetarCandidato, &ctxColeta);

    // 2. Ordena por peso (cmp) ascendente.
    if(ctxColeta.numCandidatos > 0){
        qsort(ctxColeta.candidatos, (size_t) ctxColeta.numCandidatos,
              sizeof(CandidatoAresta), compararCandidatos);
    }

    // 3. Union-Find - cada candidato é tratado como conexão não direcionada entre origem e destino
    HashGenerica *pai = criarHash(AMPLIACAO_CAPACIDADE_INICIAL_AUX, free);
    if(pai == NULL){
        free(ctxColeta.candidatos);
        return NULL;
    }
    percorrerVertices(g, visitanteInicializarPai, pai);

    // 4. Processa candidatos em ordem crescente de peso
    int *entrouNaFloresta = NULL;
    if(ctxColeta.numCandidatos > 0){
        entrouNaFloresta = malloc((size_t) ctxColeta.numCandidatos * sizeof(int));
        assert(entrouNaFloresta != NULL);
    }

    for(int i = 0; i < ctxColeta.numCandidatos; i++){
        CandidatoAresta *cand = &ctxColeta.candidatos[i];
        const char *idDestino = getVerticeId(getArestaDestino(cand->aresta));

        char raizOrigem[VERTICE_ID_MAX];
        char raizDestino[VERTICE_ID_MAX];
        find(pai, cand->idOrigem, raizOrigem);
        find(pai, idDestino, raizDestino);

        if(strcmp(raizOrigem, raizDestino) != 0){
            unir(pai, cand->idOrigem, idDestino);
            entrouNaFloresta[i] = 1;
        } else {
            entrouNaFloresta[i] = 0; // formaria ciclo - rejeitada
        }
    }

    destruirHash(pai);

    // 5. Entre as arestas, seleciona as com vm < vl, aumenta o vm em 50%, e monta o resultado.
    Ampliacao *resultado = malloc(sizeof(Ampliacao));
    if(resultado == NULL){
        free(entrouNaFloresta);
        free(ctxColeta.candidatos);
        return NULL;
    }
    resultado->arestas = NULL;
    resultado->numArestas = 0;
    resultado->capacidade = 0;

    for(int i = 0; i < ctxColeta.numCandidatos; i++){
        if(!entrouNaFloresta[i]) continue;

        Aresta *aresta = ctxColeta.candidatos[i].aresta;
        double vmAtual = getArestaVm(aresta);

        if(vmAtual < vl){
            double vmNovo = vmAtual * AMPLIACAO_FATOR_UPGRADE;
            setArestaVm(aresta, vmNovo); // vmNovo >= 0 sempre, nunca falha aqui

            if(resultado->numArestas == resultado->capacidade){
                int novaCap = (resultado->capacidade == 0) ? 8 : resultado->capacidade * 2;
                ArestaAmpliada *novos = realloc(resultado->arestas,
                                                  (size_t) novaCap * sizeof(ArestaAmpliada));
                assert(novos != NULL);
                resultado->arestas = novos;
                resultado->capacidade = novaCap;
            }

            ArestaAmpliada *aa = &resultado->arestas[resultado->numArestas];
            strncpy(aa->idOrigem, ctxColeta.candidatos[i].idOrigem, VERTICE_ID_MAX - 1);
            aa->idOrigem[VERTICE_ID_MAX - 1] = '\0';
            aa->aresta = aresta;
            resultado->numArestas++;
        }
    }

    free(entrouNaFloresta);
    free(ctxColeta.candidatos);

    return resultado;
}

void destruirAmpliacao(Ampliacao *a){
    assert(a != NULL);
    free(a->arestas); // não toca nas Aresta* referenciadas - emprestadas do Grafo
    free(a);
}

int ampliacaoNumArestas(const Ampliacao *a){
    assert(a != NULL);
    return a->numArestas;
}

void percorrerArestasAmpliadas(const Ampliacao *a, VisitanteArestaCompleta visitante, void *contexto){
    assert(a != NULL);
    assert(visitante != NULL);

    for(int i = 0; i < a->numArestas; i++){
        visitante(a->arestas[i].idOrigem, a->arestas[i].aresta, contexto);
    }
}