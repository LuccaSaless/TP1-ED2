#ifndef ARVOREBIN_H
#define ARVOREBIN_H

#include <stdio.h>
#include <stdbool.h> // Para bool
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include "item.h"     // Para Item e AnaliseExperimental

typedef struct NoArvoreBin {
    int chave;
    long int offset;
    struct NoArvoreBin *esquerda;
    struct NoArvoreBin *direita;
} NoArvoreBin;

NoArvoreBin* criarNoArvoreBin(int chave, long int offset);
NoArvoreBin* inserirNaArvoreBin(NoArvoreBin* raiz, int chave, long int offset, AnaliseExperimental *analise);
Item* pesquisarArvoreBin(NoArvoreBin* raiz, int chave, FILE* arquivo, AnaliseExperimental *analise);
void imprimirChavesArvoreBin(NoArvoreBin* raiz);
void liberarArvoreBin(NoArvoreBin* raiz);
NoArvoreBin* construirArvoreBin(const char* nomeArquivo, AnaliseExperimental *analiseCriacao);
void RodaArvoreBinaria(const char* nomeArq, int chave_procurada, int quantReg, bool P_flag);

#endif // ARVOREBIN_H