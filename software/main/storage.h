#ifndef STORAGE_H
#define STORAGE_H

#include <stdio.h>

#define STORAGE_NAME    "storage"

bool init_storage();
uint32_t read_storage_value(char* name, uint32_t default_value);
bool write_storage_values(char* name1, uint32_t value1, char* name2, uint32_t value2, char* name3, uint32_t value3, char* name4, uint32_t value4);

#endif

