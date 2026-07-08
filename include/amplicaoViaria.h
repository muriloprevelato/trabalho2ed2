#ifndef AMPLIACAO_VIARIA_H
#define AMPLIACAO_VIARIA_H

#include "grafo.h"

/*
Interface do módulo AmpliacaoViaria.

Implementa o comando exp do .qry: calcula a árvore geradora mínima
(MST) do sistema viário, seleciona dentro dela os trechos com
velocidade média abaixo de vl, e aumenta o vm desses trechos em 50% -

*** Obs: - vl aqui NÃO tem o mesmo papel que em ComponentesConexos: lá,
vl é limiar de exclusão (trechos abaixo dele não contam para
conectividade). Aqui, vl é limiar de seleção(trechos da MST abaixo
dele são os que recebem o upgrade).

Decisão de direcionalidade (mesma lógica de ComponentesConexos):
trata cada Aresta como uma conexão não direcionada entre
origem e destino - uma rua de mão única ainda conecta fisicamente os
dois lados para fins de montar a árvore.

Este módulo muda o grafo - diferente de ComponentesConexos e
CaminhoMinimo, que são cálculos somente-leitura. calcularAmpliacaoViaria
chama setArestaVm nas arestas selecionadas ANTES de retornar; por isso
recebe Grafo* (não const Grafo*).

Union-Find: implementado de forma duplicada e independente do usado em
ComponentesConexos (dois algoritmos pequenos,
duplicação mais simples que compartilhar um módulo para dois
consumidores só). 

^^^ Preferi não criar um módulo só para esse algoritmo, uma vez que é algo enxuto.
Traria uma economia de poucas linhas dentro desse código e só.

Ampliacao é um tipo opaco: guarda, para cada aresta selecionada, o id
de origem (copiado - necessário porque Aresta não expõe origem, ver
grafo.h) e um ponteiro emprestado para a própria Aresta (já mutada, com
o vm novo). Nunca copia nem destrói a Aresta; seu ciclo de vida deve
durar no máximo o tempo de vida do Grafo que o originou.

Pré-condições:
  - Ponteiros de Grafo/Ampliacao passados às funções nunca devem ser
    NULL.
  - Violações de pré-condição abortam o programa (assert).
*/

// Tipo opaco para representar o resultado: as arestas selecionadas e já ampliadas (vm aumentado em 50%).
typedef struct Ampliacao Ampliacao;

/**
 * @brief Calcula a árvore geradora mínima do grafo (peso = cmp),
 *        seleciona dentro dela as arestas com vm < vl, e aumenta o vm
 *        dessas arestas em 50%.
 * @param g Grafo a ser analisado E mutado. Não deve ser NULL (assert).
 *        As arestas selecionadas têm seu vm alterado por esta chamada
 * @param vl Limiar de SELEÇÃO: arestas da MST com vm < vl são as que
 *        recebem o upgrade.
 * @return Ponteiro para o resultado (as arestas selecionadas e já
 *         ampliadas), ou NULL em caso de falha de alocação - nesse
 *         caso nenhuma aresta é mutada. O chamador deve liberar com
 *         destruirAmpliacao().
 * @details Se nenhuma aresta da MST tiver vm < vl, o resultado é
 *          válido com zero arestas (não é tratado como erro). Se o
 *          grafo for desconexo (caso típico, não exceção.
 */
Ampliacao* calcularAmpliacaoViaria(Grafo *g, double vl);

/**
 * @brief Libera a memória associada a uma Ampliacao.
 * @param a Ponteiro para o resultado. Não deve ser NULL (assert).
 * @details Libera apenas a estrutura de agrupamento - as Aresta*
 *          referenciadas são emprestadas do Grafo e NUNCA são
 *          destruídas aqui. O vm já aumentado permanece no Grafo
 *          mesmo depois de destruirAmpliacao - a mutação já aconteceu
 *          e não é desfeita.
 */
void destruirAmpliacao(Ampliacao *a);

/**
 * @brief Retorna o número de arestas selecionadas (e já ampliadas).
 * @param a Ponteiro para o resultado. Não deve ser NULL (assert).
 * @return Número de arestas no resultado. Pode ser 0.
 */
int ampliacaoNumArestas(const Ampliacao *a);

/**
 * @brief Percorre as arestas selecionadas (já ampliadas).
 * @param a Ponteiro para o resultado. Não deve ser NULL (assert).
 * @param visitante Função chamada para cada aresta selecionada. Não
 *        deve ser NULL (assert). Reusa VisitanteArestaCompleta
 *        (grafo.h) - mesma assinatura de percorrerTodasArestas, já que
 *        aqui também é necessário saber a origem de cada aresta (que
 *        Aresta não guarda por conta própria).
 * @param contexto Ponteiro opaco repassado a cada chamada do
 *        visitante. Pode ser NULL se o visitante não precisar de
 *        estado externo.
 * @details A ordem de visita não é especificada.
 */
void percorrerArestasAmpliadas(const Ampliacao *a, VisitanteArestaCompleta visitante, void *contexto);

#endif