#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "grafo.h"
#include "vertice.h"
#include "hashGenerica.h"


#define GRAFO_CAPACIDADE_INICIAL 16 // rehash da HashGenerica cuida do crescimento.

struct Aresta {
    Vertice *destino; // ponteiro emprestado 
    char nome[ARESTA_NOME_MAX];
    char ldir[ARESTA_LADO_MAX];
    char lesq[ARESTA_LADO_MAX];
    double cmp;
    double vm;
    Aresta *prox; // próximo nó na lista de adjacência do mesmo idOrigem
};

// Lista de adjacência de um vértice (cabeça da lista ligada de Aresta) 
typedef struct {
    Aresta *cabeca;
} ListaAdjacencia;

struct Grafo {
    HashGenerica *vertices; // id -> Vertice* (dona real dos vértices)
    HashGenerica *adjacencias; // id -> ListaAdjacencia*(dona das Aresta, não dos Vertice)
};

// Adaptadores de destrutor 
static void destruirVerticeAdaptador(void *valor){
    destruirVertice((Vertice*) valor);
}


// Livrar os vértices é responsabilidade exclusiva da hash "vertices".
static void destruirListaAdjacenciaAdaptador(void *valor){
    ListaAdjacencia *lista = (ListaAdjacencia*) valor;

    Aresta *atual = lista->cabeca;
    while(atual != NULL){
        Aresta *proximo = atual->prox;
        free(atual); // nome/ldir/lesq são arrays internos, liberados junto
        atual = proximo;
    }

    free(lista);
}

Grafo* criarGrafo(void){
    Grafo *g = malloc(sizeof(Grafo));
    if(g == NULL) return NULL;

    g->vertices = criarHash(GRAFO_CAPACIDADE_INICIAL, destruirVerticeAdaptador);
    if(g->vertices == NULL){
        free(g);
        return NULL;
    }

    g->adjacencias = criarHash(GRAFO_CAPACIDADE_INICIAL, destruirListaAdjacenciaAdaptador);
    if(g->adjacencias == NULL){
        destruirHash(g->vertices);
        free(g);
        return NULL;
    }

    return g;
}

void destruirGrafo(Grafo *g){
    assert(g != NULL);

    // Primeiro as arestas depois os vértices.
    destruirHash(g->adjacencias);
    destruirHash(g->vertices);

    free(g);
}

void grafoInserirVertice(Grafo *g, Vertice *v){
    assert(g != NULL);
    assert(v != NULL);

    const char *id = getVerticeId(v);
    inserirHash(g->vertices, id, v);
}

Vertice* buscarVertice(const Grafo *g, const char *id){
    assert(g != NULL);
    assert(id != NULL);

    return (Vertice*) buscarHash(g->vertices, id);
}

int grafoContemVertice(const Grafo *g, const char *id){
    assert(g != NULL);
    assert(id != NULL);

    return contemChaveHash(g->vertices, id);
}

int grafoNumVertices(const Grafo *g){
    assert(g != NULL);
    return tamanhoHash(g->vertices);
}

int grafoInserirAresta(Grafo *g, const char *idOrigem, const char *idDestino,
                        const char *ldir, const char *lesq,
                        double cmp, double vm, const char *nome){
    assert(g != NULL);
    assert(idOrigem != NULL);
    assert(idDestino != NULL);
    assert(ldir != NULL);
    assert(lesq != NULL);
    assert(nome != NULL);

    /* Existência de ambos os vértices: erro de uso (fronteira do
    leitorVia, que deve checar grafoContemVertice antes de chamar).
    */
    Vertice *destino = buscarVertice(g, idDestino);
    assert(destino != NULL);
    assert(grafoContemVertice(g, idOrigem) == GRAFO_OK);

    if(cmp < 0.0) return GRAFO_ERRO;
    if(vm  < 0.0) return GRAFO_ERRO;

    Aresta *a = malloc(sizeof(Aresta));
    assert(a != NULL); // falha de alocação tratada como fatal aqui

    a->destino = destino; 

    // Cópia defensiva das 3 strings 
    strncpy(a->nome, nome, ARESTA_NOME_MAX - 1);
    a->nome[ARESTA_NOME_MAX - 1] = '\0';
    strncpy(a->ldir, ldir, ARESTA_LADO_MAX - 1);
    a->ldir[ARESTA_LADO_MAX - 1] = '\0';
    strncpy(a->lesq, lesq, ARESTA_LADO_MAX - 1);
    a->lesq[ARESTA_LADO_MAX - 1] = '\0';

    a->cmp = cmp;
    a->vm  = vm;

    // Busca (ou cria, se for a primeira aresta de idOrigem) a lista de
    // adjacência correspondente, e prepende o novo nó.
    ListaAdjacencia *lista = (ListaAdjacencia*) buscarHash(g->adjacencias, idOrigem);
    if(lista == NULL){
        lista = malloc(sizeof(ListaAdjacencia));
        assert(lista != NULL);
        lista->cabeca = NULL;
        inserirHash(g->adjacencias, idOrigem, lista);
    }

    a->prox = lista->cabeca;
    lista->cabeca = a;

    return GRAFO_OK;
}

