#ifndef SEQUENCIALINDEX_H
#define SEQUENCIALINDEX_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <time.h>

#include "item.h"
#include "analiseExperimental.h"

typedef struct{
    int chave;
}Indice;

int tamanhoPagina(int);
int buscaIndice(Indice *, int, int, AnaliseExperimental *);
bool pesquisaPagina(FILE *, Item *, int, int, Indice *, AnaliseExperimental *);
bool geraTabela(FILE *, int, int, Indice **, int *, AnaliseExperimental *);
bool indexado(FILE *, int, Item *, AnaliseExperimental *);

#endif