#ifndef LEITOR_VIA_H
#define LEITOR_VIA_H

#include "grafo.h"

/*
Interface do módulo LeitorVia.

Lê um arquivo .via (descrição do sistema viário, comandos v/e) e popula
um Grafo já existente com os vértices e arestas encontrados.

Formato do .via:
  nv                       Primeira linha do arquivo: um inteiro com o
                           número de vértices. Este módulo consome essa
                           linha mas não a usa para nada (não dimensiona
                           nem valida contagem) - confia inteiramente
                           nas linhas v que seguem. Se a linha 1 não for
                           um inteiro válido, isso é reportado no stderr,
                           mas a leitura prossegue normalmente a partir
                           da linha 2.
  v id x y                 Cria o vértice id na coordenada (x, y).
  e i j ldir lesq cmp vm nome
                           Cria a aresta direcionada i -> j, com CEP das
                           quadras lateral (ldir/lesq, ou "-" se
                           ausente), comprimento (cmp), velocidade média
                           (vm) e nome da rua.

Convenção do hífen: ldir/lesq podem chegar como a string "-" (ausência
de quadra naquele lado). Este módulo não trata esse caso de forma
especial - repassa a string lida diretamente para grafoInserirAresta,
que já espera "-" como valor literal válido (ausência é modelada como
dado, nunca como NULL). As duas pontas já combinam sem tradução.

Pré-condições (valendo para todo o módulo):
  - O caminho do arquivo e o ponteiro de Grafo nunca devem ser NULL.
  - Violações de pré-condição abortam o programa (assert). Falha ao
    abrir o arquivo (caminho inválido, sem permissão) é erro de
    ambiente, não de lógica - sinalizada via retorno VIA_ERRO, nunca
    via assert.
*/

// Códigos de retorno padrão para facilitar os testes.
#define VIA_OK 1
#define VIA_ERRO 0

/**
 * @brief Lê um arquivo .via e insere os vértices e arestas descritos no grafo informado.
 * @param caminho Caminho do arquivo .via a ser lido. Não deve ser NULL (assert).
 * @param grafo Grafo já criado, a ser populado com vértices e arestas lidos. Não deve ser NULL (assert).
 * @return VIA_OK se o arquivo foi aberto e processado (mesmo que linhas
 *         individuais tenham sido reportadas como problemáticas no
 *         stderr); VIA_ERRO se o arquivo não pôde ser aberto.
 * @details O retorno reflete apenas a abertura do arquivo, não a
 *          qualidade de cada linha - problemas pontuais (linha
 *          malformada, comando desconhecido, vértice duplicado, aresta
 *          para vértice inexistente, cmp/vm inválidos) são reportados
 *          no stderr e não afetam o valor de retorno.
 */
int lerArquivoVia(const char *caminho, Grafo *grafo);

#endif