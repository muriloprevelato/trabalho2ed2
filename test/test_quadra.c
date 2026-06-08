#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "quadra.h"
#include "unity.h"

// Sem arquivos em disco então setUp e tearDown sem estado global neste módulo.
void setUp(void){}
void tearDown(void){}

/* Só para não ficar repetindo parâmetros em cada teste!
Quadra padrão: âncora SE=(10,20), w=50, h=30
Bússola (notação invertida — eixo x na horizontal, y cresce para baixo)
L (Leste) fica à esquerda (menor x = âncora.x).
O (Oeste) fica à direita  (maior x = âncora.x + w).
*/
static Quadra* quadra_padrao(void){
    return criarQuadra("cep15", 10.0, 20.0, 50.0, 30.0, 2.0, "orange", "black");
}

// Inicialização e destruição 

void test_criarQuadra_DeveRetornarPonteiroValido(void){
    Quadra *q = quadra_padrao();
    TEST_ASSERT_NOT_NULL(q);
    destruirQuadra(q);
}

// Caminho infeliz: parâmetros obrigatórios nulos devem retornar NULL.
// Contrato fixado aqui: cep/cfill/cstrk NULL -> erro de criação.
void test_criar_ComCEPNulo_DeveRetornarNull(void){
    Quadra *q = criarQuadra(NULL, 10.0, 20.0, 50.0, 30.0, 2.0, "orange", "black");
    TEST_ASSERT_NULL(q);
}

void test_criar_ComCFillNulo_DeveRetornarNull(void){
    Quadra *q = criarQuadra("cep15", 10.0, 20.0, 50.0, 30.0, 2.0, NULL, "black");
    TEST_ASSERT_NULL(q);
}

void test_criar_ComCStrkNulo_DeveRetornarNull(void){
    Quadra *q = criarQuadra("cep15", 10.0, 20.0, 50.0, 30.0, 2.0, "orange", NULL);
    TEST_ASSERT_NULL(q);
}

void test_criar_ComDimensoesNegativas_DeveRetornarNull(void){
    Quadra *q = criarQuadra("cep01", 10.0, 20.0, -5.0, 30.0, 2.0, "orange", "black");
    TEST_ASSERT_NULL(q);
}

void test_destruirComParametroNull_naoQuebrar(void){
    destruirQuadra(NULL);
}

// Getters

void test_getters_RetornarValoresCorretos(void){
    Quadra *q = quadra_padrao();
    TEST_ASSERT_NOT_NULL(q);

    TEST_ASSERT_EQUAL_STRING("cep15",  getQuadraCep(q));
    TEST_ASSERT_EQUAL_STRING("orange", getQuadraCFill(q));
    TEST_ASSERT_EQUAL_STRING("black",  getQuadraCStrk(q));

    TEST_ASSERT_FLOAT_WITHIN(0.001, 10.0, getQuadraX(q));
    TEST_ASSERT_FLOAT_WITHIN(0.001, 20.0, getQuadraY(q));
    TEST_ASSERT_FLOAT_WITHIN(0.001, 50.0, getQuadraW(q));
    TEST_ASSERT_FLOAT_WITHIN(0.001, 30.0, getQuadraH(q));
    TEST_ASSERT_FLOAT_WITHIN(0.001,  2.0, getQuadraSw(q));

    destruirQuadra(q);
}

void test_getters_ParametrosNull_naoQuebrar(void){
    TEST_ASSERT_NULL(getQuadraCep(NULL));
    TEST_ASSERT_NULL(getQuadraCFill(NULL));
    TEST_ASSERT_NULL(getQuadraCStrk(NULL));

    TEST_ASSERT_FLOAT_WITHIN(0.001, 0.0, getQuadraX(NULL));
    TEST_ASSERT_FLOAT_WITHIN(0.001, 0.0, getQuadraY(NULL));
    TEST_ASSERT_FLOAT_WITHIN(0.001, 0.0, getQuadraW(NULL));
    TEST_ASSERT_FLOAT_WITHIN(0.001, 0.0, getQuadraH(NULL));
}

