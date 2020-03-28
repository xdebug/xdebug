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

#include <stdlib.h>
#include <string.h>
#ifndef WIN32
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/file.h>
#else
#define PATH_MAX MAX_PATH
#include <winsock2.h>
#include <io.h>
#include "win32/time.h"
#include <process.h>
#endif
#include "php_xdebug.h"
#include "mm.h"
#include "crc32.h"
#include "str.h"
#include "usefulstuff.h"
#include "ext/standard/php_lcg.h"
#include "ext/standard/flock_compat.h"
#include "main/php_ini.h"

#ifndef NAME_MAX
# ifdef _AIX
#  include <unistd.h>
#  define NAME_MAX pathconf("/dev/null",_PC_NAME_MAX)
# else
#  define NAME_MAX (MAXNAMELEN-1)
# endif
#endif

ZEND_EXTERN_MODULE_GLOBALS(xdebug)

xdebug_str* xdebug_join(const char *delim, xdebug_arg *args, int begin, int end)
{
	int         i;
	xdebug_str *ret = xdebug_str_new();

	if (begin < 0) {
		begin = 0;
	}
	if (end > args->c - 1) {
		end = args->c - 1;
	}
	for (i = begin; i < end; i++) {
		xdebug_str_add(ret, args->args[i], 0);
		xdebug_str_add(ret, delim, 0);
	}
	xdebug_str_add(ret, args->args[end], 0);
	return ret;
}

void xdebug_explode(const char *delim, char *str, xdebug_arg *args, int limit)
{
	char *p1, *p2, *endp;

	endp = str + strlen(str);

	p1 = str;
	p2 = xdebug_memnstr(str, delim, strlen(delim), endp);

	if (p2 == NULL) {
		args->c++;
		args->args = (char**) xdrealloc(args->args, sizeof(char*) * args->c);
		args->args[args->c - 1] = (char*) xdmalloc(strlen(str) + 1);
		memcpy(args->args[args->c - 1], p1, strlen(str));
		args->args[args->c - 1][strlen(str)] = '\0';
	} else {
		do {
			args->c++;
			args->args = (char**) xdrealloc(args->args, sizeof(char*) * args->c);
			args->args[args->c - 1] = (char*) xdmalloc(p2 - p1 + 1);
			memcpy(args->args[args->c - 1], p1, p2 - p1);
			args->args[args->c - 1][p2 - p1] = '\0';
			p1 = p2 + strlen(delim);
		} while ((p2 = xdebug_memnstr(p1, delim, strlen(delim), endp)) != NULL && (limit == -1 || --limit > 1));

		if (p1 <= endp) {
			args->c++;
			args->args = (char**) xdrealloc(args->args, sizeof(char*) * args->c);
			args->args[args->c - 1] = (char*) xdmalloc(endp - p1 + 1);
			memcpy(args->args[args->c - 1], p1, endp - p1);
			args->args[args->c - 1][endp - p1] = '\0';
		}
	}
}

char* xdebug_memnstr(char *haystack, const char *needle, int needle_len, char *end)
{
	char *p = haystack;
	char first = *needle;

	/* let end point to the last character where needle may start */
	end -= needle_len;

	while (p <= end) {
		while (*p != first)
			if (++p > end)
				return NULL;
		if (memcmp(p, needle, needle_len) == 0)
			return p;
		p++;
	}
	return NULL;
}

char* xdebug_strrstr(const char* haystack, const char* needle)
{
	char  *loc = NULL;
	char  *found = NULL;
	size_t pos = 0;

	while ((found = strstr(haystack + pos, needle)) != 0) {
		loc = found;
		pos = (found - haystack) + 1;
	}

	return loc;
}

double xdebug_get_utime(void)
{
#ifdef HAVE_GETTIMEOFDAY
	struct timeval tp;
	long sec = 0L;
	double msec = 0.0;

	if (gettimeofday((struct timeval *) &tp, NULL) == 0) {
		sec = tp.tv_sec;
		msec = (double) (tp.tv_usec / MICRO_IN_SEC);

		if (msec >= 1.0) {
			msec -= (long) msec;
		}
		return msec + sec;
	}
#endif
	return 0;
}

char* xdebug_get_time(void)
{
	time_t cur_time;
	char  *str_time;

	str_time = xdmalloc(24);
	cur_time = time(NULL);
	strftime(str_time, 24, "%Y-%m-%d %H:%M:%S", gmtime (&cur_time));
	return str_time;
}

