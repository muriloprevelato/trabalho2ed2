PROJ_NAME = ted
ALUNO     = muriloP

CC     = gcc
CFLAGS = -ggdb -O0 -std=c99 -fstack-protector-all \
         -Werror=implicit-function-declaration \
         -Wall -Wextra
LDFLAGS = -O0

# Diretórios
SRC  = .
INC  = ../include
TST  = ../test
UNI  = ../unity

# =============================================================================
# Objetos dos módulos
# =============================================================================
OBJS = $(SRC)/hashGenerica.o \
       $(SRC)/quadra.o       \
       $(SRC)/cidade.o       \
       $(SRC)/svg.o          \
       $(SRC)/leitorGeo.o    \
       $(SRC)/vertice.o      \
       $(SRC)/grafo.o        \
       $(SRC)/leitorVia.o    \
       $(SRC)/filaPrioridade.o    \
       $(SRC)/caminhoMinimo.o     \
       $(SRC)/componentesConexos.o \
       $(SRC)/ampliacaoViaria.o \
       $(SRC)/registradores.o \
       $(SRC)/resolveEndereco.o \
       $(SRC)/leitorQry.o

# =============================================================================
# Flags de include compartilhadas
# =============================================================================
INC_FLAGS = -I $(INC) -I $(UNI)

# =============================================================================
# Target principal
# =============================================================================
$(PROJ_NAME): $(OBJS) $(SRC)/main.o
	$(CC) $(LDFLAGS) -o $(PROJ_NAME) $(OBJS) $(SRC)/main.o

ted: $(PROJ_NAME)

# =============================================================================
# Regra genérica para compilar .c -> .o dentro de src/
# =============================================================================
$(SRC)/%.o: $(SRC)/%.c
	$(CC) -c $(CFLAGS) $(INC_FLAGS) $< -o $@

# =============================================================================
# Dependências explícitas de cada módulo
# =============================================================================
$(SRC)/hashGenerica.o: $(SRC)/hashGenerica.c $(INC)/hashGenerica.h

$(SRC)/quadra.o: $(SRC)/quadra.c $(INC)/quadra.h

$(SRC)/cidade.o: $(SRC)/cidade.c $(INC)/cidade.h \
                 $(INC)/hashGenerica.h $(INC)/quadra.h

$(SRC)/svg.o: $(SRC)/svg.c $(INC)/svg.h

$(SRC)/leitorGeo.o: $(SRC)/leitorGeo.c $(INC)/leitorGeo.h \
                    $(INC)/cidade.h $(INC)/quadra.h

$(SRC)/vertice.o: $(SRC)/vertice.c $(INC)/vertice.h
 
$(SRC)/grafo.o: $(SRC)/grafo.c $(INC)/grafo.h $(INC)/vertice.h $(INC)/hashGenerica.h

$(SRC)/leitorVia.o: $(SRC)/leitorVia.c $(INC)/leitorVia.h \
                    $(INC)/grafo.h $(INC)/vertice.h

$(SRC)/filaPrioridade.o: $(SRC)/filaPrioridade.c $(INC)/filaPrioridade.h

$(SRC)/caminhoMinimo.o: $(SRC)/caminhoMinimo.c $(INC)/caminhoMinimo.h \
                        $(INC)/grafo.h $(INC)/vertice.h $(INC)/hashGenerica.h \
                        $(INC)/filaPrioridade.h

$(SRC)/componentesConexos.o: $(SRC)/componentesConexos.c $(INC)/componentesConexos.h \
                             $(INC)/grafo.h $(INC)/vertice.h $(INC)/hashGenerica.h

$(SRC)/ampliacaoViaria.o: $(SRC)/ampliacaoViaria.c $(INC)/ampliacaoViaria.h \
                          $(INC)/grafo.h $(INC)/vertice.h $(INC)/hashGenerica.h

$(SRC)/registradores.o: $(SRC)/registradores.c $(INC)/registradores.h

$(SRC)/resolveEndereco.o: $(SRC)/resolveEndereco.c $(INC)/resolveEndereco.h \
                          $(INC)/cidade.h $(INC)/quadra.h $(INC)/grafo.h $(INC)/vertice.h

$(SRC)/leitorQry.o: $(SRC)/leitorQry.c $(INC)/leitorQry.h \
                    $(INC)/cidade.h $(INC)/quadra.h $(INC)/grafo.h $(INC)/vertice.h \
                    $(INC)/registradores.h $(INC)/svg.h $(INC)/resolveEndereco.h \
                    $(INC)/caminhoMinimo.h $(INC)/componentesConexos.h $(INC)/ampliacaoViaria.h

