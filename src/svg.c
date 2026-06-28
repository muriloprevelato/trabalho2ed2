#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "svg.h"

struct ArqSvg {
    FILE *fp;
};

/*

Escape de XML
Devolve um buffer alocado (chamador libera com free) com o texto
escapado. O buffer é dimensionado para o pior caso (todo caractere
sendo '&', a entidade mais longa: "&amp;", 5 bytes).

*/

static char* escaparXml(const char *texto){
    size_t lenOriginal = strlen(texto);
    char *saida = malloc(lenOriginal * 5 + 1); // pior caso: todo '&'
    assert(saida != NULL);

    char *p = saida;
    for(size_t i = 0; i < lenOriginal; i++){
        char c = texto[i];
        if(c == '&'){
            memcpy(p, "&amp;", 5);
            p += 5;
        } else if(c == '<'){
            memcpy(p, "&lt;", 4);
            p += 4;
        } else if(c == '>'){
            memcpy(p, "&gt;", 4);
            p += 4;
        } else {
            *p++ = c;
        }
    }
    *p = '\0';

    return saida;
}

ArqSvg* abreEscritaSvg(const char *caminho, double largura, double altura){
    assert(caminho != NULL);

    if(largura < 0.0) return NULL;
    if(altura  < 0.0) return NULL;

    FILE *fp = fopen(caminho, "w");
    if(fp == NULL) return NULL; // erro de ambiente: diretório inválido, sem permissão etc.

    ArqSvg *f = malloc(sizeof(ArqSvg));
    if(f == NULL){
        fclose(fp);
        return NULL;
    }

    f->fp = fp;

    fprintf(f->fp,
            "<svg xmlns=\"http://www.w3.org/2000/svg\" "
            "width=\"%.2f\" height=\"%.2f\" "
            "viewBox=\"0 0 %.2f %.2f\">\n",
            largura, altura, largura, altura);

    return f;
}

void fechaSvg(ArqSvg *f){
    assert(f != NULL);

    fprintf(f->fp, "</svg>\n");
    fclose(f->fp);
    free(f);
}

void svgRetangulo(ArqSvg *f, double x, double y, double w, double h,
                   const char *cfill, const char *cstrk, double sw){
    assert(f     != NULL);
    assert(cfill != NULL);
    assert(cstrk != NULL);
    assert(w  >= 0.0);
    assert(h  >= 0.0);
    assert(sw >= 0.0);

    fprintf(f->fp,
            "<rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\" "
            "fill=\"%s\" stroke=\"%s\" stroke-width=\"%.2f\" />\n",
            x, y, w, h, cfill, cstrk, sw);
}

void svgTexto(ArqSvg *f, double x, double y, const char *texto, const char *cor){
    assert(f     != NULL);
    assert(texto != NULL);
    assert(cor   != NULL);

    char *textoEscapado = escaparXml(texto);

    fprintf(f->fp,
            "<text x=\"%.2f\" y=\"%.2f\" fill=\"%s\">%s</text>\n",
            x, y, cor, textoEscapado);

    free(textoEscapado);
}

void svgLinha(ArqSvg *f, double x1, double y1, double x2, double y2,
              const char *cor, double sw){
    assert(f   != NULL);
    assert(cor != NULL);
    assert(sw  >= 0.0);

    fprintf(f->fp,
            "<line x1=\"%.2f\" y1=\"%.2f\" x2=\"%.2f\" y2=\"%.2f\" "
            "stroke=\"%s\" stroke-width=\"%.2f\" />\n",
            x1, y1, x2, y2, cor, sw);
}