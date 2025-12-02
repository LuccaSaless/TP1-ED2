// --- OBRIGATÓRIO: ISSO DEVE FICAR NA PRIMEIRA LINHA ---
#define _FILE_OFFSET_BITS 64 
// -----------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#include "item.h"
#include "index.h"
#include "arvoreb.h"
#include "arvorebe.h"
#include "arvorebin.h"
#include "geradorDeArquivos.h"

int main(int argc, char *argv[]) {
    // Verificando argumentos
    if (argc < 5 || argc > 6) {
        printf("Uso: %s <método> <quantidade> <situação> <chave> [-P]\n", argv[0]);
        return 1;
    }

    int metodo = atoi(argv[1]); 
    int quantReg = atoi(argv[2]);
    int situacao = atoi(argv[3]);
    int chaveBuscada = atoi(argv[4]); // Simplifiquei para int, pois a busca é por chave
    bool P = false; 

    if(argc == 6 && ((strcmp(argv[5], "-P") == 0) || (strcmp(argv[5], "-p") == 0))) P = true;

    char nomeArq[100];
    sprintf(nomeArq, "dados_%d_%d.dat", quantReg, situacao);
    
    // Gera o arquivo (use a versão segura que passei antes se for 2 milhões!)
    if (gerarArquivo(quantReg, situacao, nomeArq)){
        printf("Arquivo verificado/gerado com sucesso: %s\n", nomeArq);
    } else {
        printf("Erro ao gerar o arquivo.\n");
        return 1;
    }

    // Mostra arquivo APENAS se P for true (cuidado com arquivos grandes!)
    if (P && quantReg <= 1000) { // Proteção para não printar 2 milhões de linhas
        mostraArquivo(nomeArq);
    }

    // Abertura do arquivo para métodos que precisam dele aberto externamente
    FILE *arquivo = fopen(nomeArq, "rb");
    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo %s\n", nomeArq);
        return 1;
    }

    // Estruturas auxiliares
    Item itemPesquisa;
    itemPesquisa.chave = chaveBuscada;
    AnaliseExperimental analise;

    switch (metodo) {
        case 1: // Acesso Sequencial Indexado
            if (situacao != 1){
                printf("O acesso sequencial indexado só funciona para arquivos ordenados.\n");
                fclose(arquivo);
                return 1;
            }
            
            if(chaveBuscada != -1){ 
                // A função Indexado retorna 1 (achou) ou 0 (não achou)
                // Ela preenche itemPesquisa com os dados se achar
                int encontrou = Indexado(&itemPesquisa, arquivo, &analise, quantReg);

                printf("\n========================================================\n");
                printf("METODO: 1 - ACESSO SEQUENCIAL INDEXADO\n");
                printf("========================================================\n");

                printf("--- FASE 1: CRIACAO/CARREGAMENTO DO INDICE ---\n");
                // Nota: No indexado, a tabela é criada durante a execução da função Indexado.
                // O tempo retornado pela função 'Indexado' engloba tudo.
                // Para ser justo, consideramos aqui os valores retornados na struct analise.
                printf("Tempo Total:    %.6f s\n", (double)analise.tempoExecucao / CLOCKS_PER_SEC);
                printf("Transferencias: %d (Carregamento + Pesquisa)\n", analise.numTransferencias);
                printf("Comparacoes:    %d\n", analise.numComparacoes);

                printf("\n--- FASE 2: RESULTADO DA PESQUISA ---\n");
                if (encontrou) {
                    printf("[STATUS]: ITEM ENCONTRADO\n");
                    printf("> Chave:  %d\n", itemPesquisa.chave);
                    printf("> Dado1:  %ld\n", itemPesquisa.dado1);
                    if(P) printf("> Dado2:  %.50s...\n", itemPesquisa.dado2);
                } else {
                    printf("[STATUS]: ITEM NAO ENCONTRADO (Chave %d)\n", chaveBuscada);
                }
                printf("========================================================\n");
            } else {
                RodaIndexadoExperimento(arquivo, quantReg);
            }
            break;

        case 2: // Árvore Binária Externa
            // Nota: Certifique-se que sua implementação de RodaArvoreBinaria suporta arquivos grandes
            RodaArvoreBinaria(nomeArq, chaveBuscada, quantReg, P);
            break;

        case 3: // Árvore B (Corrigida anteriormente)
            // Fecha o arquivo pois RodaArvoreB abre internamente para gerenciar offsets
            fclose(arquivo); 
            arquivo = NULL; 
            RodaArvoreB(nomeArq, chaveBuscada, quantReg, P);
            break;

        case 4: // Árvore B* (B-Estrela)
            // Fecha o arquivo pois vamos reabrir para criar o índice do zero
            if(arquivo) { fclose(arquivo); arquivo = NULL; }
            
            // Alterei para passar nomeArq, garantindo consistência com o método 3
            RodaArvoreBEstrela(nomeArq, chaveBuscada, quantReg, P);
            break;

        default:
            printf("Metodo %d nao implementado.\n", metodo);
            break;
    }

    if (arquivo != NULL) fclose(arquivo);
    return 0;
}