dnl $Id: config.m4,v 1.3 2002-05-14 09:38:36 derick Exp $
dnl config.m4 for extension xdebug

PHP_ARG_ENABLE(xdebug, whether to enable eXtended debugging support,
[  --enable-xdebug         Enable xdebug support])

if test "$PHP_XDEBUG" != "no"; then
dnl PHP < 4.3 config
  PHP_EXTENSION(xdebug, $ext_shared)
dnl PHP >= 4.3 config
dnl   PHP_NEW_EXTENSION(xdebug, xdebug.c xdebug_llist.c, $ext_shared)
  AC_DEFINE(HAVE_XDEBUG,1,[ ])
fi
