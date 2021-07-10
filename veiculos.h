typedef struct vehicle vehicle_t;

#define NUMBER_OF_COLUMNS_VEHICLE 6

vehicle_t* createVehicleRegister();
void insertVehicleDataInStructure(char** vehicleDataFields, vehicle_t* vehicleRegister);
void printVehicleRegister(vehicle_t* vehicleRegister);
int createVehicleTable(char *dataFileName, char *tableFileName);
void selectVehicleRegistersFrom(char* tableFileName);
void selectVehicleRegistersFromWhere(char* tableFileName, char* fieldName, char* value);
int insertVehicleRegistersIntoTable(char* tableFileName, vehicle_t** vehicleRegisters, int numberOfRegisters);

///////////////////////////////////////////////////////////////////////////////////

char* readVehicleRegister(FILE* dataFileReference, vehicle_t* vehicleRegister);
int calculateTamanhoDoRegistroVeiculo(vehicle_t* vehicleRegister);
void writeVehicleRegister(FILE* tableFileReference,vehicle_t* vehicleRegister);
void freeVehicleRegister(vehicle_t* vehicleRegister);
void getDataNula(char* data);
int isNullVehicleRegister(FILE* tableFileReference);
void jumpVehicleHeader(FILE* tableFileReference);
char* getFormatedDate(char* date);
void readVehicleRegistersFromBinaryTable(FILE* tableFileReference);
void readVehicleRegistersFromBinaryTableWithCondition(
  FILE* tableFileReference,  
  char* fieldName, 
  char* value
);
void readVehicleRegisterBIN(FILE* tableFileReference, vehicle_t* vehicleRegister);
void printVehicleRegisterSelectedBy(vehicle_t* vehicleRegister, char* fieldName,char* value);
void setByteOffset(FILE* tableFileReference, long long byteOffset);
int getNroDeRegistros(FILE* tableFileReference);
void setNroDeRegistros(FILE* tableFileReference, int nroDeRegistros);

char* getPrefixo(vehicle_t* vehicleRegister);
void setRemovidoVehicle(vehicle_t* vehicleRegister, char campoRemovido);

int insertVehicleRegisterIntoTable(
  FILE* tableFileReference, 
  vehicle_t* vehicleRegisters, 
  long* lastResgisterInserted
);