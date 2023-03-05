#ifndef STORAGE_H
#define STORAGE_H

#include <stdio.h>

#define STORAGE_NAME    "storage"

bool init_storage();
uint32_t read_storage_value(char const* name, uint32_t default_value);
bool write_storage_values(char const* name1, uint32_t value1, char const* name2, uint32_t value2, char const* name3, uint32_t value3);

#endif

