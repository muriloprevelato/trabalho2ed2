#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "leitorGeo.h"
#include "cidade.h"
#include "quadra.h"

#define LINHA_MAX 512 

typedef struct {
    char   cfill[QUADRA_COR_MAX];
    char   cstrk[QUADRA_COR_MAX];
    double sw;
} EstadoCor;

// Inicializa com os valores default (antes do primeiro cq no arquivo).
static void inicializarEstadoCor(EstadoCor *ec){
    strncpy(ec->cfill, "white", QUADRA_COR_MAX - 1);
    ec->cfill[QUADRA_COR_MAX - 1] = '\0';
    strncpy(ec->cstrk, "black", QUADRA_COR_MAX - 1);
    ec->cstrk[QUADRA_COR_MAX - 1] = '\0';
    ec->sw = 1.0;
}

// Processa um comando 'q'
// Retorna 1 se inseriu com sucesso, 0 se descartou (com aviso no stderr).
static int processarQ(const char *linha, int numLinha, Cidade *cidade, const EstadoCor *ec){
    char   cep[QUADRA_CEP_MAX];
    double x, y, w, h;

    // %19s: limite de largura = QUADRA_CEP_MAX - 1, evita overflow de buffer com strings mais longas que o campo suporta.
    int campos = sscanf(linha, "q %19s %lf %lf %lf %lf", cep, &x, &y, &w, &h);
    if(campos != 5){
        fprintf(stderr, "[leitorGeo] linha %d: comando 'q' malformado"" (esperado 5 campos, lidos %d) - descartada\n", numLinha, campos);
        return 0;
    }

    if(cidadeContemCep(cidade, cep)){
        fprintf(stderr, "[leitorGeo] linha %d: CEP '%s' duplicado" " - quadra descartada, primeira ocorrência mantida\n", numLinha, cep);
        return 0;
    }

    Quadra *q = criarQuadra(cep, x, y, w, h, ec->sw, ec->cfill, ec->cstrk);
    if(q == NULL){
        fprintf(stderr, "[leitorGeo] linha %d: dados inválidos para CEP '%s'" " (dimensão negativa ou outro erro) - quadra descartada\n", numLinha, cep);
        return 0;
    }

    inserirQuadraCidade(cidade, q);
    return 1;
}

// Processa um comando 'cq'
// Retorna 1 se atualizou com sucesso, 0 se malformado (com aviso no stderr).
static int processarCq(const char *linha, int numLinha, EstadoCor *ec){
    char   cfill[QUADRA_COR_MAX];
    char   cstrk[QUADRA_COR_MAX];
    double sw;

    // %31s: limite = QUADRA_COR_MAX - 1
    int campos = sscanf(linha, "cq %lf %31s %31s", &sw, cfill, cstrk);
    if(campos != 3){
        fprintf(stderr, "[leitorGeo] linha %d: comando 'cq' malformado" " (esperado 3 campos, lidos %d) - descartado\n", numLinha, campos);
        return 0;
    }

    ec->sw = sw;
    strncpy(ec->cfill, cfill, QUADRA_COR_MAX - 1);
    ec->cfill[QUADRA_COR_MAX - 1] = '\0';
    strncpy(ec->cstrk, cstrk, QUADRA_COR_MAX - 1);
    ec->cstrk[QUADRA_COR_MAX - 1] = '\0';

    return 1;
}

// Função principal

int lerArquivoGeo(const char *caminho, Cidade *cidade){
    assert(caminho != NULL);
    assert(cidade  != NULL);

    FILE *fp = fopen(caminho, "r");
    if(fp == NULL) return GEO_ERRO; // erro de ambiente: caminho inválido, sem permissão etc.

    EstadoCor ec;
    inicializarEstadoCor(&ec);

    char linha[LINHA_MAX];
    int numLinha = 0;

    while(fgets(linha, sizeof(linha), fp) != NULL){
        numLinha++;

        // Remove o '\n' que o fgets deixa no final - senão o sscanf
        linha[strcspn(linha, "\n")] = '\0';

        // Linha em branco ou só espaços: ignora silenciosamente.
        if(linha[0] == '\0') continue;

        // Identifica o comando pelo primeiro token da linha.
        char cmd[16];
        if(sscanf(linha, "%15s", cmd) != 1) continue;

        if(strcmp(cmd, "q") == 0){
            processarQ(linha, numLinha, cidade, &ec);
        } else if(strcmp(cmd, "cq") == 0){
            processarCq(linha, numLinha, &ec);
        } else {
            fprintf(stderr, "[leitorGeo] linha %d: comando desconhecido '%s'" " - ignorado\n", numLinha, cmd);
        }
    }

    fclose(fp);
    return GEO_OK;
}