// isso aqui eh obrigatorio pra lidar com arquivos maiores que 2gb, senao buga o fseek
#define _FILE_OFFSET_BITS 64 

#include "arvoreBe.h"


// --- PARTE DE GERENCIAMENTO DE MEMORIA ---

// cria um novo no da arvore ja zerado
TipoApontadorBE CriaNoBE() {
    // uso calloc pq ele aloca e ja enche de zeros. 
    // isso evita lixo de memoria e ja deixa n=0 e os ponteiros NULL
    TipoApontadorBE novo = (TipoApontadorBE)calloc(1, sizeof(TipoPaginaBE));
    if (!novo) { 
        perror("[FATAL] deu ruim ao alocar memoria pro no"); 
        exit(EXIT_FAILURE); 
    }
    return novo;
}

// so pra garantir que o ponteiro da raiz comece nulo
void InicializaArvoreBEst(TipoApontadorBE* Arvore) {
    *Arvore = NULL;
}

// funcao recursiva pra limpar a memoria, senao o pc explode rodando 20 vezes
void LiberaArvoreBE(TipoApontadorBE Ap) {
    if (Ap == NULL) return;

    // se o p[0] nao for nulo, quer dizer que tem filhos abaixo.
    // entao eu desço recursivamente neles antes de apagar o pai.
    if (Ap->p[0] != NULL) { 
        for (int i = 0; i <= Ap->n; i++) {
            LiberaArvoreBE(Ap->p[i]);
        }
    }
    // agora que os filhos foram, posso liberar esse no
    free(Ap);
}

// --- PARTE DA PESQUISA ---

// busca padrao na arvore b
TipoOffset PesquisaBE(int chave, TipoApontadorBE Ap, AnaliseExperimental *analise) {
    if (Ap == NULL) return -1; // nao achou

    int i = 0;
    // varre o no atual pra encontrar a posicao da chave
    // aqui conta comparacao pq to testando chave por chave
    while (i < Ap->n && chave > Ap->chaves[i]) {
        analise->numComparacoes++;
        i++;
    }

    // verifica se achou a chave exatamente
    if (i < Ap->n) {
        analise->numComparacoes++; // conta o teste de igualdade
        if (chave == Ap->chaves[i])
            return Ap->offsets[i]; // achou! retorna onde ta no arquivo
    }

    // se nao achou e nao eh folha, desce pro filho correspondente
    return PesquisaBE(chave, Ap->p[i], analise);
}

// --- LOGICA ESPECIFICA DA B* (REDISTRIBUICAO E SPLIT) ---

// isso aqui eh o segredo da B*. antes de dividir, ela tenta jogar pro vizinho da esquerda
void RedistribuiEsquerda(TipoApontadorBE pai, int i) {
    TipoApontadorBE origem  = pai->p[i];     // o no que ta cheio
    TipoApontadorBE destino = pai->p[i-1];   // o irmao da esquerda (que tem espaco)

    // passo 1: desce a chave do pai pro final do irmao da esquerda
    destino->chaves[destino->n]   = pai->chaves[i-1];
    destino->offsets[destino->n]  = pai->offsets[i-1];
    destino->p[destino->n + 1]    = origem->p[0]; // o primeiro filho da origem vira o ultimo do destino
    destino->n++;

    // passo 2: sobe a primeira chave da origem pro lugar vago no pai
    pai->chaves[i-1]  = origem->chaves[0];
    pai->offsets[i-1] = origem->offsets[0];

    // passo 3: arruma a casa na origem (shifta tudo pra esquerda pra tapar o buraco do 0)
    memmove(&origem->chaves[0],  &origem->chaves[1],  (origem->n - 1) * sizeof(int));
    memmove(&origem->offsets[0], &origem->offsets[1], (origem->n - 1) * sizeof(TipoOffset));
    memmove(&origem->p[0],       &origem->p[1],       origem->n * sizeof(TipoApontadorBE));
    
    origem->n--; // diminuiu um item na origem
}

