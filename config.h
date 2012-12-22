#ifndef _CONFIG_H
#define _CONFIG_H

#include "uthash.h"

typedef struct config_s {
    char* key;
    int value;
    UT_hash_handle hh;
} config_t;

extern config_t* config;

void config_put(char* key, int value);
int config_get(char* key, int default_value);

#endif
