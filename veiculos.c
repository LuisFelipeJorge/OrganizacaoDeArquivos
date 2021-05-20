#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "veiculos.h"
#include "arquivos.h"

#define STRING_SIZE 100
#define HEADER_SIZE 174
#define NUMBER_OF_COLUMNS 6

#define TAMANHO_STATUS 1
#define TAMANHO_DESCREVE_PREFIXO 18
#define TAMANHO_DESCREVE_DATA 35
#define TAMANHO_DESCREVE_LUGARES 42
#define TAMANHO_DESCREVE_LINHA 26
#define TAMANHO_DESCREVE_MODELO 17
#define TAMANHO_DESCREVE_CATEGORIA 20

#define TAMANHO_PREFIXO 5
#define TAMANHO_DATA 10
#define TAMANHO_REMOVIDO 1

#define NULL_FIELD_INT -1

typedef struct cabecalho
{
  char status[TAMANHO_STATUS];
  long long int byteProxReg;
  int nroRegistros;
  int nroRegRemovidos;
  char descrevePrefixo[TAMANHO_DESCREVE_PREFIXO+1];
  char descreveData[TAMANHO_DESCREVE_DATA+1];
  char descreveLugares[TAMANHO_DESCREVE_LUGARES+1];
  char descreveLinha[TAMANHO_DESCREVE_LINHA+1];
  char descreveModelo[TAMANHO_DESCREVE_MODELO+1];
  char descreveCategoria[TAMANHO_DESCREVE_CATEGORIA+1];
}cabecalho_t;

struct vehicle
{
  char prefixo[TAMANHO_PREFIXO+1];
  char data[TAMANHO_DATA+1];
  int quantidadeDeLugares;
  int codigoLinha;
  char modelo[STRING_SIZE];
  char categoria[STRING_SIZE];
  char removido[TAMANHO_REMOVIDO];
  int tamanhoRegistro;
  int tamanhoModelo;
  int tamanhoCategoria;
};

vehicle_t* createVehicleRegister();
char** readHeader(FILE *dataFileReference);
void freeHeader(char **header, int numberOfFields);
char* readVehicleRegister(FILE* dataFileReference, vehicle_t* vehicleRegister);
void insertVehicleDataInStructure(char** vehicleDataFields, vehicle_t* vehicleRegister);
int isNullField(char* field);
int calculateTamanhoDoRegistro(vehicle_t* vehicleRegister);
void printVehicleRegister(vehicle_t* vehicleRegister);
void writeVehicleRegister(FILE* tableFileReference,vehicle_t* vehicleRegister);
cabecalho_t* createHeader();
void insertHeaderDataInStructure(FILE* tableFileReference, char** headerFields,int nroRegistros, int nroRegRemovidos, cabecalho_t* cabecalho);
void goToFileEnd(FILE* fileReference);
void writeHeader(FILE* tableFileReference, cabecalho_t* cabecalho);
void goToFileStart(FILE* fileReference);
void getDataNula(char* data);






void createVehicleTable(char *dataFileName, char *tableFileName)
{
  FILE* dataFileReference = fopen(dataFileName, "r");    
  fileDidOpen(dataFileReference);
  
  FILE* tableFileReference = fopen(tableFileName, "rb+");
  fileDidOpen(tableFileReference);

  char **headerFields = readHeader(dataFileReference);
  fseek(tableFileReference, HEADER_SIZE+1, SEEK_SET);

  vehicle_t *vehicleRegister = createVehicleRegister();

  int countRemovidos=0;
  int countRegistros=0;
  while(readVehicleRegister(dataFileReference, vehicleRegister) != NULL)
  { 
    countRegistros++;
    if(vehicleRegister->removido[0] == '0') { countRemovidos++; }
    writeVehicleRegister(tableFileReference, vehicleRegister);
  }

  free(vehicleRegister);

  cabecalho_t* cabecalho = createHeader();

  insertHeaderDataInStructure(tableFileReference, headerFields, countRegistros, countRemovidos,  cabecalho);    
  writeHeader(tableFileReference, cabecalho);
  freeHeader(headerFields, NUMBER_OF_COLUMNS);

  fclose(dataFileReference);
  fclose(tableFileReference);
}

vehicle_t* createVehicleRegister()
{
  return (vehicle_t*)malloc(sizeof(vehicle_t));
}

