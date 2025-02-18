/* module.c - the module itself
 *
 * Copyright (C) 2004-2010 Gerhard Häring <gh@ghaering.de>
 *
 * This file is part of pysqlite.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#include "connection.h"
#include "statement.h"
#include "cursor.h"
#include "prepare_protocol.h"
#include "microprotocols.h"
#include "row.h"

#if SQLITE_VERSION_NUMBER < 3007015
#error "SQLite 3.7.15 or higher required"
#endif

#define clinic_state() (pysqlite_get_state(module))
#include "clinic/module.c.h"
#undef clinic_state

/*[clinic input]
module _sqlite3
[clinic start generated code]*/
/*[clinic end generated code: output=da39a3ee5e6b4b0d input=81e330492d57488e]*/

// NOTE: This must equal sqlite3.Connection.__init__ argument spec!
/*[clinic input]
_sqlite3.connect as pysqlite_connect

    database: object(converter='PyUnicode_FSConverter')
    timeout: double = 5.0
    detect_types: int = 0
    isolation_level: object = NULL
    check_same_thread: bool(accept={int}) = True
    factory: object(c_default='(PyObject*)clinic_state()->ConnectionType') = ConnectionType
    cached_statements: int = 128
    uri: bool = False

Opens a connection to the SQLite database file database.

You can use ":memory:" to open a database connection to a database that resides
in RAM instead of on disk.
[clinic start generated code]*/

static PyObject *
pysqlite_connect_impl(PyObject *module, PyObject *database, double timeout,
                      int detect_types, PyObject *isolation_level,
                      int check_same_thread, PyObject *factory,
                      int cached_statements, int uri)
/*[clinic end generated code: output=450ac9078b4868bb input=ea6355ba55a78e12]*/
{
    if (isolation_level == NULL) {
        isolation_level = PyUnicode_FromString("");
        if (isolation_level == NULL) {
            return NULL;
        }
    }
    else {
        Py_INCREF(isolation_level);
    }
    PyObject *res = PyObject_CallFunction(factory, "OdiOiOii", database,
                                          timeout, detect_types,
                                          isolation_level, check_same_thread,
                                          factory, cached_statements, uri);
    Py_DECREF(database);  // needed bco. the AC FSConverter
    Py_DECREF(isolation_level);
    return res;
}

/*[clinic input]
_sqlite3.complete_statement as pysqlite_complete_statement

    statement: str

Checks if a string contains a complete SQL statement. Non-standard.
[clinic start generated code]*/

static PyObject *
pysqlite_complete_statement_impl(PyObject *module, const char *statement)
/*[clinic end generated code: output=e55f1ff1952df558 input=f6b24996b31c5c33]*/
{
    if (sqlite3_complete(statement)) {
        return Py_NewRef(Py_True);
    } else {
        return Py_NewRef(Py_False);
    }
}

/*[clinic input]
_sqlite3.enable_shared_cache as pysqlite_enable_shared_cache

    do_enable: int

Enable or disable shared cache mode for the calling thread.

Experimental/Non-standard.
[clinic start generated code]*/

static PyObject *
pysqlite_enable_shared_cache_impl(PyObject *module, int do_enable)
/*[clinic end generated code: output=259c74eedee1516b input=8400e41bc58b6b24]*/
{
    int rc;

    rc = sqlite3_enable_shared_cache(do_enable);

    if (rc != SQLITE_OK) {
        pysqlite_state *state = pysqlite_get_state(module);
        PyErr_SetString(state->OperationalError, "Changing the shared_cache flag failed");
        return NULL;
    } else {
        Py_RETURN_NONE;
    }
}

/*[clinic input]
_sqlite3.register_adapter as pysqlite_register_adapter

    type: object(type='PyTypeObject *')
    caster: object
    /

Registers an adapter with pysqlite's adapter registry. Non-standard.
[clinic start generated code]*/

static PyObject *
pysqlite_register_adapter_impl(PyObject *module, PyTypeObject *type,
                               PyObject *caster)
