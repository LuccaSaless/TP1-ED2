// Garante suporte a offsets de 64 bits para arquivos grandes (>2GB)
#define _FILE_OFFSET_BITS 64

#include "arvoreBinaria.h"
#include <time.h>

// Estrutura do nó da árvore binária em memória
typedef struct No {
    int chave;
    long int offset; // Posição do registro no arquivo
    struct No *esq;
    struct No *dir;
} No;

// Cria um novo nó
No* criarNo(int chave, long int offset) {
    No *novo = (No*)malloc(sizeof(No));
    if(novo) {
        novo->chave = chave;
        novo->offset = offset;
        novo->esq = NULL;
        novo->dir = NULL;
    }
    return novo;
}

// Libera a memória da árvore recursivamente
void liberarArvore(No *raiz) {
    if(raiz) {
        liberarArvore(raiz->esq);
        liberarArvore(raiz->dir);
        free(raiz);
    }
}

// Insere um nó na árvore (contabiliza comparações)
No* inserir(No *raiz, int chave, long int offset, AnaliseExperimental *analise) {
    if(raiz == NULL) return criarNo(chave, offset);
    
    analise->numComparacoes++;
    if(chave < raiz->chave)
        raiz->esq = inserir(raiz->esq, chave, offset, analise);
    else
        raiz->dir = inserir(raiz->dir, chave, offset, analise);
        
    return raiz;
}

// Constrói o índice (árvore) lendo o arquivo sequencialmente
No* construirIndice(FILE *arquivo, int quantReg, AnaliseExperimental *analise) {
    No *raiz = NULL;
    Item item;
    rewind(arquivo); // Garante início do arquivo

    // Lê todos os registros para montar a árvore em memória
    for(int i = 0; i < quantReg; i++) {
        long int posAtual = ftell(arquivo);
        // Lê apenas o necessário se possível, mas aqui lemos o Item padrão
        if(fread(&item, sizeof(Item), 1, arquivo) != 1) break;
        
        analise->numTransferencias++; // Leitura sequencial do disco
        raiz = inserir(raiz, item.chave, posAtual, analise);
    }
    return raiz;
}

// Realiza a pesquisa na árvore e busca o dado final no disco
bool pesquisar(No *raiz, Item *itemP, FILE *arquivo, AnaliseExperimental *analise) {
    if(raiz == NULL) return false;

    analise->numComparacoes++;
    if(itemP->chave == raiz->chave) {
        // Chave encontrada no índice: busca registro completo no arquivo
        fseek(arquivo, raiz->offset, SEEK_SET);
        fread(itemP, sizeof(Item), 1, arquivo);
        analise->numTransferencias++; // Acesso direto ao disco
        return true;
    }

    if(itemP->chave < raiz->chave)
        return pesquisar(raiz->esq, itemP, arquivo, analise);
    else
        return pesquisar(raiz->dir, itemP, arquivo, analise);
}

// Função Wrapper principal chamada pelo main/experimento
bool arvoreBinaria(FILE *arquivo, int quantReg, Item *itemP, AnaliseExperimental *analise) {
    // 1. Fase de Construção do Índice
    // O tempo de construção não é somado em 'analise->tempoExecucao' aqui,
    // mas transferências e comparações são acumuladas.
    No *raiz = construirIndice(arquivo, quantReg, analise);

    // 2. Fase de Pesquisa
    clock_t inicio = clock();
    
    bool encontrado = pesquisar(raiz, itemP, arquivo, analise);
    
    clock_t fim = clock();
    // Armazena o tempo apenas da pesquisa (similar ao sequencialIndex)
    analise->tempoExecucao = (double)(fim - inicio) / CLOCKS_PER_SEC;

    liberarArvore(raiz);
    return encontrado;
}