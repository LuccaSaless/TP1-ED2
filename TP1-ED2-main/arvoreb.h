#ifndef ARVOREB_H
#define ARVOREB_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

// --- ADICIONE ESTA LINHA OBRIGATORIAMENTE ---
#include "item.h" 
// -------------------------------------------

#define M 100
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

B_No* B_criaNo(bool folha);
void B_arvoreInsere(ArvoreB* arvore, int chave, FilePos offset, AnaliseExperimental* analise);
FilePos B_arvoreBusca(B_No* raiz, int chave, AnaliseExperimental* analise);
void RodaArvoreB(const char* nomeArq, int chave_procurada, int quantReg, bool P_flag);
void B_liberaArvoreB(ArvoreB arvore);

#endif