#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "json.h"

static const char *g_pos;

static void skip_ws() {
    while (*g_pos && (*g_pos == ' ' || *g_pos == '\t' || *g_pos == '\n' || *g_pos == '\r' || *g_pos == ',')) {
        g_pos++;
    }
}

static int parse_string(char *out, int max) {
    if (*g_pos != '"') return -1;
    g_pos++;
    int i = 0;
    while (*g_pos && *g_pos != '"' && i < max - 1) {
        if (*g_pos == '\\') {
            g_pos++;
            switch (*g_pos) {
                case 'n': out[i++] = '\n'; break;
                case 't': out[i++] = '\t'; break;
                case '"': out[i++] = '"'; break;
                case '\\': out[i++] = '\\'; break;
                default: out[i++] = *g_pos; break;
            }
        } else {
            out[i++] = *g_pos;
        }
        g_pos++;
    }
    if (*g_pos == '"') g_pos++;
    out[i] = '\0';
    return i;
}

static void parse_object(JsonObject *obj) {
    if (*g_pos != '{') return;
    g_pos++;
    skip_ws();
    
    while (*g_pos && *g_pos != '}') {
        char key[64] = {0};
        parse_string(key, sizeof(key));
        skip_ws();
        if (*g_pos == ':') g_pos++;
        skip_ws();
        
        char val[MAX_JSON_VALUE] = {0};
        if (*g_pos == '"') {
            parse_string(val, sizeof(val));
        } else if (isdigit(*g_pos) || *g_pos == '-') {
            char num[32] = {0};
            int j = 0;
            while (isdigit(*g_pos) || *g_pos == '.' || *g_pos == '-') {
                num[j++] = *g_pos++;
            }
            strcpy(val, num);
        } else if (strncmp(g_pos, "true", 4) == 0) {
            strcpy(val, "true"); g_pos += 4;
        } else if (strncmp(g_pos, "false", 5) == 0) {
            strcpy(val, "false"); g_pos += 5;
        }
        
        if (obj->count < MAX_JSON_KEYS) {
            strcpy(obj->keys[obj->count], key);
            strcpy(obj->values[obj->count], val);
            obj->count++;
        }
        
        skip_ws();
    }
    if (*g_pos == '}') g_pos++;
}

JsonObject* json_parse_object(const char *str) {
    JsonObject *obj = calloc(1, sizeof(JsonObject));
    g_pos = str;
    skip_ws();
    parse_object(obj);
    return obj;
}

void json_free_object(JsonObject *obj) { free(obj); }

const char* json_get_string(const JsonObject *obj, const char *key) {
    if (!obj) return "";
    for (int i = 0; i < obj->count; i++) {
        if (strcmp(obj->keys[i], key) == 0) {
            return obj->values[i];
        }
    }
    return "";
}

int json_get_int(const JsonObject *obj, const char *key, int default_val) {
    const char *val = json_get_string(obj, key);
    if (val && *val) return atoi(val);
    return default_val;
}

char* json_read_file(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) return NULL;
    
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    char *buf = malloc(len + 1);
    fread(buf, 1, len, f);
    buf[len] = '\0';
    fclose(f);
    return buf;
}
