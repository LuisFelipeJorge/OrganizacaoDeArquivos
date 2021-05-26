#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "linhas.h"
#include "arquivos.h"

#define STRING_SIZE 100
#define HEADER_SIZE 82

#define TAMANHO_STATUS 1
#define TAMANHO_DESCREVE_CODIGO 15
#define TAMANHO_DESCREVE_CARTAO 13
#define TAMANHO_DESCREVE_NOME 13
#define TAMANHO_DESCREVE_LINHA 24

#define TAMANHO_REMOVIDO 1
#define TAMANHO_CARTAO 1

#define NULL_FIELD_INT -1

typedef struct cabecalho
{
  char status[TAMANHO_STATUS];
  long long int byteProxReg;
  int nroRegistros;
  int nroRegRemovidos;
  char descreveCodigo[TAMANHO_DESCREVE_CODIGO+1];
  char descreveCartao[TAMANHO_DESCREVE_CARTAO+1];
  char descreveNome[TAMANHO_DESCREVE_NOME+1];
  char descreveLinha[TAMANHO_DESCREVE_LINHA+1];
}cabecalho_t;

struct line
{
  char nomeLinha[STRING_SIZE];
  char corLinha[STRING_SIZE];
  char removido[TAMANHO_REMOVIDO];
  char aceitaCartao[TAMANHO_CARTAO+1];
  int codigoLinha;
  int tamanhoRegistro;
  int tamanhoNome;
  int tamanhoCor;
};



char** readHeader(FILE* dataFileReference);
void setStatus(FILE* fileReference, char status);
void goToFileStart(FILE* fileReference);
void goToFileEnd(FILE* fileReference);
void jumpHeader(FILE* tableFileReference);
line_t* createLineRegister();
void freeLineRegister(line_t* lineRegister);
char* readLineRegister(FILE* dataFileReference, line_t* lineRegister);
void writeLineRegister(FILE* tableFileReference, line_t* lineRegister);
void insertLineDataInStructure(char** lineDataFields, line_t* lineRegister);
int isNullField(char* field);
int calculateTamanhoDoRegistro(line_t* lineRegister);
cabecalho_t* createHeader();
void insertHeaderDataInStructure(
  char status,
  long long byteProxRegistro,
  char** headerFields,
  int nroRegistros, 
  int nroRegRemovidos, 
  cabecalho_t* cabecalho
);  
void writeHeader(FILE* tableFileReference, cabecalho_t* cabecalho);
void freeHeader(char **header, int numberOfFields);



int createLineTable(char *dataFileName, char* tableFileName)
{
  FILE* dataFileReference = fopen(dataFileName, "r");   
  if(!fileDidOpen(dataFileReference, "Falha no processamento do arquivo."))
  {
    return 0;
  } 
    
  FILE* tableFileReference = fopen(tableFileName, "wb+");
  if (!fileDidOpen(tableFileReference, "Falha no processamento do arquivo."))
  {
    return 0;
  }

  setStatus(tableFileReference, '0');

  char **headerFields = readHeader(dataFileReference);
  jumpHeader(tableFileReference);

  line_t* lineRegister = createLineRegister();

  int countRemovidos = 0;
  int countRegistros = 0;
  while(readLineRegister(dataFileReference, lineRegister) != NULL)
  { 
    countRegistros++;
    if(lineRegister->removido[0] == '0') { countRemovidos++; }
    writeLineRegister(tableFileReference, lineRegister);
  }

  freeLineRegister(lineRegister);

  cabecalho_t* cabecalho = createHeader();

  insertHeaderDataInStructure(
    '1', 
    ftell(tableFileReference), 
    headerFields, countRegistros, 
    countRemovidos,  
    cabecalho
  );    

  writeHeader(tableFileReference, cabecalho);
  freeHeader(headerFields, NUMBER_OF_COLUMNS);

  fclose(dataFileReference);
  fclose(tableFileReference);

  return 1;

}

char** readHeader(FILE* dataFileReference)
{
  char header[HEADER_SIZE];
  fgets(header, HEADER_SIZE, dataFileReference);
  return splitString(header, NUMBER_OF_COLUMNS, ",");
}

void setStatus(FILE* fileReference, char status)
{
  goToFileStart(fileReference);
  fwrite(&status, sizeof(char), 1, fileReference);
  goToFileStart(fileReference);
}

void goToFileStart(FILE* fileReference)
{
  fseek(fileReference, 0, SEEK_SET);
}

void goToFileEnd(FILE* fileReference)
{
  fseek(fileReference, 0, SEEK_END);
}

void jumpHeader(FILE* tableFileReference) 
{
  fseek(tableFileReference, HEADER_SIZE, SEEK_SET);
}

line_t* createLineRegister()
{
  return (line_t*)malloc(sizeof(line_t));
}

void freeLineRegister(line_t* lineRegister)
{
  free(lineRegister);
}

char* readLineRegister(FILE* dataFileReference, line_t* lineRegister)
{
  char lineData[STRING_SIZE];

  char *referenceCopy = fgets(lineData, STRING_SIZE, dataFileReference);
  
  char **lineDataFields = splitString(lineData, NUMBER_OF_COLUMNS, ",");
  
  insertLineDataInStructure(lineDataFields, lineRegister);

  return referenceCopy; 
}

