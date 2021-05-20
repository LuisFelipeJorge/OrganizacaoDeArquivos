#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arquivos.h"
#include "veiculos.h"

#define STRING_SIZE 50

int main(int argc, char const *argv[])
{
    char dataFileName[STRING_SIZE];
    char registersFileName[STRING_SIZE];

    getFileName(dataFileName);
    getFileName(registersFileName);
    
    createVehicleTable(dataFileName, registersFileName);
    binarioNaTela(registersFileName);    
    return 0;
}