/*[clinic end generated code: output=a287e8db18e8af23 input=839dad90e2492725]*/
{
    int rc;

    /* a basic type is adapted; there's a performance optimization if that's not the case
     * (99 % of all usages) */
    if (type == &PyLong_Type || type == &PyFloat_Type
            || type == &PyUnicode_Type || type == &PyByteArray_Type) {
        pysqlite_state *state = pysqlite_get_state(module);
        state->BaseTypeAdapted = 1;
    }

    pysqlite_state *state = pysqlite_get_state(module);
    PyObject *protocol = (PyObject *)state->PrepareProtocolType;
    rc = pysqlite_microprotocols_add(state, type, protocol, caster);
    if (rc == -1) {
        return NULL;
    }

    Py_RETURN_NONE;
}

/*[clinic input]
_sqlite3.register_converter as pysqlite_register_converter

    name as orig_name: unicode
    converter as callable: object
    /

Registers a converter with pysqlite. Non-standard.
[clinic start generated code]*/

static PyObject *
pysqlite_register_converter_impl(PyObject *module, PyObject *orig_name,
                                 PyObject *callable)
/*[clinic end generated code: output=a2f2bfeed7230062 input=e074cf7f4890544f]*/
{
    PyObject* name = NULL;
    PyObject* retval = NULL;

    /* convert the name to upper case */
    pysqlite_state *state = pysqlite_get_state(module);
    name = PyObject_CallMethodNoArgs(orig_name, state->str_upper);
    if (!name) {
        goto error;
    }

    if (PyDict_SetItem(state->converters, name, callable) != 0) {
        goto error;
    }

    retval = Py_NewRef(Py_None);
error:
    Py_XDECREF(name);
    return retval;
}

/*[clinic input]
_sqlite3.enable_callback_tracebacks as pysqlite_enable_callback_trace

    enable: int
    /

Enable or disable callback functions throwing errors to stderr.
[clinic start generated code]*/

static PyObject *
pysqlite_enable_callback_trace_impl(PyObject *module, int enable)
/*[clinic end generated code: output=4ff1d051c698f194 input=cb79d3581eb77c40]*/
{
    pysqlite_state *state = pysqlite_get_state(module);
    state->enable_callback_tracebacks = enable;

    Py_RETURN_NONE;
}

/*[clinic input]
_sqlite3.adapt as pysqlite_adapt

    obj: object
    proto: object(c_default='(PyObject *)clinic_state()->PrepareProtocolType') = PrepareProtocolType
    alt: object = NULL
    /

Adapt given object to given protocol. Non-standard.
[clinic start generated code]*/

static PyObject *
pysqlite_adapt_impl(PyObject *module, PyObject *obj, PyObject *proto,
                    PyObject *alt)
/*[clinic end generated code: output=0c3927c5fcd23dd9 input=c8995aeb25d0e542]*/
{
    pysqlite_state *state = pysqlite_get_state(module);
    return pysqlite_microprotocols_adapt(state, obj, proto, alt);
}

static int converters_init(PyObject* module)
{
    pysqlite_state *state = pysqlite_get_state(module);
    state->converters = PyDict_New();
    if (state->converters == NULL) {
        return -1;
    }

    return PyModule_AddObjectRef(module, "converters", state->converters);
}

static int
load_functools_lru_cache(PyObject *module)
{
    PyObject *functools = PyImport_ImportModule("functools");
    if (functools == NULL) {
        return -1;
    }

    pysqlite_state *state = pysqlite_get_state(module);
    state->lru_cache = PyObject_GetAttrString(functools, "lru_cache");
    Py_DECREF(functools);
    if (state->lru_cache == NULL) {
        return -1;
    }
    return 0;
}

static PyMethodDef module_methods[] = {
    PYSQLITE_ADAPT_METHODDEF
    PYSQLITE_COMPLETE_STATEMENT_METHODDEF
    PYSQLITE_CONNECT_METHODDEF
    PYSQLITE_ENABLE_CALLBACK_TRACE_METHODDEF
    PYSQLITE_ENABLE_SHARED_CACHE_METHODDEF
    PYSQLITE_REGISTER_ADAPTER_METHODDEF
    PYSQLITE_REGISTER_CONVERTER_METHODDEF
    {NULL, NULL}
};

