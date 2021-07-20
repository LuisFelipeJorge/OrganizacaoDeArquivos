typedef struct vehicle vehicle_t;
typedef struct cabecalhoVeiculo cabecalhoVeiculo_t;
#define NUMBER_OF_COLUMNS_VEHICLE 6

vehicle_t* createVehicleRegister();
void insertVehicleDataInStructure(char** vehicleDataFields, vehicle_t* vehicleRegister);
void printVehicleRegister(vehicle_t* vehicleRegister);
int createVehicleTable(char *dataFileName, char *tableFileName);
void selectVehicleRegistersFrom(char* tableFileName);
void selectVehicleRegistersFromWhere(char* tableFileName, char* fieldName, char* value);
int insertVehicleRegistersIntoTable(char* tableFileName, vehicle_t** vehicleRegisters, int numberOfRegisters);

///////////////////////////////////////////////////////////////////////////////////

void writeVehicleRegister(FILE* tableFileReference,vehicle_t* vehicleRegister);
void freeVehicleRegister(vehicle_t* vehicleRegister);

void jumpVehicleHeader(FILE* tableFileReference);

void readVehicleRegisterBIN(FILE* tableFileReference, vehicle_t* vehicleRegister);
void printVehicleRegisterSelectedBy(vehicle_t* vehicleRegister, char* fieldName,char* value);
void setByteOffset(FILE* tableFileReference, long long byteOffset);
int getNroDeRegistros(FILE* tableFileReference);
void setNroDeRegistros(FILE* tableFileReference, int nroDeRegistros);

char* getPrefixo(vehicle_t* vehicleRegister);
void setRemovidoVehicle(vehicle_t* vehicleRegister, char campoRemovido);
int getCodLinhaVeiculo(vehicle_t* vehicleRegister);
int insertVehicleRegisterIntoTable(
  FILE* tableFileReference, 
  vehicle_t* vehicleRegisters, 
  long* lastResgisterInserted
);

void freeVehicleHeader(cabecalhoVeiculo_t* cabecalhoVeiculo);
cabecalhoVeiculo_t* createVehicleHeader();
void insertVehicleHeaderDataInStructure(
  char status,
  long long byteProxRegistro,
  char** headerFields,
  int nroRegistros, 
  int nroRegRemovidos, 
  cabecalhoVeiculo_t* cabecalhoVeiculo
);
cabecalhoVeiculo_t* getCabecalhoVeiculo(FILE* tableFileReference);
int sortVehicleTable(char* tableVehicleName, char* sortedFileName);
vehicle_t** sortVehicleRegisters(FILE* tableVehicleReference);
void freeSortedVehicleRegister(vehicle_t** array, int arraySize);