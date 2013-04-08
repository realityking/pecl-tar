/*
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,       |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt.                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Rouven Weßling <me@rouvenwessling.de>                      |
   +----------------------------------------------------------------------+
*/

#ifndef PHP_TAR_H
#define PHP_TAR_H

#ifdef  __cplusplus
extern "C" {
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <php.h>

#ifdef HAVE_TAR
#define PHP_TAR_VERSION "0.0.1dev"


#include <ext/standard/info.h>
#include <Zend/zend_extensions.h>
#include "lib/libtar.h"

#ifdef  __cplusplus
} // extern "C"
#endif
#ifdef  __cplusplus
extern "C" {
#endif

/* {{{ ZIP_OPENBASEDIR_CHECKPATH(filename) */
#if PHP_API_VERSION < 20100412
# define TAR_OPENBASEDIR_CHECKPATH(filename) \
	(PG(safe_mode) && (!php_checkuid(filename, NULL, CHECKUID_CHECK_FILE_AND_DIR))) || php_check_open_basedir(filename TSRMLS_CC)
#else
#define TAR_OPENBASEDIR_CHECKPATH(filename) \
	php_check_open_basedir(filename TSRMLS_CC)
#endif
/* }}} */

extern zend_module_entry tar_module_entry;
#define phpext_tar_ptr &tar_module_entry

#ifdef PHP_WIN32
#define PHP_TAR_API __declspec(dllexport)
#else
#define PHP_TAR_API
#endif

PHP_MINIT_FUNCTION(tar);
PHP_MSHUTDOWN_FUNCTION(tar);
PHP_RINIT_FUNCTION(tar);
PHP_RSHUTDOWN_FUNCTION(tar);
PHP_MINFO_FUNCTION(tar);

#ifdef ZTS
#include "TSRM.h"
#endif

#define FREE_RESOURCE(resource) zend_list_delete(Z_LVAL_P(resource))

#define PROP_GET_LONG(name)    Z_LVAL_P(zend_read_property(_this_ce, _this_zval, #name, strlen(#name), 1 TSRMLS_CC))
#define PROP_SET_LONG(name, l) zend_update_property_long(_this_ce, _this_zval, #name, strlen(#name), l TSRMLS_CC)

#define PROP_GET_DOUBLE(name)    Z_DVAL_P(zend_read_property(_this_ce, _this_zval, #name, strlen(#name), 1 TSRMLS_CC))
#define PROP_SET_DOUBLE(name, d) zend_update_property_double(_this_ce, _this_zval, #name, strlen(#name), d TSRMLS_CC)

#define PROP_GET_STRING(name)    Z_STRVAL_P(zend_read_property(_this_ce, _this_zval, #name, strlen(#name), 1 TSRMLS_CC))
#define PROP_GET_STRLEN(name)    Z_STRLEN_P(zend_read_property(_this_ce, _this_zval, #name, strlen(#name), 1 TSRMLS_CC))
#define PROP_SET_STRING(name, s) zend_update_property_string(_this_ce, _this_zval, #name, strlen(#name), s TSRMLS_CC)
#define PROP_SET_STRINGL(name, s, l) zend_update_property_stringl(_this_ce, _this_zval, #name, strlen(#name), s, l TSRMLS_CC)

/* Extends zend object */
typedef struct _tar_object {
	zend_object std;
	TAR *tar;
	char *filename;
	int filename_len;
} tar_object;

PHP_METHOD(TarArchive, open);
ZEND_BEGIN_ARG_INFO_EX(TarArchive__open_args, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
  ZEND_ARG_INFO(0, filename)
ZEND_END_ARG_INFO()

PHP_METHOD(TarArchive, extractTo);
ZEND_BEGIN_ARG_INFO_EX(TarArchive__extractTo_args, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
  ZEND_ARG_INFO(0, destination)
ZEND_END_ARG_INFO()

PHP_METHOD(TarArchive, statIndex);
ZEND_BEGIN_ARG_INFO_EX(TarArchive__statIndex_args, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
  ZEND_ARG_INFO(0, index)
ZEND_END_ARG_INFO()

PHP_METHOD(TarArchive, close);
ZEND_BEGIN_ARG_INFO_EX(TarArchive__close_args, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
  ZEND_ARG_INFO(0, )
ZEND_END_ARG_INFO()

#ifdef  __cplusplus
} // extern "C"
#endif

#endif /* PHP_HAVE_TAR */

#endif /* PHP_TAR_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */

