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

typedef struct cabecalhoVeiculo
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
}cabecalhoVeiculo_t;

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

void freeVehicleHeaderFields(char **header, int numberOfFields);
void freeVehicleHeader(cabecalhoVeiculo_t* cabecalhoVeiculo);
char* readVehicleRegister(FILE* dataFileReference, vehicle_t* vehicleRegister);
int calculateTamanhoDoRegistroVeiculo(vehicle_t* vehicleRegister);
void writeVehicleRegister(FILE* tableFileReference,vehicle_t* vehicleRegister);
void freeVehicleRegister(vehicle_t* vehicleRegister);
cabecalhoVeiculo_t* createVehicleHeader();
void insertVehicleHeaderDataInStructure(
  char status,
  long long byteProxRegistro,
  char** headerFields,
  int nroRegistros, 
  int nroRegRemovidos, 
  cabecalhoVeiculo_t* cabecalhoVeiculo
);
void writeVehicleHeader(FILE* tableFileReference, cabecalhoVeiculo_t* cabecalhoVeiculo);
void getDataNula(char* data);
int isNullVehicleRegister(FILE* tableFileReference);
void jumpVehicleHeader(FILE* tableFileReference);
char* getFormatedDate(char* date);
void readVehicleRegistersFromBinaryTable(FILE* tableFileReference, vehicle_t* vehicleRegister);
void readVehicleRegistersFromBinaryTableWithCondition(
  FILE* tableFileReference, 
  vehicle_t* vehicleRegister, 
  char* fieldName, 
  char* value
);
void readVehicleRegisterBIN(FILE* tableFileReference, vehicle_t* vehicleRegister);
void printVehicleRegisterSelectedBy(vehicle_t* vehicleRegister, char* fieldName,char* value);
void setByteOffset(FILE* tableFileReference, long long byteOffset);
int getNroDeRegistros(FILE* tableFileReference);
void setNroDeRegistros(FILE* tableFileReference, int nroDeRegistros);


int createVehicleTable(char *dataFileName, char *tableFileName)
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

  char **headerFields = readHeader(dataFileReference, HEADER_SIZE, NUMBER_OF_COLUMNS_VEHICLE);
  jumpVehicleHeader(tableFileReference);

  vehicle_t *vehicleRegister = createVehicleRegister();

  int countRemovidos=0;
  int countRegistros=0;
  while(readVehicleRegister(dataFileReference, vehicleRegister) != NULL)
  { 
    countRegistros++;
    if(vehicleRegister->removido[0] == '0') { countRemovidos++; }
    writeVehicleRegister(tableFileReference, vehicleRegister);
  }

  freeVehicleRegister(vehicleRegister);

  cabecalhoVeiculo_t* cabecalhoVeiculo = createVehicleHeader();

  insertVehicleHeaderDataInStructure(
    '1', 
    ftell(tableFileReference),
    headerFields, 
    countRegistros, 
    countRemovidos,  
    cabecalhoVeiculo
  );    
  writeVehicleHeader(tableFileReference, cabecalhoVeiculo);
  freeVehicleHeaderFields(headerFields, NUMBER_OF_COLUMNS_VEHICLE);
  freeVehicleHeader(cabecalhoVeiculo);
  fclose(dataFileReference);
  fclose(tableFileReference);

  return 1;
}

void jumpVehicleHeader(FILE* tableFileReference) 
{
  fseek(tableFileReference, HEADER_SIZE+1, SEEK_SET);
}

vehicle_t* createVehicleRegister()
{
  return (vehicle_t*)malloc(sizeof(vehicle_t));
}

void freeVehicleHeaderFields(char **header, int numberOfFields)
{
  for(int i=0; i < numberOfFields; i++)
  {
    free(header[i]);
  }
  free(header);
}

void freeVehicleHeader(cabecalhoVeiculo_t* cabecalhoVeiculo)
{
  free(cabecalhoVeiculo);
}

void freeVehicleRegister(vehicle_t* vehicleRegister)
{
  free(vehicleRegister);
}

char* readVehicleRegister(FILE* dataFileReference, vehicle_t* vehicleRegister)
{
  char vehicleData[STRING_SIZE];

  char *referenceCopy = fgets(vehicleData, STRING_SIZE, dataFileReference);
  
  char **vehicleDataFields = splitString(vehicleData, NUMBER_OF_COLUMNS_VEHICLE, ",");
  
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
    tamanhoDoRegistro = calculateTamanhoDoRegistroVeiculo(vehicleRegister) + 1;

  } else 
  { 
    strcpy(vehicleRegister->prefixo, vehicleDataFields[0]);
    vehicleRegister->removido[0] ='1'; 
    tamanhoDoRegistro = calculateTamanhoDoRegistroVeiculo(vehicleRegister);
  }

  vehicleRegister->tamanhoRegistro = tamanhoDoRegistro;
}

void getDataNula(char* data)
{
  for (int i = 0; i < TAMANHO_DATA; i++)
  {
    data[i] = '@';
  }
};

int calculateTamanhoDoRegistroVeiculo(vehicle_t* vehicleRegister)
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

cabecalhoVeiculo_t* createVehicleHeader()
{
  return (cabecalhoVeiculo_t*)malloc(sizeof(cabecalhoVeiculo_t));
}