/* not all versions of php export this */
static int xdebug_htoi(char *s)
{
	int value;
	int c;

	c = s[0];
	if (isupper(c)) {
		c = tolower(c);
	}
	value = (c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10) * 16;

	c = s[1];
	if (isupper(c)) {
		c = tolower(c);
	}
	value += c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10;

	return value;
}

/* not all versions of php export this */
int xdebug_raw_url_decode(char *str, int len)
{
	char *dest = str;
	char *data = str;

	while (len--) {
		if (*data == '%' && len >= 2 && isxdigit((int) *(data + 1)) && isxdigit((int) *(data + 2))) {
			*dest = (char) xdebug_htoi(data + 1);
			data += 2;
			len -= 2;
		} else {
			*dest = *data;
		}
		data++;
		dest++;
	}
	*dest = '\0';
	return dest - str;
}

static unsigned char hexchars[] = "0123456789ABCDEF";

char *xdebug_raw_url_encode(char const *s, int len, int *new_length, int skip_slash)
{
	register int x, y;
	unsigned char *str;

	str = (unsigned char *) xdmalloc(3 * len + 1);
	for (x = 0, y = 0; len--; x++, y++) {
		str[y] = (unsigned char) s[x];
		if ((str[y] < '0' && str[y] != '-' && str[y] != '.' && (str[y] != '/' || !skip_slash)) ||
			(str[y] < 'A' && str[y] > '9' && str[y] != ':') ||
			(str[y] > 'Z' && str[y] < 'a' && str[y] != '_' && (str[y] != '\\' || !skip_slash)) ||
			(str[y] > 'z'))
		{
			str[y++] = '%';
			str[y++] = hexchars[(unsigned char) s[x] >> 4];
			str[y] = hexchars[(unsigned char) s[x] & 15];
		}
	}
	str[y] = '\0';
	if (new_length) {
		*new_length = y;
	}
	return ((char *) str);
}

/* fake URI's per IETF RFC 1738 and 2396 format */
char *xdebug_path_from_url(const char *fileurl)
{
	/* deal with file: url's */
	char *dfp = NULL;
	const char *fp = NULL, *efp = fileurl;
#ifdef PHP_WIN32
	int l = 0;
	int i;
#endif
	char *tmp = NULL, *ret = NULL;

	dfp = xdstrdup(efp);
	fp = dfp;
	xdebug_raw_url_decode(dfp, strlen(dfp));
	tmp = strstr(fp, "file://");

	if (tmp) {
		fp = tmp + 7;
		if (fp[0] == '/' && fp[2] == ':') {
			fp++;
		}
		ret = xdstrdup(fp);
#ifdef PHP_WIN32
		l = strlen(ret);
		/* convert '/' to '\' */
		for (i = 0; i < l; i++) {
			if (ret[i] == '/') {
				ret[i] = '\\';
			}
		}
#endif
	} else {
		ret = xdstrdup(fileurl);
	}

	free(dfp);
	return ret;
}

/* fake URI's per IETF RFC 1738 and 2396 format */
char *xdebug_path_to_url(const char *fileurl)
{
	int l, i, new_len;
	char *tmp = NULL;
	char *encoded_fileurl;

	/* encode the url */
	encoded_fileurl = xdebug_raw_url_encode(fileurl, strlen(fileurl), &new_len, 1);

	if (strncmp(fileurl, "phar://", 7) == 0) {
		/* ignore, phar is cool */
		tmp = xdstrdup(fileurl);
	} else if (fileurl[0] != '/' && fileurl[0] != '\\' && fileurl[1] != ':') {
		/* convert relative paths */
		cwd_state new_state;
		char cwd[MAXPATHLEN];
		char *result;

		result = VCWD_GETCWD(cwd, MAXPATHLEN);
		if (!result) {
			cwd[0] = '\0';
		}

		new_state.cwd = estrdup(cwd);
		new_state.cwd_length = strlen(cwd);

		if (!virtual_file_ex(&new_state, fileurl, NULL, 1)) {
			char *s = estrndup(new_state.cwd, new_state.cwd_length);
			tmp = xdebug_sprintf("file://%s",s);
			efree(s);
		}
		efree(new_state.cwd);

	} else if (fileurl[1] == '/' || fileurl[1] == '\\') {
		/* convert UNC paths (eg. \\server\sharepath) */
		/* See http://blogs.msdn.com/ie/archive/2006/12/06/file-uris-in-windows.aspx */
		tmp = xdebug_sprintf("file:%s", encoded_fileurl);
	} else if (fileurl[0] == '/' || fileurl[0] == '\\') {
		/* convert *nix paths (eg. /path) */
		tmp = xdebug_sprintf("file://%s", encoded_fileurl);
	} else if (fileurl[1] == ':') {
		/* convert windows drive paths (eg. c:\path) */
		tmp = xdebug_sprintf("file:///%s", encoded_fileurl);
	} else {
		/* no clue about it, use it raw */
		tmp = xdstrdup(encoded_fileurl);
	}
	l = strlen(tmp);
	/* convert '\' to '/' */
	for (i = 0; i < l; i++) {
		if (tmp[i] == '\\') {
			tmp[i]='/';
		}
	}
	xdfree(encoded_fileurl);
	return tmp;
}