/* SQLite C API result codes. See also:
 * - https://www.sqlite.org/c3ref/c_abort_rollback.html
 * - https://sqlite.org/changes.html#version_3_3_8
 * - https://sqlite.org/changes.html#version_3_7_16
 * - https://sqlite.org/changes.html#version_3_7_17
 * - https://sqlite.org/changes.html#version_3_8_0
 * - https://sqlite.org/changes.html#version_3_8_3
 * - https://sqlite.org/changes.html#version_3_14
 *
 * Note: the SQLite changelogs rarely mention new result codes, so in order to
 * keep the 'error_codes' table in sync with SQLite, we must manually inspect
 * sqlite3.h for every release.
 *
 * We keep the SQLITE_VERSION_NUMBER checks in order to easily declutter the
 * code when we adjust the SQLite version requirement.
 */
static const struct {
    const char *name;
    long value;
} error_codes[] = {
#define DECLARE_ERROR_CODE(code) {#code, code}
    // Primary result code list
    DECLARE_ERROR_CODE(SQLITE_ABORT),
    DECLARE_ERROR_CODE(SQLITE_AUTH),
    DECLARE_ERROR_CODE(SQLITE_BUSY),
    DECLARE_ERROR_CODE(SQLITE_CANTOPEN),
    DECLARE_ERROR_CODE(SQLITE_CONSTRAINT),
    DECLARE_ERROR_CODE(SQLITE_CORRUPT),
    DECLARE_ERROR_CODE(SQLITE_DONE),
    DECLARE_ERROR_CODE(SQLITE_EMPTY),
    DECLARE_ERROR_CODE(SQLITE_ERROR),
    DECLARE_ERROR_CODE(SQLITE_FORMAT),
    DECLARE_ERROR_CODE(SQLITE_FULL),
    DECLARE_ERROR_CODE(SQLITE_INTERNAL),
    DECLARE_ERROR_CODE(SQLITE_INTERRUPT),
    DECLARE_ERROR_CODE(SQLITE_IOERR),
    DECLARE_ERROR_CODE(SQLITE_LOCKED),
    DECLARE_ERROR_CODE(SQLITE_MISMATCH),
    DECLARE_ERROR_CODE(SQLITE_MISUSE),
    DECLARE_ERROR_CODE(SQLITE_NOLFS),
    DECLARE_ERROR_CODE(SQLITE_NOMEM),
    DECLARE_ERROR_CODE(SQLITE_NOTADB),
    DECLARE_ERROR_CODE(SQLITE_NOTFOUND),
    DECLARE_ERROR_CODE(SQLITE_OK),
    DECLARE_ERROR_CODE(SQLITE_PERM),
    DECLARE_ERROR_CODE(SQLITE_PROTOCOL),
    DECLARE_ERROR_CODE(SQLITE_RANGE),
    DECLARE_ERROR_CODE(SQLITE_READONLY),
    DECLARE_ERROR_CODE(SQLITE_ROW),
    DECLARE_ERROR_CODE(SQLITE_SCHEMA),
    DECLARE_ERROR_CODE(SQLITE_TOOBIG),
#if SQLITE_VERSION_NUMBER >= 3007017
    DECLARE_ERROR_CODE(SQLITE_NOTICE),
    DECLARE_ERROR_CODE(SQLITE_WARNING),
#endif
    // Extended result code list
    DECLARE_ERROR_CODE(SQLITE_ABORT_ROLLBACK),
    DECLARE_ERROR_CODE(SQLITE_BUSY_RECOVERY),
    DECLARE_ERROR_CODE(SQLITE_CANTOPEN_FULLPATH),
    DECLARE_ERROR_CODE(SQLITE_CANTOPEN_ISDIR),
    DECLARE_ERROR_CODE(SQLITE_CANTOPEN_NOTEMPDIR),
    DECLARE_ERROR_CODE(SQLITE_CORRUPT_VTAB),
    DECLARE_ERROR_CODE(SQLITE_IOERR_ACCESS),
    DECLARE_ERROR_CODE(SQLITE_IOERR_BLOCKED),
    DECLARE_ERROR_CODE(SQLITE_IOERR_CHECKRESERVEDLOCK),
    DECLARE_ERROR_CODE(SQLITE_IOERR_CLOSE),
    DECLARE_ERROR_CODE(SQLITE_IOERR_DELETE),
    DECLARE_ERROR_CODE(SQLITE_IOERR_DELETE_NOENT),
    DECLARE_ERROR_CODE(SQLITE_IOERR_DIR_CLOSE),
    DECLARE_ERROR_CODE(SQLITE_IOERR_DIR_FSYNC),
    DECLARE_ERROR_CODE(SQLITE_IOERR_FSTAT),
    DECLARE_ERROR_CODE(SQLITE_IOERR_FSYNC),
    DECLARE_ERROR_CODE(SQLITE_IOERR_LOCK),
    DECLARE_ERROR_CODE(SQLITE_IOERR_NOMEM),
    DECLARE_ERROR_CODE(SQLITE_IOERR_RDLOCK),
    DECLARE_ERROR_CODE(SQLITE_IOERR_READ),
    DECLARE_ERROR_CODE(SQLITE_IOERR_SEEK),
    DECLARE_ERROR_CODE(SQLITE_IOERR_SHMLOCK),
    DECLARE_ERROR_CODE(SQLITE_IOERR_SHMMAP),
    DECLARE_ERROR_CODE(SQLITE_IOERR_SHMOPEN),
    DECLARE_ERROR_CODE(SQLITE_IOERR_SHMSIZE),
    DECLARE_ERROR_CODE(SQLITE_IOERR_SHORT_READ),
    DECLARE_ERROR_CODE(SQLITE_IOERR_TRUNCATE),
    DECLARE_ERROR_CODE(SQLITE_IOERR_UNLOCK),
    DECLARE_ERROR_CODE(SQLITE_IOERR_WRITE),
    DECLARE_ERROR_CODE(SQLITE_LOCKED_SHAREDCACHE),
    DECLARE_ERROR_CODE(SQLITE_READONLY_CANTLOCK),
    DECLARE_ERROR_CODE(SQLITE_READONLY_RECOVERY),
#if SQLITE_VERSION_NUMBER >= 3007016
    DECLARE_ERROR_CODE(SQLITE_CONSTRAINT_CHECK),
    DECLARE_ERROR_CODE(SQLITE_CONSTRAINT_COMMITHOOK),
    DECLARE_ERROR_CODE(SQLITE_CONSTRAINT_FOREIGNKEY),
    DECLARE_ERROR_CODE(SQLITE_CONSTRAINT_FUNCTION),
    DECLARE_ERROR_CODE(SQLITE_CONSTRAINT_NOTNULL),
    DECLARE_ERROR_CODE(SQLITE_CONSTRAINT_PRIMARYKEY),
    DECLARE_ERROR_CODE(SQLITE_CONSTRAINT_TRIGGER),
    DECLARE_ERROR_CODE(SQLITE_CONSTRAINT_UNIQUE),
    DECLARE_ERROR_CODE(SQLITE_CONSTRAINT_VTAB),
    DECLARE_ERROR_CODE(SQLITE_READONLY_ROLLBACK),
#endif
#if SQLITE_VERSION_NUMBER >= 3007017
    DECLARE_ERROR_CODE(SQLITE_IOERR_MMAP),
    DECLARE_ERROR_CODE(SQLITE_NOTICE_RECOVER_ROLLBACK),
    DECLARE_ERROR_CODE(SQLITE_NOTICE_RECOVER_WAL),
#endif
#if SQLITE_VERSION_NUMBER >= 3008000
    DECLARE_ERROR_CODE(SQLITE_BUSY_SNAPSHOT),
    DECLARE_ERROR_CODE(SQLITE_IOERR_GETTEMPPATH),
    DECLARE_ERROR_CODE(SQLITE_WARNING_AUTOINDEX),
#endif
#if SQLITE_VERSION_NUMBER >= 3008001
    DECLARE_ERROR_CODE(SQLITE_CANTOPEN_CONVPATH),
    DECLARE_ERROR_CODE(SQLITE_IOERR_CONVPATH),
#endif
#if SQLITE_VERSION_NUMBER >= 3008002
    DECLARE_ERROR_CODE(SQLITE_CONSTRAINT_ROWID),
#endif
#if SQLITE_VERSION_NUMBER >= 3008003
    DECLARE_ERROR_CODE(SQLITE_READONLY_DBMOVED),
#endif
#if SQLITE_VERSION_NUMBER >= 3008007
    DECLARE_ERROR_CODE(SQLITE_AUTH_USER),
#endif
#if SQLITE_VERSION_NUMBER >= 3009000
    DECLARE_ERROR_CODE(SQLITE_IOERR_VNODE),
#endif
#if SQLITE_VERSION_NUMBER >= 3010000
    DECLARE_ERROR_CODE(SQLITE_IOERR_AUTH),
#endif
#if SQLITE_VERSION_NUMBER >= 3014001
    DECLARE_ERROR_CODE(SQLITE_OK_LOAD_PERMANENTLY),
#endif
#if SQLITE_VERSION_NUMBER >= 3021000
    DECLARE_ERROR_CODE(SQLITE_IOERR_BEGIN_ATOMIC),
    DECLARE_ERROR_CODE(SQLITE_IOERR_COMMIT_ATOMIC),
    DECLARE_ERROR_CODE(SQLITE_IOERR_ROLLBACK_ATOMIC),
#endif
#if SQLITE_VERSION_NUMBER >= 3022000
    DECLARE_ERROR_CODE(SQLITE_ERROR_MISSING_COLLSEQ),
    DECLARE_ERROR_CODE(SQLITE_ERROR_RETRY),
    DECLARE_ERROR_CODE(SQLITE_READONLY_CANTINIT),
    DECLARE_ERROR_CODE(SQLITE_READONLY_DIRECTORY),
#endif
#if SQLITE_VERSION_NUMBER >= 3024000
    DECLARE_ERROR_CODE(SQLITE_CORRUPT_SEQUENCE),
    DECLARE_ERROR_CODE(SQLITE_LOCKED_VTAB),
#endif
#if SQLITE_VERSION_NUMBER >= 3025000
    DECLARE_ERROR_CODE(SQLITE_CANTOPEN_DIRTYWAL),
    DECLARE_ERROR_CODE(SQLITE_ERROR_SNAPSHOT),
#endif
#if SQLITE_VERSION_NUMBER >= 3031000
    DECLARE_ERROR_CODE(SQLITE_CANTOPEN_SYMLINK),
    DECLARE_ERROR_CODE(SQLITE_CONSTRAINT_PINNED),
    DECLARE_ERROR_CODE(SQLITE_OK_SYMLINK),
#endif
#if SQLITE_VERSION_NUMBER >= 3032000
    DECLARE_ERROR_CODE(SQLITE_BUSY_TIMEOUT),
    DECLARE_ERROR_CODE(SQLITE_CORRUPT_INDEX),
    DECLARE_ERROR_CODE(SQLITE_IOERR_DATA),
#endif
#if SQLITE_VERSION_NUMBER >= 3034000
    DECLARE_ERROR_CODE(SQLITE_IOERR_CORRUPTFS),
#endif
#undef DECLARE_ERROR_CODE
    {NULL, 0},
};

