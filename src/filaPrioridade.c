#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "filaPrioridade.h"

#define FILA_CAPACIDADE_INICIAL 16 // cresce (dobra) sob demanda

// Nó do heap: guarda uma cópia própria da chave (a fila é dona da
// string) e a prioridade usada para ordenação.
typedef struct {
    char   chave[FILA_CHAVE_MAX];
    double prioridade;
} NoFila;

struct FilaPrioridade {
    NoFila *itens;
    int tamanho;    // número de entradas atualmente na fila
    int capacidade; // tamanho alocado do array itens
};

// Helpers de heap 
static void trocar(NoFila *a, NoFila *b){
    NoFila temp = *a;
    *a = *b;
    *b = temp;
}

// Restaura a propriedade de heap subindo o elemento em i enquanto ele
// for menor que o pai. Usado logo após inserir no fim do array.
static void siftUp(FilaPrioridade *fp, int i){
    while(i > 0){
        int pai = (i - 1) / 2;
        if(fp->itens[i].prioridade < fp->itens[pai].prioridade){
            trocar(&fp->itens[i], &fp->itens[pai]);
            i = pai;
        } else {
            break;
        }
    }
}

// Restaura a propriedade de heap descendo o elemento em i enquanto
// algum filho for menor que ele. Usado após mover o último elemento
// para a raiz, na extração do mínimo.
static void siftDown(FilaPrioridade *fp, int i){
    while(1){
        int esq = 2 * i + 1;
        int dir = 2 * i + 2;
        int menor = i;

        if(esq < fp->tamanho && fp->itens[esq].prioridade < fp->itens[menor].prioridade)
            menor = esq;
        if(dir < fp->tamanho && fp->itens[dir].prioridade < fp->itens[menor].prioridade)
            menor = dir;

        if(menor == i) break;

        trocar(&fp->itens[i], &fp->itens[menor]);
        i = menor;
    }
}

static void crescerSeNecessario(FilaPrioridade *fp){
    if(fp->tamanho < fp->capacidade) return;

    int capacidadeNova = fp->capacidade * 2;
    NoFila *itensNovos = realloc(fp->itens, (size_t) capacidadeNova * sizeof(NoFila));
    assert(itensNovos != NULL); // falha de alocação tratada como fatal aqui

    fp->itens = itensNovos;
    fp->capacidade = capacidadeNova;
}

FilaPrioridade* criarFilaPrioridade(void){
    FilaPrioridade *fp = malloc(sizeof(FilaPrioridade));
    if(fp == NULL) return NULL;

    fp->itens = malloc(FILA_CAPACIDADE_INICIAL * sizeof(NoFila));
    if(fp->itens == NULL){
        free(fp);
        return NULL;
    }

    fp->tamanho    = 0;
    fp->capacidade = FILA_CAPACIDADE_INICIAL;

    return fp;
}

void destruirFilaPrioridade(FilaPrioridade *fp){
    assert(fp != NULL);
    free(fp->itens);
    free(fp);
}

void filaPrioridadeInserir(FilaPrioridade *fp, const char *chave, double prioridade){
    assert(fp != NULL);
    assert(chave != NULL);

    crescerSeNecessario(fp);

    int i = fp->tamanho;
    strncpy(fp->itens[i].chave, chave, FILA_CHAVE_MAX - 1);
    fp->itens[i].chave[FILA_CHAVE_MAX - 1] = '\0';
    fp->itens[i].prioridade = prioridade;

    fp->tamanho++;

    siftUp(fp, i);
}

int filaPrioridadeExtrairMin(FilaPrioridade *fp, char *chaveSaida, double *prioridadeSaida){
    assert(fp != NULL);
    assert(chaveSaida != NULL);
    assert(prioridadeSaida != NULL);

    if(fp->tamanho == 0) return FILA_VAZIA; // estado normal

    // Copia a raiz (sempre o mínimo, propriedade do heap) para a saída.
    strncpy(chaveSaida, fp->itens[0].chave, FILA_CHAVE_MAX - 1);
    chaveSaida[FILA_CHAVE_MAX - 1] = '\0';
    *prioridadeSaida = fp->itens[0].prioridade;

    fp->tamanho--;
    if(fp->tamanho > 0){
        fp->itens[0] = fp->itens[fp->tamanho];
        siftDown(fp, 0);
    }

    return FILA_OK;
}

int filaPrioridadeTamanho(const FilaPrioridade *fp){
    assert(fp != NULL);
    return fp->tamanho;
}