// mesma logica, so que joga pro vizinho da direita
void RedistribuiDireita(TipoApontadorBE pai, int i) {
    TipoApontadorBE origem  = pai->p[i];     // ta cheio
    TipoApontadorBE destino = pai->p[i+1];   // tem espaco

    // abre espaco no inicio do destino (shifta pra direita)
    memmove(&destino->chaves[1],  &destino->chaves[0],  destino->n * sizeof(int));
    memmove(&destino->offsets[1], &destino->offsets[0], destino->n * sizeof(TipoOffset));
    memmove(&destino->p[1],       &destino->p[0],       (destino->n + 1) * sizeof(TipoApontadorBE));

    // desce a chave do pai pro inicio do destino
    destino->chaves[0]  = pai->chaves[i];
    destino->offsets[0] = pai->offsets[i];
    destino->p[0]       = origem->p[origem->n]; // ultimo filho da origem vai pro inicio do destino
    destino->n++;

    // sobe a ultima chave da origem pro pai
    pai->chaves[i]  = origem->chaves[origem->n - 1];
    pai->offsets[i] = origem->offsets[origem->n - 1];

    origem->n--; // liberou espaco na origem
}

// se os dois vizinhos tiverem cheios, ai nao tem jeito, tem que dividir (split 1 pra 2)
void SplitFilho(TipoApontadorBE pai, int i) {
    TipoApontadorBE filho = pai->p[i];
    TipoApontadorBE novo  = CriaNoBE(); 
    novo->n = M_ESTRELA - 1; // vai ficar com metade

    // copia a metade direita do filho pro novo no
    memcpy(novo->chaves,  &filho->chaves[M_ESTRELA + 1],  (M_ESTRELA - 1) * sizeof(int));
    memcpy(novo->offsets, &filho->offsets[M_ESTRELA + 1], (M_ESTRELA - 1) * sizeof(TipoOffset));
    memcpy(novo->p,       &filho->p[M_ESTRELA + 1],       M_ESTRELA * sizeof(TipoApontadorBE));

    filho->n = M_ESTRELA; // ajusta o tamanho do original

    // abre espaco no pai pra receber a chave que vai subir (a mediana)
    memmove(&pai->p[i + 2],       &pai->p[i + 1],       (pai->n - i) * sizeof(TipoApontadorBE));
    memmove(&pai->chaves[i + 1],  &pai->chaves[i],      (pai->n - i) * sizeof(int));
    memmove(&pai->offsets[i + 1], &pai->offsets[i],     (pai->n - i) * sizeof(TipoOffset));

    // conecta o novo no e sobe a chave
    pai->p[i + 1]     = novo;
    pai->chaves[i]    = filho->chaves[M_ESTRELA];
    pai->offsets[i]   = filho->offsets[M_ESTRELA];
    pai->n++;
}

// --- INSERCAO ---

// insere numa pagina que a gente garante que nao ta cheia
void InsereNaoCheio(TipoApontadorBE x, int chave, TipoOffset offset, AnaliseExperimental* analise) {
    int i = x->n - 1;

    // CASO 1: SE FOR FOLHA
    // se p[0] eh null, todos sao null (calloc), entao eh folha
    if (x->p[0] == NULL) {
        // acha a posicao e vai empurrando pra direita
        while (i >= 0 && chave < x->chaves[i]) {
            analise->numComparacoes++;
            x->chaves[i + 1]  = x->chaves[i];
            x->offsets[i + 1] = x->offsets[i];
            i--;
        }
        if (i >= 0) analise->numComparacoes++; // conta a comparacao que falhou no while

        // insere a chave e o offset
        x->chaves[i + 1]  = chave;
        x->offsets[i + 1] = offset;
        x->n++;
    } 
    // CASO 2: SE FOR NO INTERNO
    else {
        // procura qual filho descer
        while (i >= 0 && chave < x->chaves[i]) {
            analise->numComparacoes++;
            i--;
        }
        if (i >= 0) analise->numComparacoes++;
        i++; 

        // === LOGICA B* PROATIVA ===
        // antes de descer, eu olho: o filho onde vou descer ta cheio?
        if (x->p[i]->n == MM_ESTRELA) {
            
            // se tiver cheio, tento jogar pro irmao da esquerda
            if (i > 0 && x->p[i-1]->n < MM_ESTRELA) {
                RedistribuiEsquerda(x, i); 
                // agora que abriu espaco, desço recursivo
                InsereNaoCheio(x->p[i], chave, offset, analise);
            }
            // senao, tento jogar pro irmao da direita
            else if (i < x->n && x->p[i+1]->n < MM_ESTRELA) {
                RedistribuiDireita(x, i); 
                InsereNaoCheio(x->p[i], chave, offset, analise);
            }
            // se os dois irmaos tbm tao cheios, ai paciencia, faz split
            else {
                SplitFilho(x, i);
                
                // depois do split, subiu uma chave. preciso ver se vou pra esq ou dir dela
                analise->numComparacoes++; 
                if (chave > x->chaves[i]) {
                    i++;
                }
                InsereNaoCheio(x->p[i], chave, offset, analise);
            }
        } else {
            // filho nao ta cheio, vida que segue, so desce
            InsereNaoCheio(x->p[i], chave, offset, analise);
        }
    }
}