Vertice* getArestaDestino(const Aresta *a){
    assert(a != NULL);
    return a->destino;
}

const char* getArestaNome(const Aresta *a){
    assert(a != NULL);
    return a->nome;
}

const char* getArestaLdir(const Aresta *a){
    assert(a != NULL);
    return a->ldir;
}

const char* getArestaLesq(const Aresta *a){
    assert(a != NULL);
    return a->lesq;
}

double getArestaCmp(const Aresta *a){
    assert(a != NULL);
    return a->cmp;
}

double getArestaVm(const Aresta *a){
    assert(a != NULL);
    return a->vm;
}

int setArestaVm(Aresta *a, double novoVm){
    assert(a != NULL);
 
    if(novoVm < 0.0) return GRAFO_ERRO; // mesma validação de grafoInserirAresta
 
    a->vm = novoVm;
    return GRAFO_OK;
}

// Percorrer os vértices
typedef struct {
    VisitanteVertice visitanteReal;
    void *contextoReal;
} EmbrulhoVisitanteVertice;

static void adaptadorVisitanteVertice(const char *chave, void *valor, void *contexto){
    (void) chave; // VisitanteVertice não recebe o id separado - já está no Vertice
    EmbrulhoVisitanteVertice *embrulho = (EmbrulhoVisitanteVertice*) contexto;
    embrulho->visitanteReal((Vertice*) valor, embrulho->contextoReal);
}

void percorrerVertices(const Grafo *g, VisitanteVertice visitante, void *contexto){
    assert(g != NULL);
    assert(visitante != NULL);

    EmbrulhoVisitanteVertice embrulho;
    embrulho.visitanteReal = visitante;
    embrulho.contextoReal  = contexto;

    percorrerHash(g->vertices, adaptadorVisitanteVertice, &embrulho);
}

// Percorrer as arestas saindo
void percorrerArestasSaindo(const Grafo *g, const char *idOrigem,
                             VisitanteAresta visitante, void *contexto){
    assert(g != NULL);
    assert(idOrigem != NULL);
    assert(visitante != NULL);

    ListaAdjacencia *lista = (ListaAdjacencia*) buscarHash(g->adjacencias, idOrigem);
    if(lista == NULL) return; // vértice sem arestas (ou inexistente) - mesmo resultado observável

    Aresta *atual = lista->cabeca;
    while(atual != NULL){
        visitante(atual, contexto);
        atual = atual->prox;
    }
}

/*
Composição de percorrerVertices() + percorrerArestasSaindo() para
cada vértice visitado - dois níveis de embrulho: o externo carrega o
visitante real (que espera idOrigem + Aresta*) através do nível de
percorrerVertices (que só entrega Vertice*); o interno carrega o id
do vértice atual através do nível de percorrerArestasSaindo (que só
entrega Aresta*, sem origem - daí a necessidade de capturá-la aqui).
*/
 
typedef struct {
    const char *idOrigem;
    VisitanteArestaCompleta visitanteReal;
    void *contextoReal;
} ContextoArestaIndividual;
 
static void adaptadorArestaIndividual(Aresta *a, void *contexto){
    ContextoArestaIndividual *ctx = (ContextoArestaIndividual*) contexto;
    ctx->visitanteReal(ctx->idOrigem, a, ctx->contextoReal);
}
 
typedef struct {
    const Grafo *g;
    VisitanteArestaCompleta visitanteReal;
    void *contextoReal;
} ContextoPercorrerTodasArestas;
 
static void visitanteVerticeParaTodasArestas(Vertice *v, void *contexto){
    ContextoPercorrerTodasArestas *ctx = (ContextoPercorrerTodasArestas*) contexto;
    const char *idOrigem = getVerticeId(v);
 
    ContextoArestaIndividual ctxAresta = { idOrigem, ctx->visitanteReal, ctx->contextoReal };
    percorrerArestasSaindo(ctx->g, idOrigem, adaptadorArestaIndividual, &ctxAresta);
}
 
void percorrerTodasArestas(const Grafo *g, VisitanteArestaCompleta visitante, void *contexto){
    assert(g != NULL);
    assert(visitante != NULL);
 
    ContextoPercorrerTodasArestas ctx = { g, visitante, contexto };
    percorrerVertices(g, visitanteVerticeParaTodasArestas, &ctx);
}