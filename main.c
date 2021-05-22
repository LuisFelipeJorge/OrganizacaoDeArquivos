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
    char campo[STRING_SIZE/2];
    char valor[STRING_SIZE/2];
    scanf("%s", campo);
    scan_quote_string(valor);
    selectVehicleRegistersFromWhere(binaryFileName, campo, valor);
       
    return 0;
}
