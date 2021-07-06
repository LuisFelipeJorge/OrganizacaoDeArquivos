typedef struct noArvoreB noArvoreB_t;

#define M 5 // ordem da árvore
#define NUMBER_OF_COLUMNS_INDEX 4


int createIndex(char* tableFileName, char* indexFileName);
int sgdbVehicles(char* tableFileName, char* indexFileName, char* searchValue); 
int sgdbLines(char* tableFileName, char* indexFileName, int searchValue);