#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arquivos.h"
#include "veiculos.h"

#define STRING_SIZE 50

int main(int argc, char const *argv[])
{
    char binaryFileName[STRING_SIZE];

    getFileName(binaryFileName);
    int numberOfNewRegisters;
    scanf("%d", &numberOfNewRegisters);
    vehicle_t** newRegisters = (vehicle_t**)malloc(sizeof(vehicle_t*));
    char** vehicleDataFields = (char**)malloc(sizeof(char*)*NUMBER_OF_COLUMNS);
    for(int j=0; j < NUMBER_OF_COLUMNS; j++){
        vehicleDataFields[j] = (char*)malloc(sizeof(char)*(STRING_SIZE));
    }
    // char vehicleDataFields[NUMBER_OF_COLUMNS][STRING_SIZE];    
    for (int i = 0; i < numberOfNewRegisters; i++)
    {
        newRegisters[i] = createVehicleRegister();
        // zerar antes da leitura
        for (int j = 0; j < NUMBER_OF_COLUMNS; j++)
        {
            for (int w = 0; w < STRING_SIZE; w++)
            {
                vehicleDataFields[j][w] = '\0';
            }
        }
        
        scan_quote_string(vehicleDataFields[0]);
        scan_quote_string(vehicleDataFields[1]);
        scan_quote_string(vehicleDataFields[2]);
        scan_quote_string(vehicleDataFields[3]);
        scan_quote_string(vehicleDataFields[4]);
        scan_quote_string(vehicleDataFields[5]);
        // fazer ficar igual ao csv
        vehicleDataFields[5][strlen(vehicleDataFields[5])] = '\n';
        // corrigir logica de scanquote
        vehicleDataFields[5][strlen(vehicleDataFields[5])] = '\0';
        insertVehicleDataInStructure((char**)vehicleDataFields, newRegisters[i]);
    }
       
    insertVehicleRegisterIntoTable(binaryFileName, newRegisters, numberOfNewRegisters);
    binarioNaTela(binaryFileName);
    return 0;
}
