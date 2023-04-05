#include "../headers/bplustree.h"
#include "../headers/pagina.h"

void 
BPTreeInicializar () {
  BPTree arvore;
  arvore.qtde    = 0;
  arvore.raiz    = -1;
  arvore.indexes = 0;
  
  FILE *arquivoArvore = fopen(TREE_FILE, "wb");
  verificarArquivo(arquivoArvore);
  
  fwrite(&arvore, sizeof(BPTree), 1, arquivoArvore);
  fclose(arquivoArvore);

  Cabecalho recicladosHeader;
  recicladosHeader.qtde = 0;

  FILE* arquivoReciclados = fopen(RECYCLE_FILE, "wb");
  verificarArquivo(arquivoReciclados);
  
  fwrite(&recicladosHeader, sizeof(Cabecalho), 1, arquivoReciclados);
  fclose(arquivoReciclados);
}


bool 
BPTreeProcurarElemento (int chave, int *pagIndex) {
  FILE *arquivoArvore = fopen(TREE_FILE, "rb");   

  verificarArquivo(arquivoArvore);
  
  if (!arquivoArvore) {
    perror(RED"\nA árvore ainda não foi criada!\n"reset);
    return false;
  }
  
  BPTree arvore;
  
  fread(&arvore, sizeof(BPTree), 1, arquivoArvore);
  
  if(arvore.raiz < 0){
    *pagIndex = -1;
    fclose(arquivoArvore);
    return false;
  }
  
  Pagina pag;
  fseek(arquivoArvore, sizeof(Pagina) * arvore.raiz, SEEK_CUR);
  fread(&pag, sizeof(Pagina), 1, arquivoArvore);

  while (true) {
    if (!ehFolha(pag)) {
      for (int i = 0; i < pag.qtde; i++) {
        if (i == 0 && (pag.chave[i] > chave)) {
          fseek(arquivoArvore, sizeof(BPTree) + (sizeof(Pagina) * pag.filho[0]), SEEK_SET);
          fread(&pag, sizeof(Pagina), 1, arquivoArvore);
          break;
        }
          
        else if (pag.chave[i] > chave) {
          fseek(arquivoArvore, sizeof(BPTree) + (sizeof(Pagina) * pag.filho[i]), SEEK_SET);
          fread(&pag, sizeof(Pagina), 1, arquivoArvore);
          break;
        }

        else if (pag.chave[i] == chave) {
          fseek(arquivoArvore, sizeof(BPTree) + (sizeof(Pagina) * pag.filho[i + 1]), SEEK_SET);
          fread(&pag, sizeof(Pagina), 1, arquivoArvore);
          break;
        }

        else if (pag.qtde == (i + 1)) {
          fseek(arquivoArvore, sizeof(BPTree) + (sizeof(Pagina) * pag.filho[i + 1]), SEEK_SET);
          fread(&pag, sizeof(Pagina), 1, arquivoArvore);
          break;
        }
      }
    } 
    else {
      *pagIndex = pag.index;
      for (int i = 0; i < pag.qtde; i++) {
        if (pag.chave[i] == chave) {
          fclose(arquivoArvore);
          return true;
        }
      }
      
      fclose(arquivoArvore);
      return false;
    }
  }
}


void 
BPTreeIncrementarNivel (FILE *arquivoArvore, Pagina *pag) {
  if (ehFolha(*pag)) {
    pag->nivel++;
      
    fseek(arquivoArvore, sizeof(BPTree) + (sizeof(Pagina) * pag->index), SEEK_SET);
    fwrite(pag, sizeof(Pagina), 1, arquivoArvore);
    
  } else {
    fseek(arquivoArvore, sizeof(BPTree) + (sizeof(Pagina) * pag->index), SEEK_SET);
    pag->nivel++;
    fwrite(pag, sizeof(Pagina), 1, arquivoArvore);
    
    for(int i = 0; i < (pag->qtde + 1); i++){
      fseek(arquivoArvore, sizeof(BPTree) + (sizeof(Pagina) * pag->filho[i]), SEEK_SET);
      
      Pagina aux;
      fread(&aux, sizeof(Pagina), 1, arquivoArvore);
      BPTreeIncrementarNivel(arquivoArvore, &aux);
    }
  }
}


void 
BPTreeDecrementarNivel (FILE *arquivoArvore, Pagina *pag) {
  if (ehFolha(*pag)) {
    pag->nivel--;
      
    fseek(arquivoArvore, sizeof(BPTree) + (sizeof(Pagina) * pag->index), SEEK_SET);
    fwrite(pag, sizeof(Pagina), 1, arquivoArvore);
    
  } else {
    pag->nivel--;
    fseek(arquivoArvore, sizeof(BPTree) + (sizeof(Pagina) * pag->index), SEEK_SET);
    fwrite(pag, sizeof(Pagina), 1, arquivoArvore);
    
    for(int i = 0; i < (pag->qtde + 1); i++){
      fseek(arquivoArvore, sizeof(BPTree) + (sizeof(Pagina) * pag->filho[i]), SEEK_SET);
      
      Pagina aux;
      fread(&aux, sizeof(Pagina), 1, arquivoArvore);
      BPTreeDecrementarNivel(arquivoArvore, &aux);
    }
  }
}


