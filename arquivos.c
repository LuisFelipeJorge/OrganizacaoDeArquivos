#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "arquivos.h"

// Funções auxiliares para o tratamento generico dos 2 tipos de arquivos

#define STRING_SIZE 50

void getFileName(char *fileName)
{
  scanf("%s", fileName);
}

int fileDidOpen(FILE* fileRefence) {
  if (!fileRefence) 
  {
    return 0;
  } return 1;
}

char **splitString(char *string, int numberOfFields, char *delimeter)
{
  // Função para separar os campos de uma string por um delimitador definido
  // retorna os campos no formato de um vetor de string, isto é, uma matriz de caracteres
  // cada linha da matriz é uma string que representa um dos campos
  
  // Alocando a memoria
  char **splitedStrings = (char**)malloc(sizeof(char*)*numberOfFields);
  memorySuccessfullyAllocated((void**)splitedStrings);

  for (int i = 0; i < numberOfFields; i++)
  {
      splitedStrings[i] = (char*)malloc(sizeof(char)*STRING_SIZE);
      memorySuccessfullyAllocated((void**)&splitedStrings[i]);
  }
  
  // Cópia da referência, para assegurar que o valor original não é alterado
  char *stringCopy = strdup(string);
  // string já separada
  char *separatedString;  

  int fieldAdress = 0;
  // Enquanto houver tolkens, continue separando
  while((separatedString = strsep(&stringCopy, delimeter)))
  { 
    // copiando o token dentro do vetor de strings
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


void scan_quote_string(char *str) {

	/*
	*	Use essa função para ler um campo string delimitado entre aspas (").
	*	Chame ela na hora que for ler tal campo. Por exemplo:
	*
	*	A entrada está da seguinte forma:
	*		nomeDoCampo "MARIA DA SILVA"
	*
	*	Para ler isso para as strings já alocadas str1 e str2 do seu programa, você faz:
	*		scanf("%s", str1); // Vai salvar nomeDoCampo em str1
	*		scan_quote_string(str2); // Vai salvar MARIA DA SILVA em str2 (sem as aspas)
	*
	*/

	char R;

	while((R = getchar()) != EOF && isspace(R)); // ignorar espaços, \r, \n...

	if(R == 'N' || R == 'n') { // campo NULO
		getchar(); getchar(); getchar(); // ignorar o "ULO" de NULO.
		strcpy(str, ""); // copia string vazia
	} else if(R == '\"') {
		if(scanf("%[^\"]", str) != 1) { // ler até o fechamento das aspas
			strcpy(str, "");
		}
		getchar(); // ignorar aspas fechando
	} else if(R != EOF){ // vc tá tentando ler uma string que não tá entre aspas! Fazer leitura normal %s então, pois deve ser algum inteiro ou algo assim...
		str[0] = R;
		scanf("%s", &str[1]);
	} else { // EOF
		strcpy(str, "");
	}
}


/* ---------------- EXTRA ----------------

OPCIONAL: dicas sobre scanf() e fscanf():

scanf("%[^,]", string) -> lê até encontrar o caractere ',', não incluindo o mesmo na leitura.

Exemplo de entrada: "Oi, esse é um exemplo."
Nesse caso, o scanf("%[^,]") tem como resultado a string "Oi";

scanf("%[^\"]", string) -> lê até encontrar o caractere '"', não incluindo o mesmo na leitura.
scanf("%[^\n]", string) -> lê até encontrar o fim da linha, não incluindo o '\n' na leitura.

scanf("%*c") --> lê um char e não guarda em nenhuma variável, como se tivesse ignorado ele

*/


void setStatus(FILE* fileReference, char status)
{
  goToFileStart(fileReference);
  fwrite(&status, sizeof(char), 1, fileReference);
  goToFileStart(fileReference);
}

char getStatus(FILE* fileReference)
{
  char status;
  goToFileStart(fileReference);
  fread(&status, sizeof(char), 1, fileReference);
  goToFileStart(fileReference);
  return status;
}

void goToFileStart(FILE* fileReference)
{
  fseek(fileReference, 0, SEEK_SET);
}

void goToFileEnd(FILE* fileReference)
{
  fseek(fileReference, 0, SEEK_END);
}

int isNullField(char* field)
{
  return (strcmp(field, "NULO") == 0 || strlen(field) == 0);
}

char** readHeader(FILE* dataFileReference, int headerSize, int numberOfColumns)
{
  char header[headerSize];
  fgets(header, headerSize, dataFileReference);
  return splitString(header, numberOfColumns, ",");
}

long long getByteOffset(FILE* tableFileReference)
{
  fseek(tableFileReference, 1, SEEK_SET);
  long long byteProxRegistro;
  fread(&byteProxRegistro, sizeof(long long), 1, tableFileReference);
  return byteProxRegistro;
}

int isFileCorrupted(FILE* tableFileReference) 
{
  return (getStatus(tableFileReference) == '0');
}

void setByteOffset(FILE* tableFileReference, long long byteOffset)
{
  fseek(tableFileReference, 1, SEEK_SET);
  fwrite(&byteOffset, sizeof(long long), 1, tableFileReference);
}

int getNroDeRegistros(FILE* tableFileReference)
{
  int nroDeRegistros;
  fseek(tableFileReference, 9, SEEK_SET);
  fread(&nroDeRegistros, sizeof(int), 1,tableFileReference);
  return nroDeRegistros;
}

int getNroDeRegistrosRemovidos(FILE* tableFileReference)
{
  int nroDeRegistrosRemovidos;
  fseek(tableFileReference, 13, SEEK_SET);
  fread(&nroDeRegistrosRemovidos, sizeof(int), 1,tableFileReference);
  return nroDeRegistrosRemovidos;
}

void setNroDeRegistros(FILE* tableFileReference, int nroDeRegistros)
{
  fseek(tableFileReference, 9, SEEK_SET);
  fwrite(&nroDeRegistros, sizeof(int), 1, tableFileReference);
}

void setNroDeRegistrosRemovidos(FILE* tableFileReference, int nroDeRegistrosRemovidos)
{
  fseek(tableFileReference, 13, SEEK_SET);
  fwrite(&nroDeRegistrosRemovidos, sizeof(int), 1, tableFileReference);
}


void resetStrings(char **fields, int numberOfFields, int stringsSize)
{
  for (int j = 0; j < numberOfFields; j++)
  {
    for (int w = 0; w < stringsSize; w++)
    {
        fields[j][w] = '\0';
    }
  }
}

int isRegisterRemoved(char campoRemovido)
{
  return (campoRemovido == '0');
}