#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "veiculos.h"
#include "arquivos.h"

#define STRING_SIZE 100
#define HEADER_SIZE 174

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

char** readHeader(FILE *dataFileReference);
void freeHeader(char **header, int numberOfFields);
char* readVehicleRegister(FILE* dataFileReference, vehicle_t* vehicleRegister);
int isNullField(char* field);
int calculateTamanhoDoRegistro(vehicle_t* vehicleRegister);
void writeVehicleRegister(FILE* tableFileReference,vehicle_t* vehicleRegister);
cabecalho_t* createHeader();
void insertHeaderDataInStructure(FILE* tableFileReference, char** headerFields,int nroRegistros, int nroRegRemovidos, cabecalho_t* cabecalho);
void goToFileEnd(FILE* fileReference);
void writeHeader(FILE* tableFileReference, cabecalho_t* cabecalho);
void goToFileStart(FILE* fileReference);
void getDataNula(char* data);
int isNullRegister(FILE* tableFileReference);
void jumpHeader(FILE* tableFileReference);
char* getFormatedDate(char* date);
void readVehicleRegistersFromBinaryTable(FILE* tableFileReference, vehicle_t* vehicleRegister);
void readVehicleRegistersFromBinaryTableWithCondition(FILE* tableFileReference, vehicle_t* vehicleRegister, char* fieldName, char* value);
void readVehicleRegisterBIN(FILE* tableFileReference, vehicle_t* vehicleRegister);
void printVehicleRegisterSelectedBy(vehicle_t* vehicleRegister, char* fieldName,char* value);
void setHeader(FILE* fileReference, int numberOfRegisters);

