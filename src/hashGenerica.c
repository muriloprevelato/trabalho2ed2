#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "hashGenerica.h"

#define HASH_FATOR_CARGA_MAX 0.75 // acima disso, dobra a capacidade
#define HASH_FATOR_CRESCIMENTO 2

// Nó de bucket: guarda uma cópia própria da chave (a tabela é dona da
// string) e o ponteiro de valor recebido (a tabela também é dona dele,
// liberado via destrutor no momento certo).
typedef struct NoHash {
    char *chave;
    void *valor;
    struct NoHash *prox;
} NoHash;

struct HashGenerica {
    NoHash **buckets;
    int capacidade;
    int tamanho;          // número de entradas atualmente armazenadas
    HashDestrutor destrutor;
};



// Algoritmo djb2
// Retorna o índice dado a chave de acordo com a capacidade
static unsigned long hashString(const char *chave){
    unsigned long hash = 5381;
    int c;
    while((c = (unsigned char) *chave++)){
        hash = ((hash << 5) + hash) + (unsigned long) c; // hash * 33 + c
    }
    return hash;
}

static int indiceParaCapacidade(const char *chave, int capacidade){
    return (int) (hashString(chave) % (unsigned long) capacidade);
}

HashGenerica* criarHash(int capacidadeInicial, HashDestrutor destrutor){
    if(capacidadeInicial < 1) return NULL;
    if(destrutor == NULL)     return NULL;

    HashGenerica *h = malloc(sizeof(HashGenerica));
    if(h == NULL) return NULL;

    h->buckets = calloc((size_t) capacidadeInicial, sizeof(NoHash*));
    if(h->buckets == NULL){
        free(h);
        return NULL;
    }

    h->capacidade = capacidadeInicial;
    h->tamanho    = 0;
    h->destrutor  = destrutor;

    return h;
}

void destruirHash(HashGenerica *h){
    assert(h != NULL);

    for(int i = 0; i < h->capacidade; i++){
        NoHash *atual = h->buckets[i];
        while(atual != NULL){
            NoHash *proximo = atual->prox;
            h->destrutor(atual->valor);
            free(atual->chave);
            free(atual);
            atual = proximo;
        }
    }

    free(h->buckets);
    free(h);
}

// ─── Rehash ─────────────────────────────────────────────────────────────────────
//
// Obs: Ao crescer a tabela, cada nó precisa
// ser REALOCADO para o bucket correto na capacidade NOVA — nunca basta
// copiar o array de buckets antigo, porque o índice de cada chave muda
// com a capacidade. 
static void redistribuirNo(NoHash **bucketsNovos, int capacidadeNova, NoHash *no){
    int indiceNovo = indiceParaCapacidade(no->chave, capacidadeNova);
    no->prox = bucketsNovos[indiceNovo];
    bucketsNovos[indiceNovo] = no;
}

static void crescerSeNecessario(HashGenerica *h){
    double fatorCarga = (double) h->tamanho / (double) h->capacidade;
    if(fatorCarga <= HASH_FATOR_CARGA_MAX) return;

    int capacidadeNova = h->capacidade * HASH_FATOR_CRESCIMENTO;
    NoHash **bucketsNovos = calloc((size_t) capacidadeNova, sizeof(NoHash*));
    if(bucketsNovos == NULL){
        // Falha ao crescer não é erro fatal: a tabela continua funcional
        // na capacidade atual, só não cresce desta vez!
        return;
    }

    // Move cada nó existente para o array novo, recalculando seu índice.
    for(int i = 0; i < h->capacidade; i++){
        NoHash *atual = h->buckets[i];
        while(atual != NULL){
            NoHash *proximo = atual->prox; // guarda antes de remontar o prox
            redistribuirNo(bucketsNovos, capacidadeNova, atual);
            atual = proximo;
        }
    }

    free(h->buckets);
    h->buckets    = bucketsNovos;
    h->capacidade = capacidadeNova;
}

void inserirHash(HashGenerica *h, const char *chave, void *valor){
    assert(h != NULL);
    assert(chave != NULL);
    assert(valor != NULL);
    assert(contemChaveHash(h, chave) == HASH_ERRO);

    NoHash *no = malloc(sizeof(NoHash));
    assert(no != NULL); // falha de alocação aqui é tratada como erro fatal

    no->chave = malloc(strlen(chave) + 1);
    assert(no->chave != NULL);
    strcpy(no->chave, chave);

    no->valor = valor;

    int indice = indiceParaCapacidade(chave, h->capacidade);
    no->prox = h->buckets[indice];
    h->buckets[indice] = no;

    h->tamanho++;

    crescerSeNecessario(h);
}

void* buscarHash(const HashGenerica *h, const char *chave){
    assert(h != NULL);
    assert(chave != NULL);

    int indice = indiceParaCapacidade(chave, h->capacidade);
    NoHash *atual = h->buckets[indice];

    while(atual != NULL){
        if(strcmp(atual->chave, chave) == 0){
            return atual->valor;
        }
        atual = atual->prox;
    }

    return NULL;
}

int contemChaveHash(const HashGenerica *h, const char *chave){
    assert(h != NULL);
    assert(chave != NULL);

    return (buscarHash(h, chave) != NULL) ? HASH_OK : HASH_ERRO;
}

int tamanhoHash(const HashGenerica *h){
    assert(h != NULL);
    return h->tamanho;
}

void percorrerHash(const HashGenerica *h, HashVisitante visitante, void *contexto){
    assert(h != NULL);
    assert(visitante != NULL);

    for(int i = 0; i < h->capacidade; i++){
        NoHash *atual = h->buckets[i];
        while(atual != NULL){
            visitante(atual->chave, atual->valor, contexto);
            atual = atual->prox;
        }
    }
}