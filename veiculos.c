#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "veiculos.h"
#include "arquivos.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Tamanho dos campos de cabeçalho e de registro (em Bytes) definidos no documento de requisitos

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

// definindo como deve ser considerado um campo do tipo int nulo
#define NULL_FIELD_INT -1

struct cabecalhoVeiculo
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
};

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

///////////////////////////////////////////////////////////////////////////////////////////////////
// Funções auxiliares para o tratamento dos registros de veículos

void freeVehicleHeaderFields(char **header, int numberOfFields);
char* readVehicleRegister(FILE* dataFileReference, vehicle_t* vehicleRegister);
int calculateTamanhoDoRegistroVeiculo(vehicle_t* vehicleRegister);
void getDataNula(char* data);
int isNullVehicleRegister(FILE* tableFileReference);
char* getFormatedDate(char* date);
void readVehicleRegistersFromBinaryTable(FILE* tableFileReference);
void readVehicleRegistersFromBinaryTableWithCondition(
  FILE* tableFileReference,  
  char* fieldName, 
  char* value
);

vehicle_t** sortVehicleRegisters(FILE* tableVehicleReference);
void constructVehicleRegisterArray(FILE* tableVehicleReference, vehicle_t** array);
int compareVehicles(const void* v1, const void* v2);
void freeSortedVehicleRegister(vehicle_t** array, int arraySize);
void writeSortedVehicleTable(vehicle_t** array, int arraySize, FILE* fileReference, cabecalhoVeiculo_t* header);
void copyVehicleRegister(vehicle_t* destination, vehicle_t* source);
///////////////////////////////////////////////////////////////////////////////////////////////////
// Definição da Funções

// VEICULOS - Cria uma tabela de nome tableFileName com as informaçoes de dataFileName (.csv)
//retorna 0 se tiver dado erro e 1 se tiver dado certo
int createVehicleTable(char *dataFileName, char *tableFileName)
{
  //abre o arquivo de referencia para leitura
  FILE* dataFileReference = fopen(dataFileName, "r");   
  //ARQUIVOS - se não conseguir abrir mostra mensagem de erro de processamento de arquivo
  if(!fileDidOpen(dataFileReference))
  {
    printf("Falha no processamento do arquivo.\n");
    return 0;
  } 
  //cria o arquivo para insercao de dados/tabela e o abre para escrita
  FILE* tableFileReference = fopen(tableFileName, "wb+");
  //ARQUIVOS - se não conseguir criar e inserir mostra mensagem de erro de processamento de arquivo
  if (!fileDidOpen(tableFileReference))
  {
    printf("Falha no processamento do arquivo.\n");
    return 0;
  }
  //ARQUIVOS - insere o status do arquivo de abertura como 0, para verificar que o arquivo foi manuseado de forma
  // adequada 
  setStatus(tableFileReference, '0');

  //ARQUIVOS -  separa os campos do header de acordo com as especificacoes do trabalho
  char **headerFields = readHeader(dataFileReference, HEADER_SIZE, NUMBER_OF_COLUMNS_VEHICLE);
  
  //VEICULOS - seta o ponteiro do arquivo para depois do header
  jumpVehicleHeader(tableFileReference);

  // cria um registro para salvar os veiculos
  vehicle_t *vehicleRegister = createVehicleRegister();

  int countRemovidos=0;
  int countRegistros=0;
  // Enquanto houver conteúdo dentro do CSV (arquivo de dados), continue lendo e fazendo a escrita no arquivo binário(tabela)
  while(readVehicleRegister(dataFileReference, vehicleRegister) != NULL)
  { 
    countRegistros++;
    //se o arquivo tiver sido logicemente removido adiciona um ao contador de removidos
    if(vehicleRegister->removido[0] == '0') { countRemovidos++; }
    //insere o registro do veiculo na tabela/arquivo binario
    writeVehicleRegister(tableFileReference, vehicleRegister);
  }
  //libera o ponteiro de registro de veiculo
  freeVehicleRegister(vehicleRegister);

  //cria um ponteiro novo para o cabecalho do veiculo
  cabecalhoVeiculo_t* cabecalhoVeiculo = createVehicleHeader();

  // preechendo a estrutura cabeçalho com a informação colhida do arquivo csv
  insertVehicleHeaderDataInStructure(
    '1', 
    ftell(tableFileReference),
    headerFields, 
    countRegistros, 
    countRemovidos,  
    cabecalhoVeiculo
  );    
  // Com a estrutura completa, escrevemos ela dentro do arquivo binário
  writeVehicleHeader(tableFileReference, cabecalhoVeiculo);
  //libera estruturas
  freeVehicleHeaderFields(headerFields, NUMBER_OF_COLUMNS_VEHICLE);
  freeVehicleHeader(cabecalhoVeiculo);
  //fecha arquivos
  fclose(dataFileReference);
  fclose(tableFileReference);

  return 1;
}

