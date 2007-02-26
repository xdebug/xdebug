dnl
dnl $Id: acinclude.m4,v 1.3 2007-02-26 14:38:42 derick Exp $
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
AC_DEFUN([XDEBUG_MODULE_ADD],[
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
AC_DEFUN([XDEBUG_HELP_SEPARATOR],[
AC_ARG_ENABLE([],[
$1],[])
])

