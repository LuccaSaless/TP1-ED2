#ifndef ARVOREBINARIA_H
#define ARVOREBINARIA_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "item.h"
#include "analiseExperimental.h"

// Função principal que implementa o método da Árvore Binária
// Constrói o índice em memória e realiza a pesquisa
bool arvoreBinaria(FILE *arquivo, int quantReg, Item *itemP, AnaliseExperimental *analise);

#endif