// seta o ponteiro do arquivo para depois do header
void jumpVehicleHeader(FILE* tableFileReference) 
{
  fseek(tableFileReference, HEADER_SIZE+1, SEEK_SET);
}
//Veiculos - cria um ponteiro do tipo cabecalhoVeiculo que é uma struct e aloca apenas o tamanho na memoria de um cabecalho 
vehicle_t* createVehicleRegister()
{
  vehicle_t* newVehicle = (vehicle_t*)malloc(sizeof(vehicle_t));
  if (newVehicle == NULL)
  {
    perror("Falha em alocar memoria para veiculo");
    exit(EXIT_FAILURE);
  }
  return newVehicle;
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
  // String para ler uma linha inteira do arquivo csv
  char vehicleData[STRING_SIZE];

  char *referenceCopy = fgets(vehicleData, STRING_SIZE, dataFileReference);
  //   Separando os campos da linha pela vírgula
  char **vehicleDataFields = splitString(vehicleData, NUMBER_OF_COLUMNS_VEHICLE, ",");
  // copiando as informações dos campos em uma estrutura de veículo
  insertVehicleDataInStructure(vehicleDataFields, vehicleRegister);

  return referenceCopy; // estado da leitura, (NULL) -> sem mais dados para ler
}

// Função para o tratamento da informação
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
    // pelo comportamento da função strcpy, é necessário adicionoar o \0 após o preenchimeno da estrutura 
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
    // no csv, os campos de categoria possuem \n, que é contabilizado pelo strlen, dai a necessidade de 
    // se corrigir o tamanho da string
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
    // o campo prefixo de um registro logicamente removido possui 4 caracteres ao inves de 5
    // dai a necessidade de se corrigir o tamanho do registro
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
  cabecalhoVeiculo_t* newHeader = (cabecalhoVeiculo_t*)malloc(sizeof(cabecalhoVeiculo_t));
  if (newHeader == NULL)
  {
    perror("Falha em alocar memoria");
    exit(EXIT_FAILURE);
  }
  
  return newHeader;
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

