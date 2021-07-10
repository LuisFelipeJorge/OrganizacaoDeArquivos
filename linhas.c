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

typedef struct cabecalhoLinha
{
  char status[TAMANHO_STATUS];
  long long int byteProxReg;
  int nroRegistros;
  int nroRegRemovidos;
  char descreveCodigo[TAMANHO_DESCREVE_CODIGO+1];
  char descreveCartao[TAMANHO_DESCREVE_CARTAO+1];
  char descreveNome[TAMANHO_DESCREVE_NOME+1];
  char descreveLinha[TAMANHO_DESCREVE_LINHA+1];
}cabecalhoLinha_t;

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

void writeLineRegister(FILE* tableFileReference, line_t* lineRegister);
int calculateTamanhoDoRegistroLinha(line_t* lineRegister);
cabecalhoLinha_t* createLineHeader();
void insertLineHeaderDataInStructure(
  char status,
  long long byteProxRegistro,
  char** headerFields,
  int nroRegistros, 
  int nroRegRemovidos, 
  cabecalhoLinha_t* cabecalhoLinha
);  
void writeLineHeader(FILE* tableFileReference, cabecalhoLinha_t* cabecalhoLinha);
void freeLineHeaderFields(char **header, int numberOfFields);
void freeLineHeader(cabecalhoLinha_t* cabecalhoLinha);
int isNullLineRegister(FILE* tableFileReference);
void readLineRegistersFromBinaryTable(FILE* tableFileReference, line_t* lineRegister);
void readLineRegistersFromBinaryTableWithCondition
(
  FILE* tableFileReference, 
  line_t* lineRegister,
  char* fieldName,
  char* value
);
void printLineRegisterSelectedBy(line_t* lineRegister, char* fieldName,char* value);





int createLineTable(char *dataFileName, char* tableFileName)
{
  FILE* dataFileReference = fopen(dataFileName, "r");   
  if(!fileDidOpen(dataFileReference))
  {
    printf("Falha no processamento do arquivo.\n");
    return 0;
  } 
    
  FILE* tableFileReference = fopen(tableFileName, "wb+");
  if (!fileDidOpen(tableFileReference))
  {
    printf("Falha no processamento do arquivo.\n");
    return 0;
  }

  setStatus(tableFileReference, '0');

  char **headerFields = readHeader(dataFileReference, HEADER_SIZE, NUMBER_OF_COLUMNS_LINES);
  jumpLineHeader(tableFileReference);

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

  cabecalhoLinha_t* cabecalhoLinha = createLineHeader();

  insertLineHeaderDataInStructure(
    '1', 
    ftell(tableFileReference), 
    headerFields, 
    countRegistros, 
    countRemovidos,  
    cabecalhoLinha
  );    

  writeLineHeader(tableFileReference, cabecalhoLinha);
  freeLineHeaderFields(headerFields, NUMBER_OF_COLUMNS_LINES);
  freeLineHeader(cabecalhoLinha);

  fclose(dataFileReference);
  fclose(tableFileReference);

  return 1;

}

void jumpLineHeader(FILE* tableFileReference) 
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
  
  char **lineDataFields = splitString(lineData, NUMBER_OF_COLUMNS_LINES, ",");
  
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
    tamanhoDoRegistro = calculateTamanhoDoRegistroLinha(lineRegister);
  } else 
  { 
    lineRegister->codigoLinha = atoi(lineDataFields[0]);
    lineRegister->removido[0] ='1'; 
    tamanhoDoRegistro = calculateTamanhoDoRegistroLinha(lineRegister);
  }

  lineRegister->tamanhoRegistro = tamanhoDoRegistro;
}

int calculateTamanhoDoRegistroLinha(line_t* lineRegister)
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

