#include "sequencialIndex.h"

//Cálculo do tamanho da página para melhorar a performance
int tamanhoPagina(int quantReg){
    int tamanho = sqrt(quantReg); //Para ficar proporcional com a quantidade de registros

    if (tamanho < 50) tamanho = 50; //Limite mínimo de itens por página
    else if (tamanho > 2000) tamanho = 2000; //Limite máximo de itens por página

    return tamanho;
}

//Busca binária na tabela de índices para encontrar a página correta
int buscaIndice(Indice *tabela, int tamTabela, int chave, AnaliseExperimental *analise){
    int esq = 0, dir = tamTabela - 1, indice = tamTabela;

    while(esq <= dir){
        int meio = (esq + dir) / 2;

        if(tabela[meio].chave > chave){ //Busca na metade esquerda
            analise->numComparacoes++;
            indice = meio;
            dir = meio - 1;
        } else { //Busca na metade direita
            esq = meio + 1;
        }
    }

    return indice; //Retorna o índice da página onde a chave pode estar
}


bool pesquisaPagina(FILE *arquivo, Item *itemP, int tamTabela, int itensPag, Indice *tabela, AnaliseExperimental *analise){
    
     __clock_t inicio = clock(); //Medindo o tempo de pesquisa

    Item pag[itensPag]; //Para a leitura de uma página inteira

    int i = buscaIndice(tabela, tamTabela, itemP->chave, analise); //Encontra a página correta

    if (i == 0){
        return false; //Chave menor que a primeira chave da tabela
    }

    //Quantidade de registros caso a última página não esteja completa
    fseek(arquivo, 0, SEEK_END);
    long tamArquivo = ftell(arquivo);

    if (tamArquivo == 0){
        return false; //Arquivo vazio
    }

    int quantidade;
    //Se não for a última página, lê uma página completa
    if(i < tamTabela){
        quantidade = itensPag; //A quantidade de itens é igual ao número de itens por página
    } else {
        quantidade = (tamArquivo / sizeof(Item)) % itensPag; //Quantidade de itens na última página
        if(quantidade == 0){
            quantidade = itensPag; //Se a última página estiver completa
        }
    }

    long desloc = (i - 1) * itensPag * sizeof(Item); //Calcula o deslocamento para a página correta

    rewind(arquivo); //Garante que a leitura comece do início do arquivo
    fseek(arquivo, desloc, SEEK_SET); //Move o ponteiro do arquivo para o início da página correta

    fread(pag, sizeof(Item), quantidade, arquivo); //Lê a página inteira
    analise->numTransferencias++; //Conta a transferência da página

    //Pesquisa binária dentro da página
    int esq = 0, dir = quantidade - 1;
    while (esq <= dir){
        int meio = (esq + dir) / 2;
        if (pag[meio].chave == itemP->chave){
            analise->numComparacoes++;
            *itemP = pag[meio]; //Copia o item encontrado para o ponteiro fornecido
            clock_t fim = clock(); //Medindo o tempo de pesquisa
            analise->tempoExecucao = ((double)(fim - inicio)) / CLOCKS_PER_SEC;
            return true; //Item encontrado
        } else if (pag[meio].chave < itemP->chave){ //Busca na metade direita
            analise->numComparacoes++;
            esq = meio + 1;
        } else { //Busca na metade esquerda
            dir = meio - 1;
        }
    }

    __clock_t fim = clock(); //Medindo o tempo de pesquisa
    analise->tempoExecucao = ((double)(fim - inicio)) / CLOCKS_PER_SEC;

    return false; //Item não encontrado na página
}

bool geraTabela(FILE *arquivo, int quantReg, int itensPag, Indice **tabela, int *tamTabela, AnaliseExperimental *analise){
    
    //Número de itens por página e tamanho da tabela de índices
    *tamTabela = (quantReg / itensPag) +  (quantReg % itensPag != 0); 
    
    *tabela = malloc((*tamTabela) * sizeof(Indice));
    if (*tabela == NULL){
        return false;
    }

    rewind(arquivo); //Para garantir que a leitura comece do início do arquivo

    Item aux; //Item auxiliar para leitura dos registros
    for (int i = 0; i < *tamTabela; i++){
        long deslocamento = i * itensPag * sizeof(Item); //Posição do primeiro item da página
        fseek(arquivo, deslocamento, SEEK_SET); //Move o ponteiro do arquivo para o início da página
        fread(&aux, sizeof(Item), 1, arquivo); //Lê o primeiro item da página
        analise->numTransferencias++; //Conta a transferência do item
        (*tabela)[i].chave = aux.chave; //Armazena a chave na tabela de índices
    }
    return true;
}

//Chama as funções necessárias para realizar a pesquisa indexada
bool indexado(FILE *arquivo, int quantReg, Item *itemP, AnaliseExperimental *analise){
    Indice *tabela = NULL;
    int itensPag = tamanhoPagina(quantReg);
    int tamTabela = 0; //Precisa guardar para chamar a pesquisa na página

    if (!geraTabela(arquivo, quantReg, itensPag, &tabela, &tamTabela, analise)){
        printf("Erro ao gerar tabela de índices.\n");
        return false;
    }

    if (!pesquisaPagina(arquivo, itemP, tamTabela, itensPag, tabela, analise)){
        free(tabela);
        return false;
    }
    
    free(tabela);
    return true;
}