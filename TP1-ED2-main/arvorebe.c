// OBRIGATÓRIO PARA ARQUIVOS > 2GB
#define _FILE_OFFSET_BITS 64 

#include "arvorebe.h"
#include <string.h> // Memmove e Memcpy
#include <time.h>

// --- GERENCIAMENTO DE MEMÓRIA ---

TipoApontadorBE CriaNoBE(bool folha) {
    TipoApontadorBE novo = (TipoApontadorBE)calloc(1, sizeof(TipoPaginaBE));
    if (!novo) { 
        perror("[FATAL] Erro ao alocar nó B*"); 
        exit(EXIT_FAILURE); 
    }
    // Como usamos calloc, n=0 e ponteiros=NULL já estão inicializados
    return novo;
}

void InicializaArvoreBEst(TipoApontadorBE* Arvore) {
    *Arvore = NULL;
}

void LiberaArvoreBE(TipoApontadorBE Ap) {
    if (Ap == NULL) return;

    // Se não for folha (verificado pelo primeiro ponteiro de filho), libera recursivamente
    if (Ap->p[0] != NULL) { 
        for (int i = 0; i <= Ap->n; i++) {
            LiberaArvoreBE(Ap->p[i]);
        }
    }
    free(Ap);
}

// --- PESQUISA ---

TipoOffset PesquisaBE(int chave, TipoApontadorBE Ap, int *comp) {
    if (Ap == NULL) return -1;

    int i = 0;
    // Busca a posição da chave no nó atual
    while (i < Ap->n && chave > Ap->chaves[i]) {
        (*comp)++;
        i++;
    }

    (*comp)++; // Comparação de igualdade
    if (i < Ap->n && chave == Ap->chaves[i]) {
        return Ap->offsets[i]; // Encontrou
    }

    // Busca recursiva no filho apropriado
    return PesquisaBE(chave, Ap->p[i], comp);
}

// --- LÓGICA CORE DA B* (REDISTRIBUIÇÃO E SPLIT) ---

// Transfere chave: Origem(i) -> Pai -> Destino(i-1) (Irmão Esquerdo)
void RedistribuiEsquerda(TipoApontadorBE pai, int i) {
    TipoApontadorBE origem  = pai->p[i];     // Nó que está cheio
    TipoApontadorBE destino = pai->p[i-1];   // Irmão esquerdo (tem espaço)

    // 1. Desce a chave separadora do Pai para o final do Destino
    destino->chaves[destino->n]   = pai->chaves[i-1];
    destino->offsets[destino->n]  = pai->offsets[i-1];
    destino->p[destino->n + 1]    = origem->p[0]; // O primeiro filho da origem vira o último do destino
    destino->n++;

    // 2. Sobe a primeira chave da Origem para o Pai (substituindo a que desceu)
    pai->chaves[i-1]  = origem->chaves[0];
    pai->offsets[i-1] = origem->offsets[0];

    // 3. Remove a primeira chave da Origem (Shift Left para cobrir o buraco)
    memmove(&origem->chaves[0],  &origem->chaves[1],  (origem->n - 1) * sizeof(int));
    memmove(&origem->offsets[0], &origem->offsets[1], (origem->n - 1) * sizeof(TipoOffset));
    memmove(&origem->p[0],       &origem->p[1],       origem->n * sizeof(TipoApontadorBE));
    
    origem->n--;
}

// Transfere chave: Origem(i) -> Pai -> Destino(i+1) (Irmão Direito)
void RedistribuiDireita(TipoApontadorBE pai, int i) {
    TipoApontadorBE origem  = pai->p[i];     // Nó que está cheio
    TipoApontadorBE destino = pai->p[i+1];   // Irmão direito (tem espaço)

    // 1. Abre espaço no início do Destino (Shift Right)
    memmove(&destino->chaves[1],  &destino->chaves[0],  destino->n * sizeof(int));
    memmove(&destino->offsets[1], &destino->offsets[0], destino->n * sizeof(TipoOffset));
    memmove(&destino->p[1],       &destino->p[0],       (destino->n + 1) * sizeof(TipoApontadorBE));

    // 2. Desce a chave separadora do Pai para o início do Destino
    destino->chaves[0]  = pai->chaves[i];
    destino->offsets[0] = pai->offsets[i];
    destino->p[0]       = origem->p[origem->n]; // Último filho da origem vira o primeiro do destino
    destino->n++;

    // 3. Sobe a última chave da Origem para o Pai
    pai->chaves[i]  = origem->chaves[origem->n - 1];
    pai->offsets[i] = origem->offsets[origem->n - 1];

    // 4. Remove a última chave da Origem (apenas decrementa contador)
    origem->n--;
}