#ifndef PHP_WIN32
static FILE *xdebug_open_file(char *fname, const char *mode, const char *extension, char **new_fname)
{
	FILE *fh;
	char *tmp_fname;

	if (extension) {
		tmp_fname = xdebug_sprintf("%s.%s", fname, extension);
	} else {
		tmp_fname = xdstrdup(fname);
	}
	fh = fopen(tmp_fname, mode);
	if (fh && new_fname) {
		*new_fname = tmp_fname;
	} else {
		xdfree(tmp_fname);
	}
	return fh;
}

static FILE *xdebug_open_file_with_random_ext(char *fname, const char *mode, const char *extension, char **new_fname)
{
	FILE *fh;
	char *tmp_fname;

	if (extension) {
		tmp_fname = xdebug_sprintf("%s.%06x.%s", fname, (long) (1000000 * php_combined_lcg()), extension);
	} else {
		tmp_fname = xdebug_sprintf("%s.%06x", fname, (long) (1000000 * php_combined_lcg()), extension);
	}
	fh = fopen(tmp_fname, mode);
	if (fh && new_fname) {
		*new_fname = tmp_fname;
	} else {
		xdfree(tmp_fname);
	}
	return fh;
}

FILE *xdebug_fopen(char *fname, const char *mode, const char *extension, char **new_fname)
{
	int   r;
	FILE *fh;
	struct stat buf;
	char *tmp_fname = NULL;
	int   filename_len = 0;

	/* We're not doing any tricks for append mode... as that has atomic writes
	 * anyway. And we ignore read mode as well. */
	if (mode[0] == 'a' || mode[0] == 'r') {
		return xdebug_open_file(fname, mode, extension, new_fname);
	}

	/* Make sure we don't open a file with a path that's too long */
	filename_len += (fname ? strlen(fname) : 0); /* filename */
	filename_len += (extension ? strlen(extension) : 0) + 1; /* extension (+ ".") */
	filename_len += 8; /* possible random extension (+ ".") */
	if (filename_len > NAME_MAX) {
		fname[NAME_MAX - (extension ? strlen(extension) : 0 )] = '\0';
	}

	/* In write mode however we do have to do some stuff. */
	/* 1. Check if the file exists */
	if (extension) {
		tmp_fname = xdebug_sprintf("%s.%s", fname, extension);
	} else {
		tmp_fname = xdstrdup(fname);
	}
	r = stat(tmp_fname, &buf);
	/* We're not freeing "tmp_fname" as that is used in the freopen as well. */

	if (r == -1) {
		/* 2. Cool, the file doesn't exist so we can open it without probs now. */
		fh = xdebug_open_file(fname, "w", extension, new_fname);
		goto lock;
	}

	/* 3. It exists, check if we can open it. */
	fh = xdebug_open_file(fname, "r+", extension, new_fname);
	if (!fh) {
		/* 4. If fh == null we couldn't even open the file, so open a new one with a new name */
		fh = xdebug_open_file_with_random_ext(fname, "w", extension, new_fname);
		goto lock;
	}

	/* 5. It exists and we can open it, check if we can exclusively lock it. */
	r = flock(fileno(fh), LOCK_EX | LOCK_NB);
	if (r == -1) {
		if (errno == EWOULDBLOCK) {
			fclose(fh);
			/* 6. The file is in use, so we open one with a new name. */
			fh = xdebug_open_file_with_random_ext(fname, "w", extension, new_fname);
			goto lock;
		}
	}

	/* 7. We established a lock, now we truncate and return the handle */
	fh = freopen(tmp_fname, "w", fh);

lock: /* Yes yes, an evil goto label here!!! */
	if (fh) {
		/* 8. We have to lock again after the reopen as that basically closes
		 * the file and opens it again. There is a small race condition here...
		 */
		flock(fileno(fh), LOCK_EX | LOCK_NB);
	}
	xdfree(tmp_fname);
	return fh;
}
#else
FILE *xdebug_fopen(char *fname, char *mode, char *extension, char **new_fname)
{
	char *tmp_fname;
	FILE *ret;

	if (extension) {
		tmp_fname = xdebug_sprintf("%s.%s", fname, extension);
	} else {
		tmp_fname = xdstrdup(fname);
	}
	ret = fopen(tmp_fname, mode);
	if (new_fname) {
		*new_fname = tmp_fname;
	} else {
		xdfree(tmp_fname);
	}
	return ret;
}
#endif

