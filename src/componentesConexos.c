#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "componentesConexos.h"
#include "grafo.h"
#include "vertice.h"
#include "hashGenerica.h"

#define COMPONENTES_CAPACIDADE_INICIAL_AUX 16 // capacidade inicial da hash 'pai'

// Um grupo: os Vertice* (emprestados do Grafo) que compartilham a
// mesma raiz no union-find, mais a própria raiz (usada só durante o
// agrupamento, para achar o grupo certo).
typedef struct {
    char raizId[VERTICE_ID_MAX];
    Vertice **vertices; // array dinâmico, cresce via realloc (dobra)
    int tamanho;
    int capacidade;
} GrupoComponente;

struct Componentes {
    GrupoComponente *grupos; // array dinâmico de grupos, cresce via realloc
    int numComponentes;
    int capacidadeGrupos;
};

static void find(HashGenerica *pai, const char *id, char *raizSaida){
    char atual[VERTICE_ID_MAX];
    strncpy(atual, id, VERTICE_ID_MAX - 1);
    atual[VERTICE_ID_MAX - 1] = '\0';

    // Passo 1: anda até a raiz (pai[X] == X), sem comprimir ainda.
    while(1){
        char *paiAtual = (char*) buscarHash(pai, atual);
        assert(paiAtual != NULL); // todo vértice foi inicializado como seu próprio pai
        if(strcmp(paiAtual, atual) == 0) break;
        strncpy(atual, paiAtual, VERTICE_ID_MAX - 1);
        atual[VERTICE_ID_MAX - 1] = '\0';
    }
    strncpy(raizSaida, atual, VERTICE_ID_MAX - 1);
    raizSaida[VERTICE_ID_MAX - 1] = '\0';

    // Passo 2: anda de novo desde id até a raiz, comprimindo cada nó no
    // caminho para apontar direto para a raiz. Usa um buffer LOCAL
    // ('proximo') para guardar o próximo salto antes de sobrescrever o
    // buffer da hash - já que estou lendo e escrevendo o mesmo local.
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

// União simples
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

// União das arestas suficientes (vm >= vl)

typedef struct {
    HashGenerica *pai;
    double vl;
} ContextoUnirArestas;

static void visitanteUnirSeSuficiente(const char *idOrigem, Aresta *a, void *contexto){
    ContextoUnirArestas *ctx = (ContextoUnirArestas*) contexto;

    if(getArestaVm(a) < ctx->vl) return; 

    const char *idDestino = getVerticeId(getArestaDestino(a));
    unir(ctx->pai, idOrigem, idDestino);
}

// Agrupamento final: organiza vértices por raiz em Componentes 

typedef struct {
    HashGenerica *pai;
    Componentes *c;
} ContextoAgruparVertices;

static void visitanteAgruparVertice(Vertice *v, void *contexto){
    ContextoAgruparVertices *ctx = (ContextoAgruparVertices*) contexto;

    char raiz[VERTICE_ID_MAX];
    find(ctx->pai, getVerticeId(v), raiz);

    // Busca linear por um grupo existente com essa raiz - aceitável
    // dado que o número de componentes tende a ser pequeno na escala do projeto
    int indice = -1;
    for(int i = 0; i < ctx->c->numComponentes; i++){
        if(strcmp(ctx->c->grupos[i].raizId, raiz) == 0){
            indice = i;
            break;
        }
    }

    if(indice == -1){
        // Primeira vez vendo esta raiz - cria um grupo novo.
        if(ctx->c->numComponentes == ctx->c->capacidadeGrupos){
            int novaCapacidade = (ctx->c->capacidadeGrupos == 0) ? 4 : ctx->c->capacidadeGrupos * 2;
            GrupoComponente *gruposNovos = realloc(ctx->c->grupos, (size_t) novaCapacidade * sizeof(GrupoComponente));
            assert(gruposNovos != NULL);
            ctx->c->grupos = gruposNovos;
            ctx->c->capacidadeGrupos = novaCapacidade;
        }

        indice = ctx->c->numComponentes;
        strncpy(ctx->c->grupos[indice].raizId, raiz, VERTICE_ID_MAX - 1);
        ctx->c->grupos[indice].raizId[VERTICE_ID_MAX - 1] = '\0';
        ctx->c->grupos[indice].vertices = NULL;
        ctx->c->grupos[indice].tamanho = 0;
        ctx->c->grupos[indice].capacidade = 0;
        ctx->c->numComponentes++;
    }

    // Adiciona v ao grupo (cresce o array de vértices se necessário).
    GrupoComponente *grupo = &ctx->c->grupos[indice];
    if(grupo->tamanho == grupo->capacidade){
        int novaCapacidadeV = (grupo->capacidade == 0) ? 4 : grupo->capacidade * 2;
        Vertice **verticesNovos = realloc(grupo->vertices, (size_t) novaCapacidadeV * sizeof(Vertice*));
        assert(verticesNovos != NULL);
        grupo->vertices   = verticesNovos;
        grupo->capacidade = novaCapacidadeV;
    }
    grupo->vertices[grupo->tamanho] = v; // ponteiro EMPRESTADO - nunca liberado por Componentes
    grupo->tamanho++;
}

Componentes* calcularComponentesConexos(const Grafo *g, double vl){
    assert(g != NULL);

    HashGenerica *pai = criarHash(COMPONENTES_CAPACIDADE_INICIAL_AUX, free);
    if(pai == NULL) return NULL;

    percorrerVertices(g, visitanteInicializarPai, pai);

    ContextoUnirArestas ctxUnir = { pai, vl };
    percorrerTodasArestas(g, visitanteUnirSeSuficiente, &ctxUnir);

    Componentes *c = malloc(sizeof(Componentes));
    if(c == NULL){
        destruirHash(pai);
        return NULL;
    }
    c->grupos = NULL;
    c->numComponentes = 0;
    c->capacidadeGrupos = 0;

    ContextoAgruparVertices ctxAgrupar = { pai, c };
    percorrerVertices(g, visitanteAgruparVertice, &ctxAgrupar);

    destruirHash(pai); 

    return c;
}

void destruirComponentes(Componentes *c){
    assert(c != NULL);

    // Libera só a estrutura de agrupamento - os Vertice* dentro de cada
    // grupo são emprestados do Grafo e NUNCA são destruídos aqui
    for(int i = 0; i < c->numComponentes; i++){
        free(c->grupos[i].vertices);
    }
    free(c->grupos);
    free(c);
}

int componentesNumComponentes(const Componentes *c){
    assert(c != NULL);
    return c->numComponentes;
}

void percorrerVerticesDoComponente(const Componentes *c, int indiceComponente, VisitanteVertice visitante, void *contexto){
    assert(c != NULL);
    assert(indiceComponente >= 0 && indiceComponente < c->numComponentes);
    assert(visitante != NULL);

    GrupoComponente *grupo = &c->grupos[indiceComponente];
    for(int i = 0; i < grupo->tamanho; i++){
        visitante(grupo->vertices[i], contexto);
    }
}