$(SRC)/main.o: $(SRC)/main.c \
               $(INC)/cidade.h $(INC)/quadra.h $(INC)/hashGenerica.h \
               $(INC)/svg.h $(INC)/leitorGeo.h $(INC)/vertice.h $(INC)/grafo.h \
               $(INC)/leitorVia.h

# =============================================================================
# Testes unitários — target genérico
# =============================================================================
DEPS_quadra      = $(SRC)/quadra.o
DEPS_hashGenerica = $(SRC)/hashGenerica.o
DEPS_cidade      = $(SRC)/cidade.o $(SRC)/quadra.o $(SRC)/hashGenerica.o
DEPS_svg         = $(SRC)/svg.o
DEPS_leitorGeo   = $(SRC)/leitorGeo.o $(SRC)/cidade.o $(SRC)/quadra.o $(SRC)/hashGenerica.o
DEPS_vertice     = $(SRC)/vertice.o
DEPS_grafo       = $(SRC)/grafo.o $(SRC)/vertice.o $(SRC)/hashGenerica.o
DEPS_leitorVia   = $(SRC)/leitorVia.o $(SRC)/grafo.o $(SRC)/vertice.o $(SRC)/hashGenerica.o
DEPS_filaPrioridade = $(SRC)/filaPrioridade.o
DEPS_caminhoMinimo = $(SRC)/caminhoMinimo.o $(SRC)/grafo.o $(SRC)/vertice.o \
                     $(SRC)/hashGenerica.o $(SRC)/filaPrioridade.o
DEPS_componentesConexos = $(SRC)/componentesConexos.o $(SRC)/grafo.o $(SRC)/vertice.o \
                          $(SRC)/hashGenerica.o
DEPS_ampliacaoViaria = $(SRC)/ampliacaoViaria.o $(SRC)/grafo.o $(SRC)/vertice.o \
                       $(SRC)/hashGenerica.o
DEPS_registradores = $(SRC)/registradores.o
DEPS_resolveEndereco = $(SRC)/resolveEndereco.o $(SRC)/cidade.o $(SRC)/quadra.o \
                       $(SRC)/hashGenerica.o $(SRC)/grafo.o $(SRC)/vertice.o
DEPS_leitorQry = $(SRC)/leitorQry.o $(SRC)/cidade.o $(SRC)/quadra.o $(SRC)/hashGenerica.o \
                 $(SRC)/grafo.o $(SRC)/vertice.o $(SRC)/svg.o $(SRC)/registradores.o \
                 $(SRC)/resolveEndereco.o $(SRC)/caminhoMinimo.o $(SRC)/filaPrioridade.o \
                 $(SRC)/componentesConexos.o $(SRC)/ampliacaoViaria.o

test:
ifndef MODULO
	$(error Uso: make test MODULO=<nome>. Ex: make test MODULO=quadra)
endif
	$(MAKE) $(DEPS_$(MODULO))
	$(CC) $(CFLAGS) $(INC_FLAGS) \
	    $(DEPS_$(MODULO)) \
	    $(TST)/test_$(MODULO).c $(UNI)/unity.c \
	    -o $(TST)/run_$(MODULO)
	$(TST)/run_$(MODULO)

# =============================================================================
# Roda todos os módulos de teste em sequência
# =============================================================================
MODULOS_TESTAVEIS = quadra hashGenerica cidade svg leitorGeo vertice grafo \
                    leitorVia filaPrioridade caminhoMinimo componentesConexos \
                    ampliacaoViaria registradores resolveEndereco leitorQry

tstall:
	@for m in $(MODULOS_TESTAVEIS); do \
	    $(MAKE) test MODULO=$$m || exit 1; \
	done

# =============================================================================
# Limpeza
# =============================================================================
clean:
	rm -f $(SRC)/*.o $(PROJ_NAME)
	rm -f $(addprefix $(TST)/run_,$(MODULOS_TESTAVEIS))
	rm -f temp_*.geo temp_*.pm temp_*.svg temp_*.qry temp_*.txt
	rm -f teste_*_tmp.geo teste_*_tmp.svg teste_*_tmp.via teste_*_tmp.qry teste_*_tmp.txt

.PHONY: ted test tstall clean