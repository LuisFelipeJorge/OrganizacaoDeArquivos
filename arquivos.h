
void getFileName(char *fileName);
int fileDidOpen(FILE* fileRefence);
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
long long getByteOffset(FILE* tableFileReference);
int isFileCorrupted(FILE* tableFileReference);
void setByteOffset(FILE* tableFileReference, long long byteOffset);
int getNroDeRegistros(FILE* tableFileReference);
void setNroDeRegistros(FILE* tableFileReference, int nroDeRegistros);