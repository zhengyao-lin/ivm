#include "pub/mem.h"
#include "pub/com.h"
#include "pub/const.h"
#include "pub/type.h"
#include "pub/vm.h"

#include "std/string.h"
#include "std/io.h"

#include "serial.h"

ivm_serial_exec_t
_ivm_serial_serializeExec(ivm_exec_t *exec,
						  ivm_size_t pool_id)
{
	ivm_serial_exec_t ret = {
		pool_id,
		ivm_exec_length(exec)
	};
	ivm_instr_t *i, *end;
	ivm_serial_instr_t *tmp;
	ivm_instr_t tmp_instr;
	ivm_bool_t cached = ivm_exec_cached(exec);

	// IVM_ASSERT(!ivm_exec_cached(exec),
	//		   IVM_ERROR_MSG_SERIALIZE_CACHED_EXEC);

	tmp = ret.instrs = MEM_ALLOC(
		sizeof(*ret.instrs) * ret.size,
		ivm_serial_instr_t *
	);

	IVM_ASSERT(ret.instrs, IVM_ERROR_MSG_FAILED_ALLOC_NEW("serialized executable"));

	for (i = ivm_exec_instrPtrStart(exec),
		 end = i + ret.size;
		 i != end; i++, tmp++) {
		if (cached) {
			tmp_instr = ivm_exec_decache(exec, i);
			*tmp = (ivm_serial_instr_t) {
				.opc = ivm_instr_opcode(&tmp_instr),
				.arg = ivm_instr_arg(&tmp_instr)
			};
		} else {
			*tmp = (ivm_serial_instr_t) {
				.opc = ivm_instr_opcode(i),
				.arg = ivm_instr_arg(i)
			};
		}
	}

	return ret;
}

ivm_exec_t *
_ivm_serial_unserializeExec(ivm_serial_exec_t *exec,
							ivm_string_pool_list_t *pool_list)
{
	ivm_exec_t *ret = ivm_exec_new(
		ivm_string_pool_list_at(pool_list, exec->pool)
	);
	ivm_serial_instr_t *i, *end;

	for (i = exec->instrs,
		 end = i + exec->size;
		 i != end; i++) {
		ivm_exec_addInstr_c(ret, ivm_instr_build(i->opc, i->arg));
	}

	return ret;
}

ivm_serial_exec_list_t *
_ivm_serial_serializeExecList(ivm_exec_list_t *list)
{
	ivm_serial_exec_list_t *
	ret = MEM_ALLOC(sizeof(*ret),
					ivm_serial_exec_list_t *);
	ivm_exec_list_iterator_t eiter;
	ivm_exec_t *tmp_exec;
	ivm_string_pool_list_t *pools;
	ivm_size_t tmp_id;
	ivm_serial_exec_t *i;

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("serialized executable list"));

	pools = ret->pool_list = ivm_string_pool_list_new();
	i = ret->execs = MEM_ALLOC(
		sizeof(*ret->execs)
		* (ret->size = ivm_exec_list_size(list)),
		ivm_serial_exec_t *
	);

	IVM_ASSERT(i, IVM_ERROR_MSG_FAILED_ALLOC_NEW("serialized executable list"));

	IVM_EXEC_LIST_EACHPTR(list, eiter) {
		tmp_exec = IVM_EXEC_LIST_ITER_GET(eiter);
		tmp_id = ivm_string_pool_list_find(pools, ivm_exec_pool(tmp_exec));

		if (tmp_id == (ivm_size_t)-1) {
			tmp_id = ivm_string_pool_list_register(
				pools, ivm_exec_pool(tmp_exec)
			);
		}

		*i++ = _ivm_serial_serializeExec(tmp_exec, tmp_id);
	}

	return ret;
}

ivm_exec_list_t *
_ivm_serial_unserializeExecList(ivm_serial_exec_list_t *list)
{
	ivm_exec_list_t *ret = ivm_exec_list_new();
	ivm_serial_exec_t *i, *end;

	for (i = list->execs,
		 end = i + list->size;
		 i != end; i++) {
		ivm_exec_list_push(
			ret,
			_ivm_serial_unserializeExec(i, list->pool_list)
		);
	}

	return ret;
}

