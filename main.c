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

    selectVehicleRegistersFrom(binaryFileName);
       
    return 0;
}
