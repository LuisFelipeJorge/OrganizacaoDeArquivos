#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arquivos.h"
#include "veiculos.h"
#include "linhas.h"
#include "index.h"
#include "merge.h"

#define STRING_SIZE 50

void readNewVehicleRegisters();
void freeNewVehicleRegisters(int numberOfNewRegisters, vehicle_t** newVehicleRegisters);

void readNewLineRegisters(int numberOfNewRegisters, line_t** newLineRegisters);
void freeNewLineRegisters(int numberOfNewRegisters, line_t** newLineRegisters);

int main(int argc, char const *argv[])
{
    int escolhaDafuncao;
    scanf("%d", &escolhaDafuncao);

    char dataFileName[STRING_SIZE];
    char tableFileName[STRING_SIZE];
    char indexFileName[STRING_SIZE];

    char campo[STRING_SIZE/2];
    char valor[STRING_SIZE/2];
    int numberOfNewRegisters;
    int codLinha;

    //menu pra escolher qual função vai executar
    switch (escolhaDafuncao)
    {
        // le os registros presentes em veiculo.csv e os armazena num arquivo de saida
        case 1:
            getFileName(dataFileName);
            getFileName(tableFileName); 

            //mostra o arquivo binario na tela apenas se conseguir criar a tabela
            if(createVehicleTable(dataFileName, tableFileName))
            {
                binarioNaTela(tableFileName);
            }
            break;
        // le os registros presentes em linha.csv e os grava em um arquivo de dados de saída
        case 2:
            getFileName(dataFileName);
            getFileName(tableFileName); 

            //mostra o arquivo binario na tela apenas se conseguir criar a tabela
            if(createLineTable(dataFileName, tableFileName))
            {
                binarioNaTela(tableFileName);
            }

            break;    

        // Recupera os dados de todos os registros armazenados no arquivo de dados veiculos.bin, mostrando os dados de forma organizada
        case 3:
            getFileName(tableFileName); 
            selectVehicleRegistersFrom(tableFileName);
            break;
    
        // Recupera os dados de todos os registros armazenados no arquivo de dados linhas.bin, mostrando os dados de forma organizada
        case 4:
            getFileName(tableFileName); 

            selectLineRegistersFrom(tableFileName);
            break;

        // Recupera os dados de todos os registros do arquivo veiculos.bin que satisfaçam um critério de busca determinado pelo usuário
        case 5:
            getFileName(tableFileName);
            scanf("%s", campo);
            scan_quote_string(valor);
            selectVehicleRegistersFromWhere(tableFileName, campo, valor);
            
            break;
        
        // Recupera os dados de todos os registros do arquivo linhas.bin que satisfaçam um critério de busca determinado pelo usuário
        case 6:

            getFileName(tableFileName);

            scanf("%s", campo);
            scan_quote_string(valor);
            selectLineRegistersFromWhere(tableFileName, campo, valor);
            
            break;

        // Permita a inserção de novos registros no arquivo de entrada veiculos.bin ao final do arquivo de dados.
        case 7:
            getFileName(tableFileName);
            scanf("%d", &numberOfNewRegisters);
            
            //aloca uma matriz para armazenar os registros dos novos veiculos
            vehicle_t** newVehicleRegisters = (vehicle_t**)malloc(sizeof(vehicle_t*));
            readNewVehicleRegisters(numberOfNewRegisters, newVehicleRegisters);
            

            if(insertVehicleRegistersIntoTable(tableFileName, newVehicleRegisters, numberOfNewRegisters))
            {
                binarioNaTela(tableFileName);
            }
            freeNewVehicleRegisters(numberOfNewRegisters, newVehicleRegisters);
            break;
        
        // Permita a inserção de novos registros no arquivo de entrada linhas.bin ao final do arquivo de dados.
        case 8:
            getFileName(tableFileName);
        
            scanf("%d", &numberOfNewRegisters);

            line_t** newLineRegisters = (line_t**)malloc(sizeof(line_t*));
            readNewLineRegisters(numberOfNewRegisters, newLineRegisters);
            
            if(insertLineRegistersIntoTable(tableFileName, newLineRegisters, numberOfNewRegisters))
            {
                binarioNaTela(tableFileName);
            }

            freeNewLineRegisters(numberOfNewRegisters, newLineRegisters);

            break;

        case 9:
            getFileName(tableFileName);
            getFileName(indexFileName); 

            //mostra o arquivo binario na tela apenas se conseguir criar a tabela
            if(createVehicleIndex(tableFileName, indexFileName))
            {
                binarioNaTela(indexFileName);
            }
            break;

        case 10:
            getFileName(tableFileName);
            getFileName(indexFileName); 

            //mostra o arquivo binario na tela apenas se conseguir criar a tabela
            if(createLineIndex(tableFileName, indexFileName))
            {
                binarioNaTela(indexFileName);
            }
            break;

        case 11:
            getFileName(tableFileName);
            getFileName(indexFileName); 

            scanf("%s", campo);
            scan_quote_string(valor);

            // buscar pelo registro
            sgdbVehicles(tableFileName, indexFileName, valor);

            break;

        case 12:
            getFileName(tableFileName);
            getFileName(indexFileName); 

            scanf("%s", campo);
            scanf("%d", &codLinha);

            // buscar pelo registro
            sgdbLines(tableFileName, indexFileName, codLinha);
            break;

        case 13:
            getFileName(tableFileName);
            getFileName(indexFileName);
            scanf("%d", &numberOfNewRegisters);
            
            //aloca uma matriz para armazenar os registros dos novos veiculos
            vehicle_t** newVehicleRegistersForIndex = (vehicle_t**)malloc(sizeof(vehicle_t*));
            readNewVehicleRegisters(numberOfNewRegisters, newVehicleRegistersForIndex);

            if(insertNewVehicleRegisters(indexFileName, tableFileName, newVehicleRegistersForIndex, numberOfNewRegisters))
            {
                binarioNaTela(indexFileName);
            }

            freeNewVehicleRegisters(numberOfNewRegisters, newVehicleRegistersForIndex);

            break;


        case 14:
            getFileName(tableFileName);
            getFileName(indexFileName);
            scanf("%d", &numberOfNewRegisters);
            
            //aloca uma matriz para armazenar os registros das novas linhas
            line_t** newLineRegistersForIndex = (line_t**)malloc(sizeof(line_t*));
            readNewLineRegisters(numberOfNewRegisters, newLineRegistersForIndex);

            if(insertNewLineRegisters(indexFileName, tableFileName, newLineRegistersForIndex, numberOfNewRegisters))
            {
                binarioNaTela(indexFileName);
            }

            freeNewLineRegisters(numberOfNewRegisters, newLineRegistersForIndex);

            break;

        case 15:
            getFileName(tableFileName);
            getFileName(indexFileName);
            scanf("%s %s", campo, valor);
            selectFromJoinBruto(tableFileName, indexFileName);
            
            break;

        case 16:
            getFileName(tableFileName);
            getFileName(dataFileName);
            scanf("%s %s", campo, valor);
            getFileName(indexFileName);

            selectFromJoinPorIndice(tableFileName, dataFileName, indexFileName);
            
            break;
            
        case 17:
            getFileName(tableFileName);
            getFileName(dataFileName);
            scanf("%s", campo);
            
            if(sortVehicleTable(tableFileName, dataFileName))
            {
                binarioNaTela(dataFileName);
            }
            
            break;

        case 18:
            getFileName(tableFileName);
            getFileName(dataFileName);
            scanf("%s", campo);
            
            if(sortLineTable(tableFileName, dataFileName))
            {
                binarioNaTela(dataFileName);
            }
            
            break;
        default:
            break;
    }

    return 0;
}