// SELECT - pega todos os dados da tabela
void selectVehicleRegistersFrom(char* tableFileName) 
{
  //abre o arquivo da tabela 
  FILE* tableFileReference = fopen(tableFileName, "rb");
  //verifica se teve problemas ao abrir o arquivo ou se ele está comrrompido
  if(!fileDidOpen(tableFileReference) 
    || isFileCorrupted(tableFileReference))
  {
    printf("Falha no processamento do arquivo.\n");
    return;
  }
  // verifica se tem algum registri cadastro na tabela
  if(isNullVehicleRegister(tableFileReference))
  { 
    printf("Registro inexistente.\n"); 
  } else
  {
    //pula o cabecalho
    jumpVehicleHeader(tableFileReference);
    
    // lê a tabela de registros de veiculos
    readVehicleRegistersFromBinaryTable(tableFileReference);
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

void readVehicleRegistersFromBinaryTable(FILE* tableFileReference)
{
  // cria um registro de veiculos vazio
  vehicle_t* vehicleRegister = createVehicleRegister();

  // enquanto houver bytes para serem lidos continue printando
  while (fread(vehicleRegister->removido, sizeof(char), 1, tableFileReference) != 0)
  {
    //le cada registro 
    readVehicleRegisterBIN(tableFileReference, vehicleRegister);
    //imprime o que foi lido de forma organizada    
    printVehicleRegister(vehicleRegister);
  }
  
  freeVehicleRegister(vehicleRegister);
}

void readVehicleRegisterBIN(FILE* tableFileReference, vehicle_t* vehicleRegister)
{
  // necessario adicionar \0 nas strings, para o funcionamento adequada da função printf
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
  }
}

char* getFormatedDate(char* date)
{
  char** dateFields = splitString(date, 3, "-");
  int month = atoi(dateFields[1]);
  char* dateString = (char*)calloc((STRING_SIZE/4), sizeof(char));

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

    readVehicleRegistersFromBinaryTableWithCondition(
      tableFileReference, 
      fieldName, 
      value
    );
  }

  fclose(tableFileReference);
}

