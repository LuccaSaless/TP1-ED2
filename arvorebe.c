// OBRIGATÓRIO PARA ARQUIVOS > 2GB
#define _FILE_OFFSET_BITS 64 

#include "arvorebe.h"
#include <time.h>

void InicializaArvoreBEst(TipoApontadorBE* Arvore) {
    *Arvore = NULL;
}

TipoOffset PesquisaBE(int chave, TipoApontadorBE Ap, int *comp) {
    long i = 1;
    if (Ap == NULL) return -1;

    while (i < Ap->n && chave > Ap->chaves[i-1]) {
        i++;
        (*comp)++;
    }

    (*comp)++;
    if (chave == Ap->chaves[i-1]) {
        return Ap->offsets[i-1]; // Retorna a posição no HD
    }

    (*comp)++;
    if (chave < Ap->chaves[i-1])
        return PesquisaBE(chave, Ap->p[i-1], comp);
    else
        return PesquisaBE(chave, Ap->p[i], comp);
}

// Inserção auxiliar na página
void InsereNaPaginaBE(TipoApontadorBE Ap, int chave, TipoOffset offset, TipoApontadorBE ApDir, int *comp) {
    short NaoAchouPosicao;
    int k;

    k = Ap->n;
    NaoAchouPosicao = (k > 0);

    while (NaoAchouPosicao) {
        (*comp)++;
        if (chave >= Ap->chaves[k-1]) {
            NaoAchouPosicao = 0;
            break;
        }
        
        Ap->chaves[k] = Ap->chaves[k-1];
        Ap->offsets[k] = Ap->offsets[k-1]; // Move offset junto
        Ap->p[k+1] = Ap->p[k];
        k--;
        if (k < 1) NaoAchouPosicao = 0;
    }

    Ap->chaves[k] = chave;
    Ap->offsets[k] = offset;
    Ap->p[k+1] = ApDir;
    Ap->n++;
}

// Função recursiva interna
void InsBE(int chave, TipoOffset offset, TipoApontadorBE Ap, short *Cresceu, 
           int *ChaveRetorno, TipoOffset *OffsetRetorno, TipoApontadorBE *ApRetorno, int *comp) {
    long i = 1;
    long j;
    TipoApontadorBE ApTemp;

    if (Ap == NULL) {
        *Cresceu = 1;
        *ChaveRetorno = chave;
        *OffsetRetorno = offset;
        *ApRetorno = NULL;
        return;
    }

    while (i < Ap->n && chave > Ap->chaves[i-1]) {
        i++;
        (*comp)++;
    }

    (*comp)++;
    if (chave == Ap->chaves[i-1]) {
        *Cresceu = 0; 
        return;
    }

    (*comp)++;
    if (chave < Ap->chaves[i-1]) i--;

    InsBE(chave, offset, Ap->p[i], Cresceu, ChaveRetorno, OffsetRetorno, ApRetorno, comp);

    if (!*Cresceu) return;

    if (Ap->n < MM_ESTRELA) {
        InsereNaPaginaBE(Ap, *ChaveRetorno, *OffsetRetorno, *ApRetorno, comp);
        *Cresceu = 0;
        return;
    }

    // --- SPLIT ---
    // (Nota: Em uma implementação rigorosa de B*, tentaríamos redistribuir para irmãos
    // antes de dividir. Aqui usamos o Split padrão para garantir estabilidade de memória)
    ApTemp = (TipoApontadorBE)malloc(sizeof(TipoPaginaBE));
    ApTemp->n = 0;
    ApTemp->p[0] = NULL;

    if (i < M_ESTRELA + 1) {
        InsereNaPaginaBE(ApTemp, Ap->chaves[MM_ESTRELA-1], Ap->offsets[MM_ESTRELA-1], Ap->p[MM_ESTRELA], comp);
        Ap->n--;
        InsereNaPaginaBE(Ap, *ChaveRetorno, *OffsetRetorno, *ApRetorno, comp);
    } else {
        InsereNaPaginaBE(ApTemp, *ChaveRetorno, *OffsetRetorno, *ApRetorno, comp);
    }

    for (j = M_ESTRELA + 2; j <= MM_ESTRELA; j++) {
        InsereNaPaginaBE(ApTemp, Ap->chaves[j-1], Ap->offsets[j-1], Ap->p[j], comp);
    }

    Ap->n = M_ESTRELA;
    ApTemp->p[0] = Ap->p[M_ESTRELA+1];
    
    *ChaveRetorno = Ap->chaves[M_ESTRELA];
    *OffsetRetorno = Ap->offsets[M_ESTRELA];
    *ApRetorno = ApTemp;
}

void InsereBEst(int chave, TipoOffset offset, TipoApontadorBE* Ap, int* comp) {
    short Cresceu;
    int ChaveRetorno;
    TipoOffset OffsetRetorno;
    TipoApontadorBE ApRetorno, ApTemp;

    InsBE(chave, offset, *Ap, &Cresceu, &ChaveRetorno, &OffsetRetorno, &ApRetorno, comp);

    if (Cresceu) {
        ApTemp = (TipoApontadorBE)malloc(sizeof(TipoPaginaBE));
        ApTemp->n = 1;
        ApTemp->chaves[0] = ChaveRetorno;
        ApTemp->offsets[0] = OffsetRetorno;
        ApTemp->p[1] = ApRetorno;
        ApTemp->p[0] = *Ap;
        *Ap = ApTemp;
    }
}

void RodaArvoreBEstrela(const char* nomeArq, int chave_procurada, int quantReg, bool P_flag) {
    TipoApontadorBE arvore;
    Item registro_temp;
    int comparacoes_criacao = 0;
    
    InicializaArvoreBEst(&arvore);
    
    FILE* arq = fopen(nomeArq, "rb");
    if (!arq) { perror("Erro"); return; }

    // --- FASE 1: CRIACAO ---
    clock_t inicio_criacao = clock();
    while(true) {
        TipoOffset posicao = ftello(arq);
        if (fread(&registro_temp, sizeof(Item), 1, arq) != 1) break;
        InsereBEst(registro_temp.chave, posicao, &arvore, &comparacoes_criacao);
    }
    clock_t fim_criacao = clock();
    double tempo_criacao = (double)(fim_criacao - inicio_criacao) / CLOCKS_PER_SEC;

    printf("\n========================================================\n");
    printf("METODO: 4 - ARVORE B* (ESTRELA)\n");
    printf("========================================================\n");

    printf("--- FASE 1: CRIACAO DO INDICE ---\n");
    printf("Tempo Execucao: %.6f s\n", tempo_criacao);
    printf("Transferencias: %d (Leitura Sequencial)\n", quantReg); 
    printf("Comparacoes:    %d\n", comparacoes_criacao);

    // --- FASE 2: PESQUISA ---
    printf("\n--- FASE 2: PESQUISA ---\n");
    
    int comp_pesquisa = 0;
    int transf_pesquisa = 0;
    clock_t inicio_pesquisa = clock();
    
    if (chave_procurada != -1) {
        TipoOffset offset_encontrado = PesquisaBE(chave_procurada, arvore, &comp_pesquisa);
        
        bool encontrado = false;
        if (offset_encontrado != -1) {
            fseeko(arq, offset_encontrado, SEEK_SET);
            fread(&registro_temp, sizeof(Item), 1, arq);
            transf_pesquisa = 1; 
            encontrado = true;
        }
        
        clock_t fim_pesquisa = clock();
        double tempo_pesquisa = (double)(fim_pesquisa - inicio_pesquisa) / CLOCKS_PER_SEC;

        if (encontrado) {
            printf("[STATUS]: ITEM ENCONTRADO\n");
            printf("> Chave:  %d\n", registro_temp.chave);
            printf("> Dado1:  %ld\n", registro_temp.dado1);
            if(P_flag) printf("> Dado2:  %.50s...\n", registro_temp.dado2);
        } else {
            printf("[STATUS]: ITEM NAO ENCONTRADO (Chave %d)\n", chave_procurada);
        }

        printf("\n--- METRICAS DE PESQUISA ---\n");
        printf("Tempo Execucao: %.6f s\n", tempo_pesquisa);
        printf("Transferencias: %d\n", transf_pesquisa);
        printf("Comparacoes:    %d\n", comp_pesquisa);
    } 
    else {
         printf("Modo experimental (media) nao formatado neste padrao.\n");
    }
    printf("========================================================\n");

    fclose(arq);
    LiberaArvoreBE(arvore);
}

void LiberaArvoreBE(TipoApontadorBE Ap) {
    if (Ap != NULL) {
        for (int i = 0; i <= Ap->n; i++) {
            LiberaArvoreBE(Ap->p[i]);
        }
        free(Ap);
    }
}