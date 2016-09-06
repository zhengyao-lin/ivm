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

IVM_NATIVE_FUNC(_math_rad)
{
	ivm_number_t val;

	MATCH_ARG("n", &val);

	return ivm_numeric_new(NAT_STATE(), DEG_TO_RAD(val));
}

IVM_NATIVE_FUNC(_math_deg)
{
	ivm_number_t val;

	MATCH_ARG("n", &val);

	return ivm_numeric_new(NAT_STATE(), RAD_TO_DEG(val));
}

IVM_NATIVE_FUNC(_math_sin)
{
	ivm_number_t val;

	MATCH_ARG("n", &val);

	return ivm_numeric_new(NAT_STATE(), sin(val));
}

IVM_NATIVE_FUNC(_math_cos)
{
	ivm_number_t val;

	MATCH_ARG("n", &val);

	return ivm_numeric_new(NAT_STATE(), cos(val));
}

IVM_NATIVE_FUNC(_math_tan)
{
	ivm_number_t val;

	MATCH_ARG("n", &val);

	return ivm_numeric_new(NAT_STATE(), tan(val));
}

ivm_object_t *
ivm_mod_main(ivm_vmstate_t *state,
			 ivm_coro_t *coro,
			 ivm_context_t *context)
{
	ivm_object_t *mod = ivm_object_new_c(state, 4);

	ivm_object_setSlot_r(mod, state, "rad", IVM_NATIVE_WRAP(state, _math_rad)); // means `to radians`
	ivm_object_setSlot_r(mod, state, "deg", IVM_NATIVE_WRAP(state, _math_deg)); // means `to degrees`
	ivm_object_setSlot_r(mod, state, "sin", IVM_NATIVE_WRAP(state, _math_sin));
	ivm_object_setSlot_r(mod, state, "cos", IVM_NATIVE_WRAP(state, _math_cos));
	ivm_object_setSlot_r(mod, state, "tan", IVM_NATIVE_WRAP(state, _math_tan));

	return mod;
}
