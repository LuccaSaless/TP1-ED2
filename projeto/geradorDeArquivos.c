#include "geradorDeArquivos.h"

// Gera string aleatória para preencher dado2 (até 4999 chars)
void gerarString(char *str, int tamanho){
    for (int i = 0; i < tamanho - 1; i++){
        str[i] = 'a' + rand() % 26;   // letra aleatória
    }
    str[tamanho - 1] = '\0';          // termina a string
}

// Embaralha o vetor de chaves usando Fisher-Yates
void embaralhar(int *vetor, int n){
    for (int i = n - 1; i > 0; i--){
        int j = rand() % (i + 1);
        int temp = vetor[i];
        vetor[i] = vetor[j];
        vetor[j] = temp;
    }
}

int gerarArquivo(int quantReg, int situacao, const char *nomeArq){
    srand(time(NULL));

    int *chaves = malloc(sizeof(int) * quantReg);
    if (chaves == NULL) return 0;

    Item item;

    // Situação 2 = decrescente
    if (situacao == 2){
        for (int j = quantReg; j > 0; j--){
            chaves[quantReg - j] = j;
        }
    } 
    else {
        // Situação 1 = crescente, Situação 3 = aleatório
        for (int i = 0; i < quantReg; i++){
            chaves[i] = i + 1;
        }

        if (situacao == 3){
            embaralhar(chaves, quantReg);
        }
    }

    FILE *arquivo = fopen(nomeArq, "wb");
    if (arquivo == NULL){
        free(chaves);
        return 0;
    }

    // Gerar registros
    for (int i = 0; i < quantReg; i++){
        item.chave = chaves[i];
        item.dado1 = rand();

        // Gera string grande (4999 chars úteis + '\0')
        gerarString(item.dado2, sizeof(item.dado2));

        fwrite(&item, sizeof(Item), 1, arquivo);
    }

    free(chaves);
    fclose(arquivo);
    return 1;
}

void mostraArquivo(const char *nomeArq){
    FILE *arquivo = fopen(nomeArq, "rb");
    if (arquivo == NULL){
        printf("Erro ao abrir o arquivo %s\n", nomeArq);
        return;
    }

    Item item;

    while (fread(&item, sizeof(Item), 1, arquivo) == 1){
        printf("Chave: %d\n", item.chave);
        printf("Dado1: %ld\n", item.dado1);
        printf("Dado2 (primeiros 80 chars): %.80s...\n\n", item.dado2);
    }

    fclose(arquivo);
}