cabecalhoLinha_t* createLineHeader()
{
  return (cabecalhoLinha_t*)malloc(sizeof(cabecalhoLinha_t));
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

void insertLineHeaderDataInStructure(
  char status,
  long long byteProxRegistro,
  char** headerFields,
  int nroRegistros, 
  int nroRegRemovidos, 
  cabecalhoLinha_t* cabecalhoLinha
)
{
  cabecalhoLinha->status[0] = status;
  cabecalhoLinha->byteProxReg = byteProxRegistro;
  cabecalhoLinha->nroRegistros = nroRegistros - nroRegRemovidos;
  cabecalhoLinha->nroRegRemovidos = nroRegRemovidos;
  strcpy(cabecalhoLinha->descreveCodigo, headerFields[0]);
  strcpy(cabecalhoLinha->descreveCartao, headerFields[1]);
  strcpy(cabecalhoLinha->descreveNome, headerFields[2]);
  strcpy(cabecalhoLinha->descreveLinha, headerFields[3]);
}  
  
void writeLineHeader(FILE* tableFileReference, cabecalhoLinha_t* cabecalhoLinha)
{
  goToFileStart(tableFileReference);
  
  fwrite(cabecalhoLinha->status, sizeof(char), 1, tableFileReference);
  fwrite(&cabecalhoLinha->byteProxReg, sizeof(long long), 1, tableFileReference);
  fwrite(&cabecalhoLinha->nroRegistros, sizeof(int), 1, tableFileReference);
  fwrite(&cabecalhoLinha->nroRegRemovidos, sizeof(int), 1, tableFileReference);
  fwrite(cabecalhoLinha->descreveCodigo, sizeof(char), TAMANHO_DESCREVE_CODIGO, tableFileReference);
  fwrite(cabecalhoLinha->descreveCartao, sizeof(char), TAMANHO_DESCREVE_CARTAO, tableFileReference);
  fwrite(cabecalhoLinha->descreveNome, sizeof(char), TAMANHO_DESCREVE_NOME, tableFileReference);
  fwrite(cabecalhoLinha->descreveLinha, sizeof(char), TAMANHO_DESCREVE_LINHA, tableFileReference);
}  

void freeLineHeaderFields(char **header, int numberOfFields)
{
  for(int i=0; i < numberOfFields; i++)
  {
    free(header[i]);
  }
  free(header);
}

void freeLineHeader(cabecalhoLinha_t* cabecalhoLinha)
{
  free(cabecalhoLinha);
}

void selectLineRegistersFrom(char* tableFileName) 
{
  FILE* tableFileReference = fopen(tableFileName, "rb");
  if(!fileDidOpen(tableFileReference) 
    || isFileCorrupted(tableFileReference))
  {
    printf("Falha no processamento do arquivo.\n");
    return;
  }

  if(isNullLineRegister(tableFileReference))
  { 
    printf("Registro inexistente.\n"); 
  } else
  {
    jumpLineHeader(tableFileReference);

    line_t* lineRegister = createLineRegister();
    readLineRegistersFromBinaryTable(tableFileReference, lineRegister);
    
    freeLineRegister(lineRegister);
  }

  fclose(tableFileReference);
}

int isNullLineRegister(FILE* tableFileReference)
{
  long long byteProxRegistro = getByteOffset(tableFileReference);
  fread(&byteProxRegistro, sizeof(long long), 1, tableFileReference);
  goToFileStart(tableFileReference);
  return (byteProxRegistro <= HEADER_SIZE+1);
}

void readLineRegistersFromBinaryTable(FILE* tableFileReference, line_t* lineRegister)
{
  while (fread(lineRegister->removido, sizeof(char), 1, tableFileReference) != 0)
  {
    readLineRegisterBIN(tableFileReference, lineRegister);
        
    printLineRegister(lineRegister);
  }
}

void readLineRegisterBIN(FILE* tableFileReference, line_t* lineRegister)
{
  fread(&lineRegister->tamanhoRegistro, sizeof(int), 1, tableFileReference);
  fread(&lineRegister->codigoLinha, sizeof(int), 1, tableFileReference);
  fread(lineRegister->aceitaCartao, sizeof(char), 1, tableFileReference);
  lineRegister->aceitaCartao[TAMANHO_CARTAO] = '\0';
  fread(&lineRegister->tamanhoNome, sizeof(int), 1, tableFileReference);
  fread(lineRegister->nomeLinha, sizeof(char), lineRegister->tamanhoNome, tableFileReference);
  lineRegister->nomeLinha[lineRegister->tamanhoNome] = '\0';
  fread(&lineRegister->tamanhoCor, sizeof(int), 1, tableFileReference);
  fread(lineRegister->corLinha, sizeof(char), lineRegister->tamanhoCor, tableFileReference);
  lineRegister->corLinha[lineRegister->tamanhoCor] = '\0';
}

void printLineRegister(line_t* lineRegister)
{

  if (lineRegister->removido[0] == '1')
  {
    printf("Codigo da linha: %d\n", lineRegister->codigoLinha);
    if(lineRegister->tamanhoNome != 0)
    {
      printf("Nome da linha: %s\n", lineRegister->nomeLinha);
    }else
    {
      printf("Nome da linha: campo com valor nulo\n");
    }
    if(lineRegister->tamanhoCor != 0)
    {
      printf("Cor que descreve a linha: %s\n", lineRegister->corLinha);
    }else
    {
      printf("Cor que descreve a linha: campo com valor nulo\n");
    }
    if (strlen(lineRegister->aceitaCartao) != 0)
    {
      printf("Aceita cartao: ");
      if (lineRegister->aceitaCartao[0] == 'S')
      {
        printf("PAGAMENTO SOMENTE COM CARTAO SEM PRESENCA DE COBRADOR\n");
      } else if (lineRegister->aceitaCartao[0] == 'N')
      {
        printf("PAGAMENTO EM CARTAO E DINHEIRO\n");
      }else if(lineRegister->aceitaCartao[0] == 'F')
      {
        printf("PAGAMENTO EM CARTAO SOMENTE NO FINAL DE SEMANA\n");
      }
    }else
    {
      printf("Aceita cartao: campo com valor nulo\n");
    }

    printf("\n");
  }
}

void selectLineRegistersFromWhere(char* tableFileName, char* fieldName, char* value) 
{
  FILE* tableFileReference = fopen(tableFileName, "rb");
  if(!fileDidOpen(tableFileReference) 
    || isFileCorrupted(tableFileReference))
  {
    printf("Falha no processamento do arquivo.\n");
    return;
  }

  if(isNullLineRegister(tableFileReference))
  { 
    printf("Registro inexistente.\n"); 
  } else 
  {
    jumpLineHeader(tableFileReference);

    line_t* lineRegister = createLineRegister();
    readLineRegistersFromBinaryTableWithCondition(
      tableFileReference, 
      lineRegister, 
      fieldName, 
      value
    );

    freeLineRegister(lineRegister);
  }

  fclose(tableFileReference);
}


void readLineRegistersFromBinaryTableWithCondition
(
  FILE* tableFileReference, 
  line_t* lineRegister,
  char* fieldName,
  char* value
)
{
  while ( fread(lineRegister->removido, sizeof(char), 1, tableFileReference) != 0)
  {
    readLineRegisterBIN(tableFileReference, lineRegister);    

    printLineRegisterSelectedBy(lineRegister, fieldName, value);
  }
}


void printLineRegisterSelectedBy(line_t* lineRegister, char* fieldName,char* value)
{
  if (strcmp(fieldName, "codLinha") == 0)
  {
    if(lineRegister->codigoLinha == atoi(value))
    {
      printLineRegister(lineRegister);
    }
  }
  if (strcmp(fieldName, "aceitaCartao") == 0)
  {
    if(strcmp(lineRegister->aceitaCartao, value) == 0)
    {
      printLineRegister(lineRegister);
    }
  }
  if (strcmp(fieldName, "nomeLinha") == 0)
  {
    if(strcmp(lineRegister->nomeLinha, value) == 0)
    {
      printLineRegister(lineRegister);
    }
  }
  if (strcmp(fieldName, "corLinha") == 0)
  {
    if(strcmp(lineRegister->corLinha, value) == 0)
    {
      printLineRegister(lineRegister);
    }
  }
}

int insertLineRegistersIntoTable(
  char* tableFileName, 
  line_t** lineRegisters, 
  int numberOfRegisters
)
{
  FILE* tableFileReference = fopen(tableFileName, "rb+");
  if(!fileDidOpen(tableFileReference) 
    || isFileCorrupted(tableFileReference))
  {
    printf("Falha no processamento do arquivo.\n");
    return 0;
  }

  setStatus(tableFileReference, '0');

  if(isNullLineRegister(tableFileReference))
  { 
    printf("Registro inexistente.\n"); 
  } else
  {
    goToFileEnd(tableFileReference);
  
    for (int i = 0; i < numberOfRegisters; i++)
    {
      writeLineRegister(tableFileReference, lineRegisters[i]);
    }
  }

  long long newByteOffset = ftell(tableFileReference);
  setByteOffset(tableFileReference, newByteOffset);

  int newNroDeRegistros = getNroDeRegistros(tableFileReference);
  newNroDeRegistros += numberOfRegisters;
  setNroDeRegistros(tableFileReference, newNroDeRegistros);


  setStatus(tableFileReference, '1');
  fclose(tableFileReference);

  return 1;
}


//////////////////////////////////////////////////////////
// funçoes para manter o tad fechado e auxiliar à arvore B

int getCodLinha(line_t* lineRegister)
{
  return lineRegister->codigoLinha;
}

void setRemovidoLine(line_t* lineRegister, char campoRemovido)
{
  lineRegister->removido[0] = campoRemovido;
}


int insertLineRegisterIntoTable(
  FILE* tableFileReference, 
  line_t* lineRegister, 
  long* lastResgisterInserted
)
{
  if(isNullLineRegister(tableFileReference))
  { 
    printf("Registro inexistente.\n");
    return 0; 
  } 
  goToFileEnd(tableFileReference);
  *lastResgisterInserted = ftell(tableFileReference);
  writeLineRegister(tableFileReference, lineRegister);

  long long newByteOffset = ftell(tableFileReference);
  setByteOffset(tableFileReference, newByteOffset);

  int newNroDeRegistros = getNroDeRegistros(tableFileReference);
  newNroDeRegistros++;
  setNroDeRegistros(tableFileReference, newNroDeRegistros);

  return 1;
}
