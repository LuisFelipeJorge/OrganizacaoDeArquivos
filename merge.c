#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arquivos.h"
#include "veiculos.h"
#include "linhas.h"
#include "index.h"
#include "merge.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Tamanho dos campos de cabeçalho e de registro (em Bytes) definidos no documento de requisitos

#define STRING_SIZE 100

int mergeLoopAninhado(FILE* tableVehicleReference, FILE* tableLineReference);
int mergePorIndice(FILE* tableVehicleReference, FILE* tableLineReference, FILE* indexLineReference);
int mergeOrdenado(
  vehicle_t** sortedVehicleRegisters, 
  int vehicleArraySize, 
  line_t** sortedLineRegisters,
  int lineArraySize
);

int selectFromJoinBruto(char* tableVehicleName, char* tableLineName)
{
  FILE* tableVehicleReference = fopen(tableVehicleName, "r");
  if(!fileDidOpen(tableVehicleReference) || isFileCorrupted(tableVehicleReference))
  {
    printf("Falha no processamento do arquivo.\n");
    return 0;
  }
  FILE* tableLineReference = fopen(tableLineName, "r");
  if(!fileDidOpen(tableLineReference) || isFileCorrupted(tableLineReference))
  {
    printf("Falha no processamento do arquivo.\n");
    return 0;
  }

  if (!mergeLoopAninhado(tableVehicleReference, tableLineReference))
  {
    printf("Registro inexistente.\n");
  }

  fclose(tableVehicleReference);
  fclose(tableLineReference);
  return 1;
}


int mergeLoopAninhado(FILE* tableVehicleReference, FILE* tableLineReference)
{
  int hasJoined = 0;
  // pular os bytes do cabeçalho para iniciar a leitura dos registros
  jumpVehicleHeader(tableVehicleReference);

  vehicle_t* vehicleRegister = createVehicleRegister();
  line_t* lineRegister = createLineRegister();

  char campoRemovidoVeiculo;
  while (fread(&campoRemovidoVeiculo, sizeof(char), 1, tableVehicleReference) != 0)
  {
    setRemovidoVehicle(vehicleRegister, campoRemovidoVeiculo);
    readVehicleRegisterBIN(tableVehicleReference, vehicleRegister);
    if (!isRegisterRemoved(campoRemovidoVeiculo))
    {
      jumpLineHeader(tableLineReference);

      char camporRemovidoLinha;
      while (fread(&camporRemovidoLinha, sizeof(char), 1, tableLineReference) != 0)
      {
        setRemovidoLine(lineRegister, camporRemovidoLinha);
        readLineRegisterBIN(tableLineReference, lineRegister);
        if (!isRegisterRemoved(camporRemovidoLinha))
        {
          int codLinhaVeiculo = getCodLinhaVeiculo(vehicleRegister);
          int codLinha = getCodLinha(lineRegister);
          if (codLinha == codLinhaVeiculo)
          {
            hasJoined = 1;
            printVehicleRegister(vehicleRegister);
            printLineRegister(lineRegister);
            printf("\n");
          }
        }
      }
      goToFileStart(tableLineReference);
    }
  }
  return hasJoined;
}


int selectFromJoinPorIndice(char* tableVehicleName, char* tableLineName, char* indexLineName)
{
  FILE* tableVehicleReference = fopen(tableVehicleName, "r");
  if(!fileDidOpen(tableVehicleReference) || isFileCorrupted(tableVehicleReference))
  {
    printf("Falha no processamento do arquivo.\n");
    return 0;
  }
  
  FILE* tableLineReference = fopen(tableLineName, "r");
  if(!fileDidOpen(tableLineReference) || isFileCorrupted(tableLineReference))
  {
    printf("Falha no processamento do arquivo.\n");
    return 0;
  }

  FILE* indexLineReference = fopen(indexLineName, "r");
  if(!fileDidOpen(indexLineReference) || isFileCorrupted(indexLineReference))
  {
    printf("Falha no processamento do arquivo.\n");
    return 0;
  }

  if (!mergePorIndice(tableVehicleReference, tableLineReference, indexLineReference))
  {
    printf("Registro inexistente.\n");
  }

  fclose(tableVehicleReference);
  fclose(tableLineReference);
  return 1;
}


