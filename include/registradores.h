#ifndef REGISTRADORES_H
#define REGISTRADORES_H

/*
Interface do módulo Registradores.

Guarda o banco de registradores R0..R10 que o comando @o? do .qry
preenche (com a posição geográfica resolvida de um endereço) e que o
comando p? consome (como origem/destino de um cálculo de caminho
mínimo). Cada registrador guarda uma coordenada (x, y) e o texto do
endereço original (cep/face/num), usado para reportar no .txt de saída.

Identificação por índice inteiro (0..REGISTRADOR_MAX-1)

setRegistrador pode ser chamado múltiplas vezes sobre o mesmo índice,
sobrescrevendo o valor anterior sem erro - diferente de inserir um
vértice ou uma quadra (que são entradas únicas por natureza), um
registrador é uma variável mutável.

Pré-condições:
  - Ponteiros de Registradores passados às funções nunca devem ser NULL.
  - indice deve estar em [0, REGISTRADOR_MAX - 1] em toda função que o
    recebe (assert). A validação desse intervalo é responsabilidade de
    quem faz o parsing do .qry
  - Consultar x/y/texto de um registrador que nunca foi preenchido é
    erro de uso do chamador (assert) - quem chama deve checar
    registradorPreenchido() antes. Um registrador não preenchido NÃO é
    um erro fatal em si (é um estado normal antes do primeiro @o? que o
    usa); é a tentativa de LER um registrador vazio que é a violação de
    contrato.
  - texto nunca deve ser NULL em setRegistrador (assert).
  - Violações de pré-condição abortam o programa (assert).
*/

// Constantes.
#define REGISTRADOR_MAX 11 // R0..R10, conforme a descrição.
#define REGISTRADOR_TEXTO_MAX 64  // Limite do texto do endereço original.

// Códigos de retorno padrão para facilitar os testes.
#define REGISTRADOR_OK 1
#define REGISTRADOR_ERRO 0

// Tipo opaco para representar o banco de registradores.
typedef struct Registradores Registradores;

/**
 * @brief Cria um banco de registradores, todos inicialmente vazios (não preenchidos).
 * @return Ponteiro para o banco criado, ou NULL em caso de falha de alocação.
 */
Registradores* criarRegistradores(void);

/**
 * @brief Libera a memória associada a um banco de registradores.
 * @param r Ponteiro para o banco. Não deve ser NULL (assert).
 */
void destruirRegistradores(Registradores *r);

/**
 * @brief Preenche (ou sobrescreve) um registrador com uma posição
 *        geográfica e o texto do endereço que a originou.
 * @param r Ponteiro para o banco. Não deve ser NULL (assert).
 * @param indice Índice do registrador, em [0, REGISTRADOR_MAX - 1]
 *        (assert). Corresponde a Ri no .qry (ex: índice 6 = R6).
 * @param x Coordenada x resolvida do endereço.
 * @param y Coordenada y resolvida do endereço.
 * @param texto Texto do endereço original (ex: "cep15/S/45"), usado para reportar no .txt de saída. 
 *        Não deve ser NULL (assert).
 *        Uma cópia interna do texto é feita por este módulo.
 * @details Chamar novamente sobre um índice já preenchido sobrescreve
 *          o valor anterior sem erro
 */
void setRegistrador(Registradores *r, int indice, double x, double y, const char *texto);

/**
 * @brief Verifica se um registrador já foi preenchido por algum
 *        setRegistrador anterior.
 * @param r Ponteiro para o banco. Não deve ser NULL (assert).
 * @param indice Índice do registrador, em [0, REGISTRADOR_MAX - 1]
 *        (assert).
 * @return REGISTRADOR_OK se o registrador foi preenchido, REGISTRADOR_ERRO caso contrário.
 * @details Esta é a função que quem processa o .qry deve chamar antes
 *          de ler x/y/texto de um registrador - um p? referenciando um
 *          Ri nunca preenchido é erro de DADO (o .qry pode ser mal
 *          formado ou referenciar um registrador fora de ordem), a ser
 *          reportado e tratado por quem chama, não um assert.
 */
int registradorPreenchido(const Registradores *r, int indice);

/**
 * @brief Retorna a coordenada x de um registrador preenchido.
 * @param r Ponteiro para o banco. Não deve ser NULL (assert).
 * @param indice Índice do registrador, em [0, REGISTRADOR_MAX - 1], e já preenchido 
 * @return Double com a coordenada x.
 */
double getRegistradorX(const Registradores *r, int indice);

/**
 * @brief Retorna a coordenada y de um registrador preenchido.
 * @param r Ponteiro para o banco. Não deve ser NULL (assert).
 * @param indice Índice do registrador, em [0, REGISTRADOR_MAX - 1], e já preenchido 
 * @return Double com a coordenada y.
 */
double getRegistradorY(const Registradores *r, int indice);

/**
 * @brief Retorna o texto do endereço original de um registrador
 *        preenchido.
 * @param r Ponteiro para o banco. Não deve ser NULL (assert).
 * @param indice Índice do registrador, em [0, REGISTRADOR_MAX - 1], e já preenchido 
 * @return String com o texto do endereço original (ex: "cep15/S/45").
 */
const char* getRegistradorTexto(const Registradores *r, int indice);

#endif