typedef struct line line_t;

#define NUMBER_OF_COLUMNS_LINES 4

int createLineTable(char *dataFileName, char* tableFileName);
void selectLineRegistersFrom(char* tableFileName);
void selectLineRegistersFromWhere(char* tableFileName, char* fieldName, char* value);
