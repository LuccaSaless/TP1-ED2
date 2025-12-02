// DEVE SER A PRIMEIRA LINHA (Suporte a arquivos > 2GB)
#define _FILE_OFFSET_BITS 64

#include "arvoreb.h"
#include "item.h"
#include <string.h> 
#include <time.h>

// --- FUNÇÕES AUXILIARES ---

// Aloca e inicializa um nó (calloc zera contadores e ponteiros)
B_No* B_criaNo(bool folha) {
    B_No* no = (B_No*)calloc(1, sizeof(B_No));
    if (!no) {
        perror("[FATAL] Falha ao alocar memoria para no");
        exit(EXIT_FAILURE);
    }
    no->folha = folha;
    return no;
}

// Libera memória da árvore recursivamente
void B_liberaArvore(B_No* no) {
    if (no == NULL) return;
    
    if (!no->folha) {
        for (int i = 0; i <= no->qtdChaves; i++) {
            B_liberaArvore(no->filhos[i]);
        }
    }
    free(no);
}

// --- LÓGICA DA ÁRVORE B ---

FilePos B_pesquisa(B_No* raiz, int chave, AnaliseExperimental* analise) {
    if (raiz == NULL) return -1;

    int i = 0;
    // Busca linear pela primeira chave maior ou igual (Binary Search seria melhor p/ M muito grande)
    while (i < raiz->qtdChaves && chave > raiz->chaves[i]) {
        analise->numComparacoes++;
        i++;
    }

    // Caso 1: Encontrou a chave
    analise->numComparacoes++; // Verifica igualdade
    if (i < raiz->qtdChaves && chave == raiz->chaves[i]) {
        return raiz->offsets[i];
    }

    // Caso 2: Não encontrou e é folha
    if (raiz->folha) return -1;

    // Caso 3: Desce para o filho apropriado
    return B_pesquisa(raiz->filhos[i], chave, analise);
}

// Divide um filho cheio (Split 1-para-2)
void B_splitFilho(B_No* pai, int i, B_No* filho) {
    B_No* novo = B_criaNo(filho->folha);
    novo->qtdChaves = M - 1;

    // 1. Move a metade superior das chaves/offsets para o novo nó
    memcpy(novo->chaves, &filho->chaves[M + 1], (M - 1) * sizeof(int));
    memcpy(novo->offsets, &filho->offsets[M + 1], (M - 1) * sizeof(FilePos));

    // 2. Se não for folha, move também os filhos correspondentes
    if (!filho->folha) {
        memcpy(novo->filhos, &filho->filhos[M + 1], M * sizeof(B_No*));
    }

    filho->qtdChaves = M; // Atualiza tamanho do nó original

    // 3. Abre espaço no PAI para a chave mediana (Shift Right)
    memmove(&pai->filhos[i + 2], &pai->filhos[i + 1], (pai->qtdChaves - i) * sizeof(B_No*));
    memmove(&pai->chaves[i + 1],  &pai->chaves[i],      (pai->qtdChaves - i) * sizeof(int));
    memmove(&pai->offsets[i + 1], &pai->offsets[i],     (pai->qtdChaves - i) * sizeof(FilePos));

    // 4. Sobe a mediana e conecta o novo nó
    pai->filhos[i + 1] = novo;
    pai->chaves[i]     = filho->chaves[M];
    pai->offsets[i]    = filho->offsets[M];
    pai->qtdChaves++;
}

// Insere em nó garantidamente não cheio
void B_insereNaoCheio(B_No* no, int chave, FilePos offset, AnaliseExperimental* analise) {
    int i = no->qtdChaves - 1;

    if (no->folha) {
        // Shift manual (loop) é substituído por memmove se soubermos o índice exato,
        // mas como precisamos comparar chaves, o loop reverso é necessário para achar a posição.
        // Otimização: Achar índice -> memmove -> inserir.
        
        while (i >= 0 && chave < no->chaves[i]) {
            analise->numComparacoes++;
            no->chaves[i + 1]  = no->chaves[i];
            no->offsets[i + 1] = no->offsets[i];
            i--;
        }
        if (i >= 0) analise->numComparacoes++;

        no->chaves[i + 1]  = chave;
        no->offsets[i + 1] = offset;
        no->qtdChaves++;
    } 
    else {
        // Nó Interno: Encontrar filho para descer
        while (i >= 0 && chave < no->chaves[i]) {
            analise->numComparacoes++;
            i--;
        }
        if (i >= 0) analise->numComparacoes++;
        i++;

        // Estratégia Proativa (Top-Down): Se o filho estiver cheio, divide antes de entrar
        if (no->filhos[i]->qtdChaves == MAX) {
            B_splitFilho(no, i, no->filhos[i]);
            
            // Após o split, verifica qual dos dois novos filhos recebe a chave
            if (chave > no->chaves[i]) {
                i++;
            }
        }
        B_insereNaoCheio(no->filhos[i], chave, offset, analise);
    }
}

