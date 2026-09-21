#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "kv_engine.h"

/* == Helpers == */

size_t get_json_size(const KVEHashMap *map, size_t indentation) {
    if (!map)
        return 0;

    size_t size = 3 /* For "{}" and null terminator "\0" */
                + (indentation > 0 ? 1 : 0) /* for newline "\n" after the first parenthesis */
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

    return size - (indentation > 0 ? 1 : 2); /* Minus 2 (or 1 in case of indentation) since the last item won't have a trailing ',' seperator after it */
}

enum CurrentReadMode {
    WAITING_KEY,
    KEY,
    WAITING_VALUE,
    VALUE,
};

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

bool kve_json_save(const KVEHashMap *map, size_t indentation, const char *filepath) {
    if (!(map && filepath))
        return false;

    char *json = kve_json_serialize(map, indentation);

    FILE *fp = fopen(filepath, "w");
    if (!fp) {
        free(json);
        return false;
    }

    bool success = (fputs(json, fp) != EOF);
    fclose(fp);
    free(json);

    return success;
}

bool kve_json_deserialize(KVEHashMap *map, char *json) {
    if (!(map && json))
        return false;

    size_t len = strlen(json);
    size_t key_start = 0, key_end = 0, value_start = 0, value_end = 0;
    #define key_len key_end - key_start
    #define value_len value_end - value_start
    enum CurrentReadMode mode = WAITING_KEY;
    
    for (size_t i = 1; i < len; i++) {
        if (json[i] == '"') {
            switch (mode) {
                case WAITING_KEY:
                    mode = KEY;
                    key_start = i + 1;
                    key_end = key_start;
                    continue;

                case KEY:
                    mode = WAITING_VALUE;
                    continue;

                case WAITING_VALUE:
                    mode = VALUE;
                    value_start = i + 1;
                    value_end = value_start;
                    continue;

                case VALUE:
                    mode = WAITING_KEY;
                    
                    char *key = malloc(key_len + 1);
                    char *value = malloc(value_len + 1);

                    if (!(key && value)) {
                        free(key);
                        free(value);
                        return false;
                    }

                    memcpy(key, json + key_start, key_len);
                    key[key_len] = '\0';

                    memcpy(value, json + value_start, value_len);
                    value[value_len] = '\0';


                    kve_map_put(map, key, value);

                    free(key);
                    free(value);

                    continue;
            }
        }
        
        if (mode == KEY)
            key_end++;

        if (mode == VALUE)
            value_end++;
    }
        
    return true;
}

bool kve_json_load(KVEHashMap *map, const char *filepath) {
    if (!(map && filepath))
        return false;

    FILE *fp = fopen(filepath, "rb");
    if (!fp)
        return false;

    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return false;
    }

    long filesize = ftell(fp);
    if (filesize < 0) {
        fclose(fp);
        return false;
    }

    size_t len = (size_t) filesize;
    fseek(fp, 0, SEEK_SET);

    char *buf = malloc(len + 1);
    if(!buf) {
        fclose(fp);
        return false;
    }

    size_t bytes_read = fread(buf, 1, len, fp);
    fclose(fp);

    /* Verify complete read */
    if (bytes_read != len) {
        free(buf);
        return false;
    }
    
    buf[len] = '\0';

    if(!kve_json_deserialize(map, buf)) {
        free(buf);
        return false;
    }
    
    free(buf);

    return true;
}