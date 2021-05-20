#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arquivos.h"

#define STRING_SIZE 50

void getFileName(char *fileName)
{
  scanf("%s", fileName);
}

void fileDidOpen(FILE* fileRefence) {
  if (!fileRefence) 
  {
    perror("fopen");
    exit(EXIT_FAILURE);
  }
}

char **splitString(char *string, int numberOfFields, char *delimeter)
{
    char **splitedStrings = (char**)malloc(sizeof(char*)*numberOfFields);
    memorySuccessfullyAllocated((void**)splitedStrings);

    for (int i = 0; i < numberOfFields; i++)
    {
        splitedStrings[i] = (char*)malloc(sizeof(char)*STRING_SIZE);
        memorySuccessfullyAllocated((void**)&splitedStrings[i]);
    }
    
    char *stringCopy = strdup(string);
    char *separatedString;  
  
    int fieldAdress = 0;
    while((separatedString = strsep(&stringCopy, delimeter)))
    { 
        strcpy(splitedStrings[fieldAdress], separatedString);
        fieldAdress++;
    }

    return splitedStrings;
}

void memorySuccessfullyAllocated(void **memoryReference)
{
    if ( memoryReference == NULL) 
    {
        perror("memory allocations");
        exit(EXIT_FAILURE);
    }
}

void binarioNaTela(char *nomeArquivoBinario) 
{
  /*Você não precisa entender o código dessa função. */
  /* Use essa função para comparação no run.codes. Lembre-se de ter fechado (fclose) o arquivo anteriormente.* 
   Ela vai abrir de novo para leitura e depois fechar (você não vai perder pontos por isso se usar ela).*/
  unsigned long i, cs;
  unsigned char *mb;
  size_t fl;
  FILE *fs;
  if(nomeArquivoBinario == NULL || !(fs = fopen(nomeArquivoBinario, "rb"))) 
  {
    fprintf(stderr, "%s%s%s%s",
      "ERRO AO ESCREVER O BINARIO NA TELA (função binarioNaTela):\n",
      "não foi possível abrir o arquivo que me passou para leitura.\n",
      "Ele existe evocê tá passando o nome certo? Você lembrou de\n", 
      "fechar ele com fclose depois de usar?\n"
    );
    return;
  }
  fseek(fs, 0, SEEK_END);
  fl = ftell(fs);
  fseek(fs, 0, SEEK_SET);
  mb = (unsigned char *) malloc(fl);fread(mb, 1, fl, fs);
  cs = 0;
  for(i = 0; i < fl; i++) 
  {
    cs += (unsigned long) mb[i];
  }
  printf("%lf\n", (cs / (double) 100));
  free(mb);
  fclose(fs);
}

