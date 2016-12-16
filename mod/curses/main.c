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

#define CHECK_PTR_ERR(call, sth) RTM_ASSERT((call) != IVM_NULL, CURSES_ERROR_MSG_FAILED(sth))
#define CHECK_INT_ERR(call, sth) RTM_ASSERT((call) != ERR, CURSES_ERROR_MSG_FAILED(sth))

typedef struct {
	IVM_OBJECT_HEADER
	WINDOW *win;
	ivm_bool_t is_stdscr;
} ivm_curses_window_t;

IVM_INLINE
WINDOW *
ivm_curses_window_getWin(ivm_curses_window_t *win)
{
	return win->is_stdscr ? stdscr : win->win;
}

ivm_object_t *
ivm_curses_window_new(ivm_vmstate_t *state, WINDOW *win)
{
	ivm_curses_window_t *ret = ivm_vmstate_alloc(state, sizeof(*ret));

	ivm_object_init(IVM_AS_OBJ(ret), IVM_TPTYPE(state, WINDOW_TYPE_NAME));

	ret->win = win;
	ret->is_stdscr = IVM_FALSE;

	return IVM_AS_OBJ(ret);
}

ivm_object_t *
ivm_curses_window_newStd(ivm_vmstate_t *state)
{
	ivm_curses_window_t *ret = ivm_vmstate_alloc(state, sizeof(*ret));

	ivm_object_init(IVM_AS_OBJ(ret), IVM_TPTYPE(state, WINDOW_TYPE_NAME));

	ret->win = IVM_NULL;
	ret->is_stdscr = IVM_TRUE;

	return IVM_AS_OBJ(ret);
}

#define CHECK_GET_WIN(wobj, to) \
	RTM_ASSERT(ivm_curses_window_getWin(wobj), CURSES_ERROR_MSG_UNINIT_WIN); \
	(to) = ivm_curses_window_getWin(wobj);

IVM_NATIVE_FUNC(_curses_window)
{
	CHECK_ARG_1_TP(WINDOW_TYPE_CONS);
	return ivm_object_clone(NAT_ARG_AT(1), NAT_STATE());
}

IVM_NATIVE_FUNC(_curses_window_size)
{
	ivm_curses_window_t *wobj;
	WINDOW *raw;
	ivm_int_t miny, minx, maxy, maxx;
	ivm_object_t *ret[2];

	CHECK_BASE_TP(WINDOW_TYPE_CONS);
	
	wobj = GET_BASE_AS(ivm_curses_window_t);
	CHECK_GET_WIN(wobj, raw);

	getbegyx(raw, miny, minx);
	getmaxyx(raw, maxy, maxx);

	ret[0] = ivm_numeric_new(NAT_STATE(), maxy - miny);
	ret[1] = ivm_numeric_new(NAT_STATE(), maxx - minx);

	return ivm_list_object_new_c(NAT_STATE(), ret, 2);
}

IVM_NATIVE_FUNC(_curses_window_pos)
{
	ivm_curses_window_t *wobj;
	WINDOW *raw;
	ivm_int_t y, x;
	ivm_object_t *ret[2];

	CHECK_BASE_TP(WINDOW_TYPE_CONS);
	
	wobj = GET_BASE_AS(ivm_curses_window_t);
	CHECK_GET_WIN(wobj, raw);

	getbegyx(raw, y, x);

	ret[0] = ivm_numeric_new(NAT_STATE(), y);
	ret[1] = ivm_numeric_new(NAT_STATE(), x);

	return ivm_list_object_new_c(NAT_STATE(), ret, 2);
}

IVM_NATIVE_FUNC(_curses_window_keypad)
{
	ivm_curses_window_t *wobj;
	WINDOW *raw;

	CHECK_BASE_TP(WINDOW_TYPE_CONS);
	CHECK_ARG_1(IVM_NUMERIC_T);
	
	wobj = GET_BASE_AS(ivm_curses_window_t);
	CHECK_GET_WIN(wobj, raw);

	CHECK_INT_ERR(keypad(raw, (ivm_bool_t)ivm_numeric_getValue(NAT_ARG_AT(1))), "set keypad");

	return IVM_NONE(NAT_STATE());
}

