/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2026 Derick Rethans                               |
   +----------------------------------------------------------------------+
   | This source file is subject to version 1.01 of the Xdebug license,   |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | https://xdebug.org/license.php                                       |
   | If you did not receive a copy of the Xdebug license and are unable   |
   | to obtain it through the world-wide-web, please send a note to       |
   | derick@xdebug.org so we can mail you a copy immediately.             |
   +----------------------------------------------------------------------+
 */

#include "php_xdebug.h"

#include "debugger/debugger.h"
#include "lib/maps/maps.h"

PHP_FUNCTION(xdebug_add_source_map_directory)
{
	zend_string *directory;
	zend_string *prefix = NULL;
	zend_bool    scanned = false;

	ZEND_PARSE_PARAMETERS_START(1, 2)
		Z_PARAM_PATH_STR(directory)
		Z_PARAM_OPTIONAL
		Z_PARAM_PATH_STR(prefix)
	ZEND_PARSE_PARAMETERS_END();

	scanned = xdebug_path_maps_scan_directory(ZSTR_VAL(directory), prefix ? ZSTR_VAL(prefix) : "/.xdebug");

	if (scanned) {
		xdebug_debugger_reapply_source_maps();
	}

	RETVAL_BOOL(scanned);
}
