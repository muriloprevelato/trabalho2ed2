#ifndef CIDADE_H
#define CIDADE_H

#include "quadra.h"

/*
Interface do módulo Cidade.

Representa o "banco de dados" -> (guarda efetivamente) de quadras de Bitnópolis.
Um módulo de composição que guarda uma HashGenerica indexada por CEP, com cada
quadra como valor armazenado.

Este módulo esconde do resto do programa a decisão de COMO as quadras
são armazenadas.

A Cidade é dona das quadras inseridas: ao destruir a Cidade, todas as
quadras armazenadas são destruídas (destruirQuadra é usado como
destrutor da hash interna).

Pré-condições (valendo para todo o módulo):
  - Ponteiros de Cidade passados às funções nunca devem ser NULL.
  - CEPs (const char*) nunca devem ser NULL.
  - Inserir uma quadra cujo CEP já exista na Cidade é considerado erro
    de uso (estado logicamente impossível para um .geo bem formado) e
    é capturado por assert na implementação. É responsabilidade de
    quem chama (tipicamente o parser do .geo) verificar a duplicata
    antes de inserir, usando cidadeContemCep(). Um .geo com CEP
    repetido é erro de DADO de entrada, não de lógica de programa —
    não deve ser resolvido deixando o assert abortar o programa.
  - Violações de pré-condição abortam o programa (assert).
*/

// Códigos de retorno padrão para facilitar os testes.
#define CIDADE_OK 1
#define CIDADE_ERRO 0

// Tipo opaco para representar a cidade.
typedef struct Cidade Cidade;

/**
 * @brief Cria uma cidade vazia, sem quadras.
 * @return Ponteiro para a cidade criada, ou NULL em caso de falha de alocação.
 */
Cidade* criarCidade(void);

/**
 * @brief Libera a memória associada a uma cidade, incluindo todas as quadras armazenadas (cada uma liberada via destruirQuadra).
 * @param c Ponteiro para a cidade. Não deve ser NULL (assert).
 */
void destruirCidade(Cidade *c);

/**
 * @brief Insere uma quadra na cidade, indexada pelo seu CEP.
 * @param c Ponteiro para a cidade. Não deve ser NULL (assert).
 * @param q Ponteiro para a quadra a ser inserida. Não deve ser NULL(assert). 
 * A partir desta chamada, a cidade passa a ser dona de q — não destrua q manualmente depois de inseri-la.
 * @details O CEP de q não deve já existir na cidade (assert).
 */
void inserirQuadraCidade(Cidade *c, Quadra *q);

/**
 * @brief Busca uma quadra pelo seu CEP.
 * @param c Ponteiro para a cidade. Não deve ser NULL (assert).
 * @param cep CEP da quadra buscada. Não deve ser NULL (assert).
 * @return Ponteiro para a quadra associada ao CEP, ou NULL se nenhuma quadra com esse CEP estiver na cidade.
 */
Quadra* buscarQuadraCidade(const Cidade *c, const char *cep);

/**
 * @brief Verifica se existe uma quadra com o CEP informado.
 * @param c Ponteiro para a cidade. Não deve ser NULL (assert).
 * @param cep CEP buscado. Não deve ser NULL (assert).
 * @return CIDADE_OK se existe quadra com esse CEP, CIDADE_ERRO caso contrário.
 */
int cidadeContemCep(const Cidade *c, const char *cep);

/**
 * @brief Retorna o número de quadras armazenadas na cidade.
 * @param c Ponteiro para a cidade. Não deve ser NULL (assert).
 * @return Número de quadras atualmente armazenadas.
 */
int cidadeNumQuadras(const Cidade *c);

/**
 * @brief Assinatura de função usada para visitar cada quadra durante uma varredura da cidade
 * @param q Ponteiro para a quadra visitada.
 * @param contexto Ponteiro opaco fornecido por quem chamou percorrerQuadrasCidade, repassado sem modificação a cada visita.
 */
typedef void (*VisitanteQuadra)(Quadra *q, void *contexto);

/**
 * @brief Percorre todas as quadras da cidade, chamando o visitante fornecido para cada uma.
 * @param c Ponteiro para a cidade. Não deve ser NULL (assert).
 * @param visitante Função chamada para cada quadra. Não deve ser NULL(assert).
 * @param contexto Ponteiro opaco repassado a cada chamada do visitante, sem ser interpretado pela cidade. Pode ser NULL se o visitante não precisar de estado externo.
 * @details A ordem de visita não é especificada e não deve ser assumida pelo chamador (decorre da ordem interna da hashde CEPs). É o mecanismo usado, por exemplo, para desenhar todas as quadras no .svg de saída.
 */
void percorrerQuadrasCidade(const Cidade *c, VisitanteQuadra visitante, void *contexto);

#endif