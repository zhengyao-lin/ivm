#ifndef _IVM_STD_REF_H_
#define _IVM_STD_REF_H_

#include "pub/type.h"

#define IVM_REF_HEADER \
	ivm_uint_t __refc__;

#define ivm_ref_init(obj) ((obj)->__refc__ = 0)
#define ivm_ref_dec(obj) (--(obj)->__refc__)
#define ivm_ref_cur(obj) ((obj)->__refc__)
#define ivm_ref_inc(obj) ((obj)->__refc__++)

#define ivm_ref_deref(obj) (((ivm_int_t)--(obj)->__refc__) > 0)

#endif
