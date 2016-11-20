#ifndef _IVM_VM_THREAD_SCHED_H_
#define _IVM_VM_THREAD_SCHED_H_

#include "pub/const.h"
#include "pub/com.h"
#include "pub/vm.h"

#include "std/thread.h"
#include "std/hash.h"

IVM_COM_HEADER

// return false if thread is not built-in/enabled
ivm_bool_t
ivm_thread_enableThread();

void
ivm_thread_clean();

// must be called by the root thread
void
ivm_thread_mainThreadStart();

void
ivm_thread_mainThreadEnd();

IVM_COM_END

#endif
