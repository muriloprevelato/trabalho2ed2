#ifndef SVG_H
#define SVG_H

/*
Interface do módulo SVG.

Este módulo sósabe escrever primitivas geométricas (retângulo, texto, linha) num
arquivo .svg válido. Quem desenha uma Quadra é um trecho de código que
lê os atributos da Quadra e chama svgRetangulo(...)/svgTexto(...) com
os valores correspondentes.

Mantê-lo genérico permite reusá-lo depois para o mapa viário (ruas,
vértices) e para os percursos animados (<animateMotion/>), sem
acoplar este módulo a um domínio específico.

Pré-condições (valendo para todo o módulo):
  - Ponteiros de ArqSvg passados às funções (exceto abreEscritaSvg, que
    os cria) nunca devem ser NULL.
  - Strings passadas (cores, texto, caminho do arquivo) nunca devem
    ser NULL.
  - Violações de pré-condição abortam o programa (assert), EXCETO na
    abertura do arquivo: falha ao abrir o caminho informado (ex:
    diretório sem permissão, caminho inválido) é erro de ambiente, não
    de lógica de programa — sinalizada por retorno NULL, nunca por
    assert. Ver manual de testes, seção 7.5.
*/

// Tipo opaco para representar um arquivo SVG aberto para escrita.
typedef struct ArqSvg ArqSvg;

/**
 * @brief Abre um arquivo .svg para escrita e grava a tag <svg> de abertura, com as dimensões informadas.
 * @param caminho Caminho do arquivo .svg a ser criado/sobrescrito. Não deve ser NULL (assert).
 * @param largura Largura do viewBox, em unidades SVG. Deve ser >= 0.
 * @param altura Altura do viewBox, em unidades SVG. Deve ser >= 0.
 * @return Ponteiro para o arquivo aberto, ou NULL caso não seja
 *         possível criar o arquivo (diretório sem permissão, caminho
 *         inválido, largura/altura negativas, falha de alocação).
 * @details Erro de abertura é condição de ambiente, não de lógica — por isso sinalizada via NULL, nunca via assert.
 */
ArqSvg* abreEscritaSvg(const char *caminho, double largura, double altura);

/**
 * @brief Grava a tag </svg> de fechamento e fecha o arquivo associado, liberando os recursos do ArqSvg.
 * @param f Ponteiro para o arquivo aberto. Não deve ser NULL (assert).
 */
void fechaSvg(ArqSvg *f);

/**
 * @brief Escreve um retângulo no arquivo SVG.
 * @param f Ponteiro para o arquivo aberto. Não deve ser NULL (assert).
 * @param x Coordenada x do canto superior-esquerdo do retângulo (SVG).
 * @param y Coordenada y do canto superior-esquerdo do retângulo (SVG).
 * @param w Largura do retângulo. Deve ser >= 0 (assert).
 * @param h Altura do retângulo. Deve ser >= 0 (assert).
 * @param cfill Cor de preenchimento (nome SVG ou código hexadecimal). Não deve ser NULL (assert).
 * @param cstrk Cor da borda (nome SVG ou código hexadecimal). Não deve ser NULL (assert).
 * @param sw Espessura da borda. Deve ser >= 0 (assert).
 * @details Este módulo não interpreta x/y como âncora SUDESTE nem
 *          qualquer outra convenção de domínio — recebe diretamente as
 *          coordenadas SVG (canto superior-esquerdo, eixo y crescendo
 *          para baixo). A conversão de convenção (ex: âncora SE de uma
 *          Quadra) é responsabilidade de quem chama esta função. 
 *          ** No caso, só vai repassar aquilo que já foi delimitado no getQuadraX e assim por diante...
 */
void svgRetangulo(ArqSvg *f, double x, double y, double w, double h,
                   const char *cfill, const char *cstrk, double sw);

/**
 * @brief Escreve um texto no arquivo SVG.
 * @param f Ponteiro para o arquivo aberto. Não deve ser NULL (assert).
 * @param x Coordenada x do ponto de ancoragem do texto (SVG).
 * @param y Coordenada y do ponto de ancoragem do texto (SVG).
 * @param texto Conteúdo textual a ser escrito. Não deve ser NULL (assert).
 * @param cor Cor do texto (nome SVG ou código hexadecimal). Não deve ser NULL (assert).
 * @details Tamanho de fonte não é configurável nesta versão; usa um
 *          tamanho padrão definido pela implementação.
 *          Os caracteres especiais de XML presentes em texto (&, <, >)
 *          são escapados automaticamente antes da escrita.
 */
void svgTexto(ArqSvg *f, double x, double y, const char *texto, const char *cor);

/**
 * @brief Escreve um segmento de linha no arquivo SVG.
 * @param f Ponteiro para o arquivo aberto. Não deve ser NULL (assert).
 * @param x1 Coordenada x do ponto inicial do segmento.
 * @param y1 Coordenada y do ponto inicial do segmento.
 * @param x2 Coordenada x do ponto final do segmento.
 * @param y2 Coordenada y do ponto final do segmento.
 * @param cor Cor do traço (nome SVG ou código hexadecimal). Não deve ser NULL (assert).
 * @param sw Espessura do traço. Deve ser >= 0 (assert).
 */
void svgLinha(ArqSvg *f, double x1, double y1, double x2, double y2,
              const char *cor, double sw);

#endif