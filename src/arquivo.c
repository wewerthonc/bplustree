#define _GNU_SOURCE
#include "../headers/arquivo.h"
#include "../headers/util.h"
#include <sys/stat.h>

void
inicializarTabela () {
  FILE* tabelaReg = fopen(REG_FILE, "wb");
  
  verificarArquivo(tabelaReg);
  
  Cabecalho tab;
  tab.qtde = 0;

  fwrite(&tab, sizeof(Cabecalho), 1, tabelaReg);
  fclose(tabelaReg);
}

void 
criarTabelaRegistros (char* caminho) {
  if (remove(REG_FILE)){
    printf(RED"\nErro ao deletar a tabela de registros\n"reset);
  };
  
  inicializarTabela();
  
  FILE *arquivoTXT = fopen(caminho, "r");
  FILE *tabelaReg  = fopen(REG_FILE, "r+b");
  
  verificarArquivo(tabelaReg);
  verificarArquivo(arquivoTXT);

  Cabecalho cabecalho;
  fread(&cabecalho, sizeof(Cabecalho), 1, tabelaReg);
  
  size_t len = 100;
  char* registro = mallocSafe(len);
  const char delim[2] = ",";
  char *info;

  while (getline(&registro, &len, arquivoTXT) > 0) {
    int campo = 0;
    Artigo artigo;
    
    info = strtok(registro, delim);
    while (info != NULL) {
      switch (campo) {
        case 0: artigo.id = atoi(info); break;
        case 1: artigo.ano = atoi(info); break;
        case 2: strncpy(artigo.autor, info, TAM); break;
        case 3: strncpy(artigo.titulo, info, TAM); break;
        case 4: strncpy(artigo.revista, info, TAM); break;
        case 5: strncpy(artigo.DOI, info, TAM); break;
        case 6:
          strncpy(artigo.palavraChave, info, TAM);
          strcat(artigo.palavraChave, ",");
          break;
        default:
          strcat(artigo.palavraChave, info);
          strcat(artigo.palavraChave, ",");
          break;
      }
      
      campo++;
      info = strtok(NULL, delim);
    }  
    
    artigo.palavraChave[strlen(artigo.palavraChave) - 1] = '\0';
    fwrite(&artigo, sizeof(Artigo), 1, tabelaReg);
    cabecalho.qtde++;
  }
  
  rewind(tabelaReg);
  fwrite(&cabecalho, sizeof(Cabecalho), 1, tabelaReg);
  
  free(registro);
  fclose(arquivoTXT);
  fclose(tabelaReg);
}


void
inserirnNovoArtigo () {
  char info[TAM];
  Artigo artigo;
  setbuf(stdin, NULL);
  printf("\nID do artigo: ");
  scanf("%d", &artigo.id);
  printf("\nAno de publicação: ");
  scanf("%d", &artigo.ano);

  setbuf(stdin, NULL);
  
  printf(HCYN"\n[!] Se houver mais de um autor, separar os nomes apenas com espaço "reset);
  printf(HCYN"\nou outro caracter diferente de ',' "reset);
  printf("\nAutor(es): ");
  scanf("%[^\n]s", info); setbuf(stdin, NULL);
  strncpy(artigo.autor, info, TAM);
  
  printf("\nTitulo: ");
  scanf("%[^\n]s", info); setbuf(stdin, NULL);
  strncpy(artigo.titulo, info, TAM);
  
  printf("\nRevista: ");
  scanf("%[^\n]s", info); setbuf(stdin, NULL);
  strncpy(artigo.revista, info, TAM);
  
  printf("\nDOI: ");
  scanf("%[^\n]s", info); setbuf(stdin, NULL);
  strncpy(artigo.DOI, info, TAM);

  setbuf(stdin, NULL);
  
  printf(HCYN"\n[!] Separar as palavras chave utilizando ',' sem dar espaço."reset);
  printf("\nPalavras Chave: ");
  scanf("%[^\n]s", info); 
  strncpy(artigo.palavraChave, info, TAM);

  FILE *tabelaReg = fopen(REG_FILE, "r+b");

  verificarArquivo(tabelaReg);
  
  Cabecalho cabecalho;
  fread(&cabecalho, sizeof(Cabecalho), 1, tabelaReg);

  
  BPTreeInserir(artigo.id, cabecalho.qtde);
  
  rewind(tabelaReg);
  fwrite(&cabecalho, sizeof(Cabecalho), 1, tabelaReg);
  
  fseek(tabelaReg, sizeof(Artigo) * cabecalho.qtde, SEEK_CUR);
  fwrite(&artigo, sizeof(Artigo), 1, tabelaReg);
  fclose(tabelaReg);
}


void 
importarTabela () {
  FILE *tabelaReg = fopen(REG_FILE, "rb");
  
  verificarArquivo(tabelaReg);
  
  BPTreeInicializar();

  Cabecalho cabecalho;
  Artigo artigo;

  fread(&cabecalho, sizeof(Cabecalho), 1, tabelaReg);
  
  for (int i = 0; i < cabecalho.qtde; i++) {
    fread(&artigo, sizeof(Artigo), 1, tabelaReg);
    BPTreeInserir(artigo.id, i);
  }

  fclose(tabelaReg);
}


void 
reciclarIndex (int index) {
  FILE* arquivoReciclados = fopen(RECYCLE_FILE, "r+b");
  verificarArquivo(arquivoReciclados);

  Cabecalho cabecalho;
  fread(&cabecalho, sizeof(int), 1, arquivoReciclados);

  fseek(arquivoReciclados, sizeof(int) * cabecalho.qtde, SEEK_CUR);
  fwrite(&index, sizeof(int), 1, arquivoReciclados);
  cabecalho.qtde++;
  
  rewind(arquivoReciclados);
  fwrite(&cabecalho, sizeof(int), 1, arquivoReciclados);
  
  fclose(arquivoReciclados);
}


int 
procurarReciclados () {
  FILE* arquivoReciclados = fopen(RECYCLE_FILE, "r+b");
  int index = -1;
  
  Cabecalho cabecalho;
  fread(&cabecalho, sizeof(Cabecalho), 1, arquivoReciclados);

  if (cabecalho.qtde > 0){
    fseek(arquivoReciclados, sizeof(int) * (cabecalho.qtde - 1), SEEK_CUR);
    fread(&index, sizeof(int), 1, arquivoReciclados);
    cabecalho.qtde--;
    rewind(arquivoReciclados);
    fwrite(&cabecalho, sizeof(Cabecalho), 1, arquivoReciclados);
  }
  
  fclose(arquivoReciclados);
  return index;
}
