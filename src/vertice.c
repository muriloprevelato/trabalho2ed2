#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "vertice.h"

struct Vertice {
    char   id[VERTICE_ID_MAX];
    double x, y;
};

Vertice* criarVertice(const char *id, double x, double y){
    // Única função do módulo que valida entrada e retorna NULL em caso de erro.
    if(id == NULL) return NULL;

    Vertice *v = malloc(sizeof(Vertice));
    if(v == NULL) return NULL;

    /*
    Cópia defensiva: nunca guardar o ponteiro id recebido. O leitorVia
    reusa o buffer de fgets a cada linha - guardar por referência
    corromperia todo id já criado assim que a próxima linha for lida.
    */ 
    strncpy(v->id, id, VERTICE_ID_MAX - 1);
    v->id[VERTICE_ID_MAX - 1] = '\0'; // garante terminação mesmo se truncar

    v->x = x;
    v->y = y;

    return v;
}

void destruirVertice(Vertice *v){
    assert(v != NULL);
    free(v);
}

const char* getVerticeId(const Vertice *v){
    assert(v != NULL);
    return v->id;
}

double getVerticeX(const Vertice *v){
    assert(v != NULL);
    return v->x;
}

double getVerticeY(const Vertice *v){
    assert(v != NULL);
    return v->y;
}