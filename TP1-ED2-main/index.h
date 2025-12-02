#ifndef index_h
#define index_h

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "item.h"


// definição de uma entrada da tabela de índice das páginas
typedef struct {
    int chave;
} Indice;

void tamPag(int *, int );
int Indexado(Item *, FILE *, AnaliseExperimental *, int);
int Pesquisa(Indice tab[], int, Item *, FILE *, int *, int);
void RodaIndexadoExperimento(FILE *, int);


#endif