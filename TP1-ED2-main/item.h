#ifndef ITEM_H
#define ITEM_H

typedef struct{
    int chave;
    long int dado1;
    char dado2[1000];
    char dado3[5000];
}Item;
 
typedef struct{
    int numTransferencias, numComparacoes;
    double tempoExecucao;
}AnaliseExperimental;

#endif