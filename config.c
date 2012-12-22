#include <string.h>
#include "uthash.h"

#include "config.h"

config_t* config = NULL;

void config_put(char* key, int value) {
    config_t* entry = calloc(1, sizeof(config_t));
    entry->key = key;
    entry->value = value;
    HASH_ADD_KEYPTR(hh, config, entry->key, strlen(entry->key), entry);
}

int config_get(char* key, int default_value) {
    config_t* entry = NULL;
    HASH_FIND_STR(config, key, entry);
    if (entry) {
        return entry->value;
    }
    return default_value;
}