void
BPTreeFixUp (FILE* arquivoArvore, int pos, int removida, int nova) {
  if(removida == -1){
    return;
  } else {
    Pagina pagina;
    fseek(arquivoArvore, sizeof(BPTree) + sizeof(Pagina) * pos, SEEK_SET);
    fread(&pagina, sizeof(Pagina), 1, arquivoArvore);

    for (int i = 0; i < pagina.qtde; i++) {
      if (pagina.chave[i] == removida) {
        pagina.chave[i] = nova;
        fseek(arquivoArvore, -sizeof(Pagina), SEEK_CUR);
        fwrite(&pagina, sizeof(Pagina), 1, arquivoArvore);
        return;
      }
    }

    if (pagina.pai == -1) return;
    
    BPTreeFixUp(arquivoArvore, pagina.pai, removida, nova);
  }
}


void 
BPTreeCorrigirOverflow (FILE* arquivoArvore, BPTree *arvore, Pagina pagina) {
  if(verificarOverflow(&pagina)){
    Pagina paginaIrma;
    Pagina paginaPai;

    inicializarPagina(arvore, &paginaIrma); 

    if(ehFolha(pagina)){
      for(int i = D; i < pagina.qtde; i++){
        inserirEmFolha(&paginaIrma, pagina.chave[i], pagina.filho[i]);
      }

      pagina.qtde -= paginaIrma.qtde;

      if(pagina.proximo > -1) {
        paginaIrma.proximo = pagina.proximo;
      }
      
      pagina.proximo = paginaIrma.index;

      if(pagina.pai < 0){
        inicializarPagina(arvore, &paginaPai);
        arvore->raiz = paginaPai.index;
        paginaPai.tipo = INTERNA;
        paginaPai.filho[paginaPai.qtde] = pagina.index;
        BPTreeIncrementarNivel (arquivoArvore, &pagina);
      } else {
        fseek(arquivoArvore, sizeof(BPTree) + (sizeof(Pagina) * pagina.pai), SEEK_SET);
        fread(&paginaPai, sizeof(Pagina), 1, arquivoArvore);
      }

      paginaPai.chave[paginaPai.qtde] = paginaIrma.chave[0];
      paginaPai.filho[paginaPai.qtde + 1] = paginaIrma.index;
      pagina.pai = paginaIrma.pai = paginaPai.index;
      paginaPai.qtde++;
      
    } else {

      for(int i = D + 1; i < pagina.qtde; i++){
        paginaIrma.chave[i - D - 1] = pagina.chave[i];
        paginaIrma.qtde++;
      }

      for(int i = D + 1; i < pagina.qtde + 1; i++){
        paginaIrma.filho[i - D - 1] = pagina.filho[i];
      }

      for(int i = 0; i < paginaIrma.qtde + 1; i++){
        fseek(arquivoArvore, sizeof(BPTree) + (sizeof(Pagina) * paginaIrma.filho[i]), SEEK_SET);

        Pagina filho;
        fread(&filho, sizeof(Pagina), 1, arquivoArvore);
        filho.pai = paginaIrma.index;
        fseek(arquivoArvore, -sizeof(Pagina), SEEK_CUR);
        fwrite(&filho, sizeof(Pagina), 1, arquivoArvore);
      } 

      if(pagina.pai < 0){
        inicializarPagina(arvore, &paginaPai);
        arvore->raiz = paginaPai.index;
        paginaPai.tipo = INTERNA;
        paginaPai.filho[paginaPai.qtde] = pagina.index;
        BPTreeIncrementarNivel (arquivoArvore, &pagina);
      } else {
        fseek(arquivoArvore, sizeof(BPTree) + (sizeof(Pagina) * pagina.pai), SEEK_SET);
        fread(&paginaPai, sizeof(Pagina), 1, arquivoArvore);
      }
      
      pagina.qtde -= paginaIrma.qtde + 1;
      paginaPai.chave[paginaPai.qtde] = pagina.chave[D];
      paginaPai.qtde++;
      paginaPai.filho[paginaPai.qtde] = paginaIrma.index;
      pagina.pai = paginaIrma.pai = paginaPai.index;
      paginaIrma.tipo = INTERNA;
    }

    paginaIrma.nivel = pagina.nivel;

    fseek(arquivoArvore, sizeof(BPTree) + sizeof(Pagina) * pagina.index, SEEK_SET);
    fwrite(&pagina, sizeof(Pagina), 1, arquivoArvore);

    fseek(arquivoArvore, sizeof(BPTree) + sizeof(Pagina) * paginaIrma.index, SEEK_SET);
    fwrite(&paginaIrma, sizeof(Pagina), 1, arquivoArvore);
      
    ordenarInterna(arquivoArvore, &paginaPai);
    fseek(arquivoArvore, sizeof(BPTree) + sizeof(Pagina) * paginaPai.index, SEEK_SET);
    fwrite(&paginaPai, sizeof(Pagina), 1, arquivoArvore);

    pagina = paginaPai;
  
    BPTreeCorrigirOverflow(arquivoArvore, arvore, pagina);
  }
  else{
    return;
  }
}


