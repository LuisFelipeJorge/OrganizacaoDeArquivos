typedef struct vehicle vehicle_t;

void createVehicleTable(char *dataFileName, char *tableFileName);
void selectVehicleRegistersFrom(char* tableFileName);
void selectVehicleRegistersFromWhere(char* tableFileName, char* fieldName, char* value);
void insertVehicleRegisterIntoTable(char* tableFileName, vehicle_t** vehicleRegisters, int numberOfRegisters);