void insertLineDataInStructure(char** lineDataFields, line_t* lineRegister)
{  
  strcpy(lineRegister->aceitaCartao, lineDataFields[1]);
  if (!isNullField(lineDataFields[2])) 
  { 
    strcpy(lineRegister->nomeLinha, lineDataFields[2]); 
    lineRegister->tamanhoNome=strlen(lineDataFields[2]);
  } else 
  { 
    strcpy(lineRegister->nomeLinha, ""); 
    lineRegister->tamanhoNome = 0;
  }

  if (!isNullField(lineDataFields[3])) 
  { 
    strcpy(lineRegister->corLinha, lineDataFields[3]);
    lineRegister->tamanhoCor=strlen(lineDataFields[3])-1;
  } else 
  { 
    strcpy(lineRegister->corLinha, "");
    lineRegister->tamanhoCor = 0;
  }

  int tamanhoDoRegistro;

  if(lineDataFields[0][0] == '*') 
  { 
    lineRegister->codigoLinha = atoi(&lineDataFields[0][1]);
    lineRegister->removido[0]='0';
    tamanhoDoRegistro = calculateTamanhoDoRegistro(lineRegister);
  } else 
  { 
    lineRegister->codigoLinha = atoi(lineDataFields[0]);
    lineRegister->removido[0] ='1'; 
    tamanhoDoRegistro = calculateTamanhoDoRegistro(lineRegister);
  }

  lineRegister->tamanhoRegistro = tamanhoDoRegistro;
}

int isNullField(char* field)
{
  return (strcmp(field, "NULO") == 0 || strlen(field) == 0);
}

int calculateTamanhoDoRegistro(line_t* lineRegister)
{
  int codigoLinha, tamanhoNome, tamanhoCor;

  codigoLinha = tamanhoNome = tamanhoCor = sizeof(int);
  int aceitaCartao = sizeof(char);
  return (
    + codigoLinha
    + aceitaCartao
    + tamanhoNome
    + lineRegister->tamanhoNome
    + tamanhoCor
    + lineRegister->tamanhoCor
  );
}

cabecalho_t* createHeader()
{
  return (cabecalho_t*)malloc(sizeof(cabecalho_t));
}

void writeLineRegister(FILE* tableFileReference, line_t* lineRegister)
{
  fwrite(lineRegister->removido, sizeof(char), TAMANHO_REMOVIDO, tableFileReference);
  fwrite(&lineRegister->tamanhoRegistro, sizeof(int), 1, tableFileReference);
  fwrite(&lineRegister->codigoLinha, sizeof(int), 1, tableFileReference);
  fwrite(lineRegister->aceitaCartao, sizeof(char), 1, tableFileReference);
  fwrite(&lineRegister->tamanhoNome, sizeof(int), 1, tableFileReference);
  fwrite(lineRegister->nomeLinha, sizeof(char), lineRegister->tamanhoNome, tableFileReference);
  fwrite(&lineRegister->tamanhoCor, sizeof(int), 1, tableFileReference);
  fwrite(lineRegister->corLinha, sizeof(char), lineRegister->tamanhoCor, tableFileReference);
}

void insertHeaderDataInStructure(
  char status,
  long long byteProxRegistro,
  char** headerFields,
  int nroRegistros, 
  int nroRegRemovidos, 
  cabecalho_t* cabecalho
)
{
  cabecalho->status[0] = status;
  cabecalho->byteProxReg = byteProxRegistro;
  cabecalho->nroRegistros = nroRegistros - nroRegRemovidos;
  cabecalho->nroRegRemovidos = nroRegRemovidos;
  strcpy(cabecalho->descreveCodigo, headerFields[0]);
  strcpy(cabecalho->descreveCartao, headerFields[1]);
  strcpy(cabecalho->descreveNome, headerFields[2]);
  strcpy(cabecalho->descreveLinha, headerFields[3]);
}  
  
void writeHeader(FILE* tableFileReference, cabecalho_t* cabecalho)
{
  goToFileStart(tableFileReference);
  
  fwrite(cabecalho->status, sizeof(char), 1, tableFileReference);
  fwrite(&cabecalho->byteProxReg, sizeof(long long), 1, tableFileReference);
  fwrite(&cabecalho->nroRegistros, sizeof(int), 1, tableFileReference);
  fwrite(&cabecalho->nroRegRemovidos, sizeof(int), 1, tableFileReference);
  fwrite(cabecalho->descreveCodigo, sizeof(char), TAMANHO_DESCREVE_CODIGO, tableFileReference);
  fwrite(cabecalho->descreveCartao, sizeof(char), TAMANHO_DESCREVE_CARTAO, tableFileReference);
  fwrite(cabecalho->descreveNome, sizeof(char), TAMANHO_DESCREVE_NOME, tableFileReference);
  fwrite(cabecalho->descreveLinha, sizeof(char), TAMANHO_DESCREVE_LINHA, tableFileReference);
}  

void freeHeader(char **header, int numberOfFields)
{
  for(int i=0; i < numberOfFields; i++)
  {
    free(header[i]);
  }
  free(header);
}