char** readHeader(FILE* dataFileReference)
{
  char header[HEADER_SIZE];
  fgets(header, HEADER_SIZE, dataFileReference);
  return splitString(header, NUMBER_OF_COLUMNS, ",");
}

void freeHeader(char **header, int numberOfFields)
{
  for(int i=0; i < numberOfFields; i++)
  {
    free(header[i]);
  }
  free(header);
}

char* readVehicleRegister(FILE* dataFileReference, vehicle_t* vehicleRegister)
{
  char vehicleData[STRING_SIZE];

  char *referenceCopy = fgets(vehicleData, STRING_SIZE, dataFileReference);
  
  char **vehicleDataFields = splitString(vehicleData, NUMBER_OF_COLUMNS, ",");
  
  insertVehicleDataInStructure(vehicleDataFields, vehicleRegister);

  return referenceCopy; 
}

void insertVehicleDataInStructure(char** vehicleDataFields, vehicle_t* vehicleRegister)
{

  if(!isNullField(vehicleDataFields[1])) 
  { 
    strcpy(vehicleRegister->data, vehicleDataFields[1]);
  } else 
  {  
    char dataNula[TAMANHO_DATA];
    getDataNula(dataNula);
    strcpy(vehicleRegister->data, dataNula); 
    vehicleRegister->data[0] = '\0'; 
  }

  if (!isNullField(vehicleDataFields[2])) 
  { 
    vehicleRegister->quantidadeDeLugares = atoi(vehicleDataFields[2]);
  } else 
  { 
    vehicleRegister->quantidadeDeLugares = NULL_FIELD_INT; 
  }
  
  if (!isNullField(vehicleDataFields[3])) 
  { 
    vehicleRegister->codigoLinha = atoi(vehicleDataFields[3]);
  } else 
  { 
    vehicleRegister->codigoLinha = NULL_FIELD_INT;
  }

  if (!isNullField(vehicleDataFields[4])) 
  { 
    strcpy(vehicleRegister->modelo, vehicleDataFields[4]); 
    vehicleRegister->tamanhoModelo=strlen(vehicleDataFields[4]);
  } else 
  { 
    strcpy(vehicleRegister->modelo, ""); 
    vehicleRegister->tamanhoModelo = 0;
  }

  if (!isNullField(vehicleDataFields[5])) 
  { 
    strcpy(vehicleRegister->categoria, vehicleDataFields[5]);
    vehicleRegister->tamanhoCategoria=strlen(vehicleDataFields[5])-1;
  } else 
  { 
    strcpy(vehicleRegister->categoria, "");
    vehicleRegister->tamanhoCategoria = 0;
  }

  int tamanhoDoRegistro;
  if(vehicleDataFields[0][0] == '*') 
  { 
    strcpy(vehicleRegister->prefixo, &vehicleDataFields[0][1]);
    vehicleRegister->removido[0]='0';
    tamanhoDoRegistro = calculateTamanhoDoRegistro(vehicleRegister) + 1;

  } else 
  { 
    strcpy(vehicleRegister->prefixo, vehicleDataFields[0]);
    vehicleRegister->removido[0] ='1'; 
    tamanhoDoRegistro = calculateTamanhoDoRegistro(vehicleRegister);
  }

  vehicleRegister->tamanhoRegistro = tamanhoDoRegistro;
}

int isNullField(char* field)
{
  return (strcmp(field, "NULO") == 0);
}

void getDataNula(char* data)
{
  for (int i = 0; i < TAMANHO_DATA; i++)
  {
    data[i] = '@';
  }
};

int calculateTamanhoDoRegistro(vehicle_t* vehicleRegister)
{
  int quantidadeDeLugares, codigoLinha, tamanhoDoModelo, tamanhoDaCategoria;

  quantidadeDeLugares=codigoLinha=tamanhoDoModelo=tamanhoDaCategoria=sizeof(int);

  return (
    strlen(vehicleRegister->prefixo)
    + TAMANHO_DATA
    + quantidadeDeLugares
    + codigoLinha
    + tamanhoDoModelo
    + vehicleRegister->tamanhoModelo
    + tamanhoDaCategoria
    + vehicleRegister->tamanhoCategoria
  );
}