// funcao principal de insercao que trata a raiz
void InsereBEst(int chave, TipoOffset offset, TipoApontadorBE* Arvore, AnaliseExperimental* analise) {
    // se a arvore ta vazia, cria a raiz
    if (*Arvore == NULL) {
        *Arvore = CriaNoBE();
        (*Arvore)->chaves[0] = chave;
        (*Arvore)->offsets[0] = offset;
        (*Arvore)->n = 1;
        return;
    }

    TipoApontadorBE raiz = *Arvore;

    // se a raiz ta cheia, eh o unico caso onde a arvore cresce pra cima
    if (raiz->n == MM_ESTRELA) {
        TipoApontadorBE novaRaiz = CriaNoBE();
        novaRaiz->p[0] = raiz; // antiga raiz vira filha
        *Arvore = novaRaiz;
        
        // divide a raiz antiga
        SplitFilho(novaRaiz, 0);
        // insere a nova chave
        InsereNaoCheio(novaRaiz, chave, offset, analise);
    } else {
        InsereNaoCheio(raiz, chave, offset, analise);
    }
}

// --- WRAPPER PRA RODAR NO EXPERIMENTO ---

// essa eh a funcao "casca" que o professor quer, igual a do indexado
bool pesquisaArvoreBEstrela(FILE *arquivo, int quantReg, Item *itemP, AnaliseExperimental *analise) {
    TipoApontadorBE arvore;
    InicializaArvoreBEst(&arvore); // comeca limpa
    Item aux;
    
    clock_t inicio = clock(); // dispara o cronometro

    // 1. CARGA (INDEXACAO)
    // le o arquivo todo sequencialmente e vai montando a arvore na ram
    rewind(arquivo);
    while (true) {
        TipoOffset pos = ftello(arquivo); // pega a posicao do registro no disco
        if (fread(&aux, sizeof(Item), 1, arquivo) != 1) break; // acabou o arquivo
        
        analise->numTransferencias++; // conta leitura do disco
        InsereBEst(aux.chave, pos, &arvore, analise); // insere na arvore
    }

    // 2. PESQUISA
    // busca na estrutura em memoria pra pegar o offset
    TipoOffset offsetEncontrado = PesquisaBE(itemP->chave, arvore, analise);

    bool encontrado = false;
    if (offsetEncontrado != -1) {
        // 3. RECUPERACAO
        // se achou, vai la no disco na posicao certa e le o item completo
        fseeko(arquivo, offsetEncontrado, SEEK_SET);
        fread(itemP, sizeof(Item), 1, arquivo);
        analise->numTransferencias++; // conta esse acesso direto
        encontrado = true;
    }

    // LIMPEZA
    // muito importante liberar a memoria pq essa funcao roda num loop de 20x
    LiberaArvoreBE(arvore);

    clock_t fim = clock();
    // calcula quanto tempo levou tudo
    analise->tempoExecucao = (double)(fim - inicio) / CLOCKS_PER_SEC;

    return encontrado;
}