void insertVehicleHeaderDataInStructure(
  char status,
  long long byteProxRegistro,
  char** headerFields,
  int nroRegistros, 
  int nroRegRemovidos, 
  cabecalhoVeiculo_t* cabecalhoVeiculo
)
{
  cabecalhoVeiculo->status[0] = status;
  cabecalhoVeiculo->byteProxReg = byteProxRegistro;
  cabecalhoVeiculo->nroRegistros = nroRegistros - nroRegRemovidos;
  cabecalhoVeiculo->nroRegRemovidos = nroRegRemovidos;
  strcpy(cabecalhoVeiculo->descrevePrefixo, headerFields[0]);
  strcpy(cabecalhoVeiculo->descreveData, headerFields[1]);
  strcpy(cabecalhoVeiculo->descreveLugares, headerFields[2]);
  strcpy(cabecalhoVeiculo->descreveLinha, headerFields[3]);
  strcpy(cabecalhoVeiculo->descreveModelo, headerFields[4]);
  strcpy(cabecalhoVeiculo->descreveCategoria, headerFields[5]);
}

void writeVehicleHeader(FILE* tableFileReference, cabecalhoVeiculo_t* cabecalhoVeiculo)
{
  goToFileStart(tableFileReference);
  
  fwrite(cabecalhoVeiculo->status, sizeof(char), 1, tableFileReference);
  fwrite(&cabecalhoVeiculo->byteProxReg, sizeof(long long int), 1, tableFileReference);
  fwrite(&cabecalhoVeiculo->nroRegistros, sizeof(int), 1, tableFileReference);
  fwrite(&cabecalhoVeiculo->nroRegRemovidos, sizeof(int), 1, tableFileReference);
  fwrite(cabecalhoVeiculo->descrevePrefixo, sizeof(char), TAMANHO_DESCREVE_PREFIXO, tableFileReference);
  fwrite(cabecalhoVeiculo->descreveData, sizeof(char), TAMANHO_DESCREVE_DATA, tableFileReference);
  fwrite(cabecalhoVeiculo->descreveLugares, sizeof(char), TAMANHO_DESCREVE_LUGARES, tableFileReference);
  fwrite(cabecalhoVeiculo->descreveLinha, sizeof(char), TAMANHO_DESCREVE_LINHA, tableFileReference);
  fwrite(cabecalhoVeiculo->descreveModelo, sizeof(char), TAMANHO_DESCREVE_MODELO, tableFileReference);
  fwrite(cabecalhoVeiculo->descreveCategoria, sizeof(char), TAMANHO_DESCREVE_CATEGORIA, tableFileReference);
}

void selectVehicleRegistersFrom(char* tableFileName) 
{
  FILE* tableFileReference = fopen(tableFileName, "rb");
  if(!fileDidOpen(tableFileReference) 
    || isFileCorrupted(tableFileReference))
  {
    printf("Falha no processamento do arquivo.\n");
    return;
  }

  if(isNullVehicleRegister(tableFileReference))
  { 
    printf("Registro inexistente.\n"); 
  } else
  {
    jumpVehicleHeader(tableFileReference);

    vehicle_t* vehicleRegister = createVehicleRegister();
    readVehicleRegistersFromBinaryTable(tableFileReference, vehicleRegister);

    freeVehicleRegister(vehicleRegister);
  }

  fclose(tableFileReference);
}

int isNullVehicleRegister(FILE* tableFileReference)
{
  long long byteProxRegistro = getByteOffset(tableFileReference);
  fread(&byteProxRegistro, sizeof(long long), 1, tableFileReference);
  goToFileStart(tableFileReference);
  return (byteProxRegistro <= HEADER_SIZE+1);
}

void readVehicleRegistersFromBinaryTable(FILE* tableFileReference, vehicle_t* vehicleRegister)
{
  while ( fread(vehicleRegister->removido, sizeof(char), 1, tableFileReference) != 0)
  {
    readVehicleRegisterBIN(tableFileReference, vehicleRegister);
        
    printVehicleRegister(vehicleRegister);
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
  if(!fileDidOpen(tableFileReference) 
    || isFileCorrupted(tableFileReference))
  {
    printf("Falha no processamento do arquivo.\n");
    return;
  }

  if(isNullVehicleRegister(tableFileReference))
  { 
    printf("Registro inexistente.\n"); 
  } else 
  {
    jumpVehicleHeader(tableFileReference);

    vehicle_t* vehicleRegister = createVehicleRegister();
    readVehicleRegistersFromBinaryTableWithCondition(
      tableFileReference, 
      vehicleRegister, 
      fieldName, 
      value
    );

    freeVehicleRegister(vehicleRegister);
  }

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

int insertVehicleRegisterIntoTable(
  char* tableFileName, 
  vehicle_t** vehicleRegisters, 
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

  if(isNullVehicleRegister(tableFileReference))
  { 
    printf("Registro inexistente.\n"); 
  } else
  {
    goToFileEnd(tableFileReference);
  
    for (int i = 0; i < numberOfRegisters; i++)
    {
      writeVehicleRegister(tableFileReference, vehicleRegisters[i]);
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
