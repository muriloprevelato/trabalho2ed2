#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "registradores.h"

// Um slot individual do banco - array de tamanho fixo (REGISTRADOR_MAX), então Registradores é um único bloco alocado, sem alocações aninhadas.
typedef struct {
    double x, y;
    char texto[REGISTRADOR_TEXTO_MAX];
    int preenchido; // REGISTRADOR_OK ou REGISTRADOR_ERRO
} Registrador;

struct Registradores {
    Registrador slots[REGISTRADOR_MAX];
};

Registradores* criarRegistradores(void){
    Registradores *r = malloc(sizeof(Registradores));
    if(r == NULL) return NULL;
    for(int i = 0; i < REGISTRADOR_MAX; i++){
        r->slots[i].preenchido = REGISTRADOR_ERRO;
    }

    return r;
}

void destruirRegistradores(Registradores *r){
    assert(r != NULL);
    free(r); // bloco único, sem alocações aninhadas para liberar à parte
}

void setRegistrador(Registradores *r, int indice, double x, double y, const char *texto){
    assert(r != NULL);
    assert(indice >= 0 && indice < REGISTRADOR_MAX);
    assert(texto != NULL);

    Registrador *slot = &r->slots[indice];

    slot->x = x;
    slot->y = y;

    strncpy(slot->texto, texto, REGISTRADOR_TEXTO_MAX - 1);
    slot->texto[REGISTRADOR_TEXTO_MAX - 1] = '\0';

    slot->preenchido = REGISTRADOR_OK; // sobrescreve sem erro, mesmo se já preenchido
}

int registradorPreenchido(const Registradores *r, int indice){
    assert(r != NULL);
    assert(indice >= 0 && indice < REGISTRADOR_MAX);

    return r->slots[indice].preenchido;
}

double getRegistradorX(const Registradores *r, int indice){
    assert(r != NULL);
    assert(indice >= 0 && indice < REGISTRADOR_MAX);
    assert(r->slots[indice].preenchido == REGISTRADOR_OK);

    return r->slots[indice].x;
}

double getRegistradorY(const Registradores *r, int indice){
    assert(r != NULL);
    assert(indice >= 0 && indice < REGISTRADOR_MAX);
    assert(r->slots[indice].preenchido == REGISTRADOR_OK);

    return r->slots[indice].y;
}

const char* getRegistradorTexto(const Registradores *r, int indice){
    assert(r != NULL);
    assert(indice >= 0 && indice < REGISTRADOR_MAX);
    assert(r->slots[indice].preenchido == REGISTRADOR_OK);

    return r->slots[indice].texto;
}