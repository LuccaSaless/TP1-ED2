#include "arvorebin.h"

NoArvoreBin* criarNoArvoreBin(int chave, long int offset) {
    NoArvoreBin* novoNo = (NoArvoreBin*)malloc(sizeof(NoArvoreBin));
    if (novoNo == NULL) {
        perror("Erro ao alocar memória para o nó da árvore binária");
        exit(EXIT_FAILURE);
    }
    novoNo->chave = chave;
    novoNo->offset = offset;
    novoNo->esquerda = NULL;
    novoNo->direita = NULL;
    return novoNo;
}

NoArvoreBin* inserirNaArvoreBin(NoArvoreBin* raiz, int chave, long int offset, AnaliseExperimental *analise) {
    if (raiz == NULL) {
        return criarNoArvoreBin(chave, offset);
    }

    analise->numComparacoes++;
    if (chave < raiz->chave) {
        raiz->esquerda = inserirNaArvoreBin(raiz->esquerda, chave, offset, analise);
    } else if (chave > raiz->chave) {
        raiz->direita = inserirNaArvoreBin(raiz->direita, chave, offset, analise);
    }
    return raiz;
}

NoArvoreBin* construirArvoreBin(const char* nomeArquivo, AnaliseExperimental *analiseCriacao) {
    FILE* arquivo = fopen(nomeArquivo, "rb");
    if (arquivo == NULL) {
        perror("Erro ao abrir o arquivo para construir a árvore binária");
        return NULL;
    }

    NoArvoreBin* raiz = NULL;
    Item item;
    long int offsetAtual = 0;

    clock_t inicio = clock();

    while (fread(&item, sizeof(Item), 1, arquivo) == 1) {
        analiseCriacao->numTransferencias++;
        raiz = inserirNaArvoreBin(raiz, item.chave, offsetAtual, analiseCriacao);
        offsetAtual = ftell(arquivo);
    }

    clock_t fim = clock();
    analiseCriacao->tempoExecucao = (long int)(((double)(fim - inicio)) / CLOCKS_PER_SEC * 1000000.0);

    fclose(arquivo);
    return raiz;
}

Item* pesquisarArvoreBin(NoArvoreBin* raiz, int chave, FILE* arquivo, AnaliseExperimental *analise) {
    if (arquivo == NULL) {
        fprintf(stderr, "Erro: Arquivo não pode ser NULL para pesquisa.\n");
        return NULL;
    }

    Item* itemEncontrado = NULL;
    analise->numTransferencias = 0;
    analise->numComparacoes = 0;
    clock_t inicio = clock();

    NoArvoreBin* atual = raiz;
    while (atual != NULL) {
        analise->numComparacoes++;
        if (chave == atual->chave) {
            if (fseek(arquivo, atual->offset, SEEK_SET) != 0) {
                perror("Erro ao posicionar no arquivo para ler o Item");
                break;
            }
            itemEncontrado = (Item*)malloc(sizeof(Item));
            if (itemEncontrado == NULL) {
                perror("Erro ao alocar memória para o Item encontrado");
                break;
            }
            if (fread(itemEncontrado, sizeof(Item), 1, arquivo) == 1) {
                analise->numTransferencias++;
            } else {
                perror("Erro ao ler o Item do arquivo");
                free(itemEncontrado);
                itemEncontrado = NULL;
            }
            break;
        } else if (chave < atual->chave) {
            atual = atual->esquerda;
        } else {
            atual = atual->direita;
        }
    }

    clock_t fim = clock();
    analise->tempoExecucao = (long int)(((double)(fim - inicio)) / CLOCKS_PER_SEC * 1000000.0);

    return itemEncontrado;
}

void imprimirChavesArvoreBin(NoArvoreBin* raiz) {
    if (raiz != NULL) {
        imprimirChavesArvoreBin(raiz->esquerda);
        printf("Chave: %d, Offset: %ld\n", raiz->chave, raiz->offset);
        imprimirChavesArvoreBin(raiz->direita);
    }
}

void liberarArvoreBin(NoArvoreBin* raiz) {
    if (raiz != NULL) {
        liberarArvoreBin(raiz->esquerda);
        liberarArvoreBin(raiz->direita);
        free(raiz);
    }
}

void RodaArvoreBinaria(const char* nomeArq, int chave_procurada, int quantReg, bool P_flag) {
    NoArvoreBin* raizArvoreBin = NULL;
    AnaliseExperimental analiseCriacao = {0, 0, 0};

    // --- FASE 1: CRIACAO ---
    raizArvoreBin = construirArvoreBin(nomeArq, &analiseCriacao);
    if (raizArvoreBin == NULL) {
        fprintf(stderr, "Erro ao construir a arvore binaria.\n");
        return;
    }

    printf("\n========================================================\n");
    printf("METODO: 2 - ARVORE BINARIA EXTERNA\n");
    printf("========================================================\n");
    
    printf("--- FASE 1: CRIACAO DO INDICE ---\n");
    printf("Tempo Execucao: %.6f s\n", (double)analiseCriacao.tempoExecucao / 1000000.0);
    printf("Transferencias: %d\n", analiseCriacao.numTransferencias);
    printf("Comparacoes:    %d\n", analiseCriacao.numComparacoes);

    // --- FASE 2: PESQUISA ---
    printf("\n--- FASE 2: PESQUISA ---\n");
    
    AnaliseExperimental analisePesquisa = {0, 0, 0};
    bool encontrado = false;
    Item resultado;

    if (chave_procurada != -1) {
        FILE* arquivoParaPesquisa = fopen(nomeArq, "rb");
        if (arquivoParaPesquisa == NULL) {
            perror("Erro ao abrir arquivo");
            liberarArvoreBin(raizArvoreBin);
            return;
        }

        Item* temp_resultado = pesquisarArvoreBin(raizArvoreBin, chave_procurada, arquivoParaPesquisa, &analisePesquisa);
        if (temp_resultado != NULL) {
            resultado = *temp_resultado;
            free(temp_resultado);
            encontrado = true;
        }
        fclose(arquivoParaPesquisa);

        if (encontrado) {
            printf("[STATUS]: ITEM ENCONTRADO\n");
            printf("> Chave:  %d\n", resultado.chave);
            printf("> Dado1:  %ld\n", resultado.dado1);
            if(P_flag) printf("> Dado2:  %.50s...\n", resultado.dado2);
        } else {
            printf("[STATUS]: ITEM NAO ENCONTRADO (Chave %d)\n", chave_procurada);
        }

        printf("\n--- METRICAS DE PESQUISA ---\n");
        printf("Tempo Execucao: %.6f s\n", (double)analisePesquisa.tempoExecucao / 1000000.0);
        printf("Transferencias: %d\n", analisePesquisa.numTransferencias);
        printf("Comparacoes:    %d\n", analisePesquisa.numComparacoes);

    } else {
        printf("Modo experimental (10 buscas) nao formatado neste padrao.\n");
    }
    printf("========================================================\n");

    if (raizArvoreBin) liberarArvoreBin(raizArvoreBin);
}