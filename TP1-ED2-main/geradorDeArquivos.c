#include "geradorDeArquivos.h"

// Gera string aleatória para preencher os dados 1 e 2
void gerarString(char *str, int tamanho){
    for (int i = 0; i < tamanho - 1; i++){
        // Gera uma letra aleatória de 'a' a 'z'
        str[i] = 'a' + rand() % 26;
    }
    // Adiciona o \0 no final da string
    str[tamanho - 1] = '\0';
}

// Embaralha o vetor de chaves usando o algoritmo de Fisher-Yates
void embaralhar(int *vetor, int n){
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1); // Índice aleatório entre 0 e i
        int temp = vetor[i];
        vetor[i] = vetor[j];
        vetor[j] = temp;
    }
}

int gerarArquivo(int quantReg, int situacao, const char *nomeArq){
    // Semente para geração de números aleatórios
    srand(time(NULL));

    // Vetor para armazenar as chaves
    int *chaves = malloc(sizeof(int)*quantReg);

    // Item que iremos escrever no arquivo
    Item item;

    if (situacao == 2){
        for (int j = quantReg; j > 0; j--){
            chaves[quantReg - j] = j; // Preenche o vetor de chaves em ordem decrescente
        }
    }
    else{
        // Preenche o vetor com chaves de 1 a quantReg
        for (int i = 0; i < quantReg; i++){
            chaves[i] = i +1; 
        }
        if (situacao == 3){
            embaralhar(chaves, quantReg);
        }
        
    }

    FILE *arquivo = fopen(nomeArq, "wb");

    // Erro ao abrir o arquivo
    if (arquivo == NULL) {
        return 0; 
    }

    // Preenche o arquivo com os itens gerados
    for (int i = 0; i < quantReg; i++){
        item.chave = chaves[i];
        item.dado1 = rand(); // Número aletório
        //gerarString(item.dado2, sizeof(item.dado2));
        //gerarString(item.dado3, sizeof(item.dado3));
        gerarString(item.dado2, 3);
        gerarString(item.dado3, 5);
        fwrite(&item, sizeof(Item), 1, arquivo);
    }
    
    free(chaves);
    fclose(arquivo);
    return 1; // Sucesso
}

void mostraArquivo(const char *nomeArq){
    FILE *arquivo = fopen(nomeArq, "rb");
    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo %s\n", nomeArq);
        return;
    }

    Item item;
    while (fread(&item, sizeof(Item), 1, arquivo) == 1) {
        printf("Chave: %d, Dado1: %ld, Dado2: %s, Dado3: %s\n\n", item.chave, item.dado1, item.dado2, item.dado3);
    }

    fclose(arquivo);
}
