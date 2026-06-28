#ifndef LEITOR_GEO_H
#define LEITOR_GEO_H

#include "cidade.h"

/*
Interface do módulo LeitorGeo.

Lê um arquivo .geo (descrição da cidade, comandos q/cq) e popula uma
Cidade já existente com as quadras encontradas.

Comandos reconhecidos no .geo:
  q cep x y w h           Insere uma quadra com a cor/espessura
                          correntes (definidas pelo cq mais recente,
                          ou pelo default antes do primeiro cq).
  cq sw cfill cstrk       Atualiza a cor/espessura correntes a partir
                          deste ponto do arquivo. Linhas q anteriores
                          a este comando já foram criadas com a cor
                          corrente de então; não são retroativamente
                          alteradas. Note a ordem dos campos: espessura
                          primeiro, depois as duas cores.

Antes do primeiro cq no arquivo, a cor/espessura corrente default é:
  cfill = "white", cstrk = "black", sw = 1.0

Pré-condições (valendo para todo o módulo):
  - O caminho do arquivo e o ponteiro de Cidade nunca devem ser NULL.
  - Violações de pré-condição abortam o programa (assert). Falha ao
    abrir o arquivo (caminho inválido, sem permissão) é erro de
    AMBIENTE, não de lógica — sinalizada via retorno GEO_ERRO, nunca
    via assert.
*/

// Códigos de retorno padrão para facilitar os testes.
#define GEO_OK 1
#define GEO_ERRO 0

/**
 * @brief Lê um arquivo .geo e insere as quadras descritas na cidade informada.
 * @param caminho Caminho do arquivo .geo a ser lido. Não deve ser NULL (assert).
 * @param cidade Cidade já criada, a ser populada com as quadras lidas.
 *        Não deve ser NULL (assert). O parser não cria nem destrói esta cidade — apenas insere quadras nela.
 * @return GEO_OK se o arquivo foi aberto e processado (mesmo que linhas individuais tenham sido reportadas como problemáticas no stderr); GEO_ERRO se o arquivo não pôde ser aberto.
 * @details O retorno reflete apenas a abertura do arquivo. 
 *          Problemas pontuais são reportados no stderr e não afetam o valor de retorno.
 */
int lerArquivoGeo(const char *caminho, Cidade *cidade);

#endif