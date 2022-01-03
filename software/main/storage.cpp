#include "storage.h"

#include "nvs_flash.h"
#include "nvs.h"
#include "nvs_handle.hpp"
#include "esp_system.h"


// Initialize storage, returns boolean indicating if init was successful
bool init_storage() {
    // Initialize NVS
    esp_err_t result = nvs_flash_init();
    if (result == ESP_ERR_NVS_NO_FREE_PAGES || result == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // Recoverable error, reset and retry
        printf("Resetting NVS flash");
        ESP_ERROR_CHECK_WITHOUT_ABORT(nvs_flash_erase());
        result = nvs_flash_init();
    }
    return result == ESP_OK;
}


// Read value from storage by name, returns 0 on error or default_value if no value has been previously set
uint32_t read_storage_value(char* name, uint32_t default_value) {
    esp_err_t err;

    // Open
    // Handle will automatically close when going out of scope or when it's reset.
    std::shared_ptr<nvs::NVSHandle> handle = nvs::open_nvs_handle(STORAGE_NAME, NVS_READWRITE, &err);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
        return 0;
    }

    // Read
    uint32_t value = default_value;
    err = handle->get_item(name, value);
    switch (err) {
        case ESP_OK:
            printf("NVS: read %s = %d\n", name, value);
            return value;
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            printf("NVS: The value %s is not initialized yet, using default value\n", name);
            return default_value;
            break;
        default :
            printf("Error (%s) reading %s\n", esp_err_to_name(err), name);
            return 0;
    }
}


// Write a trio of values to storage
bool write_storage_values(char* name1, uint32_t value1, char* name2, uint32_t value2, char* name3, uint32_t value3) {
    bool success = true;
    esp_err_t err;

    // Open
    // Handle will automatically close when going out of scope or when it's reset.
    std::shared_ptr<nvs::NVSHandle> handle = nvs::open_nvs_handle(STORAGE_NAME, NVS_READWRITE, &err);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
        return false;
    }

    // Write
    // Batching writes here to cut down on the number of times we commit to NVS
    err = handle->set_item(name1, value1);
    if (err != ESP_OK) {
        success = false;
        printf("Failed to write %s\n", name1);
    }
    err = handle->set_item(name2, value2);
    if (err != ESP_OK) {
        success = false;
        printf("Failed to write %s\n", name2);
    }
    err = handle->set_item(name3, value3);
    if (err != ESP_OK) {
        success = false;
        printf("Failed to write %s\n", name3);
    }
    // Bail if any writes failed
    if (!success) return false;

    // Commit
    printf("Committing updates in NVS ... ");
    err = handle->commit();
    if (err != ESP_OK) {
        success = false;
        printf("Failed to commit NVS\n");
    }
    return success;
}


