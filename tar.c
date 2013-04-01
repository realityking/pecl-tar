/*
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.0 of the PHP license,       |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_0.txt.                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Rouven Weßling <me@rouvenwessling.de>                      |
   +----------------------------------------------------------------------+
*/

#include "php_tar.h"

#include <fcntl.h>

#if HAVE_TAR

/* {{{ Class definitions */

/* {{{ Class TarArchive */

static zend_class_entry * TarArchive_ce_ptr = NULL;

zend_object_handlers tar_object_handlers;

void tar_free_storage(void *object TSRMLS_DC)
{
	tar_object *obj = (tar_object *)object;
	if (obj->tar) {
		tar_close(obj->tar);
	}

	zend_hash_destroy(obj->std.properties);
	FREE_HASHTABLE(obj->std.properties);

	efree(obj);
}

zend_object_value tar_create_handler(zend_class_entry *type TSRMLS_DC)
{
    zval *tmp;
    zend_object_value retval;

    tar_object *obj = (tar_object *)emalloc(sizeof(tar_object));
    memset(obj, 0, sizeof(tar_object));
    obj->std.ce = type;
    obj->tar = NULL;
    obj->filename = NULL;

    ALLOC_HASHTABLE(obj->std.properties);
    zend_hash_init(obj->std.properties, 0, NULL, ZVAL_PTR_DTOR, 0);
    zend_hash_copy(obj->std.properties, &type->default_properties,
        (copy_ctor_func_t)zval_add_ref, (void *)&tmp, sizeof(zval *));

    retval.handle = zend_objects_store_put(obj, NULL,
        tar_free_storage, NULL TSRMLS_CC);
    retval.handlers = &tar_object_handlers;

    return retval;
}

/* {{{ TAR_FROM_OBJECT */
#define TAR_FROM_OBJECT(intern, object) \
	{ \
		tar_object *obj = (tar_object*) zend_object_store_get_object(object TSRMLS_CC); \
		intern = obj->tar; \
		if (!intern) { \
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid or unitialized Tar object"); \
			RETURN_FALSE; \
		} \
	}
/* }}} */

/* {{{ Methods */

/* {{{ proto mixed open(string filename)
   */
PHP_METHOD(TarArchive, open)
{
	zval * _this_zval = NULL;
	const char * filename = NULL;
	int filename_len = 0;
	char resolved_path[MAXPATHLEN];
	tar_object *ze_obj = NULL;
	int err = 0;
	TAR *intern;
	zval *this = getThis();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &filename, &filename_len) == FAILURE) {
		return;
	}

	if (this) {
		/* We do not use TAR_FROM_OBJECT here */
		ze_obj = (tar_object*) zend_object_store_get_object(this TSRMLS_CC);
	}

	if (filename_len == 0) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Empty string as source");
		RETURN_FALSE;
	}

	if (TAR_OPENBASEDIR_CHECKPATH(filename)) {
		RETURN_FALSE;
	}

	if (!expand_filepath(filename, resolved_path TSRMLS_CC)) {
		RETURN_FALSE;
	}

	if (ze_obj->tar) {
		tar_close(ze_obj->tar);
		ze_obj->tar = NULL;
	}
	if (ze_obj->filename) {
		efree(ze_obj->filename);
		ze_obj->filename = NULL;
	}

	err = tar_open(&intern, resolved_path, NULL, O_RDONLY, 0, 0);
	if (!intern || err != 0) {
		RETURN_LONG((long)err);
	}

	ze_obj->filename = estrdup(resolved_path);
	ze_obj->filename_len = filename_len;
	ze_obj->tar = intern;

	RETURN_TRUE;
}
/* }}} open */



/* {{{ proto bool extractTo(string destination)
   */
PHP_METHOD(TarArchive, extractTo)
{
	zval *this = getThis();
	tar_object *ze_obj = NULL;
	TAR *intern;
	char * destination = NULL;
	int destination_len = 0;
	php_stream_statbuf ssb;
	int ret;

	if (!this) {
		RETURN_FALSE;
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &destination, &destination_len) == FAILURE) {
		return;
	}

	if (destination_len < 1) {
		RETURN_FALSE;
	}

	if (php_stream_stat_path_ex(destination, PHP_STREAM_URL_STAT_QUIET, &ssb, NULL) < 0) {
		ret = php_stream_mkdir(destination, 0777,  PHP_STREAM_MKDIR_RECURSIVE, NULL);
		if (!ret) {
			RETURN_FALSE;
		}
	}

	TAR_FROM_OBJECT(intern, this);

	if (tar_extract_all(intern, destination) != 0) {
		RETURN_FALSE
	}

	RETURN_TRUE;
}
/* }}} extractTo */

