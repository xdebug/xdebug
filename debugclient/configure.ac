dnl
dnl Process this file with autoconf to produce a configure script.
dnl

dnl divert(1)

dnl
dnl Start
dnl
AC_INIT(debugclient, 0.12.0)
AC_PREREQ(2.59)
AC_PREFIX_DEFAULT([/usr/local])

AC_CONFIG_HEADERS(config.h)
AC_CANONICAL_HOST

# This is needed if SUBDIRS is set
AC_PROG_MAKE_SET

dnl divert(2)
dnl
dnl Checks for working compiler and other programs needed.
dnl
AC_PROG_CC

if test "$ac_cv_c_compiler_gnu" = "yes"
then
  AS="$CC -c"
  AC_SUBST(AS)
else
  AC_PATH_PROG(AS, as, as)
fi

dnl
dnl Checks for needed header files.
dnl
AC_HEADER_STDC
AC_CHECK_HEADERS(
  error.h \
  fcntl.h  \
  malloc.h \
  syslog.h \
  unistd.h \
  dlfcn.h  \
)
  

dnl
dnl Checks for typedefs, structures, and compiler characteristics.
dnl
AC_C_CONST
AC_C_INLINE
AC_CHECK_SIZEOF(char, 1)
AC_CHECK_SIZEOF(short, 2)
AC_CHECK_SIZEOF(int, 2)
AC_CHECK_SIZEOF(long, 4)
AC_CHECK_SIZEOF(long long, 8)

AC_CHECK_LIB(nsl, gethostbyname)
AC_CHECK_LIB(socket, socket)

AC_CHECK_FUNCS(strerror strtol strtoul)

dnl
dnl Debug section:
dnl
dnl Debug on/off [default: OFF]
dnl
AC_MSG_CHECKING([whether to enable debugging])
AC_ARG_ENABLE(debug,
[  --enable-debug          Enable debugging.],
[
  if test "$enableval" = "yes" ; then
    AC_DEFINE(XDEBUG_DEBUG, 1, [Whether debugging is enabled or not.]) 
    AC_MSG_RESULT([yes])
    CFLAGS="$CFLAGS -Wall -Werror"
  else
    AC_DEFINE(XDEBUG_DEBUG, 0, [Whether debugging is enabled or not.]) 
    AC_MSG_RESULT([no])
    CFLAGS="$CFLAGS -Wall -Werror"
  fi
],[
  AC_MSG_RESULT([no])
])

AC_MSG_CHECKING([whether to use dmalloc debugging])
AC_ARG_ENABLE(dmalloc,
[  --enable-dmalloc        Enable dmalloc debugging ],
[
  if test "$enableval" = "yes" ; then
    AC_MSG_RESULT([yes])
    AC_CHECK_LIB(dmalloc, dmalloc_error,
    [
      AC_DEFINE(HAVE_DMALLOC, 1, [Dmalloc])
      LIBS="-ldmalloc $LIBS"
    ])
  else
    AC_MSG_RESULT([no])
  fi
],[
  AC_MSG_RESULT([no])
])

AC_MSG_CHECKING([whether to use libedit])
AC_ARG_WITH(libedit,
[  --with-libedit          Enable libedit ],
[
  if test "$withval" = "yes" ; then
    AC_MSG_RESULT([yes])
    AC_CHECK_LIB(termcap, tgetflag,
    [
      AC_DEFINE(HAVE_TERMCAP, 1, [ ])
      LIBS="-ltermcap $LIBS"
    ])
    AC_CHECK_LIB(edit, el_set,
    [
      AC_DEFINE(HAVE_LIBEDIT, 1, [ ])
      LIBS="-ledit $LIBS"
      AC_MSG_CHECKING([for H_SETSIZE])

      AC_EGREP_CPP(yes, [
#include <histedit.h>
#ifdef H_SETSIZE
yes
#endif
], [
  AC_DEFINE([XDC_H_SETSIZE], H_SETSIZE, [H_SETSIZE or H_EVENT])
  AC_MSG_RESULT([H_SETSIZE])
], [
  AC_DEFINE([XDC_H_SETSIZE], H_EVENT, [H_SETSIZE or H_EVENT])
  AC_MSG_RESULT([H_EVENT])
])

      AC_MSG_CHECKING([for number of arguments to el_init])
      AC_TRY_COMPILE([
#include <stdio.h>
#include <histedit.h>
], [
static EditLine *el = NULL;

el = el_init("foo", stdin, stdout, stderr);
                        ],
                        XDC_ELINIT_ARGS=4,
                        [AC_TRY_COMPILE([
#include <stdio.h>
#include <histedit.h>
], [
static EditLine *el = NULL;

el = el_init("foo", stdin, stdout);
                                           ],
                                           XDC_ELINIT_ARGS=3,
                                           [AC_ERROR("you have strange libedit")]
                                          )]
                       )
      AC_DEFINE_UNQUOTED([XDC_ELINIT_ARGS], $XDC_ELINIT_ARGS, [Number of arguments to el_init])
      AC_MSG_RESULT($XDC_ELINIT_ARGS)
    ],
    [
      AC_ERROR("libedit was not found on your system.")
    ])
  else
    AC_MSG_RESULT([no])
  fi
],[
  AC_MSG_RESULT([no])
])

dnl
dnl System dependant checks
dnl
case $host_alias in
  *darwin*|*rhapsody*)
    CFLAGS="$CFLAGS -Wno-long-double"
    AC_DEFINE(XDEBUG_MACOSX,1,[ ])
    AC_DEFINE(HAVE_DYLD,1, [ ])
    ;;
esac
          

dnl
dnl Output 
dnl
AC_OUTPUT(Makefile)