ivm_serial_exec_unit_t *
ivm_serial_serializeExecUnit(ivm_exec_unit_t *unit)
{
	ivm_serial_exec_unit_t *ret = MEM_ALLOC(
		sizeof(*ret),
		ivm_serial_exec_unit_t *
	);

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("serialized executable unit"));

	ret->root = ivm_exec_unit_root(unit);
	ret->list = _ivm_serial_serializeExecList(
		ivm_exec_unit_execList(unit)
	);

	return ret;
}

ivm_exec_unit_t *
ivm_serial_unserializeExecUnit(ivm_serial_exec_unit_t *unit)
{
	return ivm_exec_unit_new(
		unit->root,
		_ivm_serial_unserializeExecList(unit->list)
	);
}

IVM_PRIVATE
void
_ivm_serial_exec_dump(ivm_serial_exec_t *exec)
{
	if (exec) {
		MEM_FREE(exec->instrs);
	}

	return;
}

void
_ivm_serial_exec_list_free(ivm_serial_exec_list_t *list)
{
	ivm_serial_exec_t *i, *end;

	if (list) {
		ivm_string_pool_list_free(list->pool_list);
		if (list->execs) {
			for (i = list->execs,
				 end = i + list->size;
				 i != end; i++) {
				_ivm_serial_exec_dump(i);
			}
		}
		MEM_FREE(list->execs);
		MEM_FREE(list);
	}

	return;
}

void
ivm_serial_exec_unit_free(ivm_serial_exec_unit_t *unit)
{
	if (unit) {
		_ivm_serial_exec_list_free(unit->list);
		MEM_FREE(unit);
	}

	return;
}

/*
 * string pool format(in bits):
 * ---------------------
 * |     size: 64      |
 * ---------------------
 * |    is_const: 1    |
 * |      len: 32      |
 * |    content: v     | string 0
 * ---------------------
 * |    is_const: 1    |
 * |      len: 32      |
 * |    content: v     | string 1
 * ---------------------
 *           .
 *           .
 *           .
 */

ivm_size_t
_ivm_serial_stringPoolToFile(ivm_string_pool_t *pool,
							 ivm_file_t *file)
{
	ivm_uint64_t size = ivm_string_pool_size(pool);
	const ivm_string_t **i, **end;
	ivm_size_t ret = 0;
	ivm_size_t size_pos = ivm_file_curPos(file);

	ret += ivm_file_write(file, &size, sizeof(size), 1);

	for (i = ivm_string_pool_table(pool),
		 end = i + size, size = 0;
		 i != end && *i; i++, size++) {
		ret += ivm_file_write(file, (void *)*i, ivm_string_size(*i), 1);
	}

	ivm_file_writeAt(file, size_pos, &size, sizeof(size), 1);

	return ret;
}

ivm_string_pool_t *
_ivm_serial_stringPoolFromFile(ivm_file_t *file)
{
	ivm_string_pool_t *ret;
	ivm_uint64_t size;
	ivm_size_t tmp_len;
	const ivm_string_t **table, **i, **end;
	ivm_string_t tmp_head;
	ivm_string_t *tmp_str;

	if (!ivm_file_read(file, &size, sizeof(size), 1))
		return IVM_NULL;
	// IVM_ASSERT(tmp, IVM_ERROR_MSG_FILE_FORMAT_ERR);

	// IVM_TRACE("pool size: %ld\n", size);

	ret = ivm_string_pool_new(IVM_TRUE);

	table = MEM_ALLOC(sizeof(*table) * size,
					  const ivm_string_t **);

	if (!table) {
		goto CLEAN;
	}

	// IVM_ASSERT(table, IVM_ERROR_MSG_FAILED_ALLOC_NEW("serialized string pool"));

	for (i = table,
		 end = i + size;
		 i != end; i++) {
		/* read header */
		if (!ivm_file_read(file, &tmp_head, sizeof(tmp_head), 1)) goto CLEAN;

		tmp_str = ivm_string_pool_alloc_s(ret, ivm_string_size(&tmp_head));
		if (!tmp_str) goto CLEAN;
		*tmp_str = tmp_head; /* copy header */
		tmp_len = ivm_string_length(tmp_str) + 1;

		if (ivm_file_read(file, ivm_string_trimHead(tmp_str),
						  sizeof(ivm_char_t), tmp_len)
			!= tmp_len) {
			goto CLEAN;
		}

		*i = tmp_str;
	}

	ivm_string_pool_setSize(ret, size);
	ivm_string_pool_setTable(ret, table);

goto CLEAN_END;
CLEAN:;

	ivm_ref_inc(ret);
	ivm_string_pool_free(ret);
	MEM_FREE(table);

	return IVM_NULL;

CLEAN_END:

	return ret;
}

