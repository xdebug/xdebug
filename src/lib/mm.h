/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2020 Derick Rethans                               |
   +----------------------------------------------------------------------+
   | This source file is subject to version 1.01 of the Xdebug license,   |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | https://xdebug.org/license.php                                       |
   | If you did not receive a copy of the Xdebug license and are unable   |
   | to obtain it through the world-wide-web, please send a note to       |
   | derick@xdebug.org so we can mail you a copy immediately.             |
   +----------------------------------------------------------------------+
   | Authors: Derick Rethans <derick@xdebug.org>                          |
   +----------------------------------------------------------------------+
 */

#ifndef __HAVE_XDEBUG_MM_H__
#define __HAVE_XDEBUG_MM_H__

/* Memory allocators */
#if 0
#define xdmalloc    emalloc
#define xdcalloc    ecalloc
#define xdrealloc   erealloc
#define xdfree      efree
#define xdstrdup    estrdup
#define xdstrndup   estrndup
#else
#define xdmalloc    malloc
#define xdcalloc    calloc
#define xdrealloc   realloc
#define xdfree      free
#define xdstrdup    strdup
#define xdstrndup   xdebug_strndup
#endif

#endif
