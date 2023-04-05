#ifndef BPLUSTREE_H_
#define BPLUSTREE_H_

#include "arquivo.h"
#include "pagina.h"
#include "util.h"

typedef struct bptree {
  int qtde, raiz, indexes;
} BPTree;

void BPTreeInicializar();
bool BPTreeProcurarElemento(int, int *);
void BPTreeInserir(int, int);
void BPTreeRemover(int);
void BPTreeImprimirArtigo(int);
void BPTreeImprimirAPartir(int);
void BPTreeImprimirTudo();
void  BPTreeImprimeNivel(Pagina *, FILE *);
void BPTreeImprimeArvoreCompleta();

#endif