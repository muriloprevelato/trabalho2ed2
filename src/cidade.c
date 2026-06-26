#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "cidade.h"
#include "quadra.h"
#include "hashGenerica.h"


#define CIDADE_CAPACIDADE_INICIAL 16 // pequena de propósito; o rehash da HashGenerica cuida do crescimento.

struct Cidade {
    HashGenerica *quadrasPorCep;
};

// Este adaptador faz o cast explícito uma única vez,
// num lugar óbvio, e chama a função certa por baixo.
static void destruirQuadraAdaptador(void *valor){
    destruirQuadra((Quadra*) valor);
}

// Criação e destruição 

Cidade* criarCidade(void){
    Cidade *c = malloc(sizeof(Cidade));
    if(c == NULL) return NULL;

    c->quadrasPorCep = criarHash(CIDADE_CAPACIDADE_INICIAL, destruirQuadraAdaptador);
    if(c->quadrasPorCep == NULL){
        free(c);
        return NULL;
    }

    return c;
}

void destruirCidade(Cidade *c){
    assert(c != NULL);

    // destruirHash chama destruirQuadraAdaptador para cada Quadra* ainda
    // presente — é aqui que a cidade efetivamente libera suas quadras.
    destruirHash(c->quadrasPorCep);
    free(c);
}

void inserirQuadraCidade(Cidade *c, Quadra *q){
    assert(c != NULL);
    assert(q  != NULL);

    const char *cep = getQuadraCep(q);

    /*
    A checagem de duplicata já mora na HashGenerica (assert em
    inserirHash); repassamos a quadra direto. O  .geo é quem
    deve checar cidadeContemCep() antes de chamar esta função.
    */ 
    inserirHash(c->quadrasPorCep, cep, q);
}

Quadra* buscarQuadraCidade(const Cidade *c, const char *cep){
    assert(c   != NULL);
    assert(cep != NULL);

    return (Quadra*) buscarHash(c->quadrasPorCep, cep);
}

int cidadeContemCep(const Cidade *c, const char *cep){
    assert(c   != NULL);
    assert(cep != NULL);

    return contemChaveHash(c->quadrasPorCep, cep);
}

int cidadeNumQuadras(const Cidade *c){
    assert(c != NULL);
    return tamanhoHash(c->quadrasPorCep);
}

/* percorrerHash só sabe chamar HashVisitante(chave, valor, contexto). Para
expor um VisitanteQuadra(quadra, contexto) mais magro (sem o CEP solto,
já que ele está acessível via getQuadraCep), precisamos embrulhar o par
(visitante do usuário, contexto do usuário) em uma única struct e passar
essa struct como o "contexto" que a hash repassa. O adaptador abaixo
desembrulha isso a cada chamada e invoca o visitante real.
*/
typedef struct {
    VisitanteQuadra visitanteReal;
    void *contextoReal;
} EmbrulhoVisitante;

static void adaptadorVisitante(const char *chave, void *valor, void *contexto){
    (void) chave; // VisitanteQuadra não recebe o CEP separado — já está na Quadra
    EmbrulhoVisitante *embrulho = (EmbrulhoVisitante*) contexto;
    embrulho->visitanteReal((Quadra*) valor, embrulho->contextoReal);
}

void percorrerQuadrasCidade(const Cidade *c, VisitanteQuadra visitante, void *contexto){
    assert(c         != NULL);
    assert(visitante != NULL);

    EmbrulhoVisitante embrulho;
    embrulho.visitanteReal = visitante;
    embrulho.contextoReal  = contexto;

    percorrerHash(c->quadrasPorCep, adaptadorVisitante, &embrulho);
}