/*
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2006 The PHP Group, (c) 2008-2015 Dmitry Zenovich |
  | "runkit7" patches (c) 2015-2020 Tyson Andre                          |
  +----------------------------------------------------------------------+
  | This source file is subject to the new BSD license,                  |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.opensource.org/licenses/BSD-3-Clause                      |
  | or at https://github.com/runkit7/runkit7/blob/master/LICENSE         |
  +----------------------------------------------------------------------+
  | Author: Sara Golemon <pollita@php.net>                               |
  | Modified by Dmitry Zenovich <dzenovich@gmail.com>                    |
  | Modified for php7 by Tyson Andre <tysonandre775@hotmail.com>         |
  +----------------------------------------------------------------------+
*/

#include "runkit.h"
#include "Zend/zend_extensions.h"
#include "SAPI.h"

/* PHP 8.0+ allows extensions to see the representation of default values. Older versions don't. */
#if PHP_VERSION_ID < 80000
#define ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(pass_by_ref, name, type_hint, allow_null, default_value) ZEND_ARG_TYPE_INFO(pass_by_ref, name, type_hint, allow_null)
#define ZEND_ARG_INFO_WITH_DEFAULT_VALUE(pass_by_ref, name, default_value) ZEND_ARG_INFO(pass_by_ref, name)
#endif

#include "runkit7_arginfo.h"

ZEND_DECLARE_MODULE_GLOBALS(runkit7)

#ifdef PHP_RUNKIT_SUPERGLOBALS
/* {{{ proto array runkit7_superglobals(void)
	Return numerically indexed array of registered superglobals */
PHP_FUNCTION(runkit7_superglobals)
{
	zend_string *key;

	array_init(return_value);
	ZEND_HASH_FOREACH_STR_KEY(CG(auto_globals), key) {
		if (key != NULL) {
			zend_string_addref(key);
			add_next_index_str(return_value, key);
		}
	} ZEND_HASH_FOREACH_END();
}
/* }}} */
#endif /* PHP_RUNKIT_SUPERGLOBALS */

/* {{{ proto array runkit7_zval_inspect(mixed var)
 */
PHP_FUNCTION(runkit7_zval_inspect)
{
	// TODO: Specify what this should do for php7 (e.g. primitives are no longer refcounted)
	zval *value;
	char *addr;
	int addr_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &value) == FAILURE) {
		return;
	}

	array_init(return_value);

	addr_len = spprintf(&addr, 0, "0x%0lx", (long)value);
	add_assoc_stringl(return_value, "address", addr, addr_len);
	efree(addr);
	addr = NULL;

	if (Z_REFCOUNTED_P(value)) {
		add_assoc_long(return_value, "refcount", Z_REFCOUNT_P(value));
		add_assoc_bool(return_value, "is_ref", Z_ISREF_P(value));
	}

	add_assoc_long(return_value, "type", Z_TYPE_P(value));
}
/* }}} */

#ifndef ZEND_ARG_INFO_WITH_DEFAULT_VALUE
#define ZEND_ARG_INFO_WITH_DEFAULT_VALUE(pass_by_ref, name, default_value) \
	ZEND_ARG_INFO(pass_by_ref, name)
#endif

