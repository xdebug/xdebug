dnl $Id: config.m4,v 1.2 2002-05-09 12:12:44 derick Exp $
dnl config.m4 for extension xdebug

PHP_ARG_ENABLE(xdebug, whether to enable eXtended debugging support,
[  --enable-xdebug         Enable xdebug support])

if test "$PHP_XDEBUG" != "no"; then
  PHP_NEW_EXTENSION(xdebug, xdebug.c xdebug_llist.c, $ext_shared)
  AC_DEFINE(HAVE_XDEBUG,1,[ ])
fi
