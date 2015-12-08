dnl $Id: config.m4,v 1.28 2008-07-31 00:43:35 sniper Exp $
dnl config.m4 for extension xdebug

PHP_ARG_ENABLE(xdebug, whether to enable Xdebug support,
[  --enable-xdebug         Enable Xdebug support])

if test "$PHP_XDEBUG" != "no"; then
dnl disabling O2 for GCC 4.8
  AC_MSG_CHECKING([compiler])
  AC_MSG_RESULT([$CC])
  AS_IF([test "x$GCC" = "xyes"],[
    AS_IF([test "x$CC" = "xcc" || test "x$CC" = "xgcc"],[
      AC_CACHE_CHECK([$CC version],[ax_cv_xdebug_gcc_version],[
        ax_cv_xdebug_gcc_version="`$CC -dumpversion`"
        AS_IF([test "x$ax_cv_xdebug_gcc_version" = "x"],[
          ax_cv_xdebug_gcc_version=""
        ])
      ])
      XDEBUG_GCC_VERSION=$ax_cv_xdebug_gcc_version
      XDEBUG_GCC_VERSION_MAJOR=$(echo $XDEBUG_GCC_VERSION | cut -d'.' -f1)
      XDEBUG_GCC_VERSION_MINOR=$(echo $XDEBUG_GCC_VERSION | cut -d'.' -f2)

      AS_IF([test "x$XDEBUG_GCC_VERSION_MAJOR" = "x4"],[
        AS_IF([test "x$XDEBUG_GCC_VERSION_MINOR" = "x8"],[
          AC_MSG_RESULT([disabling optimisation because of GCC 4.8])
          CFLAGS=`echo $CFLAGS | sed 's/O2/O0/'`
        ])
      ])
    ])
  ])

  AC_MSG_CHECKING([Check for supported PHP versions])
  PHP_XDEBUG_FOUND_VERSION=`${PHP_CONFIG} --version`
  PHP_XDEBUG_FOUND_VERNUM=`echo "${PHP_XDEBUG_FOUND_VERSION}" | $AWK 'BEGIN { FS = "."; } { printf "%d", ([$]1 * 100 + [$]2) * 100 + [$]3;}'`
  if test "$PHP_XDEBUG_FOUND_VERNUM" -lt "50400"; then
    AC_MSG_ERROR([not supported. Need a PHP version >= 5.4.0 (found $PHP_XDEBUG_FOUND_VERSION)])
  else
    AC_MSG_RESULT([supported ($PHP_XDEBUG_FOUND_VERSION)])
  fi
  
  AC_DEFINE(HAVE_XDEBUG,1,[ ])

  old_CPPFLAGS=$CPPFLAGS
  CPPFLAGS="$INCLUDES $CPPFLAGS"

  AC_CHECK_FUNCS(gettimeofday)

  PHP_CHECK_LIBRARY(m, cos, [ PHP_ADD_LIBRARY(m,, XDEBUG_SHARED_LIBADD) ])

  CPPFLAGS=$old_CPPFLAGS

  PHP_NEW_EXTENSION(xdebug, xdebug.c xdebug_branch_info.c xdebug_code_coverage.c xdebug_com.c xdebug_compat.c xdebug_handler_dbgp.c xdebug_handlers.c xdebug_llist.c xdebug_monitor.c xdebug_hash.c xdebug_private.c xdebug_profiler.c xdebug_set.c xdebug_stack.c xdebug_str.c xdebug_superglobals.c xdebug_tracing.c xdebug_trace_textual.c xdebug_trace_computerized.c xdebug_trace_html.c xdebug_var.c xdebug_xml.c usefulstuff.c, $ext_shared,,,,yes)
  PHP_SUBST(XDEBUG_SHARED_LIBADD)
  PHP_ADD_MAKEFILE_FRAGMENT
fi