/*
 * string pool list format(in bits):
 * ---------------------
 * |     size: 64      |
 * ---------------------
 * |     pool0: v      |
 * ---------------------
 * |     pool1: v      |
 * ---------------------
 * |     pool2: v      |
 * ---------------------
 *           .
 *           .
 *           .
 */

ivm_size_t
_ivm_serial_stringPoolListToFile(ivm_string_pool_list_t *list,
								 ivm_file_t *file)
{
	ivm_string_pool_list_iterator_t siter;
	ivm_uint64_t size = ivm_string_pool_list_size(list);
	ivm_size_t ret = 0;

	ret += ivm_file_write(file, &size, sizeof(size), 1);

	IVM_STRING_POOL_LIST_EACHPTR(list, siter) {
		ret += _ivm_serial_stringPoolToFile(
			IVM_STRING_POOL_LIST_ITER_GET(siter),
			file
		);
	}

	return ret;
}

ivm_string_pool_list_t *
_ivm_serial_stringPoolListFromFile(ivm_file_t *file)
{
	ivm_string_pool_list_t *ret;
	ivm_string_pool_t *tmp_pool;
	ivm_uint64_t size;
	ivm_size_t i;

	if (!ivm_file_read(file, &size, sizeof(size), 1))
		return IVM_NULL;
	// IVM_ASSERT(tmp, IVM_ERROR_MSG_FILE_FORMAT_ERR);
	
	// IVM_TRACE("pool count: %ld\n", size);

	ret = ivm_string_pool_list_new();

	for (i = 0; i < size; i++) {
		tmp_pool = _ivm_serial_stringPoolFromFile(file);
		if (!tmp_pool) goto CLEAN;
		ivm_string_pool_list_register(ret, tmp_pool);
	}

goto CLEAN_END;
CLEAN:;
	
	ivm_string_pool_list_free(ret);

	return IVM_NULL;

CLEAN_END:

	return ret;
}

/*
 * exec format(in bits):
 * ---------------------
 * |    pool_id: 64    |
 * |     size: 64      |
 * ---------------------
 * |     instr0: v     |
 * |     instr1: v     |
 * |         .         |
 * |         .         |
 * |         .         |
 * ---------------------
 */

ivm_size_t
_ivm_serial_execToFile(ivm_serial_exec_t *exec,
					   ivm_file_t *file)
{
	ivm_size_t ret = 0;

	ret += ivm_file_write(file, &exec->pool, sizeof(exec->pool), 1);
	ret += ivm_file_write(file, &exec->size, sizeof(exec->size), 1);
	ret += ivm_file_write(file, exec->instrs, sizeof(*exec->instrs), exec->size);

	return ret;
}

