#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <argparse.h>
#include "kv_engine.h"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

struct cmd_struct {
    const char *cmd;
    int (*fn) (int, const char **);
};

static const char *const cmd_put_usages[] = {
    "kv-map put --file | -f <file> <key> <value>",
    NULL,
};

/* == Helpers == */

const char *get_filename_ext(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot + 1;
}

static bool is_ext_db(const char *filename) {
    return (strcmp(get_filename_ext(filename), "db") == 0);
}

static bool is_ext_json(const char *filename) {
    return (strcmp(get_filename_ext(filename), "json") == 0);
}

/* == Commands Functions == */

int cmd_put(int argc, const char **argv) {
    const char *file = NULL;
    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_STRING('f', "file", &file, "file to load/save", NULL, 0, 0),
        OPT_END(),
    };
    struct argparse argparse;
    argparse_init(&argparse, options, cmd_put_usages, 0);
    argparse_describe(&argparse, "\nPut key-value pair into the database", NULL);
    argc = argparse_parse(&argparse, argc, argv);

    KVEHashMap *map = kve_map_create(KVE_DEFAULT_CAPACITY);
    if (is_ext_db(file)) {
        kve_db_load(map, file);
    }
    else if (is_ext_json(file)) {
        kve_json_load(map, file);
    }
    else return 1;

    const char *key = argv[0];
    const char *value = argv[1];

    if(!kve_map_put(map, key, value))
        return 1;
    
    if (is_ext_db(file)) {
        if(!kve_db_save(map, file))
            return 1;
    }
    else if (is_ext_json(file)) {
        if(!kve_json_save(map, 0, file))
            return 1;
    }
    
    kve_map_destroy(map);
    
    return 0;
}

static const char *const cmd_get_usages[] = {
    "kv-map get --file | -f <file> <key>",
    NULL,
};

int cmd_get(int argc, const char **argv) {
    const char *file = NULL;
    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_STRING('f', "file", &file, "file to load/save", NULL, 0, 0),
        OPT_END(),
    };
    struct argparse argparse;
    argparse_init(&argparse, options, cmd_get_usages, 0);
    argparse_describe(&argparse, "\nGet value of key from the database", NULL);
    argc = argparse_parse(&argparse, argc, argv);

    KVEHashMap *map = kve_map_create(KVE_DEFAULT_CAPACITY);
    if (is_ext_db(file)) {
        if(!kve_db_load(map, file))
            return 1;
    }
    else if (is_ext_json(file)) {
        if(!kve_json_load(map, file))
            return 1;
    }
    else return 1;

    const char *key = argv[0];
    const char *value = kve_map_get(map, key);

    printf("{\"%s\": \"%s\"}\n", key, value);

    kve_map_destroy(map);

    return 0;
}

static const char *const cmd_rm_usages[] = {
    "kv-map rm --file | -f <file> <key>",
    NULL,
};

int cmd_rm(int argc, const char **argv) {
    const char *file = NULL;
    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_STRING('f', "file", &file, "file to load/save", NULL, 0, 0),
        OPT_END(),
    };
    struct argparse argparse;
    argparse_init(&argparse, options, cmd_rm_usages, 0);
    argparse_describe(&argparse, "\nRemove key-value pair from the database", NULL);
    argc = argparse_parse(&argparse, argc, argv);

    KVEHashMap *map = kve_map_create(KVE_DEFAULT_CAPACITY);
    if (is_ext_db(file)) {
        if(!kve_db_load(map, file))
            return 1;
    }
    else if (is_ext_json(file)) {
        if(!kve_json_load(map, file))
            return 1;
    }
    else return 1;

    const char *key = argv[0];

    if(!kve_map_remove(map, key))
        return 1;
    
    if (is_ext_db(file)) {
        if(!kve_db_save(map, file))
            return 1;
    }
    else if (is_ext_json(file)) {
        if(!kve_json_save(map, 0, file))
            return 1;
    }
    
    kve_map_destroy(map);

    return 0;
}

static const char *const cmd_show_usages[] = {
    "kv-map show --file | -f <file> --indentation | -i <indentation>",
    NULL,
};

int cmd_show(int argc, const char **argv) {
    const char *file = NULL;
    size_t indentation = 4;
    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_STRING('f', "file", &file, "file to load/save", NULL, 0, 0),
        OPT_INTEGER('i', "indentation", &indentation, "JSON indentation (default 4)", NULL, 0, 0),
        OPT_END(),
    };
    struct argparse argparse;
    argparse_init(&argparse, options, cmd_show_usages, 0);
    argparse_describe(&argparse, "\nShow all key-value pairs from the database", NULL);
    argc = argparse_parse(&argparse, argc, argv);

    KVEHashMap *map = kve_map_create(KVE_DEFAULT_CAPACITY);
    if (is_ext_db(file)) {
        if(!kve_db_load(map, file))
            return 1;
    }
    else if (is_ext_json(file)) {
        if(!kve_json_load(map, file))
            return 1;
    }
    else return 1;

    char *json = kve_json_serialize(map, indentation);
    printf("%s", json);

    kve_map_destroy(map);
    free(json);

    return 0;
}

static const char *const cmd_contains_usages[] = {
    "kv-map contains --file | -f <file> <key>",
    NULL,
};

int cmd_contains(int argc, const char **argv) {
    const char *file = NULL;
    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_STRING('f', "file", &file, ".json/.db file to load/save (extension must be either of two)", NULL, 0, 0),
        OPT_END(),
    };
    struct argparse argparse;
    argparse_init(&argparse, options, cmd_contains_usages, 0);
    argparse_describe(&argparse, "\nCheck if key is in the database", NULL);
    argc = argparse_parse(&argparse, argc, argv);

    KVEHashMap *map = kve_map_create(KVE_DEFAULT_CAPACITY);
    if (is_ext_db(file)) {
        if(!kve_db_load(map, file))
            return 1;
    }
    else if (is_ext_json(file)) {
        if(!kve_json_load(map, file))
            return 1;
    }
    else return 1;

    const char *key = argv[0];

    int result = kve_map_contains(map, key) ? 0 : 1;

    kve_map_destroy(map);

    return result;
}

static struct cmd_struct commands[] = {
    {"put",  cmd_put},
    {"get",  cmd_get},
    {"rm",   cmd_rm},
    {"show", cmd_show},
    {"contains", cmd_contains},
};

static const char *const usages[] = {
    "kv-map [options] [cmd] [args]\n"
    "Available subcommands:\n"
    "\n"
    "\tput\n"
    "\tget\n"
    "\trm\n"
    "\tshow\n"
    "\tcontains\n"
    "\n"
    "Available options:",
    NULL,
};

int main(int argc, const char **argv) {
    struct argparse argparse;
    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_END(),
    };
    argparse_init(&argparse, options, usages, ARGPARSE_STOP_AT_NON_OPTION);
    argc = argparse_parse(&argparse, argc, argv);
    if (argc < 1) {
        argparse_usage(&argparse);
        return -1;
    }

    struct cmd_struct *cmd = NULL;
    for (size_t i = 0; i < ARRAY_SIZE(commands); i++) {
        if (!strcmp(commands[i].cmd, argv[0])) {
            cmd = &commands[i];
        }
    }
    if (cmd) {
        return cmd->fn(argc, argv);
    }

    return 0;
}
