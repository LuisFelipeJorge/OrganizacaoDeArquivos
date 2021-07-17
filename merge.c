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