// faceValida

void test_faceValida_FacesCorretas(void){
    TEST_ASSERT_EQUAL_INT(QUADRA_OK, faceValida('N'));
    TEST_ASSERT_EQUAL_INT(QUADRA_OK, faceValida('S'));
    TEST_ASSERT_EQUAL_INT(QUADRA_OK, faceValida('L'));
    TEST_ASSERT_EQUAL_INT(QUADRA_OK, faceValida('O'));
}

void test_faceValida_FacesInvalidas(void){
    TEST_ASSERT_EQUAL_INT(QUADRA_ERRO, faceValida('X'));
    TEST_ASSERT_EQUAL_INT(QUADRA_ERRO, faceValida('n')); // minúscula não vale 
    TEST_ASSERT_EQUAL_INT(QUADRA_ERRO, faceValida('\0'));
}

// Etiquetas

void test_charParaFaceQuadra_ConversaoCorreta(void){
    TEST_ASSERT_EQUAL_INT(FACE_N, charParaFaceQuadra('N'));
    TEST_ASSERT_EQUAL_INT(FACE_S, charParaFaceQuadra('S'));
    TEST_ASSERT_EQUAL_INT(FACE_L, charParaFaceQuadra('L'));
    TEST_ASSERT_EQUAL_INT(FACE_O, charParaFaceQuadra('O'));
}

// Conversão para o endereço

// Face S: y fixo na âncora (menor y = 20). num varia ao longo de x.
void test_obterCoordenadas_FaceS_YFixoNaAncora(void){
    Quadra *q = quadra_padrao();
    TEST_ASSERT_NOT_NULL(q);

    double ox, oy;
    obterCoordenadasEndereco(q, FACE_S, 0.0,  &ox, &oy);
    TEST_ASSERT_FLOAT_WITHIN(0.001, 20.0, oy);

    obterCoordenadasEndereco(q, FACE_S, 25.0, &ox, &oy);
    TEST_ASSERT_FLOAT_WITHIN(0.001, 20.0, oy);

    destruirQuadra(q);
}

// Face N: y fixo em âncora.y + h (maior y = 50). num varia ao longo de x.
void test_obterCoordenadas_FaceN_YFixoNoTopoOposto(void){
    Quadra *q = quadra_padrao();
    TEST_ASSERT_NOT_NULL(q);

    double ox, oy;
    obterCoordenadasEndereco(q, FACE_N, 0.0,  &ox, &oy);
    TEST_ASSERT_FLOAT_WITHIN(0.001, 50.0, oy); // 20 + 30 

    obterCoordenadasEndereco(q, FACE_N, 25.0, &ox, &oy);
    TEST_ASSERT_FLOAT_WITHIN(0.001, 50.0, oy);

    destruirQuadra(q);
}

// Face L: x fixo na âncora (menor x = 10). L fica à esquerda na notação invertida.
void test_obterCoordenadas_FaceL_XFixoNaAncora(void){
    Quadra *q = quadra_padrao();
    TEST_ASSERT_NOT_NULL(q);

    double ox, oy;
    obterCoordenadasEndereco(q, FACE_L, 0.0,  &ox, &oy);
    TEST_ASSERT_FLOAT_WITHIN(0.001, 10.0, ox);

    obterCoordenadasEndereco(q, FACE_L, 15.0, &ox, &oy);
    TEST_ASSERT_FLOAT_WITHIN(0.001, 10.0, ox);

    destruirQuadra(q);
}

// Face O: x fixo em âncora.x + w (maior x = 60). O fica à direita na notação invertida.
void test_obterCoordenadas_FaceO_XFixoNoLadoOposto(void){
    Quadra *q = quadra_padrao();
    TEST_ASSERT_NOT_NULL(q);

    double ox, oy;
    obterCoordenadasEndereco(q, FACE_O, 0.0,  &ox, &oy);
    TEST_ASSERT_FLOAT_WITHIN(0.001, 60.0, ox); // 10 + 50 

    obterCoordenadasEndereco(q, FACE_O, 15.0, &ox, &oy);
    TEST_ASSERT_FLOAT_WITHIN(0.001, 60.0, ox);

    destruirQuadra(q);
}

