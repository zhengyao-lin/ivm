#include <stdio.h>
#include <math.h>

#include "pub/type.h"
#include "pub/vm.h"
#include "pub/inlines.h"

#include "vm/native/native.h"
#include "vm/native/priv.h"

#define C_PI 3.14159265358979323846

#define DEG_TO_RAD(d) ((d) / 180 * C_PI)
#define RAD_TO_DEG(r) ((r) / C_PI * 180)

#define VAL1F(f) \
	{                                                   \
		ivm_number_t val;                               \
		MATCH_ARG("n", &val);                           \
		return ivm_numeric_new(NAT_STATE(), f(val));    \
	}

#define VAL2F(f) \
	{                                                   \
		ivm_number_t v1, v2;                            \
		MATCH_ARG("nn", &v1, &v2);                      \
		return ivm_numeric_new(NAT_STATE(), f(v1, v2)); \
	}

IVM_NATIVE_FUNC(_math_rad) VAL1F(DEG_TO_RAD)
IVM_NATIVE_FUNC(_math_deg) VAL1F(RAD_TO_DEG)

IVM_NATIVE_FUNC(_math_sin) VAL1F(sin)
IVM_NATIVE_FUNC(_math_cos) VAL1F(cos)
IVM_NATIVE_FUNC(_math_tan) VAL1F(tan)

IVM_NATIVE_FUNC(_math_asin) VAL1F(asin)
IVM_NATIVE_FUNC(_math_acos) VAL1F(acos)
IVM_NATIVE_FUNC(_math_atan) VAL1F(atan)
IVM_NATIVE_FUNC(_math_atan2) VAL2F(atan2)

IVM_NATIVE_FUNC(_math_abs) VAL1F(abs)
IVM_NATIVE_FUNC(_math_pow) VAL2F(pow)
IVM_NATIVE_FUNC(_math_sqrt) VAL1F(sqrt)

ivm_object_t *
ivm_mod_main(ivm_vmstate_t *state,
			 ivm_coro_t *coro,
			 ivm_context_t *context)
{
	ivm_object_t *mod = ivm_object_new_c(state, 13);

	#define DEF_FUNC(name) \
		ivm_object_setSlot_r(mod, state, #name, IVM_NATIVE_WRAP(state, _math_##name))

	#define DEF_CONST(name, val) \
		ivm_object_setSlot_r(mod, state, (name), ivm_numeric_new(state, (val)))

	DEF_FUNC(rad); // means `to radians`
	DEF_FUNC(deg); // means `to degrees`

	DEF_FUNC(sin); DEF_FUNC(cos); DEF_FUNC(tan);
	DEF_FUNC(asin); DEF_FUNC(acos); DEF_FUNC(atan); DEF_FUNC(atan2);

	DEF_FUNC(abs);

	DEF_FUNC(pow); DEF_FUNC(sqrt);

	DEF_CONST("pi", C_PI);

	return mod;
}
