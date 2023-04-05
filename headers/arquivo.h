#ifndef ARQUIVO_H_
#define ARQUIVO_H_

#include "bplustree.h"
#include "util.h"

typedef struct cabecalho {
  int qtde;
} Cabecalho;

void criarTabelaRegistros(char *);
void importarTabela();
void inserirnNovoArtigo();
void reciclarIndex(int);
int procurarReciclados();

#endif