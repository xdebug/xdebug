dnl $Id: config.m4,v 1.6 2002-08-31 08:33:26 derick Exp $
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
dnl   PHP_NEW_EXTENSION(xdebug, xdebug.c xdebug_llist.c xdebug_var.c, $ext_shared)
  AC_DEFINE(HAVE_XDEBUG,1,[ ])

dnl Check for new execute_data_ptr field in zend_executor_globals
  old_CPPFLAGS=$CPPFLAGS
  CPPFLAGS="$INCLUDES $CPPFLAGS"

  AC_TRY_COMPILE([
#include <zend_compile.h>
#include <zend_globals.h>
  ], [static struct _zend_executor_globals zeg; zend_execute_data *zed = zeg.execute_data_ptr],
    [AC_DEFINE(HAVE_EXECUTE_DATA_PTR, 1, [ ])]
  )

  CPPFLAGS=$old_CPPFLAGS
fi
