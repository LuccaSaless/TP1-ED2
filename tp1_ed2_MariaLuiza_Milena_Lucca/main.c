#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "item.h"
#include "sequencialIndex.h"
#include "arvoreB.h"
#include "arvoreBe.h"
#include "arvoreBin.h"
#include "geradorDeArquivos.h"
#include "analiseExperimental.h"

int main(int argc, char *argv[]){

    // Verificando se o número de argumentos está correto
    if (argc < 5 || argc > 6) {
        printf("Uso: %s <método> <quantidade> <situação> <chave> [-P]\n", argv[0]);
        return 1;
    }

    // Variáveis para armazenar os parâmetros de entrada
    int metodo = atoi(argv[1]); 
    int quantReg = atoi(argv[2]);
    int situacao = atoi(argv[3]);
    Item itemP;
    itemP.chave = atoi(argv[4]); // Chave a ser pesquisada
    bool P = false; //Indica se a opção -P foi fornecida

    AnaliseExperimental analise; //Aramazena os dados da análise experimental
    //Inicializando os valores da análise experimental para evitar lixo de memória
    analise.numComparacoes = 0;
    analise.numTransferencias = 0;
    analise.tempoExecucao = 0.0;

    // Tratando a opção -P para funcionar tanto com maiúscula quanto minúscula
    if(argc == 6 && ((strcmp(argv[5], "-P") == 0) || (strcmp(argv[5], "-p") == 0))) P = true;

    // Gerando o nome do arquivo com base na quantidade e situação
    char nomeArq[100];
    sprintf(nomeArq, "dados_%d_%d.dat", quantReg, situacao);

    // Gera o arquivo com os dados
    if (gerarArquivo(quantReg, situacao, nomeArq)){
        printf("Arquivo gerado com sucesso!\n");
    } else {
        printf("Erro ao gerar o arquivo.\n");
        return 1;
    }

    // Mostra o arquivo gerado se a opção -P for passada
    if (P){
        mostraArquivo(nomeArq);
    }

    FILE *arquivo = fopen(nomeArq, "rb");
    if (arquivo == NULL){
        printf("Erro ao abrir o arquivo %s\n", nomeArq);
        return 1;
    }

    switch (metodo){
        case 1: // Pesquisa indexada
            if (situacao != 1){
                printf("O acesso sequencial indexado só funciona para arquivos odernados.\n");
                return 1;
            }
            if (itemP.chave != -1){
                if(indexado(arquivo, quantReg, &itemP, &analise)){
                    imprimeAnalise(&analise, &itemP);
                } else {
                    printf("Item com chave %d não encontrado.\n", itemP.chave);
                }
            } else{
                executarExperimento(arquivo, quantReg, &analise, indexado);
                imprimeMediaAnalise(&analise);
            }
        break;

        case 2: // Árvore Binária Externa

            construirArvoreBin(nomeArq, "arvore_index.bin", &analise);

            if (itemP.chave != -1){
                // Modo de busca única
                if(pesquisaArvoreBinariaWrapper(arquivo, quantReg, &itemP, &analise)){
                    imprimeAnalise(&analise, &itemP);
                } else {
                    printf("Item com chave %d não encontrado.\n", itemP.chave);
                }
            } else {
                // Modo Experimental (Média de 20 buscas)
                executarExperimento(arquivo, quantReg, &analise, pesquisaArvoreBinariaWrapper);
                imprimeMediaAnalise(&analise);
            }
        break;

        case 3: // Árvore B (Padrão)
            // Verifica se é uma pesquisa única (chave != -1) ou experimento (chave == -1)
            if (itemP.chave != -1){
                // Chama a função wrapper 'pesquisaArvoreB' criada anteriormente
                if(pesquisaArvoreB(arquivo, quantReg, &itemP, &analise)){
                    imprimeAnalise(&analise, &itemP);
                } else {
                    printf("Item com chave %d não encontrado.\n", itemP.chave);
                }
            } else {
                // Passa a função 'pesquisaArvoreB' como ponteiro para o experimento rodar 20x
                executarExperimento(arquivo, quantReg, &analise, pesquisaArvoreB);
                imprimeMediaAnalise(&analise);
            }
        break;
        case 4: // Árvore B* (Estrela)
             if (itemP.chave != -1){
                // Pesquisa única
                if(pesquisaArvoreBEstrela(arquivo, quantReg, &itemP, &analise)){
                    imprimeAnalise(&analise, &itemP);
                } else {
                    printf("Item com chave %d não encontrado.\n", itemP.chave);
                }
            } else {
                // Modo Experimental (20 execuções)
                executarExperimento(arquivo, quantReg, &analise, pesquisaArvoreBEstrela);
                imprimeMediaAnalise(&analise);
            }
        break;

        default:
            printf("Método de pesquisa inválido.\n");
            fclose(arquivo);
            return 1;
        break;
        
    }

    if (arquivo != NULL) fclose(arquivo);
    return 0;
}