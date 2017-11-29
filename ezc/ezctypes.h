#ifndef __EZCTYPES_H__
#define __EZCTYPES_H__

typedef struct obj_t {

    int type_id;

    int data_len;

    void * data;

} obj_t;


typedef void constructor_t(obj_t *, char *);

typedef void representation_t(char **, obj_t);



typedef struct type_t {

    int id;

    char * name;

    constructor_t * constructor;
    
    representation_t * representation;

} type_t;


typedef struct estack_t {

    int len;

    obj_t ** vals;

} estack_t;




#endif
