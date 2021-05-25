typedef struct vehicle vehicle_t;

#define NUMBER_OF_COLUMNS 6

vehicle_t* createVehicleRegister();
void insertVehicleDataInStructure(char** vehicleDataFields, vehicle_t* vehicleRegister);
void printVehicleRegister(vehicle_t* vehicleRegister);
int createVehicleTable(char *dataFileName, char *tableFileName);
void selectVehicleRegistersFrom(char* tableFileName);
void selectVehicleRegistersFromWhere(char* tableFileName, char* fieldName, char* value);
int insertVehicleRegisterIntoTable(char* tableFileName, vehicle_t** vehicleRegisters, int numberOfRegisters);

