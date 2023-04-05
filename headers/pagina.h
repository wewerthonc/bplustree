#ifndef PAGINA_H_
#define PAGINA_H_

#include "util.h"
#include "bplustree.h"

typedef enum { 
  INTERNA, 
  FOLHA 
} tFolha;

typedef struct pagina {
  int chave[MAX];
  int filho[MAX+1];
  int pai;
  int proximo;
  int nivel;
  int qtde;
  int index;
  tFolha tipo;
} Pagina;

void inicializarPagina(BPTree *, Pagina *);
void inserirEmFolha(Pagina *, int, int);
int removerEmFolha(Pagina *, int );
void ordenarFolha(Pagina *);
void ordenarInterna(FILE *, Pagina*);
tFolha ehFolha(Pagina);
bool verificarOverflow(Pagina *);
bool verificarUnderflow(int);
void liberarPagina(Pagina *);
void imprimirPagina(Pagina *);
void ordenarInterna2(FILE *, Pagina*);

#endif