void createVehicleTable(char *dataFileName, char *tableFileName)
{
  FILE* dataFileReference = fopen(dataFileName, "r");    
  fileDidOpen(dataFileReference, "Falha no processamento do arquivo");
  
  FILE* tableFileReference = fopen(tableFileName, "rb+");
  fileDidOpen(tableFileReference, "Falha no processamento do arquivo");

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
  return (strcmp(field, "NULO") == 0 || strlen(field) == 0);
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

void selectVehicleRegistersFrom(char* tableFileName) 
{
  FILE* tableFileReference = fopen(tableFileName, "rb");
  fileDidOpen(tableFileReference, "Falha no processamento do arquivo");

  if(isNullRegister(tableFileReference)){ printf("Registro inexistente.\n"); }
  jumpHeader(tableFileReference);

  vehicle_t* vehicleRegister = createVehicleRegister();

  readVehicleRegistersFromBinaryTable(tableFileReference, vehicleRegister);

  fclose(tableFileReference);
}

int isNullRegister(FILE* tableFileReference)
{
  fseek(tableFileReference, 1, SEEK_SET);
  long long byteProxRegistro;
  fread(&byteProxRegistro, sizeof(long long), 1, tableFileReference);
  fseek(tableFileReference, 0, SEEK_SET);
  return (byteProxRegistro <=175);
}

void jumpHeader(FILE* tableFileReference) 
{
  fseek(tableFileReference, HEADER_SIZE+1, SEEK_SET);
}

void readVehicleRegistersFromBinaryTable(FILE* tableFileReference, vehicle_t* vehicleRegister)
{

  while ( fread(vehicleRegister->removido, sizeof(char), 1, tableFileReference) != 0)
  {
    readVehicleRegisterBIN(tableFileReference, vehicleRegister);
        
    printVehicleRegister(vehicleRegister);
  }
}

void printVehicleRegister(vehicle_t* vehicleRegister)
{

  if (vehicleRegister->removido[0] == '1')
  {
    printf("Prefixo do veiculo: %s\n", vehicleRegister->prefixo);
    if(vehicleRegister->tamanhoModelo != 0)
    {
      printf("Modelo do veiculo: %s\n", vehicleRegister->modelo);
    }else
    {
      printf("Modelo do veiculo: campo com valor nulo\n");
    }
    if(vehicleRegister->tamanhoCategoria != 0)
    {
      printf("Categoria do veiculo: %s\n", vehicleRegister->categoria);
    }else
    {
      printf("Categoria do veiculo: campo com valor nulo\n");
    }
    if (strlen(vehicleRegister->data) != 0)
    {
      printf("Data de entrada do veiculo na frota: %s\n", getFormatedDate(vehicleRegister->data));
    }else
    {
      printf("Data de entrada do veiculo na frota: campo com valor nulo\n");
    }
    if(vehicleRegister->quantidadeDeLugares != -1)
    {
      printf("Quantidade de lugares sentados disponiveis: %d\n", vehicleRegister->quantidadeDeLugares);
    }else
    {
      printf("Quantidade de lugares sentados disponiveis: campo com valor nulo\n");
    }

    printf("\n");
  }
}

char* getFormatedDate(char* date)
{
  char** dateFields = splitString(date, 3, "-");
  int month = atoi(dateFields[1]);
  char* dateString = (char*)malloc(sizeof(char)*(STRING_SIZE/4));
  char monthName[10];
  switch (month)
  {
    case 1:
      strcpy(monthName, "janeiro");
      break;

    case 2:
      strcpy(monthName, "fevereiro");
      break;

    case 3:
      strcpy(monthName, "março");
      break;

    case 4:
      strcpy(monthName, "abril");
      break;

    case 5:
      strcpy(monthName, "maio");
      break;

    case 6:
      strcpy(monthName, "junho");
      break;

    case 7:
      strcpy(monthName, "julho");
      break;

    case 8:
      strcpy(monthName, "agosto");
      break;

    case 9:
      strcpy(monthName, "setembro");
      break;

    case 10:
      strcpy(monthName, "outubro");
      break;

    case 11:
      strcpy(monthName, "novembro");
      break;

    case 12:
      strcpy(monthName, "dezembro");
      break;    

    default:
      break;
  }

  char day[3];
  strcpy(day, dateFields[2]);
  char year[5];
  strcpy(year, dateFields[0]);


  strcat(dateString, day);
  strcat(dateString, " de ");
  strcat(dateString, monthName);
  strcat(dateString, " de ");
  strcat(dateString, year);
  
  return dateString;
}

void selectVehicleRegistersFromWhere(char* tableFileName, char* fieldName, char* value) 
{
  FILE* tableFileReference = fopen(tableFileName, "rb");
  fileDidOpen(tableFileReference, "Falha no processamento do arquivo");

  if(isNullRegister(tableFileReference)){ printf("Registro inexistente.\n"); }
  jumpHeader(tableFileReference);

  vehicle_t* vehicleRegister = createVehicleRegister();

  readVehicleRegistersFromBinaryTableWithCondition(tableFileReference, vehicleRegister, fieldName, value);

  fclose(tableFileReference);
}

void readVehicleRegistersFromBinaryTableWithCondition
(
  FILE* tableFileReference, 
  vehicle_t* vehicleRegister,
  char* fieldName,
  char* value
)
{
  while ( fread(vehicleRegister->removido, sizeof(char), 1, tableFileReference) != 0)
  {
    readVehicleRegisterBIN(tableFileReference, vehicleRegister);    

    printVehicleRegisterSelectedBy(vehicleRegister, fieldName, value);
  }
}

void readVehicleRegisterBIN(FILE* tableFileReference, vehicle_t* vehicleRegister)
{
  fread(&vehicleRegister->tamanhoRegistro, sizeof(int), 1, tableFileReference);
  fread(vehicleRegister->prefixo, sizeof(char), TAMANHO_PREFIXO, tableFileReference);
  vehicleRegister->prefixo[TAMANHO_PREFIXO] = '\0';
  fread(vehicleRegister->data, sizeof(char), TAMANHO_DATA, tableFileReference);
  vehicleRegister->data[TAMANHO_DATA] = '\0';
  fread(&vehicleRegister->quantidadeDeLugares, sizeof(int), 1, tableFileReference);
  fread(&vehicleRegister->codigoLinha, sizeof(int), 1, tableFileReference);
  fread(&vehicleRegister->tamanhoModelo, sizeof(int), 1, tableFileReference);
  fread(vehicleRegister->modelo, sizeof(char), vehicleRegister->tamanhoModelo, tableFileReference);
  vehicleRegister->modelo[vehicleRegister->tamanhoModelo] = '\0';
  fread(&vehicleRegister->tamanhoCategoria, sizeof(int), 1, tableFileReference);
  fread(vehicleRegister->categoria, sizeof(char), vehicleRegister->tamanhoCategoria, tableFileReference);
  vehicleRegister->categoria[vehicleRegister->tamanhoCategoria] = '\0';
}

void printVehicleRegisterSelectedBy(vehicle_t* vehicleRegister, char* fieldName,char* value)
{
  if (strcmp(fieldName, "prefixo") == 0)
  {
    if(strcmp(vehicleRegister->prefixo, value) == 0)
    {
      printVehicleRegister(vehicleRegister);
    }
  }
  if (strcmp(fieldName, "data") == 0)
  {
    if(strcmp(vehicleRegister->data, value) == 0)
    {
      printVehicleRegister(vehicleRegister);
    }
  }
  if (strcmp(fieldName, "quantidadeLugares") == 0)
  {
    if(vehicleRegister->quantidadeDeLugares == atoi(value))
    {
      printVehicleRegister(vehicleRegister);
    }
  }
  if (strcmp(fieldName, "modelos") == 0)
  {
    if(strcmp(vehicleRegister->modelo, value) == 0)
    {
      printVehicleRegister(vehicleRegister);
    }
  }
  if (strcmp(fieldName, "categoria") == 0)
  {
    if(strcmp(vehicleRegister->categoria, value) == 0)
    {
      printVehicleRegister(vehicleRegister);
    }
  }
}

void insertVehicleRegisterIntoTable(char* tableFileName, vehicle_t** vehicleRegisters, int numberOfRegisters)
{
  FILE* tableFileReference = fopen(tableFileName, "rb+");
  fileDidOpen(tableFileReference, "Falha no processamento do arquivo.\n");

  goToFileEnd(tableFileReference);
 
  for (int i = 0; i < numberOfRegisters; i++)
  {
    writeVehicleRegister(tableFileReference, vehicleRegisters[i]);
  }
  setHeader(tableFileReference, numberOfRegisters);
  fclose(tableFileReference);
}

void setHeader(FILE* fileReference, int numberOfRegisters){
  goToFileEnd(fileReference);
  long long byteOffset = ftell(fileReference);
  goToFileStart(fileReference);
  fseek(fileReference, 1, SEEK_SET);
  fwrite(&byteOffset, sizeof(long long), 1, fileReference);
  int newNumberOfRegisters = 0;
  fread(&newNumberOfRegisters, sizeof(int), 1, fileReference);
  printf("numer registros: %d", newNumberOfRegisters);
  newNumberOfRegisters += numberOfRegisters;
  printf("numer registros: %d", newNumberOfRegisters);
  fseek(fileReference, 9, SEEK_SET);
  fwrite(&newNumberOfRegisters, sizeof(int), 1, fileReference);
}