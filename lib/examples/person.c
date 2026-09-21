#include <stdio.h>
#include "kv_engine.h"

int main(void) {
    KVEHashMap *map = kve_map_create(16);
    kve_map_put(map, "First Name", "Gottfried");
    kve_map_put(map, "Middle Name", "Wilhelm");
    kve_map_put(map, "Last Name", "Leibniz");
    kve_map_put(map, "Birth Year", "1646");
    kve_map_put(map, "Death Year", "1716");
    kve_map_put(map, "Birth Place", "Leipzig");
    kve_map_put(map, "Death Place", "Hanover");

    char *json = kve_json_serialize(map);
    printf("%s\n", json);

    KVEIterator *iter = kve_iter_create(map);
    while(kve_iter_next(iter))
        printf("%s:\t\t%s\n", kve_iter_key(iter), kve_iter_value(iter));

    kve_map_destroy(map);
    kve_iter_destroy(iter);

    return 0;
}
