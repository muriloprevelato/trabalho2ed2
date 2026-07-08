#ifndef CAMINHO_MINIMO_H
#define CAMINHO_MINIMO_H

#include "grafo.h"

/*
Interface do módulo CaminhoMinimo. (Basicamente o algoritmo de dijkstra, só nomeei diferente
para ser genérico / representar o domínio).

Calcula o caminho de menor custo entre dois vértices de um Grafo,
usando o algoritmo de Dijkstra com uma FilaPrioridade  como estrutura auxiliar interna.
O resultado é um Caminho: a sequência ordenada de ids de vértice de
origem a destino, mais o custo total acumulado.

Dois modos de custo, mesma estrutura de Dijkstra:
  CAMINHO_MAIS_CURTO   peso de cada aresta = getArestaCmp (comprimento)
  CAMINHO_MAIS_RAPIDO  peso de cada aresta = getArestaCmp / getArestaVm
                       (tempo = distância / velocidade)

Caminho é um tipo opaco só de vértices - não guarda, para cada trecho,
qual Aresta (e portanto qual nome de rua) foi percorrida.
Caminho representa a Rota, não a narrativa dela.

Pré-condições:
  - Ponteiros de Grafo/Caminho passados às funções nunca devem ser NULL.
  - idOrigem e idDestino nunca devem ser NULL, e devem existir no grafo
    (assert - mesma fronteira usada em grafoInserirAresta: existência é
    responsabilidade de quem chama checar antes, via grafoContemVertice).
  - Índice fora do intervalo [0, caminhoNumVertices(c) - 1] em
    caminhoObterVertice é erro de uso do chamador (assert) - use
    caminhoNumVertices() para conhecer o intervalo válido antes de indexar.
  - Destino inalcançável NÃO é um erro de uso - é um resultado válido e
    esperado do algoritmo (o grafo pode legitimamente ser desconexo).
    Por isso é sinalizado via retorno CAMINHO_INALCANCAVEL, nunca via
    assert. Ver detalhes em calcularCaminhoMinimo.
*/

// Códigos de retorno padrão para facilitar os testes.
#define CAMINHO_OK           1
#define CAMINHO_INALCANCAVEL 0

// Modo de custo usado para ponderar as arestas durante o cálculo.
typedef enum {
    CAMINHO_MAIS_CURTO,  // peso = comprimento (getArestaCmp)
    CAMINHO_MAIS_RAPIDO  // peso = tempo = comprimento / velocidade média
} ModoCaminho;

// Tipo opaco para representar o resultado: a sequência de vértices do
// caminho encontrado, mais o custo total acumulado.
typedef struct Caminho Caminho;

/**
 * @brief Calcula o caminho de menor custo entre dois vértices do grafo.
 * @param g Grafo onde o caminho será calculado. Não deve ser NULL(assert).
 * @param idOrigem Id do vértice de origem. Não deve ser NULL (assert). Deve existir no grafo. 
 * @param idDestino Id do vértice de destino. Não deve ser NULL (assert). Deve existir no grafo.
 * @param modo Métrica de custo usada para ponderar as arestas (CAMINHO_MAIS_CURTO ou CAMINHO_MAIS_RAPIDO).
 * @param caminhoSaida Ponteiro para onde o resultado será escrito. Não
 *        deve ser NULL (assert). Se o retorno for CAMINHO_OK, passa a
 *        apontar para um Caminho recém-alocado que o chamador deve
 *        liberar com destruirCaminho(). Se o retorno for
 *        CAMINHO_INALCANCAVEL, é definido como NULL - não há nada para
 *        destruir nesse caso.
 * @return CAMINHO_OK se um caminho foi encontrado; CAMINHO_INALCANCAVEL
 *         se idDestino não é alcançável a partir de idOrigem (grafo
 *         desconexo entre os dois pontos).
 * @details Caso idOrigem == idDestino, o caminho encontrado é trivial:
 *          um único vértice (a própria origem), custo total 0 - não é
 *          tratado como caso especial, decorre naturalmente do
 *          algoritmo.
 */
int calcularCaminhoMinimo(const Grafo *g, const char *idOrigem, const char *idDestino, ModoCaminho modo, Caminho **caminhoSaida);

/**
 * @brief Libera a memória associada a um Caminho.
 * @param c Ponteiro para o caminho. Não deve ser NULL (assert).
 */
void destruirCaminho(Caminho *c);

/**
 * @brief Retorna o número de vértices no caminho (origem e destino inclusive).
 * @param c Ponteiro para o caminho. Não deve ser NULL (assert).
 * @return Número de vértices na rota. Sempre >= 1 para um Caminho
 *         obtido de um retorno CAMINHO_OK (no mínimo, a própria origem).
 */
int caminhoNumVertices(const Caminho *c);

/**
 * @brief Retorna o id do vértice numa posição do caminho.
 * @param c Ponteiro para o caminho. Não deve ser NULL (assert).
 * @param indice Posição do vértice desejado, 0-based. 0 é a origem,
 *        caminhoNumVertices(c) - 1 é o destino. Deve estar dentro do
 *        intervalo válido (assert).
 * @return String com o id do vértice naquela posição da rota.
 */
const char* caminhoObterVertice(const Caminho *c, int indice);

/**
 * @brief Retorna o custo total acumulado do caminho.
 * @param c Ponteiro para o caminho. Não deve ser NULL (assert).
 * @return Custo total, na métrica do ModoCaminho usado no cálculo
 *         (distância para CAMINHO_MAIS_CURTO, tempo para
 *         CAMINHO_MAIS_RAPIDO). Para um caminho trivial (idOrigem ==
 *         idDestino), o custo é 0.
 */
double caminhoCustoTotal(const Caminho *c);

#endif