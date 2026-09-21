#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kv_engine.h"

bool kve_db_save(const KVEHashMap *map, const char *filepath) {
    if (!(map && filepath))
        return false;

    FILE *fp = fopen(filepath, "wb");
    if (!fp)
        return false;

    KVEDBHeader db_header = {
        .version = KVE_VERSION,
        .reserved = 0,
        .record_count = kve_map_size(map),
    };

    memcpy(db_header.magic, KVE_MAGIC, 4);

    if (fwrite(&db_header, sizeof(KVEDBHeader), 1, fp) != 1) {
        fclose(fp);
        return false;
    }

    for (size_t i = 0; i < kve_map_capacity(map); i++) {
        KVEHashNode *current = map->buckets[i];
        while (current != NULL) {
            size_t key_len = strlen(current->key);
            size_t val_len = strlen((const char *)current->value);

            if (key_len > UINT16_MAX || val_len > UINT32_MAX) {
                fclose(fp);
                return false;
            }

            KVERecordHeader rec_header = {
                .key_len = (uint16_t)key_len,
                .val_len = (uint32_t)val_len,
            };

            if (fwrite(&rec_header, sizeof(KVERecordHeader), 1, fp) != 1) {
                fclose(fp);
                return false;
            }

            if (fwrite(current->key, 1, key_len, fp) != key_len) {
                fclose(fp);
                return false;
            }

            if (fwrite(current->value, 1, val_len, fp) != val_len) {
                fclose(fp);
                return false;
            }

            current = current->next;
        }
    }

    fclose(fp);
    return true;
}

bool kve_db_load(KVEHashMap *map, const char *filepath) {
    if (!(map && filepath))
        return false;

    FILE *fp = fopen(filepath, "rb");
    if (!fp)
        return false;

    KVEDBHeader db_header;
    if (fread(&db_header, sizeof(KVEDBHeader), 1, fp) != 1) {
        fclose(fp);
        return false;
    }

    if (memcmp(db_header.magic, KVE_MAGIC, 4) != 0 || db_header.version != KVE_VERSION) {
        fclose(fp);
        return false;
    }

    for (uint64_t i = 0; i < db_header.record_count; i++) {
        KVERecordHeader rec_header;
        if (fread(&rec_header, sizeof(KVERecordHeader), 1, fp) != 1) {
            fclose(fp);
            return false;
        }

        char *key_buf = malloc(rec_header.key_len + 1);
        char *val_buf = malloc(rec_header.val_len + 1);

        if (!(key_buf && val_buf)) {
            free(key_buf);
            free(val_buf);
            fclose(fp);
            return false;
        }

        if (fread(key_buf, 1, rec_header.key_len, fp) != rec_header.key_len ||
            fread(val_buf, 1, rec_header.val_len, fp) != rec_header.val_len) {
            free(key_buf);
            free(val_buf);
            fclose(fp);
            return false;
        }

        key_buf[rec_header.key_len] = '\0';
        val_buf[rec_header.val_len] = '\0';

        if (!kve_map_put(map, key_buf, val_buf)) {
            free(key_buf);
            free(val_buf);
            fclose(fp);
            return false;
        }

        free(key_buf);
        free(val_buf);
    }

    fclose(fp);
    return true;
}