int mergePorIndice(FILE* tableVehicleReference, FILE* tableLineReference, FILE* indexLineReference)
{
  int hasJoined = 0;
  // pular os bytes do cabeçalho para iniciar a leitura dos registros
  jumpVehicleHeader(tableVehicleReference);

  vehicle_t* vehicleRegister = createVehicleRegister();
  line_t* lineRegister = createLineRegister();

  char campoRemovidoVeiculo;
  while (fread(&campoRemovidoVeiculo, sizeof(char), 1, tableVehicleReference) != 0)
  {
    setRemovidoVehicle(vehicleRegister, campoRemovidoVeiculo);
    readVehicleRegisterBIN(tableVehicleReference, vehicleRegister);
    if (!isRegisterRemoved(campoRemovidoVeiculo))
    {
      int codLinhaVeiculo = getCodLinhaVeiculo(vehicleRegister);
      long referenciaDaLinha = searchRegisterReference(codLinhaVeiculo, indexLineReference);
      if(referenciaDaLinha > 0)
      {
        hasJoined = 1;
        fseek(tableLineReference, referenciaDaLinha, SEEK_SET);
        char campoRemovidoLinha;
        fread(&campoRemovidoLinha, sizeof(char), 1, tableLineReference);
        setRemovidoLine(lineRegister, campoRemovidoLinha);
        readLineRegisterBIN(tableLineReference, lineRegister);

        printVehicleRegister(vehicleRegister);
        printLineRegister(lineRegister);
        printf("\n");
      }
    }
  }
  return hasJoined;
}

int selectFromJoinOrdenado(char* tableVehicleName, char* tableLineName)
{
  FILE* tableVehicleReference = fopen(tableVehicleName, "r");
  if(!fileDidOpen(tableVehicleReference) || isFileCorrupted(tableVehicleReference))
  {
    printf("Falha no processamento do arquivo.\n");
    return 0;
  }
  
  FILE* tableLineReference = fopen(tableLineName, "r");
  if(!fileDidOpen(tableLineReference) || isFileCorrupted(tableLineReference))
  {
    printf("Falha no processamento do arquivo.\n");
    return 0;
  }

  vehicle_t** sortedVehicleRegisters = sortVehicleRegisters(tableVehicleReference);
  line_t** sortedLineRegisters = sortLineRegisters(tableLineReference);
  if (sortedVehicleRegisters == NULL || sortedLineRegisters == NULL)
  {
    printf("Falha no carregamento do arquivo.\n");
    return 0;
  }
  int vehicleArraySize =  getNroDeRegistros(tableVehicleReference);
  int lineArraySize = getNroDeRegistros(tableLineReference);
  
  if(!mergeOrdenado(
    sortedVehicleRegisters, 
    vehicleArraySize, 
    sortedLineRegisters,
    lineArraySize
  ))
  {
    printf("Registro inexistente.\n");
  }
  
  freeSortedVehicleRegister(sortedVehicleRegisters,vehicleArraySize);
  freeSortedLineRegister(sortedLineRegisters, lineArraySize);

  fclose(tableVehicleReference);
  fclose(tableLineReference);
  return 1;
}

int mergeOrdenado(
  vehicle_t** sortedVehicleRegisters, 
  int vehicleArraySize, 
  line_t** sortedLineRegisters,
  int lineArraySize
)
{
  int hasJoined = 0;
  for (int i = 0; i < vehicleArraySize; i++)
  {
    for (int j = 0; j < lineArraySize; j++)
    {
      int vehicleCodLinha = getCodLinhaVeiculo(sortedVehicleRegisters[i]);
      int lineCodLinha = getCodLinha(sortedLineRegisters[j]);
      if(vehicleCodLinha == lineCodLinha)
      {
        printVehicleRegister(sortedVehicleRegisters[i]);
        printLineRegister(sortedLineRegisters[j]);
        printf("\n");
        hasJoined = 1;
      }
    }
  }  
  return hasJoined;
}