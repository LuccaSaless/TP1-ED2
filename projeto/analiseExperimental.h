#ifndef ANALISEEXPERIMENTAL_H
#define ANALISEEXPERIMENTAL_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#include "item.h"

typedef struct{
    int numTransferencias;
    int numComparacoes;
    double tempoExecucao;
}AnaliseExperimental;

void imprimeAnalise(AnaliseExperimental *, Item *);
void gerarChaves(int, int[20]);
void executarExperimento(FILE *, int, AnaliseExperimental *, bool (*funcaoPesquisa)(FILE*, int, Item*, AnaliseExperimental*));
void imprimeMediaAnalise(AnaliseExperimental *);

#endif