void printVehicleRegister(vehicle_t* vehicleRegister)
{
  printf("prefixo: %s\n", vehicleRegister->prefixo);
  printf("data: %s\n", vehicleRegister->data);
  printf("qtd de lugares: %d\n", vehicleRegister->quantidadeDeLugares);
  printf("cod da linha: %d\n", vehicleRegister->codigoLinha);
  printf("modelo: %s\n", vehicleRegister->modelo);
  printf("categoria: %s\n", vehicleRegister->categoria);
  printf("removido: %s\n", vehicleRegister->removido);
  printf("tamanho do reg: %d\n", vehicleRegister->tamanhoRegistro);
  printf("tamanho do mod: %d\n", vehicleRegister->tamanhoModelo);
  printf("tamanho da categ: %d\n", vehicleRegister->tamanhoCategoria);
}

void writeVehicleRegister(FILE* tableFileReference,vehicle_t* vehicleRegister)
{
  fwrite(vehicleRegister->removido, sizeof(char), TAMANHO_REMOVIDO, tableFileReference);
  fwrite(&vehicleRegister->tamanhoRegistro, sizeof(int), 1, tableFileReference);
  fwrite(vehicleRegister->prefixo, sizeof(char), TAMANHO_PREFIXO, tableFileReference);
  fwrite(vehicleRegister->data, sizeof(char), TAMANHO_DATA, tableFileReference);
  fwrite(&vehicleRegister->quantidadeDeLugares, sizeof(int), 1, tableFileReference);
  fwrite(&vehicleRegister->codigoLinha, sizeof(int), 1, tableFileReference);
  fwrite(&vehicleRegister->tamanhoModelo, sizeof(int), 1, tableFileReference);
  fwrite(vehicleRegister->modelo, sizeof(char), vehicleRegister->tamanhoModelo, tableFileReference);
  fwrite(&vehicleRegister->tamanhoCategoria, sizeof(int), 1, tableFileReference);
  fwrite(vehicleRegister->categoria, sizeof(char), vehicleRegister->tamanhoCategoria, tableFileReference);
}

cabecalho_t* createHeader()
{
  return (cabecalho_t*)malloc(sizeof(cabecalho_t));
}

void insertHeaderDataInStructure(FILE* tableFileReference, char** headerFields,int nroRegistros, int nroRegRemovidos, cabecalho_t* cabecalho)
{
  goToFileEnd(tableFileReference);

  cabecalho->status[0] = '1';
  cabecalho->byteProxReg = ftell(tableFileReference);
  cabecalho->nroRegistros = nroRegistros - nroRegRemovidos;
  cabecalho->nroRegRemovidos = nroRegRemovidos;
  strcpy(cabecalho->descrevePrefixo, headerFields[0]);
  strcpy(cabecalho->descreveData, headerFields[1]);
  strcpy(cabecalho->descreveLugares, headerFields[2]);
  strcpy(cabecalho->descreveLinha, headerFields[3]);
  strcpy(cabecalho->descreveModelo, headerFields[4]);
  strcpy(cabecalho->descreveCategoria, headerFields[5]);
}

void goToFileEnd(FILE* fileReference)
{
  fseek(fileReference, 0, SEEK_END);
}

void writeHeader(FILE* tableFileReference, cabecalho_t* cabecalho)
{
  goToFileStart(tableFileReference);
  
  fwrite(cabecalho->status, sizeof(char), 1, tableFileReference);
  fwrite(&cabecalho->byteProxReg, sizeof(long long int), 1, tableFileReference);
  fwrite(&cabecalho->nroRegistros, sizeof(int), 1, tableFileReference);
  fwrite(&cabecalho->nroRegRemovidos, sizeof(int), 1, tableFileReference);
  fwrite(cabecalho->descrevePrefixo, sizeof(char), TAMANHO_DESCREVE_PREFIXO, tableFileReference);
  fwrite(cabecalho->descreveData, sizeof(char), TAMANHO_DESCREVE_DATA, tableFileReference);
  fwrite(cabecalho->descreveLugares, sizeof(char), TAMANHO_DESCREVE_LUGARES, tableFileReference);
  fwrite(cabecalho->descreveLinha, sizeof(char), TAMANHO_DESCREVE_LINHA, tableFileReference);
  fwrite(cabecalho->descreveModelo, sizeof(char), TAMANHO_DESCREVE_MODELO, tableFileReference);
  fwrite(cabecalho->descreveCategoria, sizeof(char), TAMANHO_DESCREVE_CATEGORIA, tableFileReference);
}

void goToFileStart(FILE* fileReference)
{
  fseek(fileReference, 0, SEEK_SET);
}