#pragma once

#include "../simplekv_nvs/simplekv_nvs.hpp"
#include "system_data_def.h"

class DatabaseManager {
public:
    DatabaseManager() : database(nullptr), database_nvs(nullptr) {}
    ~DatabaseManager() {}

    // Initialize the system's database
    bool initDatabase() {
        database = new SIMPLEKV::SimpleKV_ESP(true);  // uses PSRAM

        database_nvs = new SIMPLEKV::SimpleKV_ESP_NVS(true);  
        if (!database_nvs || !database_nvs->initNVS()) {
            return false;
        }
        return true;
    }

    /*interface*/
    SIMPLEKV::SimpleKV_ESP* getDatabase() {return database;}
    SIMPLEKV::SimpleKV_ESP_NVS* getDatabase_nvs() {return database_nvs;}

private:
    SIMPLEKV::SimpleKV_ESP* database;        
    SIMPLEKV::SimpleKV_ESP_NVS* database_nvs; // NVS database instance
};