void readVehicleRegistersFromBinaryTableWithCondition
(
  FILE* tableFileReference, 
  char* fieldName,
  char* value
)
{
  vehicle_t* vehicleRegister = createVehicleRegister();

  while ( fread(vehicleRegister->removido, sizeof(char), 1, tableFileReference) != 0)
  {
    readVehicleRegisterBIN(tableFileReference, vehicleRegister);    

    printVehicleRegisterSelectedBy(vehicleRegister, fieldName, value);
  }

  freeVehicleRegister(vehicleRegister);
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

int insertVehicleRegistersIntoTable(
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

//////////////////////////////////////////////////////////
// funçoes para manter o tad fechado e auxiliar à arvore B

char* getPrefixo(vehicle_t* vehicleRegister)
{
  return vehicleRegister->prefixo;
}

void setRemovidoVehicle(vehicle_t* vehicleRegister, char campoRemovido)
{
  vehicleRegister->removido[0] = campoRemovido;
}

int getCodLinhaVeiculo(vehicle_t* vehicleRegister)
{
  return vehicleRegister->codigoLinha;
}

int insertVehicleRegisterIntoTable(
  FILE* tableFileReference, 
  vehicle_t* vehicleRegister, 
  long* lastResgisterInserted
)
{
  if(isNullVehicleRegister(tableFileReference))
  { 
    printf("Registro inexistente.\n");
    return 0; 
  } 
  goToFileEnd(tableFileReference);
  *lastResgisterInserted = ftell(tableFileReference);
  writeVehicleRegister(tableFileReference, vehicleRegister);

  long long newByteOffset = ftell(tableFileReference);
  setByteOffset(tableFileReference, newByteOffset);

  int newNroDeRegistros = getNroDeRegistros(tableFileReference);
  newNroDeRegistros++;
  setNroDeRegistros(tableFileReference, newNroDeRegistros);

  return 1;
}


int sortVehicleTable(char* tableVehicleName, char* sortedFileName)
{
  FILE* tableVehicleReference = fopen(tableVehicleName, "r");
  if(!fileDidOpen(tableVehicleReference) || isFileCorrupted(tableVehicleReference))
  {
    printf("Falha no processamento do arquivo.\n");
    return 0;
  }
  
  FILE* sortedReference = fopen(sortedFileName, "wb+");
  if(!fileDidOpen(tableVehicleReference))
  {
    printf("Falha no carregamento do arquivo.\n");
    return 0;
  }

  vehicle_t** sortedRegisters = sortVehicleRegisters(tableVehicleReference);

  if (sortedRegisters == NULL)
  {
    printf("Falha no carregamento do arquivo.\n");
    return 0;
  }

  int arraySize = getNroDeRegistros(tableVehicleReference);
  cabecalhoVeiculo_t* originalHeader = getCabecalhoVeiculo(tableVehicleReference);
  writeSortedVehicleTable(
    sortedRegisters, 
    arraySize, 
    sortedReference, 
    originalHeader
  );

  freeSortedVehicleRegister(sortedRegisters, arraySize);

  fclose(tableVehicleReference);
  fclose(sortedReference);
  return 1;
}

vehicle_t** sortVehicleRegisters(FILE* tableVehicleReference)
{
  int arraySize = getNroDeRegistros(tableVehicleReference);
  vehicle_t** sortedRegisters = (vehicle_t**)malloc(sizeof(vehicle_t*)*arraySize);
  constructVehicleRegisterArray(tableVehicleReference, sortedRegisters);
  qsort((void*)sortedRegisters, arraySize, sizeof(vehicle_t*), compareVehicles);
  return sortedRegisters;
}

void constructVehicleRegisterArray(FILE* tableVehicleReference, vehicle_t** array)
{
  jumpVehicleHeader(tableVehicleReference);
  vehicle_t* tempVehicle = createVehicleRegister();
  int arraiIdx = 0;
  while (fread(&tempVehicle->removido[0], sizeof(char), 1, tableVehicleReference) != 0)
  {
    readVehicleRegisterBIN(tableVehicleReference, tempVehicle);
    if (!isRegisterRemoved(tempVehicle->removido[0]))
    {
      array[arraiIdx] = createVehicleRegister();
      copyVehicleRegister(array[arraiIdx], tempVehicle);
      arraiIdx++;
    }
  }
}

int compareVehicles(const void* v1, const void* v2)
{
  vehicle_t* register1 = *(vehicle_t**)v1;
  vehicle_t* register2 = *(vehicle_t**)v2;   

  int codLinha1 = getCodLinhaVeiculo(register1);
  int codLinha2 = getCodLinhaVeiculo(register2);
  return codLinha1 - codLinha2;
}

void freeSortedVehicleRegister(vehicle_t** array, int arraySize)
{
  for (int i = 0; i < arraySize; i++)
  {
    freeVehicleRegister(array[i]);
  }
  free(array);
}

cabecalhoVeiculo_t* getCabecalhoVeiculo(FILE* tableFileReference)
{
  cabecalhoVeiculo_t* header = createVehicleHeader();
  
  goToFileStart(tableFileReference);
  fread(header->status, sizeof(char), 1, tableFileReference);
  fread(&header->byteProxReg, sizeof(long long), 1, tableFileReference);
  fread(&header->nroRegistros, sizeof(int), 1, tableFileReference);
  fread(&header->nroRegRemovidos, sizeof(int), 1, tableFileReference);
  fread(header->descrevePrefixo, sizeof(char), TAMANHO_DESCREVE_PREFIXO, tableFileReference);
  fread(header->descreveData, sizeof(char), TAMANHO_DESCREVE_DATA, tableFileReference);
  fread(header->descreveLugares, sizeof(char), TAMANHO_DESCREVE_LUGARES, tableFileReference);
  fread(header->descreveLinha, sizeof(char), TAMANHO_DESCREVE_LINHA, tableFileReference);
  fread(header->descreveModelo, sizeof(char), TAMANHO_DESCREVE_MODELO, tableFileReference);
  fread(header->descreveCategoria, sizeof(char), TAMANHO_DESCREVE_CATEGORIA, tableFileReference);
  return header;
}

void copyVehicleRegister(vehicle_t* destination, vehicle_t* source)
{
  memcpy(destination, source, sizeof(vehicle_t));
}

void writeSortedVehicleTable(vehicle_t** array, int arraySize, FILE* fileReference, cabecalhoVeiculo_t* header)
{
  writeVehicleHeader(fileReference, header);
  
  for (int i = 0; i < arraySize; i++)
  {
    writeVehicleRegister(fileReference, array[i]);
  }

  long long byteOffset = ftell(fileReference);
  setStatus(fileReference, '1');
  setByteOffset(fileReference, byteOffset);
  setNroDeRegistros(fileReference, arraySize);
  setNroDeRegistrosRemovidos(fileReference, 0);
}