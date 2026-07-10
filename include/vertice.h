#ifndef VERTICE_H
#define VERTICE_H

/*
Interface do módulo Vertice.

Representa um vértice do grafo viário: um ponto do mapa identificado
por um id alfanumérico e uma coordenada(x, y). 

A composição com o grafo (adjacência, arestas) é responsabilidade do
módulo Grafo, não deste módulo.

Pré-condições (valendo para todo o módulo):
  - Ponteiros de Vertice passados às funções (exceto criarVertice, que
    o cria) nunca devem ser NULL.
  - Violações de pré-condição abortam o programa (assert), EXCETO na
    criação: id NULL é erro de DADO de entrada (ex: linha malformada
    do .via), não de lógica de programa - sinalizado por retorno NULL,
    nunca por assert.
*/

// Constantes.
#define VERTICE_ID_MAX 64 // Limite máximo do id do vértice. Mudei uma vez que nos testes enviados pelo professor, a quantidade de caracteres excedia o limite anterior.


// Tipo opaco para representar o vértice.
typedef struct Vertice Vertice;

/**
 * @brief Cria um vértice com id e coordenada.
 * @param id Id alfanumérico do vértice. Não deve ser NULL.
 * @param x Coordenada x do vértice.
 * @param y Coordenada y do vértice.
 * @return Ponteiro para o vértice criado, ou NULL em caso de erro
 *         (id NULL, falha de alocação).
 */
Vertice* criarVertice(const char *id, double x, double y);

/**
 * @brief Libera a memória associada a um vértice.
 * @param v Ponteiro para o vértice. Não deve ser NULL (assert).
 */
void destruirVertice(Vertice *v);

/**
 * @brief Retorna o id de um vértice.
 * @param v Ponteiro para o vértice. Não deve ser NULL (assert).
 * @return String que representa o id alfanumérico.
 */
const char* getVerticeId(const Vertice *v);

/**
 * @brief Retorna a coordenada x de um vértice.
 * @param v Ponteiro para o vértice. Não deve ser NULL (assert).
 * @return Double que representa a coordenada x.
 */
double getVerticeX(const Vertice *v);

/**
 * @brief Retorna a coordenada y de um vértice.
 * @param v Ponteiro para o vértice. Não deve ser NULL (assert).
 * @return Double que representa a coordenada y.
 */
double getVerticeY(const Vertice *v);

#endif