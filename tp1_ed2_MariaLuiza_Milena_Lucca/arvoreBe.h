#ifndef ARVOREBE_H
#define ARVOREBE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h> // Para off_t
#include <string.h> // Preciso disso pro memmove e memcpy (pra mover blocos de memoria rapido)
#include <time.h>

#include "item.h"
#include "analiseExperimental.h"

#define M_ESTRELA 100
#define MM_ESTRELA (2 * M_ESTRELA)

typedef off_t TipoOffset;

typedef struct TipoPaginaBE* TipoApontadorBE;

typedef struct TipoPaginaBE {
    short n; // Quantidade de chaves
    int chaves[MM_ESTRELA];          
    TipoOffset offsets[MM_ESTRELA]; 
    TipoApontadorBE p[MM_ESTRELA + 1]; 
} TipoPaginaBE;

// Protótipos Instrumentados
void InicializaArvoreBEst(TipoApontadorBE* Arvore);
void InsereBEst(int chave, TipoOffset offset, TipoApontadorBE* Ap, AnaliseExperimental* analise);
TipoOffset PesquisaBE(int chave, TipoApontadorBE Ap, AnaliseExperimental* analise);
void LiberaArvoreBE(TipoApontadorBE Ap);

// Função Wrapper padrão do projeto
bool pesquisaArvoreBEstrela(FILE *arquivo, int quantReg, Item *itemP, AnaliseExperimental *analise);

#endif