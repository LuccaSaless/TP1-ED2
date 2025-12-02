#ifndef ARVOREBIN_H
#define ARVOREBIN_H

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "item.h"
#include "analiseExperimental.h" // Necessário para a struct AnaliseExperimental

#define NULL_DISK -1

typedef struct NoArvoreBin {
    int chave;
    long int offsetDado;
    long int esquerda;
    long int direita;
} NoArvoreBin;

// Funções Nucleares
void construirArvoreBin(const char* nomeArquivoDados, const char* nomeArquivoArvore, AnaliseExperimental *analiseCriacao);
Item* pesquisarArvoreBin(FILE* arqArvore, long int posRaiz, int chave, FILE* arqDados, AnaliseExperimental *analise);
long int inserirNaArvoreBin(FILE* arqArvore, long int posAtual, int chave, long int offsetDado, AnaliseExperimental *analise);

// === NOVA FUNÇÃO WRAPPER ===
// Adapta a pesquisa da árvore para a assinatura exigida pelo analiseExperimental.c
bool pesquisaArvoreBinariaWrapper(FILE* arqDados, int quantReg, Item* item, AnaliseExperimental* analise);

#endif // ARVOREBIN_H