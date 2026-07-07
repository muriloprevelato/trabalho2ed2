#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "leitorVia.h"
#include "grafo.h"
#include "vertice.h"

#define LINHA_MAX 512 // fgets: tamanho máximo de linha do .via

// Processa um comando 'v'
static int processarV(const char *linha, int numLinha, Grafo *grafo){
    char   id[VERTICE_ID_MAX];
    double x, y;

    int campos = sscanf(linha, "v %19s %lf %lf", id, &x, &y);
    if(campos != 3){
        fprintf(stderr, "[leitorVia] linha %d: comando 'v' malformado"
                        " (esperado 3 campos, lidos %d) - descartado\n",
                numLinha, campos);
        return 0;
    }

    if(grafoContemVertice(grafo, id)){
        fprintf(stderr, "[leitorVia] linha %d: vertice '%s' duplicado"
                        " - descartado\n",
                numLinha, id);
        return 0;
    }

    Vertice *v = criarVertice(id, x, y);
    if(v == NULL){
        fprintf(stderr, "[leitorVia] linha %d: falha ao criar vertice '%s'"
                        " - descartado\n", numLinha, id);
        return 0;
    }

    grafoInserirVertice(grafo, v);
    return 1;
}

// Processa um comando 'e': faz o parse dos campos, checa existência de
// ambos os vértices (origem e destino), tenta inserir a aresta.
// Retorna 1 se inseriu com sucesso, 0 se descartou (com aviso no stderr).
static int processarE(const char *linha, int numLinha, Grafo *grafo){
    char   idOrigem[VERTICE_ID_MAX];
    char   idDestino[VERTICE_ID_MAX];
    char   ldir[ARESTA_LADO_MAX];
    char   lesq[ARESTA_LADO_MAX];
    double cmp, vm;
    char   nome[ARESTA_NOME_MAX];

    int campos = sscanf(linha, "e %19s %19s %19s %19s %lf %lf %63s",
                        idOrigem, idDestino, ldir, lesq, &cmp, &vm, nome);
    if(campos != 7){
        fprintf(stderr, "[leitorVia] linha %d: comando 'e' malformado"
                        " (esperado 7 campos, lidos %d) - descartada\n",
                numLinha, campos);
        return 0;
    }

    if(!grafoContemVertice(grafo, idOrigem) || !grafoContemVertice(grafo, idDestino)){
        fprintf(stderr, "[leitorVia] linha %d: aresta '%s'->'%s' referencia"
                        " vertice inexistente - descartada\n",
                numLinha, idOrigem, idDestino);
        return 0;
    }

    int resultado = grafoInserirAresta(grafo, idOrigem, idDestino,
                                        ldir, lesq, cmp, vm, nome);
    if(resultado != GRAFO_OK){
        fprintf(stderr, "[leitorVia] linha %d: aresta '%s'->'%s' com dados"
                        " invalidos (cmp ou vm negativos) - descartada\n",
                numLinha, idOrigem, idDestino);
        return 0;
    }

    return 1;
}

// Função prinicipal

int lerArquivoVia(const char *caminho, Grafo *grafo){
    assert(caminho != NULL);
    assert(grafo   != NULL);

    FILE *fp = fopen(caminho, "r");
    if(fp == NULL) return VIA_ERRO; // erro de ambiente: caminho inválido, sem permissão etc.

    char linha[LINHA_MAX];
    int numLinha = 0;

    if(fgets(linha, sizeof(linha), fp) != NULL){
        numLinha++;
        linha[strcspn(linha, "\n")] = '\0';

        int nv;
        if(sscanf(linha, "%d", &nv) != 1){
            fprintf(stderr, "[leitorVia] linha %d: esperado nv (numero de"
                            " vertices) mas nao encontrado - ignorando e"
                            " prosseguindo da proxima linha\n", numLinha);
        }
        // O valor de nv, mesmo quando lido com sucesso, não é usado.
    }

    while(fgets(linha, sizeof(linha), fp) != NULL){
        numLinha++;

        // Remove o '\n' que o fgets deixa no final.
        linha[strcspn(linha, "\n")] = '\0';

        // Linha em branco ou só espaços: ignora silenciosamente.
        if(linha[0] == '\0') continue;

        // Identifica o comando pelo primeiro token da linha.
        char cmd[16];
        if(sscanf(linha, "%15s", cmd) != 1) continue;

        if(strcmp(cmd, "v") == 0){
            processarV(linha, numLinha, grafo);
        } else if(strcmp(cmd, "e") == 0){
            processarE(linha, numLinha, grafo);
        } else {
            fprintf(stderr, "[leitorVia] linha %d: comando desconhecido '%s'"
                            " - ignorado\n",
                    numLinha, cmd);
        }
    }

    fclose(fp);
    return VIA_OK;
}