/* {{{ proto void statIndex(int index)
   */
PHP_METHOD(TarArchive, statIndex)
{
	TAR *intern;
	zval *this = getThis();
	long index = 0;
	long i = 0;

	if (!this) {
		RETURN_FALSE;
	}

	TAR_FROM_OBJECT(intern, this);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &index) == FAILURE) {
		return;
	}

	while (i <= index)
	{
		if (th_read(intern) != 0) {
			break;
		}

		if (TH_ISREG(intern) && tar_skip_regfile(intern) != 0)
		{
			RETURN_LONG((long)strerror(errno));
		}

		i++;
	}

	array_init(return_value);
	add_assoc_string(return_value, "name", th_get_pathname(intern), 1);
	add_assoc_long(return_value, "crc", (long)th_get_crc(intern));
	add_assoc_long(return_value, "size", (long)th_get_size(intern));
	add_assoc_long(return_value, "mtime", (long)th_get_mtime(intern));
	add_assoc_long(return_value, "uid", (long)th_get_uid(intern));
	add_assoc_long(return_value, "gid", (long)th_get_gid(intern));
}
/* }}} statIndex */


/* {{{ proto void close(void)
   */
PHP_METHOD(TarArchive, close)
{
	TAR *intern;
	zval *this = getThis();
	tar_object *ze_obj;
	int err = 0;

	zval * _this_zval = NULL;

	if (!this) {
		RETURN_FALSE;
	}

	TAR_FROM_OBJECT(intern, this);

	ze_obj = (tar_object*) zend_object_store_get_object(this TSRMLS_CC);

	err = tar_close(intern);
	if (err != 0) {
		RETURN_FALSE;
	}

	efree(ze_obj->filename);
	ze_obj->filename = NULL;
	ze_obj->filename_len = 0;
	ze_obj->tar = NULL;

	RETURN_TRUE;
}
/* }}} close */


static zend_function_entry TarArchive_methods[] = {
	PHP_ME(TarArchive, open, TarArchive__open_args, /**/ZEND_ACC_PUBLIC)
	PHP_ME(TarArchive, extractTo, TarArchive__extractTo_args, /**/ZEND_ACC_PUBLIC)
	PHP_ME(TarArchive, statIndex, TarArchive__statIndex_args, /**/ZEND_ACC_PUBLIC)
	PHP_ME(TarArchive, close, TarArchive__close_args, /**/ZEND_ACC_PUBLIC)
	{ NULL, NULL, NULL }
};

/* }}} Class TarArchive */

/* }}} Class definitions*/

/* {{{ tar_functions[] */
function_entry tar_functions[] = {
	{ NULL, NULL, NULL }
};
/* }}} */


/* {{{ tar_module_entry
 */
zend_module_entry tar_module_entry = {
	STANDARD_MODULE_HEADER,
	"tar",
	tar_functions,
	PHP_MINIT(tar),     /* PHP_MINIT */
	PHP_MSHUTDOWN(tar), /* Replace with NULL if there is nothing to do at php shutdown  */
	NULL,               /* PHP_RINIT */
	NULL,               /* PHP_RSHUTDOWN */
	PHP_MINFO(tar),
	PHP_TAR_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_TAR
ZEND_GET_MODULE(tar)
#endif


/* {{{ PHP_MINIT_FUNCTION */
PHP_MINIT_FUNCTION(tar)
{
	zend_class_entry ce;

	INIT_CLASS_ENTRY(ce, "TarArchive", TarArchive_methods);
	TarArchive_ce_ptr = zend_register_internal_class(&ce TSRMLS_CC);

	TarArchive_ce_ptr->create_object = tar_create_handler;
	memcpy(&tar_object_handlers,
		zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	tar_object_handlers.clone_obj = NULL;

	return SUCCESS;
}
/* }}} */


/* {{{ PHP_MSHUTDOWN_FUNCTION */
PHP_MSHUTDOWN_FUNCTION(tar)
{

	/* add your stuff here */

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION */
PHP_MINFO_FUNCTION(tar)
{
	php_info_print_table_start();
	php_info_print_table_row(2, "Tar", "enabled");
	php_info_print_table_row(2, "Version", PHP_TAR_VERSION " (devel)");
	php_info_print_table_row(2, "Released", "2013-03-30");
	php_info_print_table_row(2, "Authors", "Rouven Weßling 'me@rouvenwessling.de' (lead)\n");
	php_info_print_table_row(2, "Libzip version", "1.2.11");
	php_info_print_table_end();
	/* add your stuff here */

}
/* }}} */

#endif /* HAVE_TAR */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */

