#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include "../headers/util.h"
#include "../headers/bplustree.h"
#include "../headers/pagina.h"

#define ERROR(msg) fprintf(stderr, "[!]: %s\n", #msg)

void *
mallocSafe (size_t nbytes) {
  void *ptr = malloc (nbytes);
  
  if (ptr == NULL) {
    ERROR(erro ao alocar memória);
    exit(EXIT_FAILURE);
  }

  return ptr;
}


void
verificarArquivo (FILE *fp) {
  if (fp == NULL) {
    perror(RED"Erro ao abrir o arquivo"reset);
    exit(0);
  }
}


void imprimirArtigo(Artigo artigo){ 
  printf("\nId.........: %d", artigo.id);
  printf("\nAno........: %d", artigo.ano);
  printf("\nAutor......: %s", artigo.autor);
  printf("\nTitulo.....: %s", artigo.titulo);
  printf("\nRevista....: %s", artigo.revista);
  printf("\nDOI........: %s", artigo.DOI);
  printf("\nKeywords...: %s\n", artigo.palavraChave);
  return;
}