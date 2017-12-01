
#include "dict.h"
#include <string.h>


void dict_init(dict_t * dict) {
    dict->len = 0;
    dict->__max_len_so_far = 0;
    dict->keys = malloc(sizeof(char *));
    dict->vals = malloc(sizeof(obj_t));
}

void dict_set(dict_t * dict, char * key, obj_t obj) {
    int idx = dict_index(dict, key);
    if (idx < 0) {
        // dictionary does not contain key yet
        dict->len++;
        if (dict->len > dict->__max_len_so_far) {
            dict->__max_len_so_far = dict->len;
            dict->keys = realloc(dict->keys, dict->len * sizeof(char *));
            dict->vals = realloc(dict->vals, dict->len * sizeof(obj_t));
        }
        dict->keys[dict->len - 1] = malloc(strlen(key));
        strcpy(dict->keys[dict->len - 1], key);
        dict->vals[dict->len - 1]  = obj;
    } else {
        dict->keys[idx] = malloc(strlen(key));
        strcpy(dict->keys[idx], key);
        dict->vals[idx] = obj;
    }
}

obj_t dict_get(dict_t * dict, char * key) {
    int idx = dict_index(dict, key);
    if (idx < 0) {
        return NULL_OBJ;
    } else {
        return dict->vals[idx];
    }
}

int dict_index(dict_t * dict, char * key) {
    int i;
    for (i = 0; i < dict->len; ++i) {
        if (strcmp(dict->keys[i], key) == 0) {
            return i;
        }
    }
    return -1;
}