// Divide o Filho[i] do Pai em dois (Split 1-para-2)
// Só é chamado se a redistribuição falhar
void SplitFilho(TipoApontadorBE pai, int i) {
    TipoApontadorBE filho = pai->p[i];
    TipoApontadorBE novo  = CriaNoBE(false); 
    novo->n = M_ESTRELA - 1;

    // 1. Transfere metade direita do filho para o novo nó
    memcpy(novo->chaves,  &filho->chaves[M_ESTRELA + 1],  (M_ESTRELA - 1) * sizeof(int));
    memcpy(novo->offsets, &filho->offsets[M_ESTRELA + 1], (M_ESTRELA - 1) * sizeof(TipoOffset));
    memcpy(novo->p,       &filho->p[M_ESTRELA + 1],       M_ESTRELA * sizeof(TipoApontadorBE));

    filho->n = M_ESTRELA; // Ajusta tamanho do original

    // 2. Abre espaço no Pai para receber a chave mediana
    memmove(&pai->p[i + 2],       &pai->p[i + 1],       (pai->n - i) * sizeof(TipoApontadorBE));
    memmove(&pai->chaves[i + 1],  &pai->chaves[i],      (pai->n - i) * sizeof(int));
    memmove(&pai->offsets[i + 1], &pai->offsets[i],     (pai->n - i) * sizeof(TipoOffset));

    // 3. Sobe a mediana e conecta o novo nó
    pai->p[i + 1]     = novo;
    pai->chaves[i]    = filho->chaves[M_ESTRELA];
    pai->offsets[i]   = filho->offsets[M_ESTRELA];
    pai->n++;
}

// --- INSERÇÃO ---

void InsereNaoCheio(TipoApontadorBE x, int chave, TipoOffset offset, int *comp) {
    int i = x->n - 1;

    // CASO 1: NÓ É FOLHA (Inserção direta)
    if (x->p[0] == NULL) {
        while (i >= 0 && chave < x->chaves[i]) {
            (*comp)++;
            x->chaves[i + 1]  = x->chaves[i];
            x->offsets[i + 1] = x->offsets[i];
            i--;
        }
        if (i >= 0) (*comp)++; 

        x->chaves[i + 1]  = chave;
        x->offsets[i + 1] = offset;
        x->n++;
    } 
    // CASO 2: NÓ INTERNO (Decidir descida)
    else {
        while (i >= 0 && chave < x->chaves[i]) {
            (*comp)++;
            i--;
        }
        if (i >= 0) (*comp)++;
        i++; // Índice do filho onde a chave deve ir

        // === LÓGICA B-STAR: PROATIVIDADE ===
        // Se o filho onde vamos descer está cheio, tentamos resolver AGORA.
        if (x->p[i]->n == MM_ESTRELA) {
            
            // A. Tenta Redistribuir para a Esquerda (Overflow)
            if (i > 0 && x->p[i-1]->n < MM_ESTRELA) {
                RedistribuiEsquerda(x, i); // Move de [i] para [i-1]
                InsereNaoCheio(x->p[i], chave, offset, comp);
            }
            // B. Tenta Redistribuir para a Direita (Overflow)
            else if (i < x->n && x->p[i+1]->n < MM_ESTRELA) {
                RedistribuiDireita(x, i); // Move de [i] para [i+1]
                InsereNaoCheio(x->p[i], chave, offset, comp);
            }
            // C. Vizinhos cheios? Faz Split (Divisão)
            else {
                SplitFilho(x, i);
                
                // Após split, a chave mediana subiu. Ver qual lado descer.
                if (chave > x->chaves[i]) {
                    i++;
                }
                InsereNaoCheio(x->p[i], chave, offset, comp);
            }
        } else {
            // Filho tem espaço, desce normalmente
            InsereNaoCheio(x->p[i], chave, offset, comp);
        }
    }
}

