#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include "../headers/bplustree.h"
#include "../headers/pagina.h"

int 
main (int argc, char *argv[]) {
  system("clear");
  int opcoes;
  
  while (true) {
    setbuf(stdin,NULL);
    printf("\n=----------------OPÇÕES----------------=");
    printf(HWHT"\n[1] - Importar .txt com artigos para a tabela de registros"reset);
    printf(HWHT"\n[2] - Importar tabela de registros para a arvore"reset);
    printf(HGRN"\n[3] - Inserir um unico artigo na àrvore"reset);
    printf(HWHT"\n[4] - Imprimir estrutura da arvore"reset);
    printf(HWHT"\n[5] - Procurar um artigo a partir do ID"reset);
    printf(HWHT"\n[6] - Imprimir todos os artigos"reset);
    printf(HWHT"\n[7] - Imprimir artigos em ordem crescente a partir de um ID"reset);
    printf(HYEL"\n[8] - Excluir artigo pelo ID\n"reset);
    printf(RED"[9] - Sair do programa\n"reset);
    printf("=--------------------------------------=\n");
    printf(HMAG"Digite a opção [1-9] -> "reset);
    scanf("%d", &opcoes);
  
    switch (opcoes) {
      case 1: {
        char caminho[TAM];
        printf(HMAG"\nInforme o caminho do um arquivo ou 1 para usar o padrão: "reset);
        scanf("%s", caminho);
        if (strcmp(caminho, "1") != 0){
          criarTabelaRegistros(caminho);
        }
        else {
          criarTabelaRegistros(TXT_FILE);
        }
        break;
      }

      case 3:
        inserirnNovoArtigo();
        break;
  
      case 2:
        importarTabela();
        break;
      
      case 4:
        printf(HGRN"\n[*] Páginas Folhas  \n"reset);
        printf(HYEL"[*] Páginas Internas\n\n"reset);
        BPTreeImprimeArvoreCompleta();
        break;

      case 5: {
        int id, pos;
        printf(HMAG"\nInforme o ID do artigo: "reset);
        scanf("%d", &id);
        BPTreeImprimirArtigo(id);
        break;
      }

      case 6:{
        //IMPRIMIR TODOS OS ARTIGOS
        BPTreeImprimirTudo();
        break;
        
      }
      
      case 7: {
        int id;
        printf(HMAG"\nInforme o ID do artigo: "reset);
        scanf("%d", &id);
        BPTreeImprimirAPartir(id);
        break;
      }

      case 8: {
        int id;
        printf(HMAG"\nInforme o ID do artigo: "reset);
        scanf("%d", &id);

        BPTreeRemover(id);
        break;
      }
      
      case 9:
        exit(EXIT_SUCCESS);
      
      default:
        continue;
    }
  }
  
  return EXIT_SUCCESS;
}