dnl config.m4 for extension Xdebug

PHP_ARG_ENABLE(xdebug, whether to enable Xdebug support,
[  --enable-xdebug         Enable Xdebug support])

PHP_ARG_ENABLE(xdebug-dev, whether to enable Xdebug developer build flags,
[  --enable-xdebug-dev       Xdebug: Enable developer flags],, no)


if test "$PHP_XDEBUG" != "no"; then
  AC_MSG_CHECKING([Check for supported PHP versions])
  PHP_XDEBUG_FOUND_VERSION=`${PHP_CONFIG} --version`
  PHP_XDEBUG_FOUND_VERNUM=`echo "${PHP_XDEBUG_FOUND_VERSION}" | $AWK 'BEGIN { FS = "."; } { printf "%d", ([$]1 * 100 + [$]2) * 100 + [$]3;}'`
  if test "$PHP_XDEBUG_FOUND_VERNUM" -lt "70100"; then
    AC_MSG_ERROR([not supported. Need a PHP version >= 7.1.0 and < 8.0.0 (found $PHP_XDEBUG_FOUND_VERSION)])
  else
    if test "$PHP_XDEBUG_FOUND_VERNUM" -ge "80000"; then
      AC_MSG_ERROR([not supported. Need a PHP version >= 7.1.0 and < 8.0.0 (found $PHP_XDEBUG_FOUND_VERSION)])
    else
      AC_MSG_RESULT([supported ($PHP_XDEBUG_FOUND_VERSION)])
    fi
  fi
  
  AC_DEFINE(HAVE_XDEBUG,1,[ ])

  old_CPPFLAGS=$CPPFLAGS
  CPPFLAGS="$INCLUDES $CPPFLAGS"

  AC_CHECK_FUNCS(gettimeofday)
  AC_CHECK_HEADERS([netinet/in.h poll.h sys/poll.h])

  PHP_CHECK_LIBRARY(m, cos, [ PHP_ADD_LIBRARY(m,, XDEBUG_SHARED_LIBADD) ])

  CPPFLAGS=$old_CPPFLAGS

  if test "$PHP_XDEBUG_DEV" = "yes"; then
    PHP_CHECK_GCC_ARG(-Wbool-conversion,                _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wbool-conversion")
    PHP_CHECK_GCC_ARG(-Wdeclaration-after-statement,    _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wdeclaration-after-statement")
    PHP_CHECK_GCC_ARG(-Wdiscarded-qualifiers,           _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wdiscarded-qualifiers")
    PHP_CHECK_GCC_ARG(-Wduplicate-enum,                 _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wduplicate-enum")
    PHP_CHECK_GCC_ARG(-Wempty-body,                     _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wempty-body")
    PHP_CHECK_GCC_ARG(-Wenum-compare,                   _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wenum-compare")
    PHP_CHECK_GCC_ARG(-Werror,                          _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Werror")
    PHP_CHECK_GCC_ARG(-Wextra,                          _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wextra")
    PHP_CHECK_GCC_ARG(-Wformat-nonliteral,              _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wformat-nonliteral")
    PHP_CHECK_GCC_ARG(-Wformat-security,                _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wformat-security")
    PHP_CHECK_GCC_ARG(-Wheader-guard,                   _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wheader-guard")
    PHP_CHECK_GCC_ARG(-Wincompatible-pointer-types-discards-qualifiers, _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wincompatible-pointer-types-discards-qualifiers")
    PHP_CHECK_GCC_ARG(-Wimplicit-fallthrough,           _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wimplicit-fallthrough")
    PHP_CHECK_GCC_ARG(-Winit-self,                      _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Winit-self")
    PHP_CHECK_GCC_ARG(-Wlogical-not-parentheses,        _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wlogical-not-parentheses")
    PHP_CHECK_GCC_ARG(-Wlogical-op,                     _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wlogical-op")
    PHP_CHECK_GCC_ARG(-Wlogical-op-parentheses,         _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wlogical-op-parentheses")
    PHP_CHECK_GCC_ARG(-Wloop-analysis,                  _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wloop-analysis")
    PHP_CHECK_GCC_ARG(-Wmaybe-uninitialized,            _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wmaybe-uninitialized")
    PHP_CHECK_GCC_ARG(-Wmissing-format-attribute,       _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wmissing-format-attribute")
    PHP_CHECK_GCC_ARG(-Wno-missing-field-initializers,  _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wno-missing-field-initializers")
    PHP_CHECK_GCC_ARG(-Wno-sign-compare,                _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wno-sign-compare")
    PHP_CHECK_GCC_ARG(-Wno-unused-but-set-variable,     _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wno-unused-but-set-variable")
    PHP_CHECK_GCC_ARG(-Wno-unused-parameter,            _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wno-unused-parameter")
    PHP_CHECK_GCC_ARG(-Wno-variadic-macros,             _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wno-variadic-macros")
    PHP_CHECK_GCC_ARG(-Wparentheses,                    _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wparentheses")
    PHP_CHECK_GCC_ARG(-Wpointer-bool-conversion,        _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wpointer-bool-conversion")
    PHP_CHECK_GCC_ARG(-Wsizeof-array-argument,          _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wsizeof-array-argument")
    PHP_CHECK_GCC_ARG(-Wstring-conversion,              _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wstring-conversion")
    PHP_CHECK_GCC_ARG(-Wwrite-strings,                  _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wwrite-strings")
    PHP_CHECK_GCC_ARG(-fdiagnostics-show-option,        _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -fdiagnostics-show-option")
    PHP_CHECK_GCC_ARG(-fno-exceptions,                  _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -fno-exceptions")
    PHP_CHECK_GCC_ARG(-fno-omit-frame-pointer,          _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -fno-omit-frame-pointer")
    PHP_CHECK_GCC_ARG(-fno-optimize-sibling-calls,      _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -fno-optimize-sibling-calls")
    PHP_CHECK_GCC_ARG(-fsanitize-address,               _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -fsanitize-address")
    PHP_CHECK_GCC_ARG(-fstack-protector,                _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -fstack-protector")

    MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS"
    STD_CFLAGS="-g -O0 -Wall"
  fi

  PHP_XDEBUG_CFLAGS="$STD_CFLAGS $MAINTAINER_CFLAGS"

  XDEBUG_BASE_SOURCES="src/base/base.c src/base/filter.c src/base/monitor.c src/base/stack.c src/base/superglobals.c"
  XDEBUG_LIB_SOURCES="src/lib/usefulstuff.c src/lib/compat.c src/lib/crc32.c src/lib/hash.c src/lib/lib.c src/lib/llist.c src/lib/set.c src/lib/str.c src/lib/var.c src/lib/var_export_html.c src/lib/var_export_line.c src/lib/var_export_serialized.c src/lib/var_export_text.c src/lib/var_export_xml.c src/lib/xml.c"

  XDEBUG_COVERAGE_SOURCES="src/coverage/branch_info.c src/coverage/code_coverage.c"
  XDEBUG_DEBUGGER_SOURCES="src/debugger/com.c src/debugger/debugger.c src/debugger/handler_dbgp.c src/debugger/handlers.c"
  XDEBUG_GCSTATS_SOURCES="src/gcstats/gc_stats.c"
  XDEBUG_PROFILER_SOURCES="src/profiler/profiler.c"
  XDEBUG_TRACING_SOURCES="src/tracing/trace_computerized.c src/tracing/trace_html.c src/tracing/trace_textual.c src/tracing/tracing.c"

  PHP_NEW_EXTENSION(xdebug, xdebug.c $XDEBUG_BASE_SOURCES $XDEBUG_LIB_SOURCES $XDEBUG_COVERAGE_SOURCES $XDEBUG_DEBUGGER_SOURCES $XDEBUG_GCSTATS_SOURCES $XDEBUG_PROFILER_SOURCES $XDEBUG_TRACING_SOURCES, $ext_shared,,$PHP_XDEBUG_CFLAGS,,yes)
  PHP_ADD_BUILD_DIR(PHP_EXT_BUILDDIR(xdebug)[/src/base])
  PHP_ADD_BUILD_DIR(PHP_EXT_BUILDDIR(xdebug)[/src/lib])
  PHP_ADD_BUILD_DIR(PHP_EXT_BUILDDIR(xdebug)[/src/coverage])
  PHP_ADD_BUILD_DIR(PHP_EXT_BUILDDIR(xdebug)[/src/debugger])
  PHP_ADD_BUILD_DIR(PHP_EXT_BUILDDIR(xdebug)[/src/gcstats])
  PHP_ADD_BUILD_DIR(PHP_EXT_BUILDDIR(xdebug)[/src/profiler])
  PHP_ADD_BUILD_DIR(PHP_EXT_BUILDDIR(xdebug)[/src/tracing])
  PHP_SUBST(XDEBUG_SHARED_LIBADD)
  PHP_ADD_MAKEFILE_FRAGMENT

  PHP_ADD_INCLUDE($ext_srcdir/src)
  PHP_ADD_INCLUDE($ext_builddir/src)
fi
