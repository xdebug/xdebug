dnl
dnl $Id: acinclude.m4,v 1.2 2003-09-22 09:04:55 derick Exp $
dnl
dnl This file contains local autoconf functions.
dnl This source file is subject to version 1.0 of the Xdebug license,
dnl that is bundled with this package in the file LICENSE, and is
dnl available at through the world-wide-web at
dnl http://xdebug.derickrethans.nl/license.php
dnl If you did not receive a copy of the Xdebug license and are unable
dnl to obtain it through the world-wide-web, please send a note to
dnl xdebug@derickrethans.nl so we can mail you a copy immediately.
dnl
dnl Authors:  Derick Rethans <derick@xdebug.org>

dnl
dnl XDEBUG_MODULE_ADD(modulename [, static])
dnl
XDEBUG_STATIC_LDADD=""
AC_DEFUN(XDEBUG_MODULE_ADD,[
  XDEBUG_MODULES="$XDEBUG_MODULES $1"
  if test a$2 != "a"; then
    XDEBUG_STATIC_MODULES="$XDEBUG_STATIC_MODULES $1"
    XDEBUG_STATIC_LDADD="$XDEBUG_STATIC_LDADD \$(top_srcdir)/modules/$1/lib$1.la"
  fi
  XDEBUG_MOD_MAKEFILES="$XDEBUG_MOD_MAKEFILES modules/$1/Makefile"
])

dnl
dnl XDEBUG_HELP_SEPARATOR(text)
dnl
AC_DEFUN(XDEBUG_HELP_SEPARATOR,[
AC_ARG_ENABLE([],[
$1],[])
])

dnl
dnl XDEBUG_SSL_CHECK
dnl
AC_DEFUN(XDEBUG_SSL_CHECK,[
  AC_MSG_CHECKING([where the openssl is installed])
  AC_ARG_WITH(ssl-dir,
  [  --with-ssl-dir=<dir>    Define the path to OpenSSL install location. [/usr/local] ],
  [ 

    test -f $withval/lib/libcrypto.so -o -f $withval/lib/libcrypto.a && XDEBUG_SSL_DIR="$withval"
    test -f $withval/include/openssl/des.h && INCLUDES="$INCLUDES -I$withval/include"

    if test -z "$XDEBUG_SSL_DIR"; then
      AC_MSG_ERROR([not found. Check the path given to --with-ssl-dir=<dir>])
    else
      AC_MSG_RESULT([$XDEBUG_SSL_DIR])
    fi

  ],[

    for i in /usr/local/ssl /usr/local /usr /usr/local/openssl; do
      test -f $i/lib/libcrypto.so -o -f $i/lib/libcrypto.a && XDEBUG_SSL_DIR="$i"
      test -f $i/include/openssl/des.h && INCLUDES="$INCLUDES -I$i/include"
    done

    if test -z "$XDEBUG_SSL_DIR"; then
      AC_MSG_RESULT([not found.])
    else
      AC_MSG_RESULT([$XDEBUG_SSL_DIR])
    fi

  ])
])