static int
add_error_constants(PyObject *module)
{
    for (int i = 0; error_codes[i].name != NULL; i++) {
        const char *name = error_codes[i].name;
        const long value = error_codes[i].value;
        if (PyModule_AddIntConstant(module, name, value) < 0) {
            return -1;
        }
    }
    return 0;
}

const char *
pysqlite_error_name(int rc)
{
    for (int i = 0; error_codes[i].name != NULL; i++) {
        if (error_codes[i].value == rc) {
            return error_codes[i].name;
        }
    }
    // No error code matched.
    return NULL;
}

static int
add_integer_constants(PyObject *module) {
#define ADD_INT(ival)                                           \
    do {                                                        \
        if (PyModule_AddIntConstant(module, #ival, ival) < 0) { \
            return -1;                                          \
        }                                                       \
    } while (0);                                                \

    ADD_INT(PARSE_DECLTYPES);
    ADD_INT(PARSE_COLNAMES);
    ADD_INT(SQLITE_DENY);
    ADD_INT(SQLITE_IGNORE);
    ADD_INT(SQLITE_CREATE_INDEX);
    ADD_INT(SQLITE_CREATE_TABLE);
    ADD_INT(SQLITE_CREATE_TEMP_INDEX);
    ADD_INT(SQLITE_CREATE_TEMP_TABLE);
    ADD_INT(SQLITE_CREATE_TEMP_TRIGGER);
    ADD_INT(SQLITE_CREATE_TEMP_VIEW);
    ADD_INT(SQLITE_CREATE_TRIGGER);
    ADD_INT(SQLITE_CREATE_VIEW);
    ADD_INT(SQLITE_DELETE);
    ADD_INT(SQLITE_DROP_INDEX);
    ADD_INT(SQLITE_DROP_TABLE);
    ADD_INT(SQLITE_DROP_TEMP_INDEX);
    ADD_INT(SQLITE_DROP_TEMP_TABLE);
    ADD_INT(SQLITE_DROP_TEMP_TRIGGER);
    ADD_INT(SQLITE_DROP_TEMP_VIEW);
    ADD_INT(SQLITE_DROP_TRIGGER);
    ADD_INT(SQLITE_DROP_VIEW);
    ADD_INT(SQLITE_INSERT);
    ADD_INT(SQLITE_PRAGMA);
    ADD_INT(SQLITE_READ);
    ADD_INT(SQLITE_SELECT);
    ADD_INT(SQLITE_TRANSACTION);
    ADD_INT(SQLITE_UPDATE);
    ADD_INT(SQLITE_ATTACH);
    ADD_INT(SQLITE_DETACH);
    ADD_INT(SQLITE_ALTER_TABLE);
    ADD_INT(SQLITE_REINDEX);
    ADD_INT(SQLITE_ANALYZE);
    ADD_INT(SQLITE_CREATE_VTABLE);
    ADD_INT(SQLITE_DROP_VTABLE);
    ADD_INT(SQLITE_FUNCTION);
    ADD_INT(SQLITE_SAVEPOINT);
#if SQLITE_VERSION_NUMBER >= 3008003
    ADD_INT(SQLITE_RECURSIVE);
#endif
    // Run-time limit categories
    ADD_INT(SQLITE_LIMIT_LENGTH);
    ADD_INT(SQLITE_LIMIT_SQL_LENGTH);
    ADD_INT(SQLITE_LIMIT_COLUMN);
    ADD_INT(SQLITE_LIMIT_EXPR_DEPTH);
    ADD_INT(SQLITE_LIMIT_COMPOUND_SELECT);
    ADD_INT(SQLITE_LIMIT_VDBE_OP);
    ADD_INT(SQLITE_LIMIT_FUNCTION_ARG);
    ADD_INT(SQLITE_LIMIT_ATTACHED);
    ADD_INT(SQLITE_LIMIT_LIKE_PATTERN_LENGTH);
    ADD_INT(SQLITE_LIMIT_VARIABLE_NUMBER);
    ADD_INT(SQLITE_LIMIT_TRIGGER_DEPTH);
#if SQLITE_VERSION_NUMBER >= 3008007
    ADD_INT(SQLITE_LIMIT_WORKER_THREADS);
#endif
#undef ADD_INT
    return 0;
}

/* Convert SQLite default threading mode (as set by the compile-time constant
 * SQLITE_THREADSAFE) to the corresponding DB-API 2.0 (PEP 249) threadsafety
 * level. */
static int
get_threadsafety(pysqlite_state *state)
{
    int mode = sqlite3_threadsafe();
    switch (mode) {
    case 0:        // Single-thread mode; threads may not share the module.
        return 0;
    case 1:        // Serialized mode; threads may share the module,
        return 3;  // connections, and cursors.
    case 2:        // Multi-thread mode; threads may share the module, but not
        return 1;  // connections.
    default:
        PyErr_Format(state->InterfaceError,
                     "Unable to interpret SQLite threadsafety mode. Got %d, "
                     "expected 0, 1, or 2", mode);
        return -1;
    }
}

static int
module_traverse(PyObject *module, visitproc visit, void *arg)
{
    pysqlite_state *state = pysqlite_get_state(module);

    // Exceptions
    Py_VISIT(state->DataError);
    Py_VISIT(state->DatabaseError);
    Py_VISIT(state->Error);
    Py_VISIT(state->IntegrityError);
    Py_VISIT(state->InterfaceError);
    Py_VISIT(state->InternalError);
    Py_VISIT(state->NotSupportedError);
    Py_VISIT(state->OperationalError);
    Py_VISIT(state->ProgrammingError);
    Py_VISIT(state->Warning);

    // Types
    Py_VISIT(state->ConnectionType);
    Py_VISIT(state->CursorType);
    Py_VISIT(state->PrepareProtocolType);
    Py_VISIT(state->RowType);
    Py_VISIT(state->StatementType);

    // Misc
    Py_VISIT(state->converters);
    Py_VISIT(state->lru_cache);
    Py_VISIT(state->psyco_adapters);

    return 0;
}

static int
module_clear(PyObject *module)
{
    pysqlite_state *state = pysqlite_get_state(module);

    // Exceptions
    Py_CLEAR(state->DataError);
    Py_CLEAR(state->DatabaseError);
    Py_CLEAR(state->Error);
    Py_CLEAR(state->IntegrityError);
    Py_CLEAR(state->InterfaceError);
    Py_CLEAR(state->InternalError);
    Py_CLEAR(state->NotSupportedError);
    Py_CLEAR(state->OperationalError);
    Py_CLEAR(state->ProgrammingError);
    Py_CLEAR(state->Warning);

    // Types
    Py_CLEAR(state->ConnectionType);
    Py_CLEAR(state->CursorType);
    Py_CLEAR(state->PrepareProtocolType);
    Py_CLEAR(state->RowType);
    Py_CLEAR(state->StatementType);

    // Misc
    Py_CLEAR(state->converters);
    Py_CLEAR(state->lru_cache);
    Py_CLEAR(state->psyco_adapters);

    // Interned strings
    Py_CLEAR(state->str___adapt__);
    Py_CLEAR(state->str___conform__);
    Py_CLEAR(state->str_executescript);
    Py_CLEAR(state->str_finalize);
    Py_CLEAR(state->str_upper);

    return 0;
}

static void
module_free(void *module)
{
    module_clear((PyObject *)module);
}

#define ADD_TYPE(module, type)                 \
do {                                           \
    if (PyModule_AddType(module, type) < 0) {  \
        goto error;                            \
    }                                          \
} while (0)

#define ADD_EXCEPTION(module, state, exc, base)                        \
do {                                                                   \
    state->exc = PyErr_NewException(MODULE_NAME "." #exc, base, NULL); \
    if (state->exc == NULL) {                                          \
        goto error;                                                    \
    }                                                                  \
    ADD_TYPE(module, (PyTypeObject *)state->exc);                      \
} while (0)

#define ADD_INTERNED(state, string)                      \
do {                                                     \
    PyObject *tmp = PyUnicode_InternFromString(#string); \
    if (tmp == NULL) {                                   \
        goto error;                                      \
    }                                                    \
    state->str_ ## string = tmp;                         \
} while (0)

static int
module_exec(PyObject *module)
{
    if (sqlite3_libversion_number() < 3007015) {
        PyErr_SetString(PyExc_ImportError, MODULE_NAME ": SQLite 3.7.15 or higher required");
        return -1;
    }

    int rc = sqlite3_initialize();
    if (rc != SQLITE_OK) {
        PyErr_SetString(PyExc_ImportError, sqlite3_errstr(rc));
        return -1;
    }

    if ((pysqlite_row_setup_types(module) < 0) ||
        (pysqlite_cursor_setup_types(module) < 0) ||
        (pysqlite_connection_setup_types(module) < 0) ||
        (pysqlite_statement_setup_types(module) < 0) ||
        (pysqlite_prepare_protocol_setup_types(module) < 0)
       ) {
        goto error;
    }

    pysqlite_state *state = pysqlite_get_state(module);
    ADD_TYPE(module, state->ConnectionType);
    ADD_TYPE(module, state->CursorType);
    ADD_TYPE(module, state->PrepareProtocolType);
    ADD_TYPE(module, state->RowType);

    /*** Create DB-API Exception hierarchy */
    ADD_EXCEPTION(module, state, Error, PyExc_Exception);
    ADD_EXCEPTION(module, state, Warning, PyExc_Exception);

    /* Error subclasses */
    ADD_EXCEPTION(module, state, InterfaceError, state->Error);
    ADD_EXCEPTION(module, state, DatabaseError, state->Error);

    /* DatabaseError subclasses */
    ADD_EXCEPTION(module, state, InternalError, state->DatabaseError);
    ADD_EXCEPTION(module, state, OperationalError, state->DatabaseError);
    ADD_EXCEPTION(module, state, ProgrammingError, state->DatabaseError);
    ADD_EXCEPTION(module, state, IntegrityError, state->DatabaseError);
    ADD_EXCEPTION(module, state, DataError, state->DatabaseError);
    ADD_EXCEPTION(module, state, NotSupportedError, state->DatabaseError);

    /* Add interned strings */
    ADD_INTERNED(state, __adapt__);
    ADD_INTERNED(state, __conform__);
    ADD_INTERNED(state, executescript);
    ADD_INTERNED(state, finalize);
    ADD_INTERNED(state, upper);

    /* Set error constants */
    if (add_error_constants(module) < 0) {
        goto error;
    }

    /* Set integer constants */
    if (add_integer_constants(module) < 0) {
        goto error;
    }

    if (PyModule_AddStringConstant(module, "version", PYSQLITE_VERSION) < 0) {
        goto error;
    }

    if (PyModule_AddStringConstant(module, "sqlite_version", sqlite3_libversion())) {
        goto error;
    }

    int threadsafety = get_threadsafety(state);
    if (threadsafety < 0) {
        goto error;
    }
    if (PyModule_AddIntConstant(module, "threadsafety", threadsafety) < 0) {
        goto error;
    }

    /* initialize microprotocols layer */
    if (pysqlite_microprotocols_init(module) < 0) {
        goto error;
    }

    /* initialize the default converters */
    if (converters_init(module) < 0) {
        goto error;
    }

    if (load_functools_lru_cache(module) < 0) {
        goto error;
    }

    return 0;

error:
    sqlite3_shutdown();
    return -1;
}

static struct PyModuleDef_Slot module_slots[] = {
    {Py_mod_exec, module_exec},
    {0, NULL},
};

struct PyModuleDef _sqlite3module = {
    .m_base = PyModuleDef_HEAD_INIT,
    .m_name = "_sqlite3",
    .m_size = sizeof(pysqlite_state),
    .m_methods = module_methods,
    .m_slots = module_slots,
    .m_traverse = module_traverse,
    .m_clear = module_clear,
    .m_free = module_free,
};

PyMODINIT_FUNC
PyInit__sqlite3(void)
{
    return PyModuleDef_Init(&_sqlite3module);
}
