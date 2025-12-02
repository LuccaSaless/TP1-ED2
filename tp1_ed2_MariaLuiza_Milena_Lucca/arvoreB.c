#define _FILE_OFFSET_BITS 64
#include "arvoreB.h"


// --- GESTÃO DE MEMÓRIA ---

B_No* B_criaNo(bool folha) {
    B_No* no = (B_No*)calloc(1, sizeof(B_No));
    if (!no) exit(EXIT_FAILURE);
    no->folha = folha;
    return no;
}

void B_liberaArvore(B_No* no) {
    if (no == NULL) return;
    if (!no->folha) {
        for (int i = 0; i <= no->qtdChaves; i++) {
            B_liberaArvore(no->filhos[i]);
        }
    }
    free(no);
}

// --- LÓGICA DA ÁRVORE B (INSTRUMENTADA) ---

FilePos B_pesquisa(B_No* raiz, int chave, AnaliseExperimental* analise) {
    if (raiz == NULL) return -1;

    int i = 0;
    // Busca sequencial dentro do nó
    while (i < raiz->qtdChaves && chave > raiz->chaves[i]) {
        analise->numComparacoes++; 
        i++;
    }

    // Verifica se encontrou (última comparação do while ou comparação direta)
    if (i < raiz->qtdChaves) {
        analise->numComparacoes++; // Comparação de igualdade
        if (chave == raiz->chaves[i])
            return raiz->offsets[i];
    }

    if (raiz->folha) return -1;

    return B_pesquisa(raiz->filhos[i], chave, analise);
}

void B_splitFilho(B_No* pai, int i, B_No* filho, AnaliseExperimental* analise) {
    B_No* novo = B_criaNo(filho->folha);
    novo->qtdChaves = M - 1;

    // Copia chaves e offsets (operações de memória, não contam como comparação de chave)
    memcpy(novo->chaves, &filho->chaves[M + 1], (M - 1) * sizeof(int));
    memcpy(novo->offsets, &filho->offsets[M + 1], (M - 1) * sizeof(FilePos));

    if (!filho->folha) {
        memcpy(novo->filhos, &filho->filhos[M + 1], M * sizeof(B_No*));
    }

    filho->qtdChaves = M;

    // Move filhos e chaves no pai para abrir espaço
    memmove(&pai->filhos[i + 2], &pai->filhos[i + 1], (pai->qtdChaves - i) * sizeof(B_No*));
    memmove(&pai->chaves[i + 1],  &pai->chaves[i],      (pai->qtdChaves - i) * sizeof(int));
    memmove(&pai->offsets[i + 1], &pai->offsets[i],     (pai->qtdChaves - i) * sizeof(FilePos));

    pai->filhos[i + 1] = novo;
    pai->chaves[i]     = filho->chaves[M];
    pai->offsets[i]    = filho->offsets[M];
    pai->qtdChaves++;
}

void B_insereNaoCheio(B_No* no, int chave, FilePos offset, AnaliseExperimental* analise) {
    int i = no->qtdChaves - 1;

    if (no->folha) {
        while (i >= 0 && chave < no->chaves[i]) {
            analise->numComparacoes++;
            no->chaves[i + 1]  = no->chaves[i];
            no->offsets[i + 1] = no->offsets[i];
            i--;
        }
        // Conta a comparação que falhou no while (ou se i<0 não conta, mas vamos simplificar)
        if(i >= 0) analise->numComparacoes++;

        no->chaves[i + 1]  = chave;
        no->offsets[i + 1] = offset;
        no->qtdChaves++;
    } 
    else {
        while (i >= 0 && chave < no->chaves[i]) {
            analise->numComparacoes++;
            i--;
        }
        if(i >= 0) analise->numComparacoes++;
        i++;

        if (no->filhos[i]->qtdChaves == MAX) {
            B_splitFilho(no, i, no->filhos[i], analise);
            
            analise->numComparacoes++; // Comparação para decidir qual lado ir após split
            if (chave > no->chaves[i]) {
                i++;
            }
        }
        B_insereNaoCheio(no->filhos[i], chave, offset, analise);
    }
}

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
        
        B_splitFilho(novaRaiz, 0, raiz, analise);
        B_insereNaoCheio(novaRaiz, chave, offset, analise);
    } else {
        B_insereNaoCheio(raiz, chave, offset, analise);
    }
}

// --- FUNÇÃO PRINCIPAL DE PESQUISA (INTERFACE PADRÃO) ---

bool pesquisaArvoreB(FILE *arquivo, int quantReg, Item *itemP, AnaliseExperimental *analise) {
    ArvoreB arvore = NULL;
    Item aux;
    
    clock_t inicio = clock();

    // 1. Construção do Índice (Carrega todo o arquivo para memória RAM na estrutura de árvore)
    rewind(arquivo);
    while (true) {
        FilePos pos = ftello(arquivo);
        // Leitura sequencial para montar a árvore
        if (fread(&aux, sizeof(Item), 1, arquivo) != 1) break;
        
        analise->numTransferencias++; // Contabiliza a leitura do disco para a RAM
        B_inserir(&arvore, aux.chave, pos, analise);
    }

    // 2. Pesquisa na Árvore
    FilePos offsetEncontrado = B_pesquisa(arvore, itemP->chave, analise);

    bool encontrado = false;
    if (offsetEncontrado != -1) {
        // 3. Recuperação do Dado (Acesso direto)
        fseeko(arquivo, offsetEncontrado, SEEK_SET);
        fread(itemP, sizeof(Item), 1, arquivo); // Lê o item final para retorno
        analise->numTransferencias++; // Contabiliza o acesso final
        encontrado = true;
    }

    // Limpeza (Crucial para testes repetitivos)
    B_liberaArvore(arvore);

    clock_t fim = clock();
    analise->tempoExecucao = (double)(fim - inicio) / CLOCKS_PER_SEC;

    return encontrado;
}