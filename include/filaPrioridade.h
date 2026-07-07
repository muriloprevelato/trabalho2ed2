#ifndef FILA_PRIORIDADE_H
#define FILA_PRIORIDADE_H

/*
Interface do módulo FilaPrioridade.

Usado pelo algoritmo de Dijkstra para sempre extrair
o vértice de menor distância conhecida durante a exploração do grafo.

Quando o Dijkstra encontra um caminho mais curto para um vértice já presente na
fila, ele simplesmente insere uma NOVA entrada com a distância menor,
sem tentar localizar e atualizar a entrada antiga. 
A entrada antiga vira "lixo" que eventualmente
será extraído e descartado.


Pré-condições (valendo para todo o módulo):
  - Ponteiros de FilaPrioridade passados às funções nunca devem ser NULL.
  - Chaves (const char*) nunca devem ser NULL.
  - Fila vazia NÃO é um erro de uso - é um estado normal e esperado
    durante a execução de um algoritmo (ex: Dijkstra termina quando a
    fila esvazia). Por isso filaPrioridadeExtrairMin sinaliza fila
    vazia via retorno FILA_VAZIA, nunca via assert.
  - Violações de pré-condição (fora do caso acima) abortam o programa
    (assert).
*/

// Limite máximo da chave armazenada em cada entrada.
#define FILA_CHAVE_MAX 20

// Códigos de retorno padrão para facilitar os testes.
#define FILA_OK    1
#define FILA_VAZIA 0

// Tipo opaco para representar a fila de prioridade.
typedef struct FilaPrioridade FilaPrioridade;

/**
 * @brief Cria uma fila de prioridade vazia.
 * @return Ponteiro para a fila criada, ou NULL em caso de falha de alocação.
 */
FilaPrioridade* criarFilaPrioridade(void);

/**
 * @brief Libera a memória associada a uma fila de prioridade.
 * @param fp Ponteiro para a fila. Não deve ser NULL (assert).
 */
void destruirFilaPrioridade(FilaPrioridade *fp);

/**
 * @brief Insere um par (chave, prioridade) na fila.
 * @param fp Ponteiro para a fila. Não deve ser NULL (assert).
 * @param chave String usada como chave. Não deve ser NULL (assert). Uma cópia interna da chave é feita pela fila.
 * @param prioridade Valor usado para ordenar a fila (menor prioridade
 *        sai primeiro). Qualquer double é aceito, incluindo negativos -
 *        este módulo não impõe restrições de domínio sobre o valor.
 * @details Chaves repetidas são permitidas e não são tratadas de forma especial.
 */
void filaPrioridadeInserir(FilaPrioridade *fp, const char *chave, double prioridade);

/**
 * @brief Remove e devolve o par (chave, prioridade) de menor prioridade atualmente na fila.
 * @param fp Ponteiro para a fila. Não deve ser NULL (assert).
 * @param chaveSaida Buffer de saída onde a chave extraída será
 *        copiada. Não deve ser NULL (assert). Deve ter capacidade de
 *        pelo menos FILA_CHAVE_MAX bytes - toda chave armazenada
 *        respeita esse limite.
 * @param prioridadeSaida Ponteiro para double onde a prioridade extraída será armazenada. Não deve ser NULL (assert).
 * @return FILA_OK se uma entrada foi extraída (chaveSaida e
 *         prioridadeSaida preenchidos); FILA_VAZIA se a fila já estava
 *         vazia (chaveSaida e prioridadeSaida não são tocados).
 * @details Fila vazia é um estado normal, não um erro de uso.
 */
int filaPrioridadeExtrairMin(FilaPrioridade *fp, char *chaveSaida, double *prioridadeSaida);

/**
 * @brief Retorna o número de entradas atualmente na fila.
 * @param fp Ponteiro para a fila. Não deve ser NULL (assert).
 * @return Número de entradas (incluindo eventuais entradas obsoletas inda não extraídas - ver nota de lazy deletion).
 */
int filaPrioridadeTamanho(const FilaPrioridade *fp);

#endif