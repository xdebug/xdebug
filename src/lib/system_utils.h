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
#ifndef __XDEBUG_SYSTEM_UTILS_H__
#define __XDEBUG_SYSTEM_UTILS_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __linux__
int xdebug_scan_mountinfo_for_private_tmp(const char *buffer, char **private_tmp);
int xdebug_read_systemd_private_tmp_directory(char **private_tmp);
#endif

#ifdef __cplusplus
}
#endif

#endif // __XDEBUG_SYSTEM_UTILS_H__
