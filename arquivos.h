
void getFileName(char *fileName);
int fileDidOpen(FILE* fileRefence, char* errorMessage);
void binarioNaTela(char *nomeArquivoBinario);
void scan_quote_string(char *str);
char **splitString(char *string, int numberOfFields, char *delimeter);
void memorySuccessfullyAllocated(void **memoryReference);
void setStatus(FILE* fileReference, char status);
char getStatus(FILE* fileReference);
void goToFileStart(FILE* fileReference);
void goToFileEnd(FILE* fileReference);
int isNullField(char* field);
char** readHeader(FILE* dataFileReference, int headerSize, int numberOfColumns);