// Todos os pontos gerados devem cair dentro (ou na borda) do retângulo.
// Usa nums dentro dos ranges válidos de cada face (S/N: 0..w=50, L/O: 0..h=30).
void test_obterCoordenadas_PontosSempreDentroDoRetangulo(void){
    Quadra *q = quadra_padrao();
    TEST_ASSERT_NOT_NULL(q);

    double xMin = getQuadraX(q); // 10 
    double yMin = getQuadraY(q); // 20 
    double xMax = xMin + getQuadraW(q); // 60 
    double yMax = yMin + getQuadraH(q); // 50 

    // S/N percorrem a largura (0..w); L/O percorrem a altura (0..h) 
    double nums_xface[] = {0.0, 10.0, 25.0, 50.0}; // ao longo de w 
    double nums_yface[] = {0.0,  5.0, 15.0, 30.0}; // ao longo de h 

    double ox, oy;

    for(int i = 0; i < 4; i++){
        obterCoordenadasEndereco(q, FACE_S, nums_xface[i], &ox, &oy);
        TEST_ASSERT(ox >= xMin - 0.001 && ox <= xMax + 0.001);
        TEST_ASSERT(oy >= yMin - 0.001 && oy <= yMax + 0.001);

        obterCoordenadasEndereco(q, FACE_N, nums_xface[i], &ox, &oy);
        TEST_ASSERT(ox >= xMin - 0.001 && ox <= xMax + 0.001);
        TEST_ASSERT(oy >= yMin - 0.001 && oy <= yMax + 0.001);

        obterCoordenadasEndereco(q, FACE_L, nums_yface[i], &ox, &oy);
        TEST_ASSERT(ox >= xMin - 0.001 && ox <= xMax + 0.001);
        TEST_ASSERT(oy >= yMin - 0.001 && oy <= yMax + 0.001);

        obterCoordenadasEndereco(q, FACE_O, nums_yface[i], &ox, &oy);
        TEST_ASSERT(ox >= xMin - 0.001 && ox <= xMax + 0.001);
        TEST_ASSERT(oy >= yMin - 0.001 && oy <= yMax + 0.001);
    }

    destruirQuadra(q);
}

int main(void){
    UNITY_BEGIN();

    RUN_TEST(test_criarQuadra_DeveRetornarPonteiroValido);
    RUN_TEST(test_criar_ComCEPNulo_DeveRetornarNull);
    RUN_TEST(test_criar_ComCFillNulo_DeveRetornarNull);
    RUN_TEST(test_criar_ComCStrkNulo_DeveRetornarNull);
    RUN_TEST(test_criar_ComDimensoesNegativas_DeveRetornarNull);
    RUN_TEST(test_destruirComParametroNull_naoQuebrar);
    RUN_TEST(test_getters_RetornarValoresCorretos);
    RUN_TEST(test_getters_ParametrosNull_naoQuebrar);
    RUN_TEST(test_faceValida_FacesCorretas);
    RUN_TEST(test_faceValida_FacesInvalidas);
    RUN_TEST(test_charParaFaceQuadra_ConversaoCorreta);
    RUN_TEST(test_obterCoordenadas_FaceS_YFixoNaAncora);
    RUN_TEST(test_obterCoordenadas_FaceN_YFixoNoTopoOposto);
    RUN_TEST(test_obterCoordenadas_FaceL_XFixoNaAncora);
    RUN_TEST(test_obterCoordenadas_FaceO_XFixoNoLadoOposto);
    RUN_TEST(test_obterCoordenadas_PontosSempreDentroDoRetangulo);

    return UNITY_END();
}