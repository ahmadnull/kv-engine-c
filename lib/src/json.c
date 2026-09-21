#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "kv_engine.h"

/* == Helpers == */

size_t get_json_size(const KVEHashMap *map, size_t indentation) {
    if (!map)
        return 0;

    size_t size = 3 /* For "{}" and null terminator "\0" */
                + indentation > 0 ? 1 : 0 /* for newline "\n" after the first parenthesis */
                ;

    if(kve_map_size(map) == 0)
        return size; /* JSON will be just "{}\0" */

    KVEIterator *iter = kve_iter_create((KVEHashMap *)map);
    while (kve_iter_next(iter)) {
        const char *key = kve_iter_key(iter);
        const char *value = kve_iter_value(iter);

        size += indentation
              + 1                    /* Opening '"' for key */
              + strlen(key)          /* Key payload */
              + 1                    /* Closing '"' for key */
              + 2                    /* Delimiter ": " */
              + 1                    /* Opening '"' for value */
              + strlen(value)        /* Value payload */
              + 1                    /* Closing '"' for value */
              + 2                    /* Separator ", " or ",\n" in case of indentation */
              ;
    }
    kve_iter_destroy(iter);

    return size - 2; /* Minus 2 since the last item won't have a trailing (,) seperator after it */
}

/* == JSON functions == */

char *kve_json_serialize(const KVEHashMap *map, size_t indentation) {
    if (!map)
        return NULL;

    size_t size = get_json_size(map, indentation);
    if (size < 3)
        return NULL;

    char *json = malloc(size);
    if (!json)
        return NULL;

    char *ptr = json;
    *ptr++ = '{';
    if (indentation > 0)
        *ptr++ = '\n';

    if (kve_map_size(map) > 0) {
        KVEIterator *iter = kve_iter_create((KVEHashMap *)map);
        if (!iter) {
            free(json);
            return NULL;
        }

        while(kve_iter_next(iter)) {

            for(size_t i = 0; i < indentation; i++)
                *ptr++ = ' ';

            const char *key = kve_iter_key(iter);
            const char *value = kve_iter_value(iter);

            *ptr++ = '"';
            while (*key) {
                *ptr++ = *key++;
            }
            *ptr++ = '"';

            *ptr++ = ':';
            *ptr++ = ' ';

            *ptr++ = '"';
            while (*value) {
                *ptr++ = *value++;
            }
            *ptr++ = '"';

            *ptr++ = ',';
            *ptr++ = indentation > 0 ? '\n' : ' ';
        }

        kve_iter_destroy(iter);

        ptr -= 2;
    }
    
    if (indentation > 0)
        *ptr++ = '\n';
    *ptr++ = '}';
    *ptr = '\0';

    return json;
}

bool kve_json_deserialize(char *json); /* TODO */
bool kve_json_save(const KVEHashMap *map, const char *filepath); /* TODO */
bool kve_json_load(KVEHashMap *map, const char *filepath); /* TODO */