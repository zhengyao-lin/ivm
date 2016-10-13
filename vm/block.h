#ifndef _IVM_VM_BLOCK_H_
#define _IVM_VM_BLOCK_H_

#include "pub/com.h"
#include "pub/const.h"
#include "pub/type.h"
#include "pub/err.h"

#include "std/mem.h"

#include "instr.h"

IVM_COM_HEADER

typedef struct {
	ivm_instr_t *catc;
	ivm_size_t sp; // count of element on the stack
} ivm_block_t;

#define ivm_block_setCatch(block, val) ((block)->catc = (val))

#define ivm_block_getCatch(block) ((block)->catc)
#define ivm_block_getSP(block) ((block)->sp)

typedef struct {
	ivm_block_t *st;
	ivm_uint_t cur;
	ivm_uint_t alloc;
} ivm_block_stack_t;

#define ivm_block_stack_checkCur(stack, n) ((stack)->cur == (n))

IVM_INLINE
void
ivm_block_stack_init(ivm_block_stack_t *stack)
{
	stack->cur = 0;
	stack->alloc = IVM_DEFAULT_BLOCK_STACK_BUFFER_SIZE;

	stack->st = STD_ALLOC(sizeof(*stack->st) * IVM_DEFAULT_BLOCK_STACK_BUFFER_SIZE);

	IVM_MEMCHECK(stack->st);

	return;
}

IVM_INLINE
void
ivm_block_stack_dump(ivm_block_stack_t *stack)
{
	if (stack) {
		STD_FREE(stack->st);
	}

	return;
}

IVM_INLINE
void
ivm_block_stack_setCur(ivm_block_stack_t *stack,
					   ivm_uint_t cur)
{
	stack->cur = cur;
	return;
}

IVM_INLINE
ivm_uint_t 
ivm_block_stack_getCur(ivm_block_stack_t *stack)
{
	return stack->cur;
}

IVM_INLINE
void
ivm_block_stack_push(ivm_block_stack_t *stack,
					 ivm_size_t sp)
{
	if (stack->cur >= stack->alloc) {
		stack->alloc <<= 1;
		stack->st = STD_REALLOC(stack->st, sizeof(*stack->st) * stack->alloc);
		IVM_MEMCHECK(stack->st);
	}

	stack->st[stack->cur++] = (ivm_block_t) {
		IVM_NULL, sp
	};

	return;
}

IVM_INLINE
ivm_size_t
ivm_block_stack_pop(ivm_block_stack_t *stack)
{
	return stack->st[--stack->cur].sp;
}

IVM_INLINE
void
ivm_block_stack_setCurCatch(ivm_block_stack_t *stack,
							ivm_instr_t *catc)
{
	stack->st[stack->cur - 1].catc = catc;
	return;
}

IVM_INLINE
ivm_instr_t *
ivm_block_stack_unsetCurCatch(ivm_block_stack_t *stack)
{
	ivm_block_t *cur;
	ivm_instr_t *catc;

	// if (!stack->cur) return IVM_NULL;

	cur = stack->st + (stack->cur - 1);
	catc = cur->catc;
	cur->catc = IVM_NULL;

	return catc;
}

/* stack sum of top n block */
IVM_INLINE
ivm_size_t
ivm_block_stack_getTopN(ivm_block_stack_t *stack,
						ivm_size_t n)
{
	register ivm_size_t ret = 0;
	register ivm_block_t *i, *end;

	for (i = stack->st + stack->cur - 1, end = i - n;
		 i != end; i--) {
		ret += i->sp;
	}

	return ret;
}

IVM_COM_END

#endif