void InsereBEst(int chave, TipoOffset offset, TipoApontadorBE* Arvore, int* comp) {
    if (*Arvore == NULL) {
        *Arvore = CriaNoBE(true);
        (*Arvore)->chaves[0] = chave;
        (*Arvore)->offsets[0] = offset;
        (*Arvore)->n = 1;
        return;
    }

    TipoApontadorBE raiz = *Arvore;

    // Se a raiz encheu, é o único caso onde a árvore cresce em altura imediatamente
    // (Raiz não tem irmãos para redistribuir)
    if (raiz->n == MM_ESTRELA) {
        TipoApontadorBE novaRaiz = CriaNoBE(false);
        novaRaiz->p[0] = raiz;
        *Arvore = novaRaiz;
        
        SplitFilho(novaRaiz, 0);
        InsereNaoCheio(novaRaiz, chave, offset, comp);
    } else {
        InsereNaoCheio(raiz, chave, offset, comp);
    }
}

// --- DRIVER ---

void RodaArvoreBEstrela(const char* nomeArq, int chave_procurada, int quantReg, bool P_flag) {
    TipoApontadorBE arvore;
    Item reg;
    int comp_criacao = 0;
    
    InicializaArvoreBEst(&arvore);
    
    FILE* arq = fopen(nomeArq, "rb");
    if (!arq) { 
        fprintf(stderr, "[ERRO] Nao foi possivel abrir o arquivo: %s\n", nomeArq);
        return; 
    }

    printf("\n========================================================\n");
    printf("METODO: 4 - ARVORE B* (ESTRELA)\n");
    printf("========================================================\n");

    // --- FASE 1: CRIACAO ---
    clock_t inicio = clock();
    while(true) {
        TipoOffset pos = ftello(arq);
        if (fread(&reg, sizeof(Item), 1, arq) != 1) break;
        InsereBEst(reg.chave, pos, &arvore, &comp_criacao);
    }
    double tempo_criacao = (double)(clock() - inicio) / CLOCKS_PER_SEC;

    printf("--- FASE 1: CRIACAO DO INDICE ---\n");
    printf("Tempo Execucao: %.6f s\n", tempo_criacao);
    printf("Transferencias: %d (Sequencial)\n", quantReg); 
    printf("Comparacoes:    %d\n", comp_criacao);

    // --- FASE 2: PESQUISA ---
    printf("\n--- FASE 2: PESQUISA ---\n");
    
    if (chave_procurada != -1) {
        int comp_pesquisa = 0;
        inicio = clock();
        
        TipoOffset offset = PesquisaBE(chave_procurada, arvore, &comp_pesquisa);
        
        bool encontrado = false;
        if (offset != -1) {
            fseeko(arq, offset, SEEK_SET); // fseeko para 64 bits
            fread(&reg, sizeof(Item), 1, arq);
            encontrado = true;
        }
        
        double tempo_pesquisa = (double)(clock() - inicio) / CLOCKS_PER_SEC;

        if (encontrado) {
            printf("[STATUS]: ITEM ENCONTRADO\n");
            printf("> Chave:  %d\n", reg.chave);
            printf("> Dado1:  %ld\n", reg.dado1);
            if(P_flag) printf("> Dado2:  %.50s...\n", reg.dado2);
        } else {
            printf("[STATUS]: ITEM NAO ENCONTRADO (Chave %d)\n", chave_procurada);
        }

        printf("\n--- METRICAS DE PESQUISA ---\n");
        printf("Tempo Execucao: %.6f s\n", tempo_pesquisa);
        printf("Transferencias: %d (Acesso Direto)\n", encontrado ? 1 : 0);
        printf("Comparacoes:    %d\n", comp_pesquisa);
    } else {
         printf("Modo experimental nao ativado.\n");
    }
    printf("========================================================\n");

    fclose(arq);
    LiberaArvoreBE(arvore);
}