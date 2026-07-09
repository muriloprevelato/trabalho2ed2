#ifndef RESOLVE_ENDERECO_H
#define RESOLVE_ENDERECO_H

#include "cidade.h"
#include "grafo.h"

/*
Interface do módulo ResolveEndereco.

Primeira ponte entre os dois mundos do projeto: Cidade/Quadra (dados
geográficos do .geo) e Grafo/Vertice (sistema viário do .via). Recebe
um endereço no formato cep/face/num (o mesmo usado por @o? no .qry) e
devolve tanto a coordenada geográfica resolvida quanto o id do vértice
do grafo mais próximo dessa coordenada.

Por que devolve os dois: o comando @o? precisa da coordenada (pra
desenhar a linha pontilhada vermelha marcando o endereço no SVG, e pra
guardar no registrador); o comando p? precisa do vértice (pra alimentar
calcularCaminhoMinimo). Em vez de duas funções separadas que refariam
os mesmos dois passos internos (busca da quadra, cálculo da coordenada,
busca do vértice mais próximo), uma função só devolve ambos.

Pré-condições:
  - Ponteiros de Cidade/Grafo passados nunca devem ser NULL.
  - cep nunca deve ser NULL.
  - Ponteiros de saída (xSaida, ySaida, idVerticeSaida) nunca devem ser
    NULL. idVerticeSaida deve apontar para um buffer de pelo menos
    VERTICE_ID_MAX bytes, fornecido por quem chama.
  - Violações das pré-condições acima abortam o programa (assert). Os
    quatro motivos de falha específicos do domínio (ver acima) NÃO são
    pré-condições - são retornados via RESOLVE_ERRO.
*/

// Códigos de retorno padrão para facilitar os testes.
#define RESOLVE_OK 1
#define RESOLVE_ERRO 0

/**
 * @brief Resolve um endereço (cep/face/num) para a coordenada geográfica correspondente e o id do vértice do grafo mais próximo dela.
 * @param cidade Cidade onde o cep será buscado. Não deve ser NULL (assert).
 * @param grafo Grafo onde o vértice mais próximo será buscado. Não deve ser NULL (assert).
 * @param cep Cep do endereço. Não deve ser NULL (assert). Pode não existir na cidade - nesse caso, RESOLVE_ERRO 
 * @param face Caracter da face do endereço (N/S/L/O). Pode ser inválido - nesse caso, RESOLVE_ERRO.
 * @param num Número do endereço sobre a face. 
 * @param xSaida Ponteiro para double onde a coordenada x resolvida
 *        será escrita, se o retorno for RESOLVE_OK. Não deve ser NULL
 *        (assert). Não é tocado se o retorno for RESOLVE_ERRO.
 * @param ySaida Ponteiro para double onde a coordenada y resolvida
 *        será escrita, se o retorno for RESOLVE_OK. Não deve ser NULL
 *        (assert). Não é tocado se o retorno for RESOLVE_ERRO.
 * @param idVerticeSaida Buffer (mínimo VERTICE_ID_MAX bytes) onde o id
 *        do vértice mais próximo será copiado, se o retorno for
 *        RESOLVE_OK. Não deve ser NULL (assert). Não é tocado se o
 *        retorno for RESOLVE_ERRO.
 * @return RESOLVE_OK se o endereço foi resolvido com sucesso;
 *         RESOLVE_ERRO se o cep não existe na cidade, a face é
 *         inválida, num está fora do intervalo válido da face, ou o
 *         grafo não tem nenhum vértice (caso de borda: não há "mais
 *         próximo" possível).
 * @details "Mais próximo" é decidido por distância euclidiana entre a
 *          coordenada resolvida e a coordenada de cada vértice do
 *          grafo - sem nenhuma noção de conectividade viária nessa
 *          escolha, só proximidade geométrica pura.
 */
int resolverEndereco(const Cidade *cidade, const Grafo *grafo, const char *cep, char face, double num, double *xSaida, double *ySaida, char *idVerticeSaida);

#endif