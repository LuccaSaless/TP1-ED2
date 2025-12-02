#include "index.h"
#include "item.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>


// Cálculo do tamanho da página
void tamPag(int *itensPag, int quantReg) {
    int ordem = log10(quantReg);
    int divisor = pow(10, ordem - 1);
    *itensPag = quantReg / divisor;
    if (*itensPag < 1)
        *itensPag = 1;
}

// Pesquisa na página determinada
int pesquisaPagina(Indice *tabela, int tamTabela, Item *itemP, FILE *arq, int *comparacoes, int itensPag) {
    int i = 0, quant;
    long desloc;
    Item pag[itensPag]; // Para a leitura de uma página completa do arquivo

    // Busca pela página correta
    while (i < tamTabela && tabela[i].chave <= itemP->chave)
        i++;

    // Se não encontrou
    if (i == 0)
        return 0;

    // Quantidade de registros da página para caso de a última página não ter o número completo de itens
    fseek(arq, 0, SEEK_END);
    long tamArquivo = ftell(arq);

    if (tamArquivo == 0) // Se o arquivo estiver vazio
        return 0;

    // Se não for a última página
    if (i < tamTabela) { 
        quant = itensPag; // A quantidade de itens é igual ao tamanho da página
    } else { // Se for a última página
        quant = (tamArquivo / sizeof(Item)) % itensPag; // Calcula a quantidade de itens na última página
        if (quant == 0) // Se a quantidade for zero, significa que a última página está completa
            quant = itensPag;
    }

    desloc = (i - 1) * itensPag * sizeof(Item); // Calcula o deslocamento para a página correta (anterior a i)
     // Se o deslocamento for maior ou igual ao tamanho do arquivo, não há registros para ler
    if (desloc >= tamArquivo)
        return 0;

    rewind(arq); // Garante que o ponteiro do arquivo volte para o início
    fseek(arq, desloc, SEEK_SET); // Move o ponteiro do arquivo para o início da página correta
    fread(pag, sizeof(Item), quant, arq);

    // Pesquisa binária na página
    int esq = 0, dir = quant - 1;
    while (esq <= dir) {
        (*comparacoes)++; // Incrementa o contador de comparações
         // Calcula o meio da página
        int meio = (esq + dir) / 2;
        
        // Verifica se o item do meio é o que estamos procurando
        if (pag[meio].chave == itemP->chave) {
            *itemP = pag[meio];
            return 1;
        } else if (pag[meio].chave < itemP->chave) { // Se o item do meio é menor que o item procurado, busca na metade direita
            esq = meio + 1;
        } else {
            dir = meio - 1; // Se o item do meio é maior que o item procurado, busca na metade esquerda
        }
    }

    return 0;
}

int Indexado(Item *itemP, FILE *arq, AnaliseExperimental *analise, int quantReg) {
    int itensPag, tamTabela, comparacoes = 0, transferencias = 0;
    Item aux; // Item auxiliar para leitura do arquivo

    tamPag(&itensPag, quantReg);

    // Calcula o tamanho (quantidade de páginas) da tabela de índices e adiciona uma página extra se necessário 
    tamTabela = (quantReg / itensPag) + (quantReg % itensPag != 0);

    // Aloca memória para a tabela de índices de acordo com o tamanho calculado
    Indice *tabela = malloc(tamTabela * sizeof(Indice));
    if (!tabela) {
        printf("Erro de alocação da tabela de índices.\n");
        exit(1);
    }
    // Para garantir que o ponteiro do arquivo esteja no início
    rewind(arq);
    for (int i = 0; i < tamTabela; i++) {
        long deslocamento = i * itensPag * sizeof(Item);  // Posição do primeiro item da página i
        fseek(arq, deslocamento, SEEK_SET); // Move o ponteiro do arquivo para o primeiro item da página i
        fread(&aux, sizeof(Item), 1, arq);  // Lê apenas o primeiro item da página
        transferencias++;  // Conta a transferência de leitura
        tabela[i].chave = aux.chave;
    }

    // Clock para medir o tempo de execução da pesquisa
    clock_t inicio = clock();
    int encontrado = pesquisaPagina(tabela, tamTabela, itemP, arq, &comparacoes, itensPag);
    clock_t fim = clock();

    analise->numTransferencias = transferencias;
    analise->numComparacoes = comparacoes;
    analise->tempoExecucao = (long)(fim - inicio);

    free(tabela);
    return encontrado;
}

void RodaIndexadoExperimento(FILE *arq, int quantReg) {
    AnaliseExperimental analise_total = {0, 0};
    double tempo_total = 0.0;
    srand(time(NULL)); // apenas uma vez no início

    int chaves[10];
    for (int i = 0; i < 10; i++){
        chaves[i] = (rand() % quantReg) + 1;
        printf("%d ", chaves[i]);
    } 

    for (int i = 0; i < 10; i++) {
        AnaliseExperimental analise_temp = {0, 0};
        Item item;
        item.chave = chaves[i];

        rewind(arq);
        clock_t inicio = clock();
        Indexado(&item, arq, &analise_temp, quantReg);
        clock_t fim = clock();

        analise_total.numComparacoes += analise_temp.numComparacoes;
        analise_total.numTransferencias += analise_temp.numTransferencias;
        tempo_total += ((double)(fim - inicio)) / CLOCKS_PER_SEC;
    }

    printf("\nAnálise dos dados! (Média de 10 buscas)\n");
    printf("Tempo Médio de Pesquisa: %.4f s\n", tempo_total / 10.0);
    printf("Transferências Médias: %.2f\n", (double)analise_total.numTransferencias / 10.0);
    printf("Comparações Médias: %.2f\n", (double)analise_total.numComparacoes / 10.0);
}