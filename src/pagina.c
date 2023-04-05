#include "../headers/pagina.h"

void
inicializarPagina (BPTree* arvore, Pagina* pagina) {
  pagina->qtde    = 0;
  pagina->tipo    = FOLHA;
  pagina->nivel   = 0;
  pagina->proximo = -1;
  pagina->pai     = -1;
  
  for (int i = 0; i < MAX; i++){
    pagina->chave[i] = -1;
    pagina->filho[i] = -1;

    if((i + 1) == MAX){
      pagina->filho[i + 1] = -1;
    }
  }

  int index = procurarReciclados();
  if (index > 0){
    pagina->index = index;
  }
  else {
    pagina->index = arvore->indexes;
    arvore->indexes++;
  }
}


void 
inserirEmFolha (Pagina* pagina, int chave, int regIndex) {
  if (pagina->tipo == FOLHA) {
    pagina->chave[pagina->qtde] = chave;
    pagina->filho[pagina->qtde] = regIndex;
    pagina->qtde++;
  }
  else {
    printf("\nERRO: Tentando inserir em pagina interna\n");
  }
}


int
removerEmFolha (Pagina* pagina, int chave) {
  int i;
   for (i = 0; i < pagina->qtde; i++){
    if (pagina->chave[i] == chave){
      pagina->chave[i] = pagina->chave[pagina->qtde - 1];
      pagina->filho[i] = pagina->filho[pagina->qtde - 1];
      pagina->qtde--;
      break;
    }
  }
  ordenarFolha(pagina);
  pagina->chave[pagina->qtde] = -1;
  pagina->filho[pagina->qtde] = -1;
  
  return i;
}


void 
ordenarFolha(Pagina* pagina) {
  int j, temp1, temp2;
  
  for (int i = 1; i < pagina->qtde; i++) {
    temp1 = pagina->chave[i];
    temp2 = pagina->filho[i];
    for (j = i; j > 0 && temp1 < pagina->chave[j - 1]; j--) {
      pagina->chave[j] = pagina->chave[j - 1];
      pagina->filho[j] = pagina->filho[j - 1];
    }
    
    pagina->chave[j] = temp1;
    pagina->filho[j] = temp2;
  }
}


void
ordenarInterna(FILE *arquivoArvore, Pagina *pagina) {
  int j, k, temp;
  
  for(k = 1; k < pagina->qtde; k++){
    for(j = 0; j < pagina->qtde - 1; j++){
      
      if(pagina->chave[j] > pagina->chave[j + 1]){
        temp = pagina->chave[j];
        pagina->chave[j] = pagina->chave[j + 1];
        pagina->chave[j + 1] = temp;
      }
    }
  }
  
  Pagina filhoI, filhoJ;

  for(k = 1; k < pagina->qtde + 1; k++){

    for(j = 0; j < pagina->qtde; j++){

      fseek(arquivoArvore, sizeof(BPTree) + (sizeof(Pagina) * pagina->filho[j]), SEEK_SET);
      fread(&filhoI, sizeof(Pagina), 1, arquivoArvore);
      
      fseek(arquivoArvore, sizeof(BPTree) + (sizeof(Pagina) * pagina->filho[j + 1]), SEEK_SET);
      fread(&filhoJ, sizeof(Pagina), 1, arquivoArvore);

      if(filhoI.chave[0] > filhoJ.chave[0]){
          temp = filhoI.index;
          pagina->filho[j] = pagina->filho[j + 1];
          pagina->filho[j + 1] = filhoI.index;
      }
    }
  }
}


tFolha
ehFolha (Pagina pagina) {
  return pagina.tipo;
}


bool 
verificarOverflow(Pagina *pagina){
  return pagina->qtde > (2 * D);
}


bool 
verificarUnderflow (int qtde) {
  return qtde < D;
}


void 
imprimirPagina (Pagina *pagina) {
  printf("\nIndex: %d | Pai: %d \n", pagina->index, pagina->pai);
  if(pagina->tipo == FOLHA){
    printf("Chaves: \n");
    for (int i = 0; i < pagina->qtde; i++){
      printf("%d | ", pagina->chave[i]);
    }
    printf("\nFilhos: \n");
    for (int i = 0; i < pagina->qtde; i++){
      printf("%d | ", pagina->filho[i]);
    }
  }
  else{
    printf("Chaves: \n");
    for (int i = 0; i < pagina->qtde; i++){
      printf("%d | ", pagina->chave[i]);
    }
    printf("\nFilhos: \n");
    for (int i = 0; i <= pagina->qtde; i++){
      printf("%d | ", pagina->filho[i]);
    }
  }
}