int xdebug_format_output_filename(char **filename, char *format, char *script_name)
{
	xdebug_str fname = XDEBUG_STR_INITIALIZER;
	char       cwd[128];

	while (*format)
	{
		if (*format != '%') {
			xdebug_str_addl(&fname, (char *) format, 1, 0);
		} else {
			format++;
			switch (*format)
			{
				case 'c': /* crc32 of the current working directory */
					if (VCWD_GETCWD(cwd, 127)) {
						xdebug_str_add(&fname, xdebug_sprintf("%lu", xdebug_crc32(cwd, strlen(cwd))), 1);
					}
					break;

				case 'p': /* pid */
					xdebug_str_add(&fname, xdebug_sprintf(ZEND_ULONG_FMT, xdebug_get_pid()), 1);
					break;

				case 'r': /* random number */
					xdebug_str_add(&fname, xdebug_sprintf("%06x", (long) (1000000 * php_combined_lcg())), 1);
					break;

				case 's': { /* script fname */
					char *char_ptr, *script_name_tmp;

					/* we do not always have script_name available, so if we
					 * don't have it and this format specifier is used then we
					 * simple do nothing for this specifier */
					if (!script_name) {
						break;
					}

					/* create a copy to work on */
					script_name_tmp = xdstrdup(script_name);

					/* replace slashes, whitespace and colons with underscores */
					while ((char_ptr = strpbrk(script_name_tmp, "/\\: ")) != NULL) {
						char_ptr[0] = '_';
					}
					/* replace .php with _php */
					char_ptr = strrchr(script_name_tmp, '.');
					if (char_ptr) {
						char_ptr[0] = '_';
					}
					xdebug_str_add(&fname, script_name_tmp, 0);
					xdfree(script_name_tmp);
				}	break;

				case 't': { /* timestamp (in seconds) */
					time_t the_time = time(NULL);
					xdebug_str_add(&fname, xdebug_sprintf("%ld", the_time), 1);
				}	break;

				case 'u': { /* timestamp (in microseconds) */
					char *char_ptr, *utime_str = xdebug_sprintf("%F", xdebug_get_utime());

					/* Replace . with _ (or should it be nuked?) */
					char_ptr = strrchr(utime_str, '.');
					if (char_ptr) {
						char_ptr[0] = '_';
					}
					xdebug_str_add(&fname, utime_str, 1);
				}	break;

				case 'H':   /* $_SERVER['HTTP_HOST'] */
				case 'U':   /* $_SERVER['UNIQUE_ID'] */
				case 'R': { /* $_SERVER['REQUEST_URI'] */
					char *char_ptr, *strval;
					zval *data = NULL;

					if (Z_TYPE(PG(http_globals)[TRACK_VARS_SERVER]) == IS_ARRAY) {
						switch (*format) {
						case 'H':
							data = zend_hash_str_find(Z_ARRVAL(PG(http_globals)[TRACK_VARS_SERVER]), "HTTP_HOST", sizeof("HTTP_HOST") - 1);
							break;
						case 'R':
							data = zend_hash_str_find(Z_ARRVAL(PG(http_globals)[TRACK_VARS_SERVER]), "REQUEST_URI", sizeof("REQUEST_URI") - 1);
							break;
						case 'U':
							data = zend_hash_str_find(Z_ARRVAL(PG(http_globals)[TRACK_VARS_SERVER]), "UNIQUE_ID", sizeof("UNIQUE_ID") - 1);
							break;
						}

						if (data) {
							strval = estrdup(Z_STRVAL_P(data));
							/* replace slashes, dots, question marks, plus
							 * signs, ampersands, spaces and other evil chars
							 * with underscores */
							while ((char_ptr = strpbrk(strval, "/\\.?&+:*\"<>| ")) != NULL) {
								char_ptr[0] = '_';
							}
							xdebug_str_add(&fname, strval, 0);
							efree(strval);
						}
					}
				}	break;

				case 'S': { /* session id */
					zval *data;
					char *char_ptr, *strval;
					char *sess_name;

					sess_name = zend_ini_string((char*) "session.name", sizeof("session.name"), 0);

					if (sess_name && Z_TYPE(PG(http_globals)[TRACK_VARS_COOKIE]) == IS_ARRAY &&
						((data = zend_hash_str_find(Z_ARRVAL(PG(http_globals)[TRACK_VARS_COOKIE]), sess_name, strlen(sess_name))) != NULL) &&
						Z_STRLEN_P(data) < 100 /* Prevent any unrealistically long data being set as filename */
					) {
						strval = estrdup(Z_STRVAL_P(data));
						/* replace slashes, dots, question marks, plus signs,
						 * ampersands and spaces with underscores */
						while ((char_ptr = strpbrk(strval, "/\\.?&+ ")) != NULL) {
							char_ptr[0] = '_';
						}
						xdebug_str_add(&fname, strval, 0);
						efree(strval);
					}
				}	break;

				case '%': /* literal % */
					xdebug_str_addl(&fname, "%", 1, 0);
					break;
			}
		}
		format++;
	}

	*filename = fname.d;

	return fname.l;
}

