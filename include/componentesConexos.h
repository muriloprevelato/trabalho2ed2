#ifndef COMPONENTES_CONEXOS_H
#define COMPONENTES_CONEXOS_H

#include "grafo.h"

/*
Interface do módulo Componentes Conexos.

Calcula os componentes conexos do sistema viário (comando regs do
.qry): agrupa os vértices do grafo em "ilhas" - conjuntos de vértices
mutuamente alcançáveis considerando apenas trechos com velocidade
média >= vl (trechos com vm < vl são "insuficientes" e não contam para
conectividade, conforme a descrição do projet).

Decisão de direcionalidade: o grafo é direcionado, mas componentes
conexos aqui são calculados tratando-o como NÃO-DIRECIONADO. Uma rua de
mão única ainda conecta fisicamente duas esquinas, mesmo que o tráfego
só ande num sentido.

Algoritmo: Union-Find (conjuntos disjuntos) com compressão de caminho,
processando a lista de TODAS as arestas do grafo (via
percorrerTodasArestas) como pares não-ordenados.

Union-Find fica embutido neste módulo (não é um módulo próprio testável
isoladamente, como filaPrioridade é). Pode ser que tenha que ser extraído para
um módulo próprio em outro momento

Pré-condições:
  - Ponteiros de Grafo/Componentes passados às funções nunca devem ser
    NULL.
  - Índice de componente fora do intervalo [0, componentesNumComponentes(c) - 1]
    em percorrerVerticesDoComponente é erro de uso do chamador (assert).
  - Violações de pré-condição abortam o programa (assert).
*/

// Tipo opaco para representar o resultado do cálculo de componentes.
typedef struct Componentes Componentes;

/**
 * @brief Calcula os componentes conexos do grafo, considerando o grafo como não-direcionado e ignorando trechos com velocidade média abaixo de vl.
 * @param g Grafo a ser analisado. Não deve ser NULL (assert).
 * @param vl Velocidade mínima para um trecho ser considerado "suficiente (contar para conectividade). Trechos com vm < vl são tratados como ausentes. 
 * @return Ponteiro para o resultado, ou NULL em caso de falha de alocação. O chamador deve liberar com destruirComponentes().
 * @details Todo vértice do grafo pertence a exatamente um componente, mesmo um vértice sem nenhum trecho suficiente conectado a ele (nesse caso, forma um componente unitário, sozinho).
 */
Componentes* calcularComponentesConexos(const Grafo *g, double vl);

/**
 * @brief Libera a memória associada a um Componentes.
 * @param c Ponteiro para o resultado. Não deve ser NULL (assert).
 * @details Libera apenas a estrutura de agrupamento - os Vertice* referenciados são emprestados do Grafo original e NUNCA são destruídos aqui.
 */
void destruirComponentes(Componentes *c);

/**
 * @brief Retorna o número de componentes conexos encontrados.
 * @param c Ponteiro para o resultado. Não deve ser NULL (assert).
 * @return Número de componentes (>= 1 se o grafo tiver ao menos um vértice; 0 se o grafo estiver vazio).
 */
int componentesNumComponentes(const Componentes *c);

/**
 * @brief Percorre os vértices pertencentes a um componente específico.
 * @param c Ponteiro para o resultado. Não deve ser NULL (assert).
 * @param indiceComponente Índice do componente, 0-based. Deve estar no intervalo [0, componentesNumComponentes(c) - 1] (assert).
 * @param visitante Função chamada para cada vértice do componente. Não deve ser NULL (assert). Reusa VisitanteVertice (grafo.h)
 * @param contexto Ponteiro opaco repassado a cada chamada do visitante. Pode ser NULL se o visitante não precisar de estado externo.
 * @details A ordem de visita dentro do componente não é especificada.Usado, por exemplo, para calcular o bounding box de cada componente na hora de desenhar o resultado do regs no SVG.
 */
void percorrerVerticesDoComponente(const Componentes *c, int indiceComponente,
                                    VisitanteVertice visitante, void *contexto);

#endif