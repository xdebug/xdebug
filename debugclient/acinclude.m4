dnl
dnl $Id: acinclude.m4,v 1.1 2002-11-12 11:07:59 derick Exp $
dnl
dnl This file contains local autoconf functions.
dnl The contents of this file are subject to the Vulcan Logic Public
dnl License Version 1.1 (the "License"); you may not use this file
dnl except in compliance with the License. You may obtain a copy of
dnl the License at http://www.vl-srm.net/vlpl/
dnl
dnl Software distributed under the License is distributed on an "AS
dnl IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
dnl implied. See the License for the specific language governing
dnl rights and limitations under the License.
dnl
dnl The Original Code is vl-srm.net code.
dnl
dnl The Initial Developer of the Original Code is the Vulcan Logic 
dnl Group.  Portions created by Vulcan Logic Group are Copyright (C) 
dnl 2000, 2001, 2002 Vulcan Logic Group. All Rights Reserved.
dnl
dnl Contributor(s): 
dnl

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