int xdebug_format_file_link(char **filename, const char *error_filename, int error_lineno)
{
	xdebug_str fname = XDEBUG_STR_INITIALIZER;
	char      *format = XINI_BASE(file_link_format);

	while (*format)
	{
		if (*format != '%') {
			xdebug_str_addl(&fname, (char *) format, 1, 0);
		} else {
			format++;
			switch (*format)
			{
				case 'f': /* filename */
					xdebug_str_add(&fname, xdebug_sprintf("%s", error_filename), 1);
					break;

				case 'l': /* line number */
					xdebug_str_add(&fname, xdebug_sprintf("%d", error_lineno), 1);
					break;

				case '%': /* literal % */
					xdebug_str_addl(&fname, "%", 1, 0);
					break;
			}
		}
		format++;
	}

	*filename = fname.d;

	return fname.l;
}

int xdebug_format_filename(char **formatted_name, const char *fmt, const char *default_fmt, const char *filename)
{
	xdebug_str fname = XDEBUG_STR_INITIALIZER;
	char *name;
	xdebug_str *parent, *ancester;
	const char *full = filename;
	xdebug_arg *parts = (xdebug_arg*) xdmalloc(sizeof(xdebug_arg));
	char *slash = xdebug_sprintf("%c", DEFAULT_SLASH);
	const char *format = fmt && fmt[0] ? fmt : default_fmt; /* If the format is empty, we use the default */

	/* Create pointers for the format chars */
	xdebug_arg_init(parts);
	xdebug_explode(slash, (char*) filename, parts, -1);
	name = parts->args[parts->c - 1];
	parent = parts->c > 1 ?
		xdebug_join(slash, parts, parts->c - 2, parts->c - 1) :
		xdebug_str_create_from_char(name);
	ancester = parts->c > 2 ?
		xdebug_join(slash, parts, parts->c - 3, parts->c - 1) :
		xdebug_str_copy(parent);

	while (*format)
	{
		if (*format != '%') {
			xdebug_str_addl(&fname, (char *) format, 1, 0);
		} else {
			format++;
			switch (*format)
			{
				case 'n': /* filename */
					xdebug_str_add(&fname, xdebug_sprintf("%s", name), 1);
					break;
				case 'p': /* parent */
					xdebug_str_add(&fname, xdebug_sprintf("%s", parent->d), 1);
					break;
				case 'a': /* ancester */
					xdebug_str_add(&fname, xdebug_sprintf("%s", ancester->d), 1);
					break;
				case 'f': /* full path */
					xdebug_str_add(&fname, xdebug_sprintf("%s", full), 1);
					break;
				case 's': /* slash */
					xdebug_str_add(&fname, xdebug_sprintf("%c", DEFAULT_SLASH), 1);
					break;
				case '%': /* literal % */
					xdebug_str_addl(&fname, "%", 1, 0);
					break;
			}
		}
		format++;
	}

	xdfree(slash);
	xdebug_str_free(ancester);
	xdebug_str_free(parent);
	xdebug_arg_dtor(parts);

	*formatted_name = fname.d;

	return fname.l;
}
