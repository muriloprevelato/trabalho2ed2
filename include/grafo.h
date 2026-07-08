#ifndef GRAFO_H
#define GRAFO_H

#include "vertice.h"

/*
Interface do módulo Grafo.

Representa o mapa viário de Bitnópolis: um grafo direcionado de
Vertice (esquinas/pontos do mapa) conectados por Aresta (segmentos de
rua, com sentido de tráfego, nome, quadras adjacentes, comprimento e
velocidade média).

Vertice é um módulo independente (não conhece Grafo). Aresta só existe
vinculada a um Grafo - não há criarAresta() exposta, pois uma aresta
sem grafo não tem significado (sempre referencia um vértice de origem
e um de destino dentro do mesmo grafo).

Ownership - regra explícita para evitar double-free:
  A HashGenerica interna (id -> Vertice*) é a ÚNICA dona dos Vertice do
  grafo; seu destrutor é um adaptador para destruirVertice, acionado
  por destruirGrafo. 

Pré-condições (valendo para todo o módulo):
  - Ponteiros de Grafo passados às funções nunca devem ser NULL.
  - Ids de vértice (const char*) nunca devem ser NULL.
  - Inserir um Vertice cujo id já exista no grafo é erro de uso (estado
    logicamente impossível para um .via bem formado) e é capturado por
    assert. É responsabilidade de quem chama (o leitorVia) verificar a
    duplicata antes de inserir, usando grafoContemVertice().
  - Inserir uma Aresta referenciando um id de origem ou destino que não
    existe no grafo é, pelo mesmo raciocínio, erro de uso capturado por
    assert - um .via malformado (aresta apontando para vértice nunca
    declarado) deve ser detectado pelo leitorVia antes de chamar
    grafoInserirAresta, usando grafoContemVertice() em ambos os ids.
  - Já cmp e vm inválidos (negativos) são validação do Valor em si, não
    uma checagem de existência - por isso não são assert, e sim
    reportados via retorno GRAFO_ERRO de grafoInserirAresta (mesmo
    raciocínio usado para w/h negativos em criarQuadra).
  - Violações de pré-condição (exceto as duas exceções acima) abortam o
    programa (assert).
*/

// Constantes.
#define ARESTA_NOME_MAX 64 // Limite máximo do nome da rua.
#define ARESTA_LADO_MAX 20 // Limite máximo de ldir/lesq (CEP ou "-").

// Códigos de retorno padrão para facilitar os testes.
#define GRAFO_OK 1
#define GRAFO_ERRO 0

// Tipos opacos para representar o grafo e uma aresta.
typedef struct Grafo Grafo;
typedef struct Aresta Aresta;

/**
 * @brief Cria um grafo vazio, sem vértices nem arestas.
 * @return Ponteiro para o grafo criado, ou NULL em caso de falha de
 *         alocação.
 */
Grafo* criarGrafo(void);

/**
 * @brief Libera a memória associada a um grafo, incluindo todos os vértices e arestas armazenados.
 * @param g Ponteiro para o grafo. Não deve ser NULL (assert).
 */
void destruirGrafo(Grafo *g);

/**
 * @brief Insere um vértice já criado no grafo, indexado pelo seu id.
 * @param g Ponteiro para o grafo. Não deve ser NULL (assert).
 * @param v Ponteiro para o vértice a ser inserido. Não deve ser NULL (assert). A partir desta chamada, o grafo passa a ser dono de v - não destrua v manualmente depois de inseri-lo.
 * @details O id de v não deve já existir no grafo (assert). Ver nota
 *          de pré-condições no topo deste arquivo.
 */
void grafoInserirVertice(Grafo *g, Vertice *v);

/**
 * @brief Busca um vértice pelo seu id.
 * @param g Ponteiro para o grafo. Não deve ser NULL (assert).
 * @param id Id do vértice buscado. Não deve ser NULL (assert).
 * @return Ponteiro para o vértice associado ao id, ou NULL se nenhum
 *         vértice com esse id estiver no grafo.
 */
Vertice* buscarVertice(const Grafo *g, const char *id);

/**
 * @brief Verifica se existe um vértice com o id informado.
 * @param g Ponteiro para o grafo. Não deve ser NULL (assert).
 * @param id Id buscado. Não deve ser NULL (assert).
 * @return GRAFO_OK se existe vértice com esse id, GRAFO_ERRO caso
 *         contrário.
 */
int grafoContemVertice(const Grafo *g, const char *id);