void 
BPTreeInserir (int chave, int regIndex) {
  int pagIndex;

  if (BPTreeProcurarElemento(chave, &pagIndex)) {
    printf("Elemento ja esta na arvore\n");
    return;
  }
  
  FILE *arquivoArvore = fopen(TREE_FILE, "r+b");

  verificarArquivo(arquivoArvore);

  BPTree arvore;
  fread(&arvore, sizeof(BPTree), 1, arquivoArvore);

  if (arvore.raiz < 0) {
    Pagina pagina;
    inicializarPagina(&arvore, &pagina);
    inserirEmFolha(&pagina, chave, regIndex);
    arvore.raiz = pagina.index;
    arvore.qtde++;
    rewind(arquivoArvore);
    fwrite(&arvore, sizeof(BPTree), 1, arquivoArvore);
    
    fwrite(&pagina, sizeof(Pagina), 1, arquivoArvore);

    fclose(arquivoArvore);
    arquivoArvore = NULL;
    return;
  }
  
  Pagina pagina; 
  fseek(arquivoArvore, sizeof(Pagina) * pagIndex, SEEK_CUR);
  fread(&pagina, sizeof(Pagina), 1, arquivoArvore);
  
  inserirEmFolha(&pagina, chave, regIndex);
  ordenarFolha(&pagina);
  arvore.qtde++;
  
  fseek(arquivoArvore, -sizeof(Pagina), SEEK_CUR);
  fwrite(&pagina, sizeof(Pagina), 1, arquivoArvore);

  BPTreeCorrigirOverflow(arquivoArvore, &arvore, pagina);

  rewind(arquivoArvore);
  fwrite(&arvore, sizeof(BPTree), 1, arquivoArvore);
  fclose(arquivoArvore);
  return;
}


