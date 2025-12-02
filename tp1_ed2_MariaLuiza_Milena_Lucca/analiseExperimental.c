#include "analiseExperimental.h"

//Imprime os dados da análise experimental
void imprimeAnalise(AnaliseExperimental *analise, Item *itemP){

    printf("========================================================\n");

    printf("Item com chave %d encontrado! \nDado 1 = %ld \nDado 2 = %.30s...\n", itemP->chave, itemP->dado1, itemP->dado2);
    
    printf("========================================================\n");

    printf("Análise Experimental:\n");
    printf("Número de transferências: %d\n", analise->numTransferencias);
    printf("Número de comparações: %d\n", analise->numComparacoes);
    printf("Tempo de execução: %.6f segundos\n", analise->tempoExecucao);

    printf("========================================================\n");
}

#include <stdbool.h>
#include <stdlib.h>

// Verifica se uma chave já foi gerada
bool chaveExiste(int chaves[], int usados, int chave) {
    for (int i = 0; i < usados; i++) {
        if (chaves[i] == chave) return true;
    }
    return false;
}

// Gera 20 chaves distribuídas, aleatórias e sem repetição
void gerarChaves(int quantReg, int chaves[20]) {
    int passo = quantReg / 20;
    if (passo == 0) passo = 1; // Para arquivos muito pequenos

    for (int i = 0; i < 20; i++) {

        int inicio = i * passo + 1;
        int fim = (i == 19 ? quantReg : (i + 1) * passo);

        int chave;

        // Garante que não repete chave
        do {
            chave = inicio + rand() % (fim - inicio + 1);
        } while (chaveExiste(chaves, i, chave));

        chaves[i] = chave;
    }
}


//Executa o experimento de pesquisa 20 vezes e calcula a média dos resultados
void executarExperimento(FILE *arquivo, int quantReg, AnaliseExperimental *media, bool (*funcaoPesquisa)(FILE*, int, Item*, AnaliseExperimental*)){
    int chaves[20];
    gerarChaves(quantReg, chaves);

    media->numComparacoes = 0;
    media->numTransferencias = 0;
    media->tempoExecucao = 0;

    for (int i = 0; i < 20; i++) {

        AnaliseExperimental parcial = {0,0,0};

        Item itemP;
        itemP.chave = chaves[i];

        rewind(arquivo); 

        clock_t ini = clock();

        funcaoPesquisa(arquivo, quantReg, &itemP, &parcial);

        clock_t fim = clock();
        parcial.tempoExecucao = (double)(fim - ini) / CLOCKS_PER_SEC;

        media->numComparacoes += parcial.numComparacoes;
        media->numTransferencias += parcial.numTransferencias;
        media->tempoExecucao += parcial.tempoExecucao;
    }

    //Médias
    media->numComparacoes /= 20;
    media->numTransferencias /= 20;
    media->tempoExecucao /= 20;
}


void imprimeMediaAnalise(AnaliseExperimental *media) {

    printf("========================================================\n");
    printf("     MÉDIA DAS 20 PESQUISAS EXPERIMENTAIS\n");
    printf("========================================================\n");

    printf("Média de transferências: %d\n", media->numTransferencias);
    printf("Média de comparações:    %d\n", media->numComparacoes);
    printf("Média do tempo:          %.6f segundos\n", media->tempoExecucao);

    printf("========================================================\n");
}