// Função Principal de Inserção
void B_inserir(ArvoreB* arvore, int chave, FilePos offset, AnaliseExperimental* analise) {
    if (*arvore == NULL) {
        *arvore = B_criaNo(true);
        (*arvore)->chaves[0] = chave;
        (*arvore)->offsets[0] = offset;
        (*arvore)->qtdChaves = 1;
        return;
    }

    B_No* raiz = *arvore;

    if (raiz->qtdChaves == MAX) {
        B_No* novaRaiz = B_criaNo(false);
        novaRaiz->filhos[0] = raiz;
        *arvore = novaRaiz;
        
        B_splitFilho(novaRaiz, 0, raiz);
        B_insereNaoCheio(novaRaiz, chave, offset, analise);
    } else {
        B_insereNaoCheio(raiz, chave, offset, analise);
    }
}

// --- DRIVER DE EXECUÇÃO ---

void RodaArvoreB(const char* nomeArq, int chave_procurada, int quantReg, bool P_flag) {
    ArvoreB arvore = NULL;
    Item registro;
    AnaliseExperimental stats = {0};

    // 1. Abertura do Arquivo
    FILE* arq = fopen(nomeArq, "rb");
    if (!arq) { 
        fprintf(stderr, "Erro ao abrir arquivo: %s\n", nomeArq); 
        return; 
    }

    printf("\n========================================================\n");
    printf("METODO: 3 - ARVORE B (Padrao)\n");
    printf("========================================================\n");

    // 2. Criação do Índice
    clock_t inicio = clock();
    while (true) {
        FilePos pos = ftello(arq);
        if (fread(&registro, sizeof(Item), 1, arq) != 1) break;
        
        // if (quantReg > 0 && stats.numTransferencias >= quantReg) break; // Opcional
        
        B_inserir(&arvore, registro.chave, pos, &stats);
        stats.numTransferencias++; // Contando leituras sequenciais
    }
    double tempo_criacao = (double)(clock() - inicio) / CLOCKS_PER_SEC;

    printf("--- FASE 1: CRIACAO DO INDICE ---\n");
    printf("Tempo Execucao: %.6f s\n", tempo_criacao);
    printf("Transferencias: %d\n", stats.numTransferencias);
    printf("Comparacoes:    %d\n", stats.numComparacoes);

    // 3. Pesquisa
    printf("\n--- FASE 2: PESQUISA ---\n");
    
    // Reset para métricas de pesquisa
    int comp_pesquisa = 0; 
    int transf_pesquisa = 0;
    
    if (chave_procurada != -1) {
        inicio = clock();
        
        FilePos offset = B_pesquisa(arvore, chave_procurada, &stats);
        comp_pesquisa = stats.numComparacoes - comp_pesquisa; // Ajuste se quiser isolar, ou use struct nova

        bool encontrado = false;
        if (offset != -1) {
            // Acesso direto ao disco (Random Access)
            fseeko(arq, offset, SEEK_SET);
            fread(&registro, sizeof(Item), 1, arq);
            transf_pesquisa = 1;
            encontrado = true;
        }
        
        double tempo_pesquisa = (double)(clock() - inicio) / CLOCKS_PER_SEC;

        if (encontrado) {
            printf("[STATUS]: ITEM ENCONTRADO\n");
            printf("> Chave:  %d\n", registro.chave);
            printf("> Dado1:  %ld\n", registro.dado1);
            if(P_flag) printf("> Dado2:  %.50s...\n", registro.dado2);
        } else {
            printf("[STATUS]: ITEM NAO ENCONTRADO (Chave %d)\n", chave_procurada);
        }

        printf("\n--- METRICAS DE PESQUISA ---\n");
        printf("Tempo Execucao: %.6f s\n", tempo_pesquisa);
        printf("Transferencias: %d (Acesso Direto)\n", transf_pesquisa);
        // Nota: stats.numComparacoes acumula criação + pesquisa. 
        // Se quiser apenas da pesquisa, deve-se zerar antes ou subtrair.
        printf("Comparacoes:    %d (Total acumulado)\n", stats.numComparacoes);
    } else {
        printf("Modo experimental nao ativado.\n");
    }

    printf("========================================================\n");

    fclose(arq);
    B_liberaArvore(arvore);
}