dnl $Id: config.m4,v 1.7 2002-09-04 18:56:49 derick Exp $
dnl config.m4 for extension xdebug

PHP_ARG_ENABLE(xdebug, whether to enable eXtended debugging support,
[  --enable-xdebug         Enable xdebug support])

if test "$PHP_XDEBUG" != "no"; then
  if test "$PHP_SAPI" != ""; then
	echo ""
    echo "You can not compile xdebug into PHP. Please read the instructions"
    echo "on the website (http://xdebug.derickrethans.nl/#source)."
	echo ""
    AC_MSG_ERROR(Can not compile in xdebug.)
  fi
dnl PHP < 4.3 config
  PHP_EXTENSION(xdebug, $ext_shared)
dnl PHP >= 4.3 config
dnl  PHP_NEW_EXTENSION(xdebug, xdebug.c xdebug_com.c xdebug_handler_php3.c xdebug_handlers.c xdebug_llist.c xdebug_var.c, $ext_shared)
  AC_DEFINE(HAVE_XDEBUG,1,[ ])

dnl Check for new current_execute_data field in zend_executor_globals
  old_CPPFLAGS=$CPPFLAGS
  CPPFLAGS="$INCLUDES $CPPFLAGS"

  AC_TRY_COMPILE([
#include <zend_compile.h>
#include <zend_globals.h>
  ], [static struct _zend_executor_globals zeg; zend_execute_data *zed = zeg.current_execute_data],
    [AC_DEFINE(HAVE_EXECUTE_DATA_PTR, 1, [ ])]
  )
  AC_CHECK_FUNCS(gettimeofday)

  CPPFLAGS=$old_CPPFLAGS
fi