/**
 * @brief Retorna o número de vértices armazenados no grafo.
 * @param g Ponteiro para o grafo. Não deve ser NULL (assert).
 * @return Número de vértices atualmente armazenados.
 */
int grafoNumVertices(const Grafo *g);

/**
 * @brief Insere uma aresta direcionada (idOrigem -> idDestino) no rafo, na lista de adjacência de idOrigem.
 * @param g Ponteiro para o grafo. Não deve ser NULL (assert).
 * @param idOrigem Id do vértice de origem. Deve existir no grafo (assert -> nota de pré-condições). Não deve ser NULL (assert).
 * @param idDestino Id do vértice de destino. Deve existir no grafo (assert -> nota de pré-condições). Não deve ser NULL (assert).
 * @param ldir CEP da quadra do lado direito do segmento, ou "-" se não houver. Não deve ser NULL (assert) 
 * @param lesq CEP da quadra do lado esquerdo do segmento, ou "-" se não houver. Não deve ser NULL (assert) 
 * @param cmp Comprimento do segmento de rua, em metros. Deve ser >= 0;
 *        Caso contrário a inserção falha.
 * @param vm Velocidade média do segmento, em m/s. Deve ser >= 0; 
 *        Caso contrário a inserção falha.
 * @param nome Nome da rua a qual o segmento pertence. Não deve ser NULL (assert).
 * @return GRAFO_OK se a aresta foi criada e inserida; GRAFO_ERRO se
 *         cmp ou vm forem negativos (aresta não inserida).
 * @details Múltiplas arestas entre o mesmo par (idOrigem, idDestino)
 *          são permitidas - este módulo não verifica nem impede arestas
 *          paralelas, já que o .via pode legitimamente descrevê-las.
 */
int grafoInserirAresta(Grafo *g, const char *idOrigem, const char *idDestino,
                        const char *ldir, const char *lesq,
                        double cmp, double vm, const char *nome);

// Getters de Aresta.

/**
 * @brief Retorna o vértice de destino de uma aresta.
 * @param a Ponteiro para a aresta. Não deve ser NULL (assert).
 * @return Ponteiro para o vértice de destino.
 * @details Ponteiro EMPRESTADO - o Grafo (via hash interna) é o único dono do Vertice retornado.
 */
Vertice* getArestaDestino(const Aresta *a);

/**
 * @brief Retorna o nome da rua de uma aresta.
 * @param a Ponteiro para a aresta. Não deve ser NULL (assert).
 * @return String com o nome da rua.
 */
const char* getArestaNome(const Aresta *a);

/**
 * @brief Retorna o CEP da quadra do lado direito da aresta.
 * @param a Ponteiro para a aresta. Não deve ser NULL (assert).
 * @return String com o CEP, ou "-" se não houver quadra desse lado.
 */
const char* getArestaLdir(const Aresta *a);

/**
 * @brief Retorna o CEP da quadra do lado esquerdo da aresta.
 * @param a Ponteiro para a aresta. Não deve ser NULL (assert).
 * @return String com o CEP, ou "-" se não houver quadra desse lado.
 */
const char* getArestaLesq(const Aresta *a);

/**
 * @brief Retorna o comprimento do segmento de rua da aresta.
 * @param a Ponteiro para a aresta. Não deve ser NULL (assert).
 * @return Double com o comprimento, em metros.
 */
double getArestaCmp(const Aresta *a);

/**
 * @brief Retorna a velocidade média do segmento de rua da aresta.
 * @param a Ponteiro para a aresta. Não deve ser NULL (assert).
 * @return Double com a velocidade média, em m/s.
 * @details Esta versão do módulo não expõe um setter de vm - apenas
 *          leitura. A mutação de vm (exigida pelos comandos mvm/exp do
 *          .qry) será adicionada quando esses comandos forem
 *          implementados.
 */
double getArestaVm(const Aresta *a);

/**
 * @brief Atualiza a velocidade média de uma aresta.
 * @param a Ponteiro para a aresta. Não deve ser NULL (assert).
 * @param novoVm Nova velocidade média, em m/s. Deve ser >= 0; caso contrário a atualização é rejeitada.
 * @return GRAFO_OK se a atualização foi aplicada; GRAFO_ERRO se novoVm for negativo.
 * @details Mesma validação usada em grafoInserirAresta para o vm
 *          inicial - vm negativo não representa uma velocidade física
 *          válida. Usado pelos comandos mvm e exp do .qry, que mutam vm
 *          de arestas já existentes.
 */
