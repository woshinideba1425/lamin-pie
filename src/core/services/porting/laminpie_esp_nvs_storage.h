#pragma once

#include "../laminpie_services_internal.h"

#if defined (LAMINPIE_SERVICES_ENABLE_ESP_NVS_STORAGE) 

// ESP NVS Storage API declarations
#ifdef __cplusplus
extern "C" {
#endif

// Initialize ESP NVS storage
int laminpie_esp_nvs_storage_init(void);

// Read data from NVS
int laminpie_esp_nvs_storage_read(const char* key, void* value, size_t* length);

// Write data to NVS
int laminpie_esp_nvs_storage_write(const char* key, const void* value, size_t length);

// Delete key from NVS
int laminpie_esp_nvs_storage_delete(const char* key);

// Commit changes to NVS
int laminpie_esp_nvs_storage_commit(void);

#ifdef __cplusplus
}
#endif

#endif // LAMINPIE_SERVICES_ENABLE_ESP_NVS_STORAGE





