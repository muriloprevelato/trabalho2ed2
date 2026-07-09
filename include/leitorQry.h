#ifndef LEITOR_QRY_H
#define LEITOR_QRY_H

#include <stdio.h>
#include "cidade.h"
#include "grafo.h"
#include "registradores.h"
#include "svg.h"

/*
Interface do módulo Qry.

O integrador: o único módulo que conhece todos os outros. Tem duas
responsabilidades deliberadamente agrupadas no mesmo módulo: desenhar o mapa base
(quadras + sistema viário) num ArqSvg, e processar um arquivo .qry
inteiro, aplicando cada comando sobre Cidade/Grafo/Registradores e
anotando o resultado num SVG combinado e num arquivo de texto.

Por que desenho de mapa mora aqui, não em main.c nem em módulo
próprio: main.c precisa desenhar o mapa base no SVG do .geo; o
processamento do .qry precisa desenhar esse MESMO mapa base como
primeira camada do SVG combinado, antes de empilhar as marcações de
cada comando por cima. Sem centralizar isso em algum lugar, a lógica de
"como desenhar uma Quadra/Vertice/Aresta num ArqSvg" duplicaria entre
main.c e qry.c.

Saída do .qry - um svg combinado por arquivo .qry: 
abre-se um único ArqSvg, desenha-se o mapa base
uma vez, e cada comando com saída gráfica anota nesse mesmo SVG, camada sobre camada.

Robustez de leitura: mesma disciplina de leitorGeo/leitorVia. Linha em
branco ignorada silenciosamente; comando desconhecido ou linha
malformada (campos insuficientes para o comando identificado) reportado
no stderr, linha descartada, leitura continua. O retorno de
processarArquivoQry reflete SÓ a abertura do arquivo .qry - problemas
pontuais de linha não afetam o retorno.

Formato do .txt de saída > respeitando a descrição geral do projeto: para
cada comando processado, uma linha "[*] " seguida do texto da consulta,
e o resultado nas linhas seguintes.

** Obs: Geometria do comando mvm - "aresta dentro da região (x,y,w,h)" - decisão
adotada é AMBOS OS EXTREMOS DENTRO -> a aresta só é afetada se tanto o
vértice de origem quanto o de destino caem em [x, x+w] × [y, y+h].

Pré-condições:
  - Ponteiros de Cidade/Registradores/ArqSvg passados nunca devem ser
    NULL. Grafo PODE ser NULL (ver nota acima - é dado válido, não
    violação de pré-condição).
  - caminho (do .qry) nunca deve ser NULL.
  - Violações de pré-condição abortam o programa (assert). Falha ao
    abrir o arquivo .qry é erro de AMBIENTE - sinalizada via retorno
    QRY_ERRO, nunca via assert.
*/

// Códigos de retorno padrão para facilitar os testes.
#define QRY_OK 1
#define QRY_ERRO 0

/**
 * @brief Calcula as dimensões do canvas necessárias para desenhar o mapa base (quadras + grafo) sem cortar nada.
 * @param cidade Cidade cujas quadras serão consideradas. Não deve ser NULL (assert).
 * @param grafo Grafo cujos vértices serão considerados. Pode ser NULL (nesse caso, só as quadras contribuem para as dimensões).
 * @param larguraSaida Ponteiro para double onde a largura do canvas será escrita. Não deve ser NULL (assert).
 * @param alturaSaida Ponteiro para double onde a altura do canvas será escrita. Não deve ser NULL (assert).
 * @details Usa a mesma convenção de canvas já adotada no projeto:
 *          largura/altura = maior coordenada absoluta entre todas as
 *          quadras e vértices (canvas de 0 até o extremo, não uma
 *          extensão relativa) - evita o bug de conteúdo cortado quando
 *          o mapa tem offset em relação à origem. Cidade/grafo vazios
 *          (sem nenhuma quadra nem vértice) resultam num canvas mínimo
 *          de 100x100, para não travar a criação do SVG.
 */
void calcularDimensoesMapa(const Cidade *cidade, const Grafo *grafo,
                            double *larguraSaida, double *alturaSaida);

/**
 * @brief Desenha o mapa base (todas as quadras e, se houver, todo o sistema viário) num ArqSvg já aberto.
 * @param svg Arquivo SVG já aberto para escrita. Não deve ser NULL (assert). Esta função não abre nem fecha o arquivo.
 * @param cidade Cidade cujas quadras serão desenhadas. Não deve ser NULL (assert).
 * @param grafo Grafo cujo sistema viário (vértices como círculos, arestas como linhas) será desenhado. Pode ser NULL.
 * @details Usada tanto por main.c (para o SVG produzido após o .geo)
 *          quanto internamente por processarArquivoQry (como primeira
 *          camada do SVG combinado, antes das marcações de cada
 *          comando). Ver nota no topo deste arquivo sobre por que essa
 *          função mora aqui em vez de em main.c ou em módulo próprio.
 */
void desenharMapaBase(ArqSvg *svg, const Cidade *cidade, const Grafo *grafo);

/**
 * @brief Lê um arquivo .qry e processa cada comando em ordem,
 *        aplicando efeitos sobre cidade/grafo/registradores, anotando
 *        o SVG combinado e escrevendo no arquivo de texto de saída.
 * @param caminho Caminho do arquivo .qry a ser lido. Não deve ser NULL (assert).
 * @param cidade Cidade sobre a qual os endereços são resolvidos. Não deve ser NULL (assert). Não é mutada por nenhum comando.
 * @param grafo Grafo sobre o qual mvm/regs/exp/p? operam. Pode ser NULL 
 * @param registradores Banco de registradores usado por @o? (escreve) e p? (lê). Não deve ser NULL (assert).
 * @param svgCombinado Arquivo SVG já aberto para escrita (dimensionado
 *        via calcularDimensoesMapa e aberto via abreEscritaSvg pelo
 *        chamador). Não deve ser NULL (assert). Esta função desenha o
 *        mapa base nele e depois anota cada comando gráfico, mas NUNCA
 *        chama fechaSvg - o chamador é responsável por fechar depois
 *        que esta função retornar.
 * @param txtSaida Arquivo de texto já aberto para escrita (via fopen
 *        pelo chamador). Não deve ser NULL (assert). Esta função
 *        escreve nele, mas NUNCA chama fclose - o chamador é
 *        responsável por fechar depois.
 * @return QRY_OK se o arquivo .qry foi aberto e processado (mesmo que
 *         comandos individuais tenham sido reportados como
 *         problemáticos); QRY_ERRO se o arquivo .qry não pôde ser
 *         aberto.
 * @details O retorno reflete apenas a abertura do arquivo .qry, não a
 *          qualidade de cada linha nem o sucesso de cada comando - ver
 *          nota de robustez de leitura no topo deste arquivo.
 */
int processarArquivoQry(const char *caminho, const Cidade *cidade, Grafo *grafo,  Registradores *registradores,ArqSvg *svgCombinado, FILE *txtSaida);

#endif