int setArestaVm(Aresta *a, double novoVm);

/**
 * @brief Assinatura de função usada para visitar cada vértice durante uma varredura do grafo (ver percorrerVertices).
 * @param v Ponteiro para o vértice visitado.
 * @param contexto Ponteiro opaco fornecido por quem chamou percorrerVertices, repassado sem modificação a cada visita.
 */
typedef void (*VisitanteVertice)(Vertice *v, void *contexto);

/**
 * @brief Percorre todos os vértices do grafo, chamando o visitante
 *        fornecido para cada um.
 * @param g Ponteiro para o grafo. Não deve ser NULL (assert).
 * @param visitante Função chamada para cada vértice. Não deve ser NULL
 *        (assert).
 * @param contexto Ponteiro opaco repassado a cada chamada do
 *        visitante. Pode ser NULL se o visitante não precisar de
 *        estado externo.
 * @details A ordem de visita não é especificada e não deve ser
 *          assumida pelo chamador. Usado, por exemplo, por algoritmos
 *          que precisam considerar todos os vértices (componentes
 *          conexos, MST).
 */
void percorrerVertices(const Grafo *g, VisitanteVertice visitante, void *contexto);

/**
 * @brief Assinatura de função usada para visitar cada aresta de saída
 *        de um vértice durante uma varredura (ver
 *        percorrerArestasSaindo).
 * @param a Ponteiro para a aresta visitada.
 * @param contexto Ponteiro opaco fornecido por quem chamou
 *        percorrerArestasSaindo, repassado sem modificação a cada
 *        visita.
 */
typedef void (*VisitanteAresta)(Aresta *a, void *contexto);

/**
 * @brief Percorre a lista de adjacência (arestas de saída) de um
 *        vértice, chamando o visitante fornecido para cada aresta.
 * @param g Ponteiro para o grafo. Não deve ser NULL (assert).
 * @param idOrigem Id do vértice cuja lista de adjacência será
 *        percorrida. Não deve ser NULL (assert).
 * @param visitante Função chamada para cada aresta de saída de
 *        idOrigem. Não deve ser NULL (assert).
 * @param contexto Ponteiro opaco repassado a cada chamada do
 *        visitante. Pode ser NULL se o visitante não precisar de
 *        estado externo.
 * @details Se idOrigem não existir no grafo, o visitante simplesmente
 *          não é chamado nenhuma vez (equivalente a um vértice sem
 *          arestas de saída) - este caso não é tratado como erro, pois
 *          "vértice inexistente" e "vértice sem arestas" produzem o
 *          mesmo resultado observável para quem percorre. Usado, por
 *          exemplo, pelo algoritmo de Dijkstra para explorar vizinhos.
 */
void percorrerArestasSaindo(const Grafo *g, const char *idOrigem,
                             VisitanteAresta visitante, void *contexto);

/**
 * @brief Assinatura de função usada para visitar cada aresta durante
 *        uma varredura de TODAS as arestas do grafo (ver
 *        percorrerTodasArestas).
 * @param idOrigem Id do vértice de origem da aresta visitada. Diferente
 *        de VisitanteAresta (usado em percorrerArestasSaindo), aqui a
 *        origem precisa vir explícita: quem chama percorrerTodasArestas
 *        não escolheu de qual vértice partir, então não tem como
 *        capturar a origem no próprio contexto - ela só é conhecida
 *        durante a varredura interna.
 * @param a Ponteiro para a aresta visitada.
 * @param contexto Ponteiro opaco fornecido por quem chamou
 *        percorrerTodasArestas, repassado sem modificação a cada visita.
 */
typedef void (*VisitanteArestaCompleta)(const char *idOrigem, Aresta *a, void *contexto);
 
/**
 * @brief Percorre TODAS as arestas do grafo (de todos os vértices, não
 *        só de um), chamando o visitante fornecido para cada uma.
 * @param g Ponteiro para o grafo. Não deve ser NULL (assert).
 * @param visitante Função chamada para cada aresta do grafo. Não deve
 *        ser NULL (assert).
 * @param contexto Ponteiro opaco repassado a cada chamada do
 *        visitante. Pode ser NULL se o visitante não precisar de
 *        estado externo.
 * @details Implementado compondo percorrerVertices() com
 *          percorrerArestasSaindo() para cada vértice visitado. 
 */
void percorrerTodasArestas(const Grafo *g, VisitanteArestaCompleta visitante, void *contexto);

#endif