#define PHP_FE_AND_FALIAS(runkit_function_name, runkit7_function_name) \
	PHP_FE(runkit7_function_name,									arginfo_ ## runkit_function_name) \
	PHP_DEP_FALIAS(runkit_function_name, runkit7_function_name,		arginfo_ ## runkit7_function_name)

PHP_FUNCTION(runkit7_zval_inspect);

#ifdef PHP_RUNKIT_SUPERGLOBALS
PHP_FUNCTION(runkit7_superglobals);
#endif

#ifdef PHP_RUNKIT_MANIPULATION

PHP_FUNCTION(runkit7_function_add);
PHP_FUNCTION(runkit7_function_remove);
PHP_FUNCTION(runkit7_function_rename);
PHP_FUNCTION(runkit7_function_redefine);
PHP_FUNCTION(runkit7_function_copy);

PHP_FUNCTION(runkit7_method_add);
PHP_FUNCTION(runkit7_method_redefine);
PHP_FUNCTION(runkit7_method_remove);
PHP_FUNCTION(runkit7_method_rename);
PHP_FUNCTION(runkit7_method_copy);

PHP_FUNCTION(runkit7_constant_redefine);
PHP_FUNCTION(runkit7_constant_remove);
PHP_FUNCTION(runkit7_constant_add);
#endif /* PHP_RUNKIT_MANIPULATION */

/* {{{ runkit7_functions[]
 */
zend_function_entry runkit7_functions[] = {

	PHP_FE_AND_FALIAS(runkit_zval_inspect,			runkit7_zval_inspect)
#ifdef PHP_RUNKIT_SUPERGLOBALS
	PHP_FE_AND_FALIAS(runkit_superglobals,			runkit7_superglobals)
#endif

#ifdef PHP_RUNKIT_MANIPULATION
	PHP_FE_AND_FALIAS(runkit_function_add,			runkit7_function_add)
	PHP_FE_AND_FALIAS(runkit_function_remove,		runkit7_function_remove)
	PHP_FE_AND_FALIAS(runkit_function_rename,		runkit7_function_rename)
	PHP_FE_AND_FALIAS(runkit_function_redefine,		runkit7_function_redefine)
	PHP_FE_AND_FALIAS(runkit_function_copy,			runkit7_function_copy)

	PHP_FE_AND_FALIAS(runkit_method_add,			runkit7_method_add)
	PHP_FE_AND_FALIAS(runkit_method_redefine,		runkit7_method_redefine)
	PHP_FE_AND_FALIAS(runkit_method_remove,			runkit7_method_remove)
	PHP_FE_AND_FALIAS(runkit_method_rename,			runkit7_method_rename)
	PHP_FE_AND_FALIAS(runkit_method_copy,			runkit7_method_copy)

	PHP_FE_AND_FALIAS(runkit_constant_redefine,		runkit7_constant_redefine)
	PHP_FE_AND_FALIAS(runkit_constant_remove,		runkit7_constant_remove)
	PHP_FE_AND_FALIAS(runkit_constant_add,			runkit7_constant_add)

#endif /* PHP_RUNKIT_MANIPULATION */

	{NULL, NULL, NULL}
};
/* }}} */

/* {{{ runkit7_module_entry
 */
zend_module_entry runkit7_module_entry = {
	STANDARD_MODULE_HEADER,
	"runkit7",
	runkit7_functions,
	PHP_MINIT(runkit7),
	PHP_MSHUTDOWN(runkit7),
	PHP_RINIT(runkit7),
	PHP_RSHUTDOWN(runkit7),
	PHP_MINFO(runkit7),
	PHP_RUNKIT7_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#if defined(PHP_RUNKIT_SUPERGLOBALS) || defined(PHP_RUNKIT_MANIPULATION)
PHP_INI_BEGIN()
#ifdef PHP_RUNKIT_SUPERGLOBALS
	PHP_INI_ENTRY("runkit.superglobal", "", PHP_INI_SYSTEM | PHP_INI_PERDIR, NULL)
#endif
#ifdef PHP_RUNKIT_MANIPULATION
	STD_PHP_INI_BOOLEAN("runkit.internal_override", "0", PHP_INI_SYSTEM, OnUpdateBool, internal_override, zend_runkit7_globals, runkit7_globals)
#endif
PHP_INI_END()
#endif

#ifdef COMPILE_DL_RUNKIT7
ZEND_GET_MODULE(runkit7)
#endif

#ifdef PHP_RUNKIT_MANIPULATION
ZEND_FUNCTION(_php_runkit_removed_function)
{
	php_error_docref(NULL, E_ERROR, "A function removed by runkit7 was somehow invoked");
}
ZEND_FUNCTION(_php_runkit_removed_method)
{
	php_error_docref(NULL, E_ERROR, "A method removed by runkit7 was somehow invoked");
}

static inline void _php_runkit_init_stub_function(const char *name, ZEND_NAMED_FUNCTION(handler), zend_function **result)
{
	zend_function *fn = pemalloc(sizeof(zend_function), 1);
	*result = fn;
	/* Ensure memory is initialized for fields that aren't listed here in stub functions. */
	memset(fn, 0, sizeof(zend_function));
	fn->common.function_name = zend_string_init(name, strlen(name), 1);  // TODO: Can this be persistent?
	fn->common.scope = NULL;
	fn->common.arg_info = NULL;
	fn->common.num_args = 0;
	fn->common.type = ZEND_INTERNAL_FUNCTION;
	fn->common.fn_flags = ZEND_ACC_PUBLIC | ZEND_ACC_STATIC;
	fn->common.arg_info = NULL;
	/* fn->common.T was added in 8.2.0beta2 */
	/* fn->common.T = 0; */
	fn->internal_function.handler = handler;
	fn->internal_function.module = &runkit7_module_entry;
}
#endif

#if defined(PHP_RUNKIT_MANIPULATION)
static void php_runkit7_globals_ctor(void *pDest)
{
	zend_runkit7_globals *runkit_global = (zend_runkit7_globals *)pDest;
#ifdef PHP_RUNKIT_MANIPULATION
	runkit_global->replaced_internal_functions = NULL;
	runkit_global->misplaced_internal_functions = NULL;
	// runkit_global->removed_default_class_members = NULL;
	runkit_global->name_str = "name";
	runkit_global->removed_method_str = "__method_removed_by_runkit__";
	runkit_global->removed_function_str = "__function_removed_by_runkit__";
	runkit_global->removed_parameter_str = "__parameter_removed_by_runkit__";
#if defined(PHP_RUNKIT_MANIPULATION)
#if PHP_VERSION_ID >= 80000
	runkit_global->original_func_resource_handle = zend_get_resource_handle("runkit7");
#else
	{
		zend_extension placeholder = {0};
		runkit_global->original_func_resource_handle = zend_get_resource_handle(&placeholder);
	}
#endif
#endif

	_php_runkit_init_stub_function("__function_removed_by_runkit__", ZEND_FN(_php_runkit_removed_function), &runkit_global->removed_function);
	_php_runkit_init_stub_function("__method_removed_by_runkit__", ZEND_FN(_php_runkit_removed_method), &runkit_global->removed_method);
#endif
}
#endif

#define REGISTER_RUNKIT7_LONG_CONSTANT(runkit_const_name, runkit7_const_name, value) \
	REGISTER_LONG_CONSTANT(runkit_const_name,  value, CONST_CS | CONST_PERSISTENT); \
	REGISTER_LONG_CONSTANT(runkit7_const_name, value, CONST_CS | CONST_PERSISTENT);

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(runkit7)
{
#ifdef ZTS
#if defined(PHP_RUNKIT_MANIPULATION)
	ts_allocate_id(&runkit7_globals_id, sizeof(zend_runkit7_globals), php_runkit7_globals_ctor, NULL);
#endif
#else
#if defined(PHP_RUNKIT_MANIPULATION)
	php_runkit7_globals_ctor(&runkit7_globals);
#endif
#endif

#if defined(PHP_RUNKIT_SUPERGLOBALS) || defined(PHP_RUNKIT_MANIPULATION)
	REGISTER_INI_ENTRIES();
#endif

	REGISTER_RUNKIT7_LONG_CONSTANT("RUNKIT_ACC_RETURN_REFERENCE", "RUNKIT7_ACC_RETURN_REFERENCE", PHP_RUNKIT_ACC_RETURN_REFERENCE);

	REGISTER_RUNKIT7_LONG_CONSTANT("RUNKIT_ACC_PUBLIC", "RUNKIT7_ACC_PUBLIC", ZEND_ACC_PUBLIC);
	REGISTER_RUNKIT7_LONG_CONSTANT("RUNKIT_ACC_PROTECTED", "RUNKIT7_ACC_PROTECTED", ZEND_ACC_PROTECTED);
	REGISTER_RUNKIT7_LONG_CONSTANT("RUNKIT_ACC_PRIVATE", "RUNKIT7_ACC_PRIVATE", ZEND_ACC_PRIVATE);
	REGISTER_RUNKIT7_LONG_CONSTANT("RUNKIT_ACC_STATIC", "RUNKIT7_ACC_STATIC", ZEND_ACC_STATIC);
	REGISTER_RUNKIT7_LONG_CONSTANT("RUNKIT_OVERRIDE_OBJECTS", "RUNKIT7_OVERRIDE_OBJECTS", PHP_RUNKIT_OVERRIDE_OBJECTS);

	/* Feature Identifying constants */
#ifdef PHP_RUNKIT_MANIPULATION
	REGISTER_RUNKIT7_LONG_CONSTANT("RUNKIT_FEATURE_MANIPULATION", "RUNKIT7_FEATURE_MANIPULATION", 1);
#else
	REGISTER_RUNKIT7_LONG_CONSTANT("RUNKIT_FEATURE_MANIPULATION", "RUNKIT7_FEATURE_MANIPULATION", 0);
#endif
#ifdef PHP_RUNKIT_SUPERGLOBALS
	REGISTER_RUNKIT7_LONG_CONSTANT("RUNKIT_FEATURE_SUPERGLOBALS", "RUNKIT7_FEATURE_SUPERGLOBALS", 1);
#else
	REGISTER_RUNKIT7_LONG_CONSTANT("RUNKIT_FEATURE_SUPERGLOBALS", "RUNKIT7_FEATURE_SUPERGLOBALS", 0);
#endif

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(runkit7)
{
#ifdef PHP_RUNKIT_MANIPULATION
	pefree(RUNKIT_G(removed_function), 1);
	pefree(RUNKIT_G(removed_method), 1);
#endif
#if defined(PHP_RUNKIT_SUPERGLOBALS) || defined(PHP_RUNKIT_MANIPULATION)
	UNREGISTER_INI_ENTRIES();
#endif

	return SUCCESS;
}
/* }}} */

#ifdef PHP_RUNKIT_SUPERGLOBALS
/* {{{ php_runkit_register_auto_global
	Register an autoglobal only if it's not already registered */
static void php_runkit_register_auto_global(char *s, int len)
{
	zend_auto_global *auto_global;
	zend_string *globalName = zend_string_init(s, len, 1);
	zval z;

	if (zend_hash_exists(CG(auto_globals), globalName)) {
		/* Registered already */
		zend_string_release(globalName);
		return;
	}

	if (zend_register_auto_global(globalName,
			0,
		    NULL) == SUCCESS) {
		// FIXME: This is broken.

		// TODO: How do I get an auto global out of a zend_hash????
		if ((auto_global = zend_hash_find_ptr(CG(auto_globals), globalName)) == NULL) {
			php_error_docref(NULL, E_ERROR, "Cannot locate the newly created autoglobal");
			return;
		}
		auto_global->armed = 0;

		if (!RUNKIT_G(superglobals)) {
			ALLOC_HASHTABLE(RUNKIT_G(superglobals));
			zend_hash_init(RUNKIT_G(superglobals), 2, NULL, NULL, 0);
		}
		// Insert a different but equivalent zend_string (superglobal name)
		// into the list for function runkit_superglobals(),
		// to make reasoning about reference tracking easier.
		ZVAL_NEW_STR(&z, zend_string_init(s, len, 1));
		zend_hash_next_index_insert(RUNKIT_G(superglobals), &z);
	}
}
/* }}} */

/* {{{ php_runkit_rinit_register_superglobals */
static void php_runkit_rinit_register_superglobals(const char *ini_s)
{
	char *s;
	char *p;
	char *dup;
	int len;

	RUNKIT_G(superglobals) = NULL;
	if (!ini_s || !strlen(ini_s)) {
		return;
	}

	s = dup = estrdup(ini_s);
	p = strchr(s, ',');
	while (p) {
		if (p - s) {
			*p = '\0';
			php_runkit_register_auto_global(s, p - s);
		}
		s = ++p;
		p = strchr(p, ',');
	}

	len = strlen(s);
	if (len) {
		php_runkit_register_auto_global(s, len);
	}
	efree(dup);
}
/* }}} */
#endif /* PHP_RUNKIT_SUPERGLOBALS */

/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(runkit7)
{
#ifdef PHP_RUNKIT_SUPERGLOBALS
	php_runkit_rinit_register_superglobals(INI_STR("runkit.superglobal"));
#endif /* PHP_RUNKIT_SUPERGLOBALS */

#ifdef PHP_RUNKIT_MANIPULATION
	RUNKIT_G(replaced_internal_functions) = NULL;
	RUNKIT_G(misplaced_internal_functions) = NULL;
	RUNKIT_G(module_moved_to_front) = 0;
#if PHP_VERSION_ID >= 80200
	/* ZEND_INIT_FCALL now asserts that the function exists in the symbol table at runtime. */
	CG(compiler_options) |= ZEND_COMPILE_IGNORE_USER_FUNCTIONS | ZEND_COMPILE_IGNORE_INTERNAL_FUNCTIONS;
#endif
	// RUNKIT_G(removed_default_class_members) = NULL;
#endif

	return SUCCESS;
}
/* }}} */

#ifdef PHP_RUNKIT_SUPERGLOBALS
/* {{{ php_runkit_superglobal_dtor */
static int php_runkit_superglobal_dtor(zval *zv)
{
	ZEND_ASSERT(Z_TYPE_P(zv) == IS_STRING);
	zend_hash_del(CG(auto_globals), Z_STR_P(zv));

	return ZEND_HASH_APPLY_REMOVE;
}
/* }}} */
#endif /* PHP_RUNKIT_SUPERGLOBALS */

/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(runkit7)
{
	// #ifdef PHP_RUNKIT_MANIPULATION
	// php_runkit_default_class_members_list_element *el;
	// #endif

#ifdef PHP_RUNKIT_SUPERGLOBALS
	if (RUNKIT_G(superglobals)) {
		zend_hash_apply(RUNKIT_G(superglobals), php_runkit_superglobal_dtor);

		zend_hash_destroy(RUNKIT_G(superglobals));
		FREE_HASHTABLE(RUNKIT_G(superglobals));
	}
#endif /* PHP_RUNKIT_SUPERGLOBALS */

#ifdef PHP_RUNKIT_MANIPULATION
	if (RUNKIT_G(misplaced_internal_functions)) {
		/* Just wipe out rename-to targets before restoring originals */
		zend_hash_apply(RUNKIT_G(misplaced_internal_functions), php_runkit_destroy_misplaced_functions);
		zend_hash_destroy(RUNKIT_G(misplaced_internal_functions));
		FREE_HASHTABLE(RUNKIT_G(misplaced_internal_functions));
		RUNKIT_G(misplaced_internal_functions) = NULL;
	}

	if (RUNKIT_G(replaced_internal_functions) && strcmp(sapi_module.name, "fpm-fcgi") == 0) {
		/* Restore internal functions */
		// Note: It only matters if we restore internal functions in certain module types, such as "fpm-fcgi".
		// "cli" doesn't matter. Not sure about other module types yet.

		// TODO: The pointer to `f` is correct, but the data inside of `f` is corrupted at the time request shutdown is reached
		zend_function *f;
		zend_string *key;
		// php_error_docref(NULL, E_WARNING, "In RSHUTDOWN: restoring replaced internal functions: count=%d sapi_module=%s", (int) zend_array_count(RUNKIT_G(replaced_internal_functions)), sapi_module.name);
		ZEND_HASH_FOREACH_STR_KEY_PTR(RUNKIT_G(replaced_internal_functions), key, f) {
			if (key != NULL) {
				// php_error_docref(NULL, E_WARNING, "In RSHUTDOWN: restoring '%s' addr=%llx", ZSTR_VAL(key), (long long)(uintptr_t)f);
				// NOTE: On modules shutdown, modules will call zend_function_dtor on the modules they declared... so it's best if this is a clone of the internal function?
				ZEND_ASSERT(f->type == ZEND_INTERNAL_FUNCTION || f->type == ZEND_USER_FUNCTION);
				php_runkit_restore_internal_function(key, f);
			}
		} ZEND_HASH_FOREACH_END();
		zend_hash_destroy(RUNKIT_G(replaced_internal_functions));
		FREE_HASHTABLE(RUNKIT_G(replaced_internal_functions));
		RUNKIT_G(replaced_internal_functions) = NULL;
	}

	// This would restore default properties that were removed by runkit_default_property_remove(...)

	/*
	el = RUNKIT_G(removed_default_class_members);
	while (el) {
		php_runkit_default_class_members_list_element *tmp;
		// TODO: Some sort of cleanup?
		zval **table = el->is_static ? el->ce->default_static_members_table : el->ce->default_properties_table;
		zval **table_el = &table[el->offset];
		if ( *table_el == NULL ) {
			ALLOC_ZVAL(*table_el);
			Z_TYPE_PP(table_el) = IS_NULL;
			Z_SET_REFCOUNT_PP(table_el, 1);
		}
		tmp = el;
		el = el->next;
		efree(tmp);
	}
	*/
#endif /* PHP_RUNKIT_MANIPULATION */

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(runkit7)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "runkit7 support", "enabled");
	php_info_print_table_header(2, "version", PHP_RUNKIT7_VERSION);

#ifdef PHP_RUNKIT_SUPERGLOBALS
	php_info_print_table_header(2, "Custom Superglobal support", "enabled");
#else
	php_info_print_table_header(2, "Custom Superglobal support", "disabled or unavailable");
#endif /* PHP_RUNKIT_SUPERGLOBALS */

#ifdef PHP_RUNKIT_MANIPULATION
	php_info_print_table_header(2, "Runtime Manipulation", "enabled");
#else
	php_info_print_table_header(2, "Runtime Manipulation", "disabled or unavailable");
#endif /* PHP_RUNKIT_MANIPULATION */

	php_info_print_table_end();

#if defined(PHP_RUNKIT_SUPERGLOBALS) || defined(PHP_RUNKIT_MANIPULATION)
	DISPLAY_INI_ENTRIES();
#endif
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
