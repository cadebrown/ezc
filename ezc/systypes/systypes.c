
#define MODULE_NAME systypes

#include "ezcmodule.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int num_types = 5;
type_t * types;

void byte_constructor(obj_t * ret, char * value) {
    ret->data = malloc(1);
    ret->data_len = 1;
    ret->type_id = types[0].id;
    
    if (strlen(value) == 1) {
        ((char *)ret->data)[0] = value[0];
    } else if (strlen(value) > 2 && strlen(value) <= 4 && value[0] == '0' && value[1] == 'x' && strtol(value, NULL, 0) <= 0xff) {
        ((char *)ret->data)[0] = strtol(value, NULL, 0);
    } else {
        printf("error constructing byte from '%s'\n", value);
    }
}

void byte_representation(char ** ret, obj_t obj) {
    char v = ((char *)obj.data)[0];
    *ret = malloc(5);
    (*ret)[0] = '0';
    (*ret)[1] = 'x';
    (*ret)[2] = (v >> 4) + '0';
    (*ret)[3] = (v & 15) + '0';
    (*ret)[4] = 0;
}

void int_constructor(obj_t * ret, char * value) {
    ret->data_len = sizeof(int);
    ret->data = malloc(ret->data_len);
    ret->type_id = types[1].id;
    
    *((int *)ret->data) = strtol(value, NULL, 0);
}

void int_representation(char ** ret, obj_t obj) {
    // 22 can hold 64 bits, best for safety
    *ret = malloc(22);
    int v = ((int *)obj.data)[0];
    sprintf(*ret, "%d", v);
}

void long_constructor(obj_t * ret, char * value) {
    ret->data_len = sizeof(long long);
    ret->data = malloc(ret->data_len);
    ret->type_id = types[2].id;
    
    *((long long *)ret->data) = strtol(value, NULL, 0);
}

void long_representation(char ** ret, obj_t obj) {
    // 22 can hold 64 bits, best for safety
    *ret = malloc(22);
    long long v = ((long long *)obj.data)[0];
    sprintf(*ret, "%lld", v);
}

void float_constructor(obj_t * ret, char * value) {
    ret->data_len = sizeof(float);
    ret->data = malloc(ret->data_len);
    ret->type_id = types[3].id;
    
    *((float *)ret->data) = strtof(value, NULL);
}

void float_representation(char ** ret, obj_t obj) {
    *ret = malloc(22);
    float v = ((float *)obj.data)[0];
    sprintf(*ret, "%f", v);
}

void str_constructor(obj_t * ret, char * value) {
    ret->data_len = strlen(value);
    ret->data = malloc(ret->data_len);
    ret->type_id = types[4].id;
    
    strcpy(ret->data, value);
}

void str_representation(char ** ret, obj_t obj) {
    *ret = malloc(obj.data_len);
    strcpy(*ret, obj.data);
}


int init (int type_id) {
    types = malloc(num_types * sizeof(type_t));

    int cid = 0;

    types[cid].name = "byte";
    types[cid].id = type_id++;
    types[cid].constructor = byte_constructor;
    types[cid].representation = byte_representation;

    cid++;

    types[cid].name = "int";
    types[cid].id = type_id++;
    types[cid].constructor = int_constructor;
    types[cid].representation = int_representation;

    cid++;

    types[cid].name = "long";
    types[cid].id = type_id++;
    types[cid].constructor = long_constructor;
    types[cid].representation = long_representation;

    cid++;

    types[cid].name = "float";
    types[cid].id = type_id++;
    types[cid].constructor = float_constructor;
    types[cid].representation = float_representation;


    cid++;

    types[cid].name = "str";
    types[cid].id = type_id++;
    types[cid].constructor = str_constructor;
    types[cid].representation = str_representation;


    return 0;
}