IVM_NATIVE_FUNC(_curses_initscr)
{
	WINDOW *win = initscr();

	RTM_ASSERT(win, CURSES_ERROR_MSG_FAILED("initialize screen"));

	return ivm_curses_window_new(NAT_STATE(), win);
}

#define INT_RET_ROUTINE(name, sth) \
	IVM_NATIVE_FUNC(_curses_##name)     \
	{                                   \
		CHECK_INT_ERR((name)(), sth);   \
		return IVM_NONE(NAT_STATE());   \
	}

#define VOID_ROUTINE(name) \
	IVM_NATIVE_FUNC(_curses_##name)     \
	{                                   \
		(name)();                       \
		return IVM_NONE(NAT_STATE());   \
	}

INT_RET_ROUTINE(clear, "clear window")
INT_RET_ROUTINE(refresh, "refresh window")
INT_RET_ROUTINE(endwin, "end window")

INT_RET_ROUTINE(cbreak, "character break")

INT_RET_ROUTINE(noecho, "unset echo")
INT_RET_ROUTINE(echo, "set echo")

INT_RET_ROUTINE(nonl, "unset newline")
INT_RET_ROUTINE(nl, "set newline")

INT_RET_ROUTINE(noraw, "unset raw mode")
INT_RET_ROUTINE(raw, "set raw mode")

VOID_ROUTINE(noqiflush)
VOID_ROUTINE(qiflush)

VOID_ROUTINE(beep)
VOID_ROUTINE(flash)

IVM_NATIVE_FUNC(_curses_timeout)
{
	CHECK_ARG_1(IVM_NUMERIC_T);

	timeout((ivm_int_t)ivm_numeric_getValue(NAT_ARG_AT(1)));

	return IVM_NONE(NAT_STATE());
}

IVM_NATIVE_FUNC(_curses_intrflush)
{
	CHECK_ARG_1(IVM_NUMERIC_T);

	RTM_ASSERT(stdscr, CURSES_ERROR_MSG_UNINIT_WIN);
	
	CHECK_INT_ERR(intrflush(stdscr, (ivm_bool_t)ivm_numeric_getValue(NAT_ARG_AT(1))), "set flush action on interruption");

	return IVM_NONE(NAT_STATE());
}

IVM_NATIVE_FUNC(_curses_getch)
{
	return ivm_numeric_new(NAT_STATE(), getch());
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

#define SET_FUNC(name) \
	ivm_object_setSlot_r(mod, state, #name, IVM_NATIVE_WRAP(state, _curses_##name))

#define SET_CONST(name, val) \
	ivm_object_setSlot_r(mod, state, #name, (val))

#define SET_WIN_METHOD(name) \
	ivm_object_setSlot_r(win_proto, state, #name, IVM_NATIVE_WRAP(state, _curses_window_##name))

	/* curses.window */
	IVM_VMSTATE_REGISTER_TPTYPE(state, coro, WINDOW_TYPE_NAME, &_window_type, {
		win_proto = ivm_curses_window_new(state, IVM_NULL);
		ivm_type_setProto(_TYPE, win_proto);
		ivm_object_setProto(win_proto, state, ivm_vmstate_getTypeProto(state, IVM_OBJECT_T));

		SET_WIN_METHOD(size);
		SET_WIN_METHOD(pos);
		SET_WIN_METHOD(keypad);
	});

	SET_FUNC(initscr);

	SET_FUNC(clear);
	SET_FUNC(refresh);
	SET_FUNC(endwin);

	SET_FUNC(cbreak);

	SET_FUNC(noecho);
	SET_FUNC(echo);

	SET_FUNC(nonl);
	SET_FUNC(nl);

	SET_FUNC(noraw);
	SET_FUNC(raw);

	SET_FUNC(noqiflush);
	SET_FUNC(qiflush);

	SET_FUNC(beep);
	SET_FUNC(flash);

	SET_FUNC(timeout);
	SET_FUNC(intrflush);
	SET_FUNC(getch);

	SET_CONST(stdscr, ivm_curses_window_newStd(state));

	return mod;
}
