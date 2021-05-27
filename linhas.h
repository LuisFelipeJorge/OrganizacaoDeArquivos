typedef struct line line_t;

#define NUMBER_OF_COLUMNS_LINES 4

int createLineTable(char *dataFileName, char* tableFileName);
void selectLineRegistersFrom(char* tableFileName);
void selectLineRegistersFromWhere(char* tableFileName, char* fieldName, char* value);
void insertLineDataInStructure(char** lineDataFields, line_t* lineRegister);
int insertLineRegisterIntoTable(
  char* tableFileName, 
  line_t** lineRegisters, 
  int numberOfRegisters
);
line_t* createLineRegister();

