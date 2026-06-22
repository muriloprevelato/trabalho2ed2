#ifndef HASH_GENERICA_H
#define HASH_GENERICA_H

/*
Interface do módulo de Tabela Hash Genérica.

Tabela hash com encadeamento (buckets de lista ligada) que mapeia
chaves string (const char*) para valores void*.

A tabela é dona dos valores armazenados: ao destruir a tabela, o
destrutor fornecido na criação é chamado sobre cada valor armazenado.
Isso permite reaproveitar o módulo.

Esta versão da tabela não oferece remoção nem sobrescrita de entradas
(inserir uma chave já existente é erro de uso). 
O destrutor configurado na criação é acionado apenas por destruirHash,
sobre cada valor ainda presente na tabela no momento da destruição.

A tabela cresce dinamicamente (rehash) quando o fator de carga
ultrapassa um limiar interno, mantendo a busca eficiente mesmo com
muitas inserções.

Pré-condições (valendo para todo o módulo):
  - Ponteiros de HashGenerica passados às funções nunca devem ser NULL.
  - Chaves (const char*) nunca deve ser NULL.
  - Inserir uma chave já existente na tabela é considerado erro de uso
    (estado logicamente impossível para o fluxo esperado do programa)
    e é capturado por assert na implementação.
  - Violações de pré-condição abortam o programa (assert).
*/

// Códigos de retorno padrão para facilitar os testes.
#define HASH_OK 1
#define HASH_ERRO 0

// Tipo opaco para representar a tabela hash.
typedef struct HashGenerica HashGenerica;

/**
 * @brief Assinatura de função usada pela tabela para liberar um valor
 *        armazenado, chamada sobre cada valor ainda presente quando a
 *        tabela é destruída (ver destruirHash).
 * @param valor Ponteiro para o valor a ser liberado.
 */
typedef void (*HashDestrutor)(void *valor);

/**
 * @brief Cria uma tabela hash vazia.
 * @param capacidadeInicial Número inicial de buckets. * Deve ser >= 1.
 * @param destrutor Função usada para liberar cada valor armazenado quando a tabela for destruída (ver     destruirHash). Não deve ser NULL.
 * @return Ponteiro para a tabela criada, ou NULL em caso de erro
 *         (capacidadeInicial < 1, destrutor NULL, falha de alocação).
 */
HashGenerica* criarHash(int capacidadeInicial, HashDestrutor destrutor);

/**
 * @brief Libera a memória associada a uma tabela hash, incluindo todos
 *        os valores armazenados (chamando o destrutor configurado em
 *        cada um) e as cópias internas das chaves.
 * @param h Ponteiro para a tabela. Não deve ser NULL (assert).
 */
void destruirHash(HashGenerica *h);

/**
 * @brief Insere um par chave/valor na tabela.
 * @param h Ponteiro para a tabela. Não deve ser NULL (assert).
 * @param chave String usada como chave. Não deve ser NULL (assert).
 *        Uma cópia interna da chave é feita pela tabela.
 * @param valor Ponteiro para o valor a ser armazenado. Não deve ser
 *        NULL (assert) — a tabela não distingue "ausente" de "valor NULL".
 * @details A chave não deve já existir na tabela (assert).
 */
void inserirHash(HashGenerica *h, const char *chave, void *valor);

/**
 * @brief Busca o valor associado a uma chave.
 * @param h Ponteiro para a tabela. Não deve ser NULL (assert).
 * @param chave String usada como chave de busca. Não deve ser NULL (assert).
 * @return Ponteiro para o valor associado à chave, ou NULL se a chave
 *         não estiver presente na tabela.
 */
void* buscarHash(const HashGenerica *h, const char *chave);

/**
 * @brief Verifica se uma chave está presente na tabela.
 * @param h Ponteiro para a tabela. Não deve ser NULL (assert).
 * @param chave String usada como chave de busca. Não deve ser NULL (assert).
 * @return HASH_OK se a chave está presente, HASH_ERRO caso contrário.
 */
int contemChaveHash(const HashGenerica *h, const char *chave);

/**
 * @brief Retorna o número de pares chave/valor armazenados na tabela.
 * @param h Ponteiro para a tabela. Não deve ser NULL (assert).
 * @return Número de entradas atualmente armazenadas.
 */
int tamanhoHash(const HashGenerica *h);

/**
 * @brief Assinatura de função usada para visitar cada par chave/valor
 *        durante uma varredura da tabela (ver percorrerHash).
 * @param chave Chave da entrada visitada (somente leitura).
 * @param valor Ponteiro para o valor da entrada visitada.
 * @param contexto Ponteiro opaco fornecido por quem chamou
 *        percorrerHash, repassado sem modificação a cada visita.
 */
typedef void (*HashVisitante)(const char *chave, void *valor, void *contexto);

/**
 * @brief Percorre todas as entradas da tabela, chamando o visitante
 *        fornecido para cada par chave/valor.
 * @param h Ponteiro para a tabela. Não deve ser NULL (assert).
 * @param visitante Função chamada para cada entrada. Não deve ser
 *        NULL (assert).
 * @param contexto Ponteiro opaco repassado a cada chamada do
 *        visitante, sem ser interpretado pela tabela. Pode ser NULL
 *        se o visitante não precisar de estado externo.
 * @details A ordem de visita não é especificada (depende da disposição
 *          interna dos buckets) e não deve ser assumida pelo chamador.
 *          O visitante não deve inserir nem destruir entradas da tabela
 *          durante a varredura.
 */
void percorrerHash(const HashGenerica *h, HashVisitante visitante, void *contexto);

#endif