ivm_bool_t
_ivm_serial_execFromFile(ivm_file_t *file,
						 ivm_serial_exec_t *ret)
{
	if (!ivm_file_read(file, &ret->pool, sizeof(ret->pool), 1)) return IVM_FALSE;
	if (!ivm_file_read(file, &ret->size, sizeof(ret->size), 1)) return IVM_FALSE;

	ret->instrs = MEM_ALLOC(
		sizeof(*ret->instrs) * ret->size,
		ivm_serial_instr_t *
	);

	IVM_ASSERT(ret->instrs, IVM_ERROR_MSG_FAILED_ALLOC_NEW("serialized executable"));

	if (ivm_file_read(file, ret->instrs, sizeof(*ret->instrs), ret->size)
		!= ret->size) {
		MEM_FREE(ret->instrs);
		ret->instrs = IVM_NULL;
		return IVM_FALSE;
	}

	return IVM_TRUE;
}

/*
 * exec list format(in bits):
 * ---------------------
 * |     pools: v      |
 * ---------------------
 * |     size: 64      |
 * ---------------------
 * |     exec0: v      |
 * ---------------------
 * |     exec1: v      |
 * ---------------------
 * |     exec2: v      |
 * ---------------------
 *           .
 *           .
 *           .
 */

ivm_size_t
_ivm_serial_execListToFile(ivm_serial_exec_list_t *list,
						   ivm_file_t *file)
{
	ivm_size_t ret = 0;
	ivm_serial_exec_t *i, *end;

	ret += _ivm_serial_stringPoolListToFile(
		list->pool_list,
		file
	);

	ret += ivm_file_write(file, &list->size, sizeof(list->size), 1);

	for (i = list->execs,
		 end = i + list->size;
		 i != end; i++) {
		ret += _ivm_serial_execToFile(i, file);
	}

	return ret;
}

ivm_serial_exec_list_t *
_ivm_serial_execListFromFile(ivm_file_t *file)
{
	ivm_serial_exec_t *i, *end;
	ivm_serial_exec_list_t *
	ret = MEM_ALLOC_INIT(
		sizeof(*ret),
		ivm_serial_exec_list_t *
	);

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("serialized executable list"));

	if (!(ret->pool_list = _ivm_serial_stringPoolListFromFile(file)))
		goto CLEAN;

	if (!ivm_file_read(file, &ret->size, sizeof(ret->size), 1))
		goto CLEAN;

	i = ret->execs = MEM_ALLOC_INIT( /* avoid error in cleaning */
		sizeof(*ret->execs)
		* ret->size,
		ivm_serial_exec_t *
	);

	IVM_ASSERT(i, IVM_ERROR_MSG_FAILED_ALLOC_NEW("serialized executable list"));

	for (end = i + ret->size;
		 i != end; i++) {
		if (!_ivm_serial_execFromFile(file, i))
			goto CLEAN;
		if (i->pool >= ivm_string_pool_list_size(ret->pool_list))
			goto CLEAN;
	}

goto CLEAN_END;
CLEAN:;
	_ivm_serial_exec_list_free(ret);
	return IVM_NULL;
CLEAN_END:

	return ret;
}

/*
 * exec unit format(in bits):
 * ---------------------
 * |     root: 64      |
 * ---------------------
 * |     execs: v      |
 * ---------------------
 */

ivm_size_t
ivm_serial_execUnitToFile(ivm_serial_exec_unit_t *unit,
						  ivm_file_t *file)
{
	ivm_size_t ret = 0;

	ret += ivm_file_write(file, &unit->root, sizeof(unit->root), 1);
	ret += _ivm_serial_execListToFile(unit->list, file);

	return ret;
}

ivm_serial_exec_unit_t *
ivm_serial_execUnitFromFile(ivm_file_t *file)
{
	ivm_serial_exec_unit_t *ret = MEM_ALLOC(
		sizeof(*ret),
		ivm_serial_exec_unit_t *
	);

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("serialized executable unit"));

	if (!ivm_file_read(file, &ret->root, sizeof(ret->root), 1)) {
		goto CLEAN;
	}

	if (!(ret->list = _ivm_serial_execListFromFile(file))) {
		goto CLEAN;
	}

goto CLEAN_END;
CLEAN:;
	ivm_serial_exec_unit_free(ret);
	return IVM_NULL;
CLEAN_END:

	return ret;
}
