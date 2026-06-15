#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "quadra.h"
#include <assert.h>

struct Quadra {
    char cep[QUADRA_CEP_MAX];
    char cfill[QUADRA_COR_MAX];
    char cstrk[QUADRA_COR_MAX];
    double x, y;   // âncora SUDESTE -> Canto Superior Esquerdo, ficar atento à representação visual
    double w, h;
    double sw;
};

Quadra* criarQuadra(const char* cep, double x, double y, double w, double h, double sw, const char* cfill, const char* cstrk){
    // Única função do módulo que valida entrada e retorna NULL em caso de erro.
    if(cep == NULL) return NULL;
    if(cfill == NULL) return NULL;
    if(cstrk == NULL) return NULL;
    if(w < 0.0) return NULL;
    if(h < 0.0) return NULL;

    Quadra *q = malloc(sizeof(Quadra));
    if(q == NULL) return NULL;

    strncpy(q->cep,   cep,   QUADRA_CEP_MAX - 1);
    strncpy(q->cfill, cfill, QUADRA_COR_MAX - 1);
    strncpy(q->cstrk, cstrk, QUADRA_COR_MAX - 1);
    q->cep  [QUADRA_CEP_MAX - 1] = '\0'; // garante terminação mesmo se truncar
    q->cfill[QUADRA_COR_MAX - 1] = '\0';
    q->cstrk[QUADRA_COR_MAX - 1] = '\0';

    q->x  = x;
    q->y  = y;
    q->w  = w;
    q->h  = h;
    q->sw = sw;

    return q;
}

void destruirQuadra(Quadra *q){
    assert(q != NULL);
    free(q);
}

const char* getQuadraCep(const Quadra *q){
    assert(q != NULL);
    return q->cep;
}

double getQuadraX(const Quadra *q){
    assert(q != NULL);
    return q->x;
}

double getQuadraY(const Quadra *q){
    assert(q != NULL);
    return q->y;
}

double getQuadraW(const Quadra *q){
    assert(q != NULL);
    return q->w;
}

double getQuadraH(const Quadra *q){
    assert(q != NULL);
    return q->h;
}

double getQuadraSw(const Quadra *q){
    assert(q != NULL);
    return q->sw;
}

const char* getQuadraCFill(const Quadra *q){
    assert(q != NULL);
    return q->cfill;
}

const char* getQuadraCStrk(const Quadra *q){
    assert(q != NULL);
    return q->cstrk;
}

int faceValida(char face){
    return (face == FACE_N || face == FACE_S || face == FACE_L || face == FACE_O) ? QUADRA_OK : QUADRA_ERRO;
}

FaceQuadra charParaFaceQuadra(char face){
    assert(faceValida(face) == QUADRA_OK);
    return (FaceQuadra) face;
}

/* Bússola (notação invertida — y cresce para baixo):
SE -------- SO
|           |
NE -------- NO
num é a distância percorrida ao longo da face a partir da âncora:
faces S/N: num percorre x, de 0 até w.
faces L/O: num percorre y, de 0 até h.
*/
void obterCoordenadasEndereco(const Quadra *q, FaceQuadra face, double num, double *outX, double *outY){
    assert(q    != NULL);
    assert(outX != NULL);
    assert(outY != NULL);
    assert(faceValida((char) face) == QUADRA_OK);

    switch(face){
        case FACE_S:
            assert(num >= 0.0 && num <= q->w);
            *outX = q->x + num;
            *outY = q->y;
            break;
        case FACE_N:
            assert(num >= 0.0 && num <= q->w);
            *outX = q->x + num;
            *outY = q->y + q->h;
            break;
        case FACE_L:
            assert(num >= 0.0 && num <= q->h);
            *outX = q->x;
            *outY = q->y + num;
            break;
        case FACE_O:
            assert(num >= 0.0 && num <= q->h);
            *outX = q->x + q->w;
            *outY = q->y + num;
            break;
        default:
            assert(0 && "Face nao tratada no switch");
    }
}