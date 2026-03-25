#ifndef JSON_H
#define JSON_H

#define MAX_JSON_KEYS 50
#define MAX_JSON_VALUE 256

typedef struct {
    char keys[MAX_JSON_KEYS][64];
    char values[MAX_JSON_KEYS][MAX_JSON_VALUE];
    int count;
} JsonObject;

JsonObject* json_parse_object(const char *json_str);
void json_free_object(JsonObject *obj);

const char* json_get_string(const JsonObject *obj, const char *key);
int json_get_int(const JsonObject *obj, const char *key, int default_val);

char* json_read_file(const char *filename);

#endif