void 
BPTreeCorrigirUnderflow (FILE* arquivoArvore, BPTree* arvore, Pagina pagina) {
  if (verificarUnderflow(pagina.qtde) && pagina.pai != -1){
    Pagina paginaPai, paginaIrma;
    fseek(arquivoArvore, sizeof(BPTree) + (sizeof(Pagina) * pagina.pai), SEEK_SET);
    fread(&paginaPai, sizeof(Pagina), 1, arquivoArvore);
    
    if (ehFolha(pagina)){
      if (paginaPai.filho[0] == pagina.index){
        fseek(arquivoArvore, sizeof(BPTree) + (sizeof(Pagina) * pagina.proximo), SEEK_SET);
        fread(&paginaIrma, sizeof(Pagina), 1, arquivoArvore);

        if(verificarUnderflow(paginaIrma.qtde - 1)){
          for (int i = 0; i < paginaIrma.qtde; i++){
            inserirEmFolha(&pagina, paginaIrma.chave[i], paginaIrma.filho[i]);
          }
          reciclarIndex(paginaIrma.index);

          if (paginaPai.qtde - 1 == 0 && paginaPai.pai == -1){
            arvore->raiz = pagina.index;
            reciclarIndex(paginaPai.index);
            pagina.pai = -1;
            pagina.nivel = 0;
          }
          else {
            paginaPai.chave[0] = paginaPai.chave[paginaPai.qtde - 1];
            paginaPai.filho[1] = paginaPai.filho[paginaPai.qtde];
            paginaPai.qtde--;

            fseek(arquivoArvore, sizeof(BPTree) + sizeof(Pagina) * pagina.index, SEEK_SET);
            fwrite(&pagina, sizeof(Pagina), 1, arquivoArvore);
            ordenarInterna(arquivoArvore, &paginaPai);
          }
          pagina.proximo = paginaIrma.proximo;
        }
        else {
          inserirEmFolha(&pagina, paginaIrma.chave[0], paginaIrma.filho[0]);
          paginaIrma.chave[0] = paginaIrma.chave[paginaIrma.qtde - 1];
          paginaIrma.filho[0] = paginaIrma.filho[paginaIrma.qtde - 1];
          paginaIrma.qtde--;
          ordenarFolha(&paginaIrma);    
          paginaPai.chave[0] = paginaIrma.chave[0];
        }
      }
      else {        
        Pagina irmaEsquerda, irmaDireita;
        
        int posNoPai = 0;
        for(posNoPai = 0; posNoPai < paginaPai.qtde; posNoPai++){
          if(paginaPai.filho[posNoPai] == pagina.index){
            break;
          }
        }
        fseek(arquivoArvore, sizeof(BPTree) + (sizeof(Pagina) * paginaPai.filho[posNoPai - 1]), SEEK_SET);
        fread(&irmaEsquerda, sizeof(Pagina), 1, arquivoArvore); 

        bool ehFilhoDireita = posNoPai ==  paginaPai.qtde; 
        if (!ehFilhoDireita){ 
          fseek(arquivoArvore, sizeof(BPTree) + (sizeof(Pagina) * paginaPai.filho[posNoPai + 1]), SEEK_SET);
          fread(&irmaDireita, sizeof(Pagina), 1, arquivoArvore);
        }
        
        if (!verificarUnderflow(irmaEsquerda.qtde - 1)){
          
          inserirEmFolha(&pagina, irmaEsquerda.chave[irmaEsquerda.qtde - 1], 
                                  irmaEsquerda.filho[irmaEsquerda.qtde - 1]);
          irmaEsquerda.qtde--;
          ordenarFolha(&pagina);
          paginaPai.chave[posNoPai - 1] = pagina.chave[0];
          paginaIrma = irmaEsquerda;
        }
        else if (!ehFilhoDireita && !verificarUnderflow(irmaDireita.qtde - 1)) {

          inserirEmFolha(&pagina, irmaDireita.chave[0], irmaDireita.filho[0]);
          irmaDireita.chave[0] = irmaDireita.chave[irmaDireita.qtde - 1];
          irmaDireita.filho[0] = irmaDireita.filho[irmaDireita.qtde - 1];
          irmaDireita.qtde--;
          ordenarFolha(&irmaDireita);
          paginaPai.chave[posNoPai] = irmaDireita.chave[0];
          paginaIrma = irmaDireita;

        }

        else {
          for (int i = 0; i < pagina.qtde; i++){
            inserirEmFolha(&irmaEsquerda, pagina.chave[i], pagina.filho[i]);
          }
          reciclarIndex(pagina.index);
          
          if (paginaPai.qtde - 1 == 0 && paginaPai.pai == -1){
            arvore->raiz = irmaEsquerda.index;
            reciclarIndex(paginaPai.index);
            irmaEsquerda.pai = -1;
            irmaEsquerda.nivel = 0;
          }
          else{
            paginaPai.chave[posNoPai - 1] = paginaPai.chave[paginaPai.qtde - 1];
            paginaPai.filho[posNoPai] = paginaPai.filho[paginaPai.qtde];
            paginaPai.qtde--;

            fseek(arquivoArvore, sizeof(BPTree) + sizeof(Pagina) * irmaEsquerda.index, SEEK_SET);
            fwrite(&irmaEsquerda, sizeof(Pagina), 1, arquivoArvore);
          
            ordenarInterna(arquivoArvore, &paginaPai);   
          }
          
          irmaEsquerda.proximo = pagina.proximo;
          paginaIrma = irmaEsquerda;
        }
      }
    }     
    else {
      if (paginaPai.filho[0] == pagina.index){
        fseek(arquivoArvore, sizeof(BPTree) + (sizeof(Pagina) * paginaPai.filho[1]), SEEK_SET);
        fread(&paginaIrma, sizeof(Pagina), 1, arquivoArvore);

        if (verificarUnderflow(paginaIrma.qtde - 1)){
          pagina.chave[pagina.qtde] = paginaPai.chave[0];
          pagina.qtde++;
          
          for (int i = 0; i < paginaIrma.qtde; i++){
            pagina.chave[pagina.qtde + i] = paginaIrma.chave[i];
          }
          
          for (int i = 0; i < paginaIrma.qtde + 1; i++){
            Pagina filho;
            fseek(arquivoArvore, sizeof(BPTree) + (sizeof(Pagina) * paginaIrma.filho[i]), SEEK_SET);
            fread(&filho, sizeof(Pagina), 1, arquivoArvore);
            
            
            pagina.filho[pagina.qtde + i] = filho.index;
            filho.pai = pagina.index;
            
            fseek(arquivoArvore, -(sizeof(Pagina)), SEEK_CUR);
            fwrite(&filho, sizeof(Pagina), 1, arquivoArvore);
          }

          pagina.qtde += paginaIrma.qtde;
          reciclarIndex(paginaIrma.index);

          fseek(arquivoArvore, sizeof(BPTree) + (sizeof(Pagina) * pagina.index), SEEK_SET);
          fwrite(&pagina, sizeof(Pagina), 1, arquivoArvore);
          
          if (paginaPai.qtde - 1 == 0 && paginaPai.pai == -1){
            arvore->raiz = pagina.index;
            reciclarIndex(paginaPai.index);
            pagina.pai = -1; 
            paginaPai = pagina;
            BPTreeDecrementarNivel(arquivoArvore, &pagina);
          }
          else {
            paginaPai.chave[0] = paginaPai.chave[paginaPai.qtde - 1];
            paginaPai.filho[1] = paginaPai.filho[paginaPai.qtde];
            paginaPai.qtde--;
            ordenarInterna(arquivoArvore, &paginaPai);
          }
        }
        else {
          pagina.chave[pagina.qtde] = paginaPai.chave[0];
          pagina.qtde++;

          paginaPai.chave[0] = paginaIrma.chave[0];
          pagina.filho[pagina.qtde] = paginaIrma.filho[0];

          Pagina filho;
          fseek(arquivoArvore, sizeof(BPTree) + (sizeof(Pagina) * paginaIrma.filho[0]), SEEK_SET);
          fread(&filho, sizeof(Pagina), 1, arquivoArvore);
          filho.pai = pagina.index;

          fseek(arquivoArvore, -sizeof(Pagina), SEEK_CUR);
          fwrite(&filho, sizeof(Pagina), 1, arquivoArvore);

          paginaIrma.filho[0] = paginaIrma.filho[paginaIrma.qtde];
          paginaIrma.chave[0] = paginaIrma.chave[paginaIrma.qtde - 1];
          paginaIrma.qtde--;
            
          ordenarInterna(arquivoArvore, &paginaIrma);
        }
      }
      else {
        Pagina irmaEsquerda, irmaDireita;
        
        int posNoPai = 0;
        for(posNoPai = 0; posNoPai < paginaPai.qtde; posNoPai++){
          if(paginaPai.filho[posNoPai] == pagina.index){
            break;
          }
        }
        
        fseek(arquivoArvore, sizeof(BPTree) + (sizeof(Pagina) * paginaPai.filho[posNoPai - 1]), SEEK_SET);
        fread(&irmaEsquerda, sizeof(Pagina), 1, arquivoArvore); 

        bool ehFilhoDireita = (posNoPai == paginaPai.qtde); 

        if (!ehFilhoDireita){ 
          fseek(arquivoArvore, sizeof(BPTree) + (sizeof(Pagina) * paginaPai.filho[posNoPai + 1]), SEEK_SET);
          fread(&irmaDireita, sizeof(Pagina), 1, arquivoArvore);
        }

        if (!verificarUnderflow(irmaEsquerda.qtde - 1)){
          pagina.chave[pagina.qtde] = paginaPai.chave[posNoPai - 1];
          pagina.qtde++;
          paginaPai.chave[posNoPai - 1] = irmaEsquerda.chave[irmaEsquerda.qtde - 1];
          pagina.filho[pagina.qtde] = irmaEsquerda.filho[irmaEsquerda.qtde];
          irmaEsquerda.qtde--;
          
          Pagina filho;
          fseek(arquivoArvore, sizeof(BPTree) + (sizeof(Pagina) * pagina.filho[pagina.qtde]), SEEK_SET);
          fread(&filho, sizeof(Pagina), 1, arquivoArvore);
          filho.pai = pagina.
            index;
          fseek(arquivoArvore, -sizeof(Pagina), SEEK_CUR);
          fwrite(&filho, sizeof(Pagina), 1, arquivoArvore);

          ordenarInterna(arquivoArvore, &pagina);
          paginaIrma = irmaEsquerda;
        }
        else if (!ehFilhoDireita && !verificarUnderflow(irmaDireita.qtde - 1)){
          pagina.chave[pagina.qtde] = paginaPai.chave[posNoPai];
          pagina.qtde++;

          paginaPai.chave[posNoPai] = irmaDireita.chave[0];
          irmaDireita.chave[0] = irmaDireita.chave[irmaDireita.qtde - 1];

          pagina.filho[pagina.qtde] = irmaDireita.filho[0];

          Pagina filho;
          fseek(arquivoArvore, sizeof(BPTree) + (sizeof(Pagina) * irmaDireita.filho[0]), SEEK_SET);
          fread(&filho, sizeof(Pagina), 1, arquivoArvore);
          filho.pai = pagina.index;
          fseek(arquivoArvore, -sizeof(Pagina), SEEK_CUR);
          fwrite(&filho, sizeof(Pagina), 1, arquivoArvore);

          irmaDireita.filho[0] = irmaDireita.filho[irmaDireita.qtde];
          irmaDireita.qtde--;
          ordenarInterna(arquivoArvore, &irmaDireita);
        
          paginaIrma = irmaDireita;
        }
        else {
          irmaEsquerda.chave[irmaEsquerda.qtde] = paginaPai.chave[posNoPai - 1];
          irmaEsquerda.qtde++;
          
          for (int i = 0; i < pagina.qtde; i++){
            irmaEsquerda.chave[irmaEsquerda.qtde + i] = pagina.chave[i];
          }

          for (int i = 0; i < pagina.qtde + 1; i++){
            Pagina filho;
            fseek(arquivoArvore, sizeof(BPTree) + (sizeof(Pagina) * pagina.filho[i]), SEEK_SET);
            fread(&filho, sizeof(Pagina), 1, arquivoArvore);
            
            irmaEsquerda.filho[irmaEsquerda.qtde + i] = pagina.filho[i];
            filho.pai = irmaEsquerda.index;
            
            fseek(arquivoArvore, -(sizeof(Pagina)), SEEK_CUR);
            fwrite(&filho, sizeof(Pagina), 1, arquivoArvore);
          }
              
          if (paginaPai.qtde - 1 == 0 && paginaPai.pai == -1){
            arvore->raiz = irmaEsquerda.index;
            reciclarIndex(paginaPai.index);
            irmaEsquerda.pai = -1;
            irmaEsquerda.qtde += pagina.qtde;
            BPTreeDecrementarNivel(arquivoArvore, &irmaEsquerda);
          }
          else{
            paginaPai.chave[posNoPai - 1] = paginaPai.chave[paginaPai.qtde - 1];
            paginaPai.filho[posNoPai] = paginaPai.filho[paginaPai.qtde];
            paginaPai.qtde--;
            ordenarInterna(arquivoArvore, &irmaEsquerda);

            fseek(arquivoArvore, sizeof(BPTree) + (sizeof(Pagina) * irmaEsquerda.index), SEEK_SET);
            fwrite(&irmaEsquerda, sizeof(Pagina), 1, arquivoArvore);
            ordenarInterna(arquivoArvore, &paginaPai);
            irmaEsquerda.qtde += pagina.qtde;
          }
          reciclarIndex(pagina.index);
          paginaIrma = irmaEsquerda;
        }
      }
    }

    fseek(arquivoArvore, sizeof(BPTree) + sizeof(Pagina) * pagina.index, SEEK_SET);
    fwrite(&pagina, sizeof(Pagina), 1, arquivoArvore);

    fseek(arquivoArvore, sizeof(BPTree) + (sizeof(Pagina) * paginaIrma.index), SEEK_SET);
    fwrite(&paginaIrma, sizeof(Pagina), 1, arquivoArvore);

    fseek(arquivoArvore, sizeof(BPTree) + (sizeof(Pagina) * paginaPai.index), SEEK_SET);
    fwrite(&paginaPai, sizeof(Pagina), 1, arquivoArvore);

    BPTreeCorrigirUnderflow(arquivoArvore, arvore,  paginaPai);
  }
  else {
    
    return;
  }
}

    
void 
BPTreeRemover (int chave) {
  int pagIndex;
  if (!BPTreeProcurarElemento(chave, &pagIndex)){
    printf(RED"\nO elemento não encontrado!"reset);
    return;
  }

  FILE *arquivoArvore = fopen(TREE_FILE, "r+b");

  BPTree arvore;
  fread(&arvore, sizeof(BPTree), 1, arquivoArvore);
  if (arvore.raiz < 0){
    printf(RED"\nA árvore está vazia!\n"reset);
    return;
  }

  Pagina pagina;
  fseek(arquivoArvore, sizeof(Pagina) * pagIndex, SEEK_CUR);
  fread(&pagina, sizeof(Pagina), 1, arquivoArvore);

  

  int posChaveRemovida = removerEmFolha(&pagina, chave);
  
  if(pagina.pai > -1 && posChaveRemovida == 0){
  
    Pagina paginaPai;
    fseek(arquivoArvore, sizeof(BPTree) + (sizeof(Pagina) * pagina.pai), SEEK_SET);
    fread(&paginaPai, sizeof(Pagina), 1, arquivoArvore);


    if (!(paginaPai.filho[0] == pagina.index)){
      int posChaveNoPai = 0;
      for (posChaveNoPai = 0; posChaveNoPai < paginaPai.qtde; posChaveNoPai++){
         if (chave == paginaPai.chave[posChaveNoPai]){
          break;
        }
      }

      if(verificarUnderflow(pagina.qtde) && D == 1){
        Pagina paginaIrma;
        fseek(arquivoArvore, sizeof(BPTree) + sizeof(Pagina) * paginaPai.filho[posChaveNoPai + 2], SEEK_SET);
        fread(&paginaIrma, sizeof(Pagina), 1, arquivoArvore);
        paginaPai.chave[posChaveNoPai] = paginaIrma.chave[0];
      } else {
        paginaPai.chave[posChaveNoPai] = pagina.chave[0];
      }
      

      fseek(arquivoArvore, sizeof(BPTree) + (sizeof(Pagina) * paginaPai.index), SEEK_SET);
      fwrite(&paginaPai, sizeof(Pagina), 1, arquivoArvore);
    } else {
      
      Pagina paginaIrma;
      fseek(arquivoArvore, sizeof(BPTree) + sizeof(Pagina) * paginaPai.filho[1], SEEK_SET);
      fread(&paginaIrma, sizeof(Pagina), 1, arquivoArvore);
      
      if(verificarUnderflow(pagina.qtde) && D == 1){
        BPTreeFixUp(arquivoArvore, pagina.pai, chave, paginaIrma.chave[0]);
      } else {
        BPTreeFixUp(arquivoArvore, pagina.pai, chave, pagina.chave[0]);
      }
    } 
  }
    
  arvore.qtde--;
  
  if (arvore.qtde == 0){
    arvore.raiz = -1;
  }

  fseek(arquivoArvore, sizeof(BPTree) + (sizeof(Pagina) * pagina.index), SEEK_SET);
  fwrite(&pagina, sizeof(Pagina), 1, arquivoArvore);

  BPTreeCorrigirUnderflow(arquivoArvore, &arvore, pagina);
  
  rewind(arquivoArvore);
  fwrite(&arvore, sizeof(BPTree), 1, arquivoArvore);
  fclose(arquivoArvore);
}


void
BPTreeImprimirArtigo(int id) {
  int posPag, posArt = -1;  
  
  if(!BPTreeProcurarElemento(id, &posPag)){
    printf(RED"\nO Artigo não foi encontrado!\n"reset);
    return;
  }
  
  FILE *arquivoArvore = fopen(TREE_FILE, "rb");

  verificarArquivo(arquivoArvore);
  
  Pagina pag;
  fseek(arquivoArvore, sizeof(BPTree) + (posPag * sizeof(Pagina)), SEEK_SET);
  fread(&pag, sizeof(Pagina), 1, arquivoArvore);
  
  for(int i = 0; i < pag.qtde; i++){
    if(pag.chave[i] == id){
      posArt = pag.filho[i];
    }
  }

  fclose(arquivoArvore);
  
  FILE *tabelaReg = fopen(REG_FILE, "rb");

  verificarArquivo(tabelaReg);
  
  fseek(tabelaReg, sizeof(Cabecalho) + (posArt * sizeof(Artigo)), SEEK_SET);
  Artigo artigo;
  fread(&artigo, sizeof(Artigo), 1, tabelaReg);
  fclose(tabelaReg);

  imprimirArtigo(artigo);
  
  return;
}


void 
BPTreeImprimirAPartir (int id) {
  int posPag;
  if (!BPTreeProcurarElemento(id, &posPag)){
    printf(RED"\nO ID não foi encontrado!\n"reset);
    return;
  };
  
  FILE* arquivoArvore = fopen(TREE_FILE, "rb");
  FILE* tabelaReg = fopen(REG_FILE, "rb");

  verificarArquivo(arquivoArvore);
  verificarArquivo(tabelaReg);

  Pagina pagina;
  fseek(arquivoArvore, sizeof(BPTree) + (sizeof(Pagina) * posPag), SEEK_SET);
  fread(&pagina, sizeof(Pagina), 1, arquivoArvore);

  int i;
  for(i = 0; i < pagina.qtde; i++){
    if (pagina.chave[i] == id){
      break;
    }
  }

  while(true) {
    while(i < pagina.qtde){
      Artigo artigo;
      fseek(tabelaReg, sizeof(Cabecalho) + (sizeof(Artigo) * pagina.filho[i]), SEEK_SET);
      fread(&artigo, sizeof(Artigo), 1, tabelaReg);

      imprimirArtigo(artigo);

      i++;
    }

    if (pagina.proximo > -1){
      fseek(arquivoArvore, sizeof(BPTree) + (sizeof(Pagina) * pagina.proximo), SEEK_SET);
      fread(&pagina, sizeof(Pagina), 1, arquivoArvore);
      i = 0;
    }
    else
      break;
  }

  fclose(arquivoArvore);
  fclose(tabelaReg);
}


