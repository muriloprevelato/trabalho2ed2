#ifndef QUADRA_H
#define QUADRA_H

/*
Interface do módulo da Quadra.

Basicamente é um retângulo que será identificado por um CEP (alfanumérico).
Tem 4 faces (pontos cardeais) e a âncora é o canto SUDESTE.
Obs: tomar cuidado com a notação invertida (eixo x fica por cima).
*/

// Constantes.
#define QUADRA_CEP_MAX 20 // Limite máximo do CEP
#define QUADRA_COR_MAX 32 // Limite máximo para cor.

// Códigos de retorno padrão para facilitar os testes.
#define QUADRA_OK 1
#define QUADRA_ERRO 0

// Etiqueta para faces da quadra.
typedef enum{
    FACE_N = 'N',
    FACE_S = 'S',
    FACE_L = 'L',
    FACE_O = 'O',
} FaceQuadra;

// Tipo opaco para representar a quadra.
typedef struct Quadra Quadra;

/**
 * @brief Função que cria uma quadra com seus respectivos atributos.
 * @param cep CEP alfanumérico (identificador).
 * @param x Coordenada x do âncora.
 * @param y Coordenada y do âncora.
 * @param w Largura do retângulo (quadra).
 * @param h Altura do retângulo (quadra).
 * @param sw Espessura da borda.
 * @param cfill Cor de preenchimento.
 * @param cstrk Cor da borda.
 * @return Retorna um ponteiro para a quadra criada.
 * @details NULL em caso de erro.
 */
Quadra* criarQuadra(const char* cep, double x, double y, double w, double h, double sw, const char* cfill, const char* cstrk);

/**
 * @brief Função que libera a memória associada a uma quadra.
 * @param Quadra Ponteiro para a quadra analisada. 
 */
void destruirQuadra(Quadra *q);

// Getters.

/**
 * @brief Função que retorna o CEP de determinada quadra.
 * @param Quadra Ponteiro para a quadra analisada.
 * @return String que representa o CEP (alfanumérico).
 */
const char* getQuadraCep(const Quadra *q);

/**
 * @brief Função que retorna a coordenada X do âncora (canto SUDESTE do retângulo) da quadra
 * @param Quadra Ponteiro para a quadra analisada. 
 * @return Double que representa a coordenada X.
 */
double getQuadraX(const Quadra *q);

/**
 * @brief Função que retorna a coordenada Y do âncora (canto SUDESTE do retângulo) da quadra
 * @param Quadra Ponteiro para a quadra analisada. 
 * @return Double que representa a coordenada Y.
 */
double getQuadraY(const Quadra *q);

/**
 * @brief Função que retorna a largura da quadra
 * @param Quadra Ponteiro para a quadra analisada. 
 * @return Double que representa a largura.
 */
double getQuadraW(const Quadra *q);

/**
 * @brief Função que retorna a altura da quadra
 * @param Quadra Ponteiro para a quadra analisada. 
 * @return Double que representa a altura.
 */
double getQuadraH(const Quadra *q);

/**
 * @brief Função que retorna a espessura da borda da quadra
 * @param Quadra Ponteiro para a quadra analisada. 
 * @return Double que representa a espessura.
 */
double getQuadraSw(const Quadra *q);

/**
 * @brief Função que retorna a cor de preenchimento da quadra.
 * @param Quadra Ponteiro para a quadra analisada.
 * @return String que representa a cor de preenchimento.
 */
const char* getQuadraCFill(const Quadra *q);

/**
 * @brief Função que retorna a cor de borda da quadra.
 * @param Quadra Ponteiro para a quadra analisada.
 * @return String que representa a cor de borda.
 */
const char* getQuadraCStrk(const Quadra *q);

// Fim getters

/**
 * @brief Confirma (ou não) se o caracter informado corresponde a uma face existente.
 * @param face Caracter que indica face.
 * @return QUADRA_OK -> 1 (válida) / Quadra_ERRO -> 0 (não válida).
 */
int faceValida(char face);

/**
 * @brief Converte um caracter de face para a etiqueta
 * @param face Caracter que indica a face
 * @return A etiqueta correspondente. 
 */
FaceQuadra charParaFaceQuadra(char face);

/**
 * @brief Calcula a coordenada (x, y) de um endereço.
 * @param Quadra Ponteiro para a quadra analisada.
 * @param face A face da quadra.
 * @param num Número do endereço.
 * @param outX Ponteiro para double onde será armazenado o x resultante.
 * @param outY Ponteiro para double onde será armazenado o Y resultante.
 * @details O cálculo baseia-se nas projeções a partir da âncora (superior-esquerda)
 */
void obterCoordenadasEndereco(const Quadra *q, FaceQuadra face, double num, double *outX, double *outY);

#endif