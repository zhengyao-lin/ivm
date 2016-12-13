#include <stdio.h>
#include <locale.h>
#include <curses.h>

#include "pub/type.h"
#include "pub/vm.h"
#include "pub/obj.h"
#include "pub/err.h"
#include "pub/inlines.h"

#include "std/string.h"
#include "std/mem.h"

#include "vm/native/native.h"
#include "vm/native/priv.h"

#define WINDOW_TYPE_NAME "curses.window"
#define WINDOW_TYPE_CONS IVM_GET_NATIVE_FUNC(_curses_window)

#define CURSES_ERROR_MSG_FAILED(sth)									("failed to " sth)
#define CURSES_ERROR_MSG_UNINIT_WIN										"uninitialized window"

#define CHECK_WIN_INIT(wobj) RTM_ASSERT((wobj)->win, CURSES_ERROR_MSG_UNINIT_WIN)

typedef struct {
	IVM_OBJECT_HEADER
	WINDOW *win;
} ivm_curses_window_t;

ivm_object_t *
ivm_curses_window_new(ivm_vmstate_t *state, WINDOW *win)
{
	ivm_curses_window_t *ret = ivm_vmstate_alloc(state, sizeof(*ret));

	ivm_object_init(IVM_AS_OBJ(ret), IVM_TPTYPE(state, WINDOW_TYPE_NAME));

	ret->win = win;

	return IVM_AS_OBJ(ret);
}

IVM_NATIVE_FUNC(_curses_window)
{
	CHECK_ARG_1_TP(WINDOW_TYPE_CONS);
	return ivm_object_clone(NAT_ARG_AT(1), NAT_STATE());
}

IVM_NATIVE_FUNC(_curses_window_size)
{
	ivm_curses_window_t *wobj;
	ivm_int_t miny, minx, maxy, maxx;
	ivm_object_t *ret[2];

	CHECK_BASE_TP(WINDOW_TYPE_CONS);
	
	wobj = GET_BASE_AS(ivm_curses_window_t);
	CHECK_WIN_INIT(wobj);

	getbegyx(wobj->win, miny, minx);
	getmaxyx(wobj->win, maxy, maxx);

	ret[0] = ivm_numeric_new(NAT_STATE(), maxy - miny);
	ret[1] = ivm_numeric_new(NAT_STATE(), maxx - minx);

	return ivm_list_object_new_c(NAT_STATE(), ret, 2);
}

IVM_NATIVE_FUNC(_curses_initscr)
{
	WINDOW *win = initscr();

	RTM_ASSERT(win, CURSES_ERROR_MSG_FAILED("initialize screen"));

	return ivm_curses_window_new(NAT_STATE(), win);
}

IVM_NATIVE_FUNC(_curses_clear)
{
	RTM_ASSERT(clear() == OK, CURSES_ERROR_MSG_FAILED("clear window"));
	return IVM_NONE(NAT_STATE());
}

IVM_NATIVE_FUNC(_curses_refresh)
{
	RTM_ASSERT(refresh() == OK, CURSES_ERROR_MSG_FAILED("refresh window"));
	return IVM_NONE(NAT_STATE());
}

IVM_NATIVE_FUNC(_curses_endwin)
{
	RTM_ASSERT(endwin() == OK, CURSES_ERROR_MSG_FAILED("end window"));
	return IVM_NONE(NAT_STATE());
}

IVM_NATIVE_FUNC(_curses_stdscr)
{
	RTM_ASSERT(stdscr, CURSES_ERROR_MSG_UNINIT_WIN);
	return ivm_curses_window_new(NAT_STATE(), stdscr);
}

ivm_object_t *
ivm_mod_main(ivm_vmstate_t *state,
			 ivm_coro_t *coro,
			 ivm_context_t *context)
{
	ivm_object_t *mod = ivm_object_new(state);
	ivm_object_t *win_proto;
	ivm_type_t _window_type = IVM_TPTYPE_BUILD(
		WINDOW_TYPE_NAME, sizeof(ivm_curses_window_t),
		IVM_NATIVE_WRAP_C(state, _curses_window),
		.const_bool = IVM_TRUE
	);

	setlocale(LC_ALL, "");

	/* curses.window */
	IVM_VMSTATE_REGISTER_TPTYPE(state, coro, WINDOW_TYPE_NAME, &_window_type, {
		win_proto = ivm_curses_window_new(state, IVM_NULL);
		ivm_type_setProto(_TYPE, win_proto);
		ivm_object_setProto(win_proto, state, ivm_vmstate_getTypeProto(state, IVM_OBJECT_T));

		ivm_object_setSlot_r(win_proto, state, "size", IVM_NATIVE_WRAP(state, _curses_window_size));
	});

	ivm_object_setSlot_r(mod, state, "initscr", IVM_NATIVE_WRAP(state, _curses_initscr));
	ivm_object_setSlot_r(mod, state, "clear", IVM_NATIVE_WRAP(state, _curses_clear));
	ivm_object_setSlot_r(mod, state, "refresh", IVM_NATIVE_WRAP(state, _curses_refresh));
	ivm_object_setSlot_r(mod, state, "endwin", IVM_NATIVE_WRAP(state, _curses_endwin));

	ivm_object_setSlot_r(mod, state, "stdscr", IVM_NATIVE_WRAP(state, _curses_stdscr));

	return mod;
}