void
BPTreeImprimirTudo () {
  FILE* arquivoArvore = fopen(TREE_FILE, "rb");
  FILE* tabelaReg = fopen(REG_FILE, "rb");
  
  BPTree arvore;
  fread(&arvore, sizeof(BPTree), 1, arquivoArvore);
  Pagina pagina;
  fseek(arquivoArvore, sizeof(Pagina) * arvore.raiz, SEEK_CUR);
  fread(&pagina, sizeof(Pagina), 1, arquivoArvore);

  while(pagina.tipo != FOLHA){
    fseek(arquivoArvore, sizeof(BPTree) + (sizeof(Pagina) * pagina.filho[0]), SEEK_SET);
    fread(&pagina, sizeof(Pagina), 1, arquivoArvore);
  }

  int i = 0;
  while(true) {
    while(i < pagina.qtde){
      Artigo artigo;
      fseek(tabelaReg, sizeof(Cabecalho) + (sizeof(Artigo) * pagina.filho[i]), SEEK_SET);
      fread(&artigo, sizeof(Artigo), 1, tabelaReg);

      imprimirArtigo(artigo);

      i++;
    }

    if (pagina.proximo > -1){
      fseek(arquivoArvore, sizeof(BPTree) + (sizeof(Pagina) * pagina.proximo), SEEK_SET);
      fread(&pagina, sizeof(Pagina), 1, arquivoArvore);
      i = 0;
    }
    else
      break;
  }

  fclose(arquivoArvore);
  fclose(tabelaReg);
}


