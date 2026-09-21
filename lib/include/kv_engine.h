#ifndef KV_ENGINE_H
#define KV_ENGINE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define KVE_DEFAULT_CAPACITY 16
#define KVE_LOAD_FACTOR_THRESHOLD 0.75f
#define KVE_RESIZE_FACTOR 2
#define KVE_MAGIC "KVDB"
#define KVE_VERSION 1

/* == Structs == */

typedef struct KVEHashNode {
    char *key;
    char *value;
    struct KVEHashNode *next;
} KVEHashNode;

typedef struct KVEHashMap {
    KVEHashNode **buckets;
    size_t capacity;
    size_t size;
} KVEHashMap;

typedef struct KVEIterator {
    KVEHashMap *map;
    size_t bucket_index;
    KVEHashNode *current;
} KVEIterator;

// Ensure 1-byte alignment so C structs mirror on-disk layout exactly
#pragma pack(push, 1)

typedef struct {
    char magic[4];         // Must equal "KVDB"
    uint16_t version;      // Version 1
    uint16_t reserved;     // Reserved space (0x0000)
    uint64_t record_count; // Total records stored
} KVEDBHeader;

typedef struct {
    uint16_t key_len; // Max key size: 65,535 bytes
    uint32_t val_len; // Max val size: ~4.2 GB
} KVERecordHeader;

#pragma pack(pop)

/* == KVEHashMap Container Functions == */
KVEHashMap *kve_map_create(size_t initial_capacity);
void kve_map_destroy(KVEHashMap *map);
bool kve_map_resize_to(KVEHashMap *map, size_t new_capacity);

/* == KVEHashMap Element Functions == */
bool kve_map_put(KVEHashMap *map, const char *key, const char *value);
const char *kve_map_get(const KVEHashMap *map, const char *key);
bool kve_map_remove(KVEHashMap *map, const char *key);
bool kve_map_contains(const KVEHashMap *map, const char *key);

/* == KVEHashMap Metadata Functions == */
size_t kve_map_size(const KVEHashMap *map);
size_t kve_map_capacity(const KVEHashMap *map);

/* == JSON functions == */
char *kve_json_serialize(const KVEHashMap *map); /* TODO */
bool kve_json_deserialize(char *json); /* TODO */
bool kve_json_save(const KVEHashMap *map, const char *filepath); /* TODO */
bool kve_json_load(KVEHashMap *map, const char *filepath); /* TODO */

/* == Binary Database Functions == */
bool kve_db_save(const KVEHashMap *map, const char *filepath);
bool kve_db_load(KVEHashMap *map, const char *filepath);

/*
    Modifying a KVEHashMap while a KVEIterator is active is undefined behavior.
    A KVEIterator MUST BE recereated after any write.
*/

/* == KVEIterator Container Functions == */
KVEIterator *kve_iter_create(KVEHashMap *map);
void kve_iter_destroy(KVEIterator *iter);

/* == KVEIterator Element Functions == */
bool kve_iter_next(KVEIterator *iter);
const char *kve_iter_key(const KVEIterator *iter);
const char *kve_iter_value(const KVEIterator *iter);

#endif // KV_ENGINE_H
