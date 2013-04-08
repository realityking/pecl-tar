dnl
dnl

AC_FUNC_FNMATCH
AC_CHECK_HEADERS(fnmatch.h libgen.h)

PHP_ARG_ENABLE(tar, whether to enable tar functions,
[  --enable-tar         Enable tar support])

if test "$PHP_TAR" != "no"; then
  export OLD_CPPFLAGS="$CPPFLAGS"
  export CPPFLAGS="$CPPFLAGS $INCLUDES -DHAVE_TAR"

  AC_MSG_CHECKING(PHP version)
  AC_TRY_COMPILE([#include <php_version.h>], [
#if PHP_VERSION_ID < 50100
#error  this extension requires at least PHP version 5.1.0
#endif
],
[AC_MSG_RESULT(ok)],
[AC_MSG_ERROR([need at least PHP 5.1.0])])

  export CPPFLAGS="$OLD_CPPFLAGS"


  PHP_SUBST(TAR_SHARED_LIBADD)
  AC_DEFINE(HAVE_TAR, 1, [ ])

  PHP_TAR_SOURCES="$PHP_TAR_SOURCES lib/handle.c lib/libtar_hash.c
  						lib/libtar_list.c lib/util.c lib/extract.c
  						lib/block.c lib/wrapper.c lib/decode.c
  						lib/append.c"

  PHP_NEW_EXTENSION(tar, tar.c $PHP_TAR_SOURCES, $ext_shared)

fi

