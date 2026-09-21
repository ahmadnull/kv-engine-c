#include <stdio.h>
#include <stdlib.h>
#include "kv_engine.h"

int main(void) {
    printf("\n===\n\n");

    KVEHashMap *map = kve_map_create(16);
    kve_map_put(map, "First Name", "Gottfried");
    kve_map_put(map, "Middle Name", "Wilhelm");
    kve_map_put(map, "Last Name", "Leibniz");
    kve_map_put(map, "Birth Year", "1646");
    kve_map_put(map, "Death Year", "1716");
    kve_map_put(map, "Birth Place", "Leipzig");
    kve_map_put(map, "Death Place", "Hanover");

    char *json_no_indent = kve_json_serialize(map, 0);
    printf("JSON with no indentation: %s\n\n===\n\n", json_no_indent);
    free(json_no_indent);

    char *json_indent = kve_json_serialize(map, 4);
    printf("JSON with indentation:\n%s\n\n===\n\n", json_indent);
    free(json_indent);

    KVEIterator *iter = kve_iter_create(map);
    while(kve_iter_next(iter))
        printf("%s:\t\t%s\n", kve_iter_key(iter), kve_iter_value(iter));

    kve_map_destroy(map);
    kve_iter_destroy(iter);

    return 0;
}