void 
BPTreeImprimeArvoreCompleta () {
  FILE *arquivoArvore = fopen(TREE_FILE, "r");

  verificarArquivo(arquivoArvore);

  // Lê o cabeçalho
  BPTree arvore;
  fread(&arvore, sizeof(BPTree), 1, arquivoArvore);
  // Verifica se a árvore está vazia
  if (arvore.qtde > 0) {
    printf(HCYN"[!] Index: Posição da página no arquivo da árvore         "reset);
    printf(HCYN"\n[!] I.R.: Indíce/Posição do registro na tabela de registros"reset);
     printf(HCYN"\n[!] --> { x, ... }: Primeira chave da próxima página encadeada\n\n"reset);
    // Lê a pagina raiz
    fseek(arquivoArvore, sizeof(BPTree) + (arvore.raiz * sizeof(Pagina)), SEEK_SET);
    Pagina pagina;
    fread(&pagina, sizeof(Pagina), 1, arquivoArvore);
    BPTreeImprimeNivel(&pagina, arquivoArvore);
  }

  fclose(arquivoArvore);
}


void 
BPTreeImprimeNivel (Pagina *pag, FILE *arquivoArvore) {
  if (pag->tipo == FOLHA) {
    // Função que imprime artigos
    if (pag->index > 9)
      printf(HGRN"Index [%d]| Pai: %d | Nível: [%d] | Chaves {", pag->index, pag->pai, pag->nivel);
    else
      printf(HGRN"Index [%d] | Pai: %d | Nível: [%d] | Chaves {", pag->index, pag->pai, pag->nivel);
    for (int j = 0; j < pag->qtde; j++) {
      printf(" %d ", pag->chave[j]);
    }
    printf("}" reset);
    printf(BGRN" Registros [");
    for (int i = 0; i < pag->qtde; i++){
      printf(" %d ", pag->filho[i]);
    }
    printf("]");

    if (pag->proximo != -1) {
      fseek(arquivoArvore, sizeof(BPTree) + (pag->proximo * sizeof(Pagina)), SEEK_SET);
      Pagina proximo;
      fread(&proximo, sizeof(Pagina), 1, arquivoArvore);
      printf(" --> { %d, ... }", proximo.chave[0]);
    }
    printf("\n"reset);
  } else {
    // Chama a função recursivamente para cada filho
    for (int i = 0; i < pag->qtde + 1; i++) {
      fseek(arquivoArvore, sizeof(BPTree) + (pag->filho[i] * sizeof(Pagina)), SEEK_SET);

      Pagina aux;
      fread(&aux, sizeof(Pagina), 1, arquivoArvore);
      BPTreeImprimeNivel(&aux, arquivoArvore);
    }
    if (pag->pai == -1){
      if (pag->index > 9)
        printf(HYEL"Index [%d]|  RAIZ  | Nível: [%d] | Chaves {", pag->index, pag->nivel);
      else
        printf(HYEL"Index [%d] |  RAIZ  | Nível: [%d] | Chaves {", pag->index, pag->nivel);
    }
    else{
      if (pag->index > 9)
        printf(HYEL"Index [%d]| Pai: %d | Nível: [%d] | Chaves {", pag->index, pag->pai, pag->nivel);
      else
        printf(HYEL"Index [%d] | Pai: %d | Nível: [%d] | Chaves {", pag->index, pag->pai, pag->nivel);
    }
    for (int j = 0; j < pag->qtde; j++) {
      printf(" %d ", pag->chave[j]);
    }
    printf( "} | Filhos {");
    for (int i = 0; i < pag->qtde + 1; i++){
      printf(" %d ", pag->filho[i]);
    }
    printf("}\n");
  }
}
