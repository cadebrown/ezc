#ifndef __EZCMACROS_H__
#define __EZCMACROS_H__

#define OBJ_ALLOC_STRUCT(obj, stype) (obj).data_len = sizeof(stype); (obj).data = malloc((obj).data_len);
#define OBJ_ALLOC_BYTES(obj, nbytes) (obj).data_len = nbytes; (obj).data = malloc((obj).data_len);
#define OBJ_FREE_MEM(obj) (obj).data_len = 0; if ((obj).data != NULL) { free((obj).data); (obj).data = NULL; }
#define OBJ_AS_POINTER(obj, stype) ((stype *)(obj).data)
#define OBJ_AS_STRUCT(obj, stype) (*OBJ_AS_POINTER(obj, stype))

#define OBJ_TYPE(obj) type_from_id((obj).type_id)
#define TYPE(name) type_from_name(name)
#define FUNCTION(name) function_from_name(name)

#define OBJ_STRUCT_COPY(to, from, stype) OBJ_AS_STRUCT(to, stype) = OBJ_AS_STRUCT(from, stype)

#define ISTYPE(v, stype) ((v).type_id == (stype).id)



#define EZC_RND MPFR_RNDN
#define GMP_MPF_DEFAULT_PREC 256


#endif
