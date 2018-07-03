
#define MODULE_NAME "ezc.controls"

#define MODULE_DESCRIPTION "defines control loops (like if, while, for, etc)"


#include "ezcmodule.h"
#include "../ezcmacros.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void run_block(runtime_t * runtime, obj_t * blk) {
    char * code_to_run = malloc(blk->data_len);
    strcpy(code_to_run, blk->data);

    runnable_t last_expr;
    runnable_init_str(&last_expr, code_to_run);
    last_expr.from = "lambda";

    run_runnable(runtime, &last_expr);

    free(code_to_run);
    //runnable_free(&last_expr);
}



void run_if(runtime_t * runtime) {
    ASSURE_STACK_LEN(1);

    type_t str_type = type_from_name("str"), block_type = type_from_name("block");
    type_t int_type = type_from_name("int"), bool_type = type_from_name("bool");

    bool going_to_run = false;

    obj_t conditional = estack_pop(&runtime->stack);
    obj_t to_run = estack_pop(&runtime->stack);

    if (to_run.type_id != block_type.id) {
        raise_exception("running 'if': second to last object was not a block", 1);
        return;
    }

    if (conditional.type_id == block_type.id) {
        run_block(runtime, &conditional);
        obj_t top_res = estack_pop(&runtime->stack);
        if (top_res.type_id == bool_type.id) {
            going_to_run = OBJ_AS_STRUCT(top_res, bool);
        } else if (top_res.type_id == int_type.id) {
            going_to_run = OBJ_AS_STRUCT(top_res, int) > 0;
        } else {
            raise_exception("unknown type to test for conditional", 1);
        }
        //obj_free(&top_res);
    } else if (conditional.type_id == bool_type.id) {
        going_to_run = OBJ_AS_STRUCT(conditional, bool);
    } else if (conditional.type_id == int_type.id) {
        going_to_run = OBJ_AS_STRUCT(conditional, int) > 0;
    } else if (conditional.type_id == int_type.id) {
        going_to_run = OBJ_AS_STRUCT(conditional, int) > 0;
    } else if (conditional.type_id == bool_type.id) {
        going_to_run = OBJ_AS_STRUCT(conditional, bool);
    } else {
        raise_exception("unknown type to test for conditional", 1);
    }

    if (going_to_run) {
        run_block(runtime, &to_run);
    }

    obj_free(&to_run);
    obj_free(&conditional);
}


void run_while(runtime_t * runtime) {
    ASSURE_STACK_LEN(1);

    type_t str_type = type_from_name("str"), block_type = type_from_name("block");
    type_t int_type = type_from_name("int"), bool_type = type_from_name("bool");

    bool going_to_run = false;
    bool conditional_is_block = false;

    obj_t conditional = estack_pop(&runtime->stack);
    obj_t to_run = estack_pop(&runtime->stack);

    if (to_run.type_id != block_type.id) {
        raise_exception("running 'if': second to last object was not a block", 1);
        return;
    }

    if (conditional.type_id == block_type.id) {
        conditional_is_block = true;
        //obj_free(&top_res);
    } else if (conditional.type_id == bool_type.id) {
        going_to_run = OBJ_AS_STRUCT(conditional, bool);
    } else if (conditional.type_id == int_type.id) {
        going_to_run = OBJ_AS_STRUCT(conditional, int) > 0;
    } else if (conditional.type_id == str_type.id) {
        going_to_run = strlen(OBJ_AS_STRUCT(conditional, char *)) > 0;
    } else if (conditional.type_id == bool_type.id) {
        going_to_run = OBJ_AS_STRUCT(conditional, bool);
    } else {
        raise_exception("unknown type to test for conditional", 1);
    }

    if (conditional_is_block) {
        run_block(runtime, &conditional);
        obj_t top_res = estack_pop(&runtime->stack);
        if (top_res.type_id == bool_type.id) {
            going_to_run = OBJ_AS_STRUCT(top_res, bool);
        } else if (top_res.type_id == int_type.id) {
            going_to_run = OBJ_AS_STRUCT(top_res, int) > 0;
        } else if (top_res.type_id == str_type.id) {
            going_to_run = strlen(OBJ_AS_STRUCT(top_res, char *)) > 0;
        } else {
            raise_exception("unknown type to test for conditional", 1);
        }
        while (going_to_run) {
            run_block(runtime, &to_run);
            run_block(runtime, &conditional);
            top_res = estack_pop(&runtime->stack);
            if (top_res.type_id == bool_type.id) {
                going_to_run = OBJ_AS_STRUCT(top_res, bool);
            } else if (top_res.type_id == int_type.id) {
                going_to_run = OBJ_AS_STRUCT(top_res, int) > 0;
            } else if (top_res.type_id == str_type.id) {
                going_to_run = strlen(OBJ_AS_STRUCT(top_res, char *)) > 0;
            } else {
                raise_exception("unknown type to test for conditional", 1);
            }
        }
        
    } else if (going_to_run) {
        while (going_to_run) {
            // infinite loop
            run_block(runtime, &to_run);
        }
    }


    obj_free(&to_run);
    obj_free(&conditional);
}


void run_do_while(runtime_t * runtime) {
    ASSURE_STACK_LEN(1);

    type_t str_type = type_from_name("str"), block_type = type_from_name("block");
    type_t int_type = type_from_name("int"), bool_type = type_from_name("bool");

    bool going_to_run = false;
    bool conditional_is_block = false;

    obj_t conditional = estack_pop(&runtime->stack);
    obj_t to_run = estack_pop(&runtime->stack);

    if (to_run.type_id != block_type.id) {
        raise_exception("running 'dowhile': second to last object was not a block", 1);
        return;
    }

    if (conditional.type_id == block_type.id) {
        conditional_is_block = true;
        //obj_free(&top_res);
    } else if (conditional.type_id == bool_type.id) {
        going_to_run = OBJ_AS_STRUCT(conditional, bool);
    } else if (conditional.type_id == int_type.id) {
        going_to_run = OBJ_AS_STRUCT(conditional, int) > 0;
    } else if (conditional.type_id == str_type.id) {
        going_to_run = strlen(OBJ_AS_STRUCT(conditional, char *)) > 0;
    } else if (conditional.type_id == bool_type.id) {
        going_to_run = OBJ_AS_STRUCT(conditional, bool);
    } else {
        raise_exception("unknown type to test for conditional", 1);
    }

    if (conditional_is_block) {
        obj_t top_res;

        do {
            run_block(runtime, &to_run);
            run_block(runtime, &conditional);
            top_res = estack_pop(&runtime->stack);
            if (top_res.type_id == bool_type.id) {
                going_to_run = OBJ_AS_STRUCT(top_res, bool);
            } else if (top_res.type_id == int_type.id) {
                going_to_run = OBJ_AS_STRUCT(top_res, int) > 0;
            } else if (top_res.type_id == str_type.id) {
                going_to_run = strlen(OBJ_AS_POINTER(top_res, char)) > 0;
            } else {
                raise_exception("unknown type to test for conditional", 1);
            }
            
        } while (going_to_run);
        
    } else {
        do {
            // infinite loop
            run_block(runtime, &to_run);
        } while (going_to_run);
    }

    obj_free(&to_run);
    obj_free(&conditional);
}




int init (int id, lib_t _lib) {
    init_exported(id, _lib);

    add_function("if", "acts as an 'if' block", run_if);
    add_function("dowhile", "acts as an 'do while' block", run_do_while);
    add_function("while", "acts as an 'while' block", run_while);


    return 0;
}



