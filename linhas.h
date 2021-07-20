typedef struct line line_t;

#define NUMBER_OF_COLUMNS_LINES 4

int createLineTable(char *dataFileName, char* tableFileName);
void selectLineRegistersFrom(char* tableFileName);
void selectLineRegistersFromWhere(char* tableFileName, char* fieldName, char* value);
void insertLineDataInStructure(char** lineDataFields, line_t* lineRegister);
int insertLineRegistersIntoTable(
  char* tableFileName, 
  line_t** lineRegisters, 
  int numberOfRegisters
);
line_t* createLineRegister();

//////////////////////////////////////////////////////////////////////////////////////////

void setRemovidoLine(line_t* lineRegister, char campoRemovido);
void readLineRegisterBIN(FILE* tableFileReference, line_t* lineRegister);
void freeLineRegister(line_t* lineRegister);
void printLineRegister(line_t* lineRegister);
void jumpLineHeader(FILE* tableFileReference);
char* readLineRegister(FILE* dataFileReference, line_t* lineRegister);
int getCodLinha(line_t* lineRegister);
int insertLineRegisterIntoTable(
  FILE* tableFileReference, 
  line_t* lineRegister, 
  long* lastResgisterInserted
);

int sortLineTable(char* tableLineName, char* sortedFileName);