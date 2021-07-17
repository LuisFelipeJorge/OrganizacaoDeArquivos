typedef struct noArvoreB noArvoreB_t;

#define M 5 // ordem da árvore
#define NUMBER_OF_COLUMNS_INDEX 4

int createVehicleIndex(char* tableFileName, char* indexFileName);
int createLineIndex(char* tableFileName, char* indexFileName);
int sgdbVehicles(char* tableFileName, char* indexFileName, char* searchValue); 
int sgdbLines(char* tableFileName, char* indexFileName, int searchValue);
int insertNewVehicleRegisters(
  char* indexFileName, 
  char* tableFileName,
  vehicle_t** vehicleRegisters, 
  int numberOfRegisters
);
int insertNewLineRegisters(
  char* indexFileName, 
  char* tableFileName,
  line_t** lineRegisters, 
  int numberOfRegisters
);
long searchRegisterReference(int key, FILE* indexFileReference);
