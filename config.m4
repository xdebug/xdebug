dnl $Id: config.m4,v 1.1.1.1 2002-04-24 14:26:19 derick Exp $
dnl config.m4 for extension xdebug

PHP_ARG_ENABLE(xdebug, whether to enable eXtended debugging support,
[  --enable-xdebug         Enable xdebug support])

if test "$PHP_XDEBUG" != "no"; then
  PHP_NEW_EXTENSION(xdebug, xdebug.c srm_llist.c, $ext_shared)
  AC_DEFINE(HAVE_XDEBUG,1,[ ])
fi
