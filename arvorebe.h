#ifndef ARVOREBE_H
#define ARVOREBE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h> // Para off_t
#include "item.h"

// M alto para garantir árvore baixa e poucos mallocs
#define M_ESTRELA 100
#define MM_ESTRELA (2 * M_ESTRELA)

// Definição de tipo para suporte a arquivos grandes no Linux
typedef off_t TipoOffset;

typedef struct TipoPaginaBE* TipoApontadorBE;

typedef struct TipoPaginaBE {
    short n;
    int chaves[MM_ESTRELA];          
    TipoOffset offsets[MM_ESTRELA];  // Guarda ONDE está no arquivo
    TipoApontadorBE p[MM_ESTRELA + 1];
} TipoPaginaBE;

// Inicialização
void InicializaArvoreBEst(TipoApontadorBE* Arvore);

// Pesquisa retorna o offset (ou -1)
TipoOffset PesquisaBE(int chave, TipoApontadorBE Ap, int* comp);

// Inserção recebe Chave e Offset (não o Item completo!)
void InsereBEst(int chave, TipoOffset offset, TipoApontadorBE* Ap, int* comp);

void LiberaArvoreBE(TipoApontadorBE Ap);

// Adicione antes do #endif
void RodaArvoreBEstrela(const char* nomeArq, int chave_procurada, int quantReg, bool P_flag);

#endif