#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arquivos.h"
#include "veiculos.h"
#include "linhas.h"


#define STRING_SIZE 50


int main(int argc, char const *argv[])
{
    int escolhaDafuncao;
    scanf("%d", &escolhaDafuncao);

    char dataFileName[STRING_SIZE];
    char tableFileName[STRING_SIZE];

    char campo[STRING_SIZE/2];
    char valor[STRING_SIZE/2];

    int numberOfNewRegisters;


    switch (escolhaDafuncao)
    {
    case 1:
        getFileName(dataFileName);
        getFileName(tableFileName); 

        if(createVehicleTable(dataFileName, tableFileName))
        {
            binarioNaTela(tableFileName);
        }
        break;

    case 2:
        getFileName(dataFileName);
        getFileName(tableFileName); 

        if(createLineTable(dataFileName, tableFileName))
        {
            binarioNaTela(tableFileName);
        }

        break;    

    case 3:
        getFileName(tableFileName); 

        selectVehicleRegistersFrom(tableFileName);
        break;

    case 4:
        getFileName(tableFileName); 

        selectLineRegistersFrom(tableFileName);
        break;

    case 5:

        getFileName(tableFileName);

        scanf("%s", campo);
        scan_quote_string(valor);
        selectVehicleRegistersFromWhere(tableFileName, campo, valor);
        
        break;

    case 6:

        getFileName(tableFileName);

        scanf("%s", campo);
        scan_quote_string(valor);
        selectLineRegistersFromWhere(tableFileName, campo, valor);
        
        break;

    case 7:
        getFileName(tableFileName);
    
        scanf("%d", &numberOfNewRegisters);

        vehicle_t** newVehicleRegisters = (vehicle_t**)malloc(sizeof(vehicle_t*));
        char** vehicleDataFields = (char**)malloc(sizeof(char*)*NUMBER_OF_COLUMNS_VEHICLE);
        for(int j=0; j < NUMBER_OF_COLUMNS_VEHICLE; j++){
            vehicleDataFields[j] = (char*)malloc(sizeof(char)*(STRING_SIZE));
        }

        for (int i = 0; i < numberOfNewRegisters; i++)
        {
            newVehicleRegisters[i] = createVehicleRegister();

            resetStrings(vehicleDataFields, NUMBER_OF_COLUMNS_VEHICLE, STRING_SIZE);

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
            insertVehicleDataInStructure((char**)vehicleDataFields, newVehicleRegisters[i]);
        }

        if(insertVehicleRegisterIntoTable(tableFileName, newVehicleRegisters, numberOfNewRegisters))
        {
            binarioNaTela(tableFileName);
        }

        for (int i = 0; i < numberOfNewRegisters; i++)
        {
            free(newVehicleRegisters[i]);
        }
        free(newVehicleRegisters);

        break;
    
    case 8:
        getFileName(tableFileName);
    
        scanf("%d", &numberOfNewRegisters);

        line_t** newLineRegisters = (line_t**)malloc(sizeof(line_t*));
        char** lineDataFields = (char**)malloc(sizeof(char*)*NUMBER_OF_COLUMNS_LINES);
        for(int j=0; j < NUMBER_OF_COLUMNS_LINES; j++){
            lineDataFields[j] = (char*)malloc(sizeof(char)*(STRING_SIZE));
        }

        for (int i = 0; i < numberOfNewRegisters; i++)
        {
            newLineRegisters[i] = createLineRegister();

            resetStrings(lineDataFields, NUMBER_OF_COLUMNS_LINES, STRING_SIZE);
            
            scanf("%s", lineDataFields[0]);
            scan_quote_string(lineDataFields[1]);
            scan_quote_string(lineDataFields[2]);
            scan_quote_string(lineDataFields[3]);

            // fazer ficar igual ao csv
            lineDataFields[3][strlen(lineDataFields[3])] = '\n';
            // corrigir logica de scanquote
            lineDataFields[3][strlen(lineDataFields[3])] = '\0';
            insertLineDataInStructure((char**)lineDataFields, newLineRegisters[i]);
        }

        if(insertLineRegisterIntoTable(tableFileName, newLineRegisters, numberOfNewRegisters))
        {
            binarioNaTela(tableFileName);
        }

        for (int i = 0; i < numberOfNewRegisters; i++)
        {
            free(newLineRegisters[i]);
        }
        free(newLineRegisters);



        break;

    default:
        break;
    }

    return 0;
}