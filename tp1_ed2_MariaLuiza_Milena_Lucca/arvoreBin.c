#include "arvoreBin.h"

// Cria um nó simples para armazenar no arquivo de árvore.
NoArvoreBin criarNo(int chave, long int offsetDado) {
    NoArvoreBin no;
    no.chave = chave;
    no.offsetDado = offsetDado;
    no.esquerda = NULL_DISK;
    no.direita = NULL_DISK;
    return no;
}

// Insere recursivamente na árvore externa.
long int inserirNaArvoreBin(FILE* arqArvore, long int posAtual, int chave, long int offsetDado, AnaliseExperimental *analise) {

    // Caso base: posição vazia - cria nó no final do arquivo
    if (posAtual == NULL_DISK) {
        NoArvoreBin novoNo = criarNo(chave, offsetDado);

        // Vai para o fim e grava o nó novo
        fseek(arqArvore, 0, SEEK_END);
        long int novaPos = ftell(arqArvore);

        fwrite(&novoNo, sizeof(NoArvoreBin), 1, arqArvore);
        analise->numTransferencias++;  // gravação do nó

        return novaPos;
    }

    // Caso contrário, lê o nó atual no arquivo
    NoArvoreBin noAtual;
    fseek(arqArvore, posAtual, SEEK_SET);
    fread(&noAtual, sizeof(NoArvoreBin), 1, arqArvore);
    analise->numTransferencias++;     // leitura do nó
    analise->numComparacoes++;        // comparação para decidir onde inserir

    // Caminha para a esquerda
    if (chave < noAtual.chave) {
        long int novoEsq = inserirNaArvoreBin(arqArvore, noAtual.esquerda, chave, offsetDado, analise);

        // Se o filho esquerdo mudou, atualiza o nó atual no arquivo
        if (novoEsq != noAtual.esquerda) {
            noAtual.esquerda = novoEsq;
            fseek(arqArvore, posAtual, SEEK_SET);
            fwrite(&noAtual, sizeof(NoArvoreBin), 1, arqArvore);
            analise->numTransferencias++;  // gravação da atualização
        }
    }
    // Caminha para a direita
    else if (chave > noAtual.chave) {
        long int novoDir = inserirNaArvoreBin(arqArvore, noAtual.direita, chave, offsetDado, analise);

        if (novoDir != noAtual.direita) {
            noAtual.direita = novoDir;
            fseek(arqArvore, posAtual, SEEK_SET);
            fwrite(&noAtual, sizeof(NoArvoreBin), 1, arqArvore);
            analise->numTransferencias++;  // gravação da atualização
        }
    }

    return posAtual;
}

// Constrói o índice externo percorrendo todo o arquivo de dados.
void construirArvoreBin(const char* nomeArquivoDados, const char* nomeArquivoArvore, AnaliseExperimental *analiseCriacao) {

    FILE* arqDados = fopen(nomeArquivoDados, "rb");
    if (!arqDados) return;

    // garante recriação do arquivo
    FILE* arqArvore = fopen(nomeArquivoArvore, "w+b");
    if (!arqArvore) { fclose(arqDados); return; }

    Item item;
    long int offsetDadoAtual = 0;
    long int raizOffset = NULL_DISK;

    clock_t inicio = clock();

    // Varre todos os itens do arquivo de dados
    while (fread(&item, sizeof(Item), 1, arqDados) == 1) {

        analiseCriacao->numTransferencias++; // leitura do item

        raizOffset = inserirNaArvoreBin(arqArvore, raizOffset,
                                        item.chave, offsetDadoAtual,
                                        analiseCriacao);

        // Guarda onde está o próximo registro no arquivo de dados
        offsetDadoAtual = ftell(arqDados);
    }

    clock_t fim = clock();
    analiseCriacao->tempoExecucao = (double)(fim - inicio) / CLOCKS_PER_SEC;

    fclose(arqDados);
    fclose(arqArvore);
}

// Pesquisa simples percorrendo nós no arquivo.
Item* pesquisarArvoreBin(FILE* arqArvore, long int posRaiz, int chave, FILE* arqDados, AnaliseExperimental *analise) {

    if (posRaiz == NULL_DISK) return NULL;

    NoArvoreBin noAtual;
    long int posAtual = posRaiz;

    while (posAtual != NULL_DISK) {

        // Move para o nó atual e lê
        fseek(arqArvore, posAtual, SEEK_SET);
        fread(&noAtual, sizeof(NoArvoreBin), 1, arqArvore);
        analise->numTransferencias++;  // leitura do nó
        analise->numComparacoes++;     // comparação da chave

        if (chave == noAtual.chave) {
            //busca o registro no arquivo de dados
            Item* itemEncontrado = malloc(sizeof(Item));

            fseek(arqDados, noAtual.offsetDado, SEEK_SET);
            if (fread(itemEncontrado, sizeof(Item), 1, arqDados) == 1) {
                analise->numTransferencias++; // leitura do registro
                return itemEncontrado;
            } else {
                free(itemEncontrado);
                return NULL;
            }
        }
        else if (chave < noAtual.chave)
            posAtual = noAtual.esquerda;
        else
            posAtual = noAtual.direita;
    }

    return NULL;
}

// Wrapper usado no trabalho para integrar com sua main.
bool pesquisaArvoreBinariaWrapper(FILE* arqDados, int quantReg, Item* item, AnaliseExperimental* analise) {
    FILE* arqArvore = fopen("arvore_index.bin", "rb");

    if (arqArvore == NULL) {
        perror("Erro ao abrir arquivo de índice na pesquisa");
        return false;
    }

    // A raiz SEMPRE começa no offset 0
    long int raizOffset = 0;

    // Verifica se o arquivo está vazio
    fseek(arqArvore, 0, SEEK_END);
    if (ftell(arqArvore) == 0)
        raizOffset = NULL_DISK;

    // Pesquisa no índice externo
    Item* resultado = pesquisarArvoreBin(arqArvore, raizOffset, item->chave, arqDados, analise);

    fclose(arqArvore);

    if (resultado != NULL) {
        *item = *resultado;
        free(resultado);
        return true;
    }

    return false;
}
