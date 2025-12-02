#ifndef ARVOREB_H
#define ARVOREB_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h> 
#include <time.h>

#include "item.h"
#include "analiseExperimental.h"

#define M 100 // Ordem da árvore (ajustável)
#define MAX (2 * M)

typedef off_t FilePos; 

typedef struct B_No {
    int qtdChaves;
    bool folha;
    int chaves[MAX];       
    FilePos offsets[MAX];  
    struct B_No* filhos[MAX + 1];
} B_No;

typedef B_No* ArvoreB; 

// Funções auxiliares de gerenciamento
B_No* B_criaNo(bool folha);
void B_liberaArvore(B_No* no);

// Funções principais instrumentadas
void B_inserir(ArvoreB* arvore, int chave, FilePos offset, AnaliseExperimental* analise);
FilePos B_pesquisa(B_No* raiz, int chave, AnaliseExperimental* analise);

// Função que segue o padrão do projeto para o experimento
bool pesquisaArvoreB(FILE *arquivo, int quantReg, Item *itemP, AnaliseExperimental *analise);

#endif