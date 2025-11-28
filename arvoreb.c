// ESTA LINHA DEVE SER A PRIMEIRA DO ARQUIVO
#define _FILE_OFFSET_BITS 64

#include "arvoreb.h"
#include "item.h"
#include <time.h>

B_No* B_criaNo(bool folha) {
    B_No* no = (B_No*)malloc(sizeof(B_No));
    if (no == NULL) {
        perror("Erro fatal: Memoria insuficiente");
        exit(1);
    }
    no->qtdChaves = 0;
    no->folha = folha;
    for (int i = 0; i <= MAX; i++) {
        no->filhos[i] = NULL;
    }
    return no;
}

FilePos B_arvoreBusca(B_No* raiz, int chave, AnaliseExperimental* analise) {
    int i = 0;
    if (raiz == NULL) return -1;

    while (i < raiz->qtdChaves) {
        analise->numComparacoes++;
        if (chave == raiz->chaves[i]) {
            return raiz->offsets[i]; 
        } else if (chave > raiz->chaves[i]) {
            i++;
        } else {
            break;
        }
    }

    if (raiz->folha) return -1;
    return B_arvoreBusca(raiz->filhos[i], chave, analise);
}

void B_splitFilho(B_No* pai, int i, B_No* filho) {
    B_No* novo = B_criaNo(filho->folha);
    novo->qtdChaves = M - 1;

    for (int j = 0; j < M - 1; j++) {
        novo->chaves[j] = filho->chaves[j + M + 1];
        novo->offsets[j] = filho->offsets[j + M + 1]; 
    }

    if (!filho->folha) {
        for (int j = 0; j < M; j++) {
            novo->filhos[j] = filho->filhos[j + M + 1];
        }
    }

    filho->qtdChaves = M;

    for (int j = pai->qtdChaves; j >= i + 1; j--) {
        pai->filhos[j + 1] = pai->filhos[j];
    }
    pai->filhos[i + 1] = novo;

    for (int j = pai->qtdChaves - 1; j >= i; j--) {
        pai->chaves[j + 1] = pai->chaves[j];
        pai->offsets[j + 1] = pai->offsets[j];
    }

    pai->chaves[i] = filho->chaves[M];
    pai->offsets[i] = filho->offsets[M];
    pai->qtdChaves++;
}

void B_insereNaoCheio(B_No* no, int chave, FilePos offset, AnaliseExperimental* analise) {
    int i = no->qtdChaves - 1;

    if (no->folha) {
        while (i >= 0 && chave < no->chaves[i]) {
            analise->numComparacoes++;
            no->chaves[i + 1] = no->chaves[i];
            no->offsets[i + 1] = no->offsets[i];
            i--;
        }
        if (i >= 0) analise->numComparacoes++;

        no->chaves[i + 1] = chave;
        no->offsets[i + 1] = offset;
        no->qtdChaves++;
    } else {
        while (i >= 0 && chave < no->chaves[i]) {
            analise->numComparacoes++;
            i--;
        }
        i++;
        if (no->filhos[i]->qtdChaves == MAX) {
            B_splitFilho(no, i, no->filhos[i]);
            if (chave > no->chaves[i]) {
                i++;
            }
        }
        B_insereNaoCheio(no->filhos[i], chave, offset, analise);
    }
}

void B_arvoreInsere(ArvoreB* arvore, int chave, FilePos offset, AnaliseExperimental* analise) {
    if (*arvore == NULL) {
        *arvore = B_criaNo(true);
    }

    B_No* raiz = *arvore;

    if (raiz->qtdChaves == MAX) {
        B_No* novaRaiz = B_criaNo(false);
        novaRaiz->filhos[0] = raiz;
        B_splitFilho(novaRaiz, 0, raiz);
        B_insereNaoCheio(novaRaiz, chave, offset, analise);
        *arvore = novaRaiz;
    } else {
        B_insereNaoCheio(raiz, chave, offset, analise);
    }
}

void B_liberaArvoreB(ArvoreB arvore) {
    if (arvore == NULL) return;
    for (int i = 0; i <= arvore->qtdChaves; i++) {
        B_liberaArvoreB(arvore->filhos[i]);
    }
    free(arvore);
}

void RodaArvoreB(const char* nomeArq, int chave_procurada, int quantReg, bool P_flag) {
    ArvoreB arvoreB = NULL;
    Item item_lido;
    AnaliseExperimental analiseCriacao = {0, 0, 0};

    // --- FASE 1: CRIACAO ---
    FILE* arquivoCriacao = fopen(nomeArq, "rb");
    if (arquivoCriacao == NULL) { perror("Erro"); return; }

    clock_t start_criacao = clock();
    while (true) {
        FilePos posicao_atual = ftello(arquivoCriacao);
        if (fread(&item_lido, sizeof(Item), 1, arquivoCriacao) != 1) break;
        analiseCriacao.numTransferencias++;
        B_arvoreInsere(&arvoreB, item_lido.chave, posicao_atual, &analiseCriacao);
    }
    clock_t end_criacao = clock();
    analiseCriacao.tempoExecucao = (double)(end_criacao - start_criacao) / CLOCKS_PER_SEC;
    fclose(arquivoCriacao);

    printf("\n========================================================\n");
    printf("METODO: 3 - ARVORE B\n");
    printf("========================================================\n");

    printf("--- FASE 1: CRIACAO DO INDICE ---\n");
    printf("Tempo Execucao: %.6f s\n", analiseCriacao.tempoExecucao);
    printf("Transferencias: %d\n", analiseCriacao.numTransferencias);
    printf("Comparacoes:    %d\n", analiseCriacao.numComparacoes);

    // --- FASE 2: PESQUISA ---
    printf("\n--- FASE 2: PESQUISA ---\n");

    FILE* arquivoPesquisa = fopen(nomeArq, "rb");
    if (!arquivoPesquisa) { B_liberaArvoreB(arvoreB); return; }

    AnaliseExperimental analisePesquisa = {0, 0, 0};

    if (chave_procurada != -1) {
        clock_t start_p = clock();
        FilePos offset = B_arvoreBusca(arvoreB, chave_procurada, &analisePesquisa);
        
        bool encontrado = false;
        if (offset != -1) {
            fseeko(arquivoPesquisa, offset, SEEK_SET);
            fread(&item_lido, sizeof(Item), 1, arquivoPesquisa);
            analisePesquisa.numTransferencias++;
            encontrado = true;
        }
        clock_t end_p = clock();
        analisePesquisa.tempoExecucao = (double)(end_p - start_p) / CLOCKS_PER_SEC;

        if (encontrado) {
            printf("[STATUS]: ITEM ENCONTRADO\n");
            printf("> Chave:  %d\n", item_lido.chave);
            printf("> Dado1:  %ld\n", item_lido.dado1);
            if(P_flag) printf("> Dado2:  %.50s...\n", item_lido.dado2);
        } else {
            printf("[STATUS]: ITEM NAO ENCONTRADO (Chave %d)\n", chave_procurada);
        }

        printf("\n--- METRICAS DE PESQUISA ---\n");
        printf("Tempo Execucao: %.6f s\n", analisePesquisa.tempoExecucao);
        printf("Transferencias: %d\n", analisePesquisa.numTransferencias);
        printf("Comparacoes:    %d\n", analisePesquisa.numComparacoes);

    } else {
        printf("Modo experimental (media) nao formatado neste padrao.\n");
    }
    printf("========================================================\n");

    fclose(arquivoPesquisa);
    B_liberaArvoreB(arvoreB);
}