void readNewVehicleRegisters(int numberOfNewRegisters, vehicle_t** newVehicleRegisters)
{
    //aloca uma matriz para armazenar os dados de cada veiculo novo
    char** vehicleDataFields = (char**)malloc(sizeof(char*)*NUMBER_OF_COLUMNS_VEHICLE);
    for(int j=0; j < NUMBER_OF_COLUMNS_VEHICLE; j++){
        vehicleDataFields[j] = (char*)malloc(sizeof(char)*(STRING_SIZE));
    }

    for (int i = 0; i < numberOfNewRegisters; i++)
    {
        // cria uma registro novo para cada iteração
        newVehicleRegisters[i] = createVehicleRegister();
        // garante que em todas as iterações começamos com os valores zerados
        // isso é necessário devido ao comportamento da strcpy, que altera apenas os bytes novos da string
        // e mantem os valores da iteração passada
        resetStrings(vehicleDataFields, NUMBER_OF_COLUMNS_VEHICLE, STRING_SIZE);

        scan_quote_string(vehicleDataFields[0]);
        scan_quote_string(vehicleDataFields[1]);
        scan_quote_string(vehicleDataFields[2]);
        scan_quote_string(vehicleDataFields[3]);
        scan_quote_string(vehicleDataFields[4]);
        scan_quote_string(vehicleDataFields[5]);

        // fazer ficar igual ao csv, para nao precisar alterar a logica do TAD veiculos
        vehicleDataFields[5][strlen(vehicleDataFields[5])] = '\n';
        // corrigir logica de scanquote, 
        // necessario add o \0 para o funcionamento adequado das funções printf e da string.h
        vehicleDataFields[5][strlen(vehicleDataFields[5])] = '\0';
        insertVehicleDataInStructure((char**)vehicleDataFields, newVehicleRegisters[i]);
    }

    for (int i = 0; i < NUMBER_OF_COLUMNS_VEHICLE; i++)
    {
        free(vehicleDataFields[i]);
    }
}

void freeNewVehicleRegisters(int numberOfNewRegisters, vehicle_t** newVehicleRegisters)
{
    for (int i = 0; i < numberOfNewRegisters; i++)
    {
        free(newVehicleRegisters[i]);
    }
}



void readNewLineRegisters(int numberOfNewRegisters, line_t** newLineRegisters)
{
    //aloca uma matriz para armazenar os dados de cada veiculo novo
    char** lineDataFields = (char**)malloc(sizeof(char*)*NUMBER_OF_COLUMNS_LINES);
    for(int j=0; j < NUMBER_OF_COLUMNS_LINES; j++){
        lineDataFields[j] = (char*)malloc(sizeof(char)*(STRING_SIZE));
    }

    for (int i = 0; i < numberOfNewRegisters; i++)
    {
        // cria uma registro novo para cada iteração
        newLineRegisters[i] = createLineRegister();
        // garante que em todas as iterações começamos com os valores zerados
        // isso é necessário devido ao comportamento da strcpy, que altera apenas os bytes novos da string
        // e mantem os valores da iteração passada
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

    for (int i = 0; i < NUMBER_OF_COLUMNS_LINES; i++)
    {
        free(lineDataFields[i]);
    }
}

void freeNewLineRegisters(int numberOfNewRegisters, line_t** newLineRegisters)
{
    for (int i = 0; i < numberOfNewRegisters; i++)
    {
        free(newLineRegisters[i]);
    }
}