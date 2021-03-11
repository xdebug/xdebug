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

#include "php_xdebug.h"
#include "ext/standard/info.h"

#include "headers.h"

#include "lib_private.h"
#include "log.h"

char* xdebug_lib_docs_base(void)
{
	char *env = getenv("XDEBUG_DOCS_BASE");

	if (env) {
		return env;
	}

	return (char*) "https://xdebug.org/docs/";
}

extern ZEND_DECLARE_MODULE_GLOBALS(xdebug);

const char *xdebug_level_msg_prefix[11] = {
	"C", "E", "", "W", "", "", "", "I", "", "", "D"
};
const char *xdebug_log_prefix[11] = {
	"CRIT:", "ERR: ", "", "WARN: ", "", "", "", "INFO: ", "", "", "DEBUG: "
};
const char *xdebug_log_prefix_emoji[11] = {
	"☠", "🛑 ", "", "⚠️ ", "", "", "", "🛈 ", "", "", "• "
};
const char *xdebug_channel_msg_prefix[8] = {
	"CFG-", "LOG-", "DBG-", "GC-", "PROF-", "TRACE-", "COV-", "BASE-"
};
const char *xdebug_channel_name[8] = {
	"[Config] ", "[Log Files] ", "[Step Debug] ", "[GC Stats] ", "[Profiler] ", "[Tracing] ", "[Coverage] ", "[Base] "
};

static inline void xdebug_internal_log(int channel, int log_level, const char *message)
{
	zend_ulong pid;

	if (!XG_LIB(log_file)) {
		return;
	}

	pid = xdebug_get_pid();

	if (!XG_LIB(log_opened_message_sent) && XG_LIB(log_open_timestring)) {
		XG_LIB(log_opened_message_sent) = 1;

		fprintf(XG_LIB(log_file), "[" ZEND_ULONG_FMT "] Log opened at %s\n", pid, XG_LIB(log_open_timestring));
		fflush(XG_LIB(log_file));
		xdfree(XG_LIB(log_open_timestring));
		XG_LIB(log_open_timestring) = NULL;
	}

	fprintf(
		XG_LIB(log_file),
		"[" ZEND_ULONG_FMT "] %s%s%s\n",
		pid,
		xdebug_channel_name[channel],
		xdebug_log_prefix[log_level],
		message
	);

	fflush(XG_LIB(log_file));
}

#define TR_START "<tr><td class=\"e\">"
#define TR_END   "</td></tr>\n"

static inline void xdebug_diagnostic_log(int channel, int log_level, const char *error_code, const char *message)
{
	if (!XG_LIB(diagnosis_buffer) || log_level > XLOG_WARN) {
		return;
	}

	if (sapi_module.phpinfo_as_text) {
		xdebug_str_add(XG_LIB(diagnosis_buffer), xdebug_channel_name[channel], 0);
		xdebug_str_add(XG_LIB(diagnosis_buffer), xdebug_log_prefix[log_level], 0);
		xdebug_str_add(XG_LIB(diagnosis_buffer), message, 0);
	} else {
		xdebug_str_add_const(XG_LIB(diagnosis_buffer), "<tr><td class=\"i\">");
		xdebug_str_add(XG_LIB(diagnosis_buffer), xdebug_log_prefix_emoji[log_level], 0);
		xdebug_str_add_const(XG_LIB(diagnosis_buffer), "</td><td class=\"v\">");
		xdebug_str_add(XG_LIB(diagnosis_buffer), xdebug_channel_name[channel], 0);
		xdebug_str_add(XG_LIB(diagnosis_buffer), message, 0);
		xdebug_str_add_const(XG_LIB(diagnosis_buffer), "</td><td class=\"d\"><a href=\"");
		xdebug_str_add(XG_LIB(diagnosis_buffer), xdebug_lib_docs_base(), 0);
		xdebug_str_add_const(XG_LIB(diagnosis_buffer), "errors#");
		xdebug_str_add(XG_LIB(diagnosis_buffer), xdebug_channel_msg_prefix[channel], 0);
		xdebug_str_add(XG_LIB(diagnosis_buffer), xdebug_level_msg_prefix[log_level], 0);
		if (error_code) {
			xdebug_str_addc(XG_LIB(diagnosis_buffer), '-');
			xdebug_str_add(XG_LIB(diagnosis_buffer), error_code, 0);
		}
		xdebug_str_add_const(XG_LIB(diagnosis_buffer), "\">🖹</a></td></tr>");
	}

	xdebug_str_addc(XG_LIB(diagnosis_buffer), '\n');
}

static inline void xdebug_php_log(int channel, int log_level, const char *error_code, const char *message)
{
	xdebug_str formatted_message = XDEBUG_STR_INITIALIZER;

	if (log_level > XLOG_ERR) {
		return;
	}

	xdebug_str_add_const(&formatted_message, "Xdebug: ");
	xdebug_str_add(&formatted_message, xdebug_channel_name[channel], 0);
	xdebug_str_add(&formatted_message, message, 0);

	if (error_code && log_level == XLOG_CRIT) {
		xdebug_str_add_const(&formatted_message, " (See: ");
		xdebug_str_add(&formatted_message, xdebug_lib_docs_base(), 0);
		xdebug_str_add_const(&formatted_message, "errors#");
		xdebug_str_add(&formatted_message, xdebug_channel_msg_prefix[channel], 0);
		xdebug_str_add(&formatted_message, xdebug_level_msg_prefix[log_level], 0);
		xdebug_str_addc(&formatted_message, '-');
		xdebug_str_add(&formatted_message, error_code, 0);
		xdebug_str_addc(&formatted_message, ')');
	}

	php_log_err(formatted_message.d);

	xdebug_str_destroy(&formatted_message);
}

void XDEBUG_ATTRIBUTE_FORMAT(printf, 4, 5) xdebug_log_ex(int channel, int log_level, const char *error_code, const char *fmt, ...)
{
	char    message[512];
	va_list argv;

	if (XINI_LIB(log_level) < log_level) {
		return;
	}

	va_start(argv, fmt);
	vsnprintf(message, sizeof(message), fmt, argv);
	va_end(argv);

	xdebug_internal_log(channel, log_level, message);

	xdebug_diagnostic_log(channel, log_level, error_code, message);

	xdebug_php_log(channel, log_level, error_code, message);
}

static void log_filename_not_opened(int channel, const char *directory, const char *filename)
{
	xdebug_str full_filename = XDEBUG_STR_INITIALIZER;

	if (directory) {
		xdebug_str_add(&full_filename, directory, 0);
		if (!IS_SLASH(directory[strlen(directory) - 1])) {
			xdebug_str_addc(&full_filename, DEFAULT_SLASH);
		}
	}
	xdebug_str_add(&full_filename, filename, 0);

	xdebug_log_ex(channel, XLOG_ERR, "OPEN", "File '%s' could not be opened.", full_filename.d);

	xdebug_str_destroy(&full_filename);
}

void xdebug_log_diagnose_permissions(int channel, const char *directory, const char *filename)
{
#ifndef WIN32
	struct stat dir_info;
#endif

	log_filename_not_opened(channel, directory, filename);

#ifndef WIN32
	if (!directory) {
		return;
	}

	if (stat(directory, &dir_info) == -1) {
		xdebug_log_ex(channel, XLOG_WARN, "STAT", "%s: %s", directory, strerror(errno));
		return;
	}

	if (!S_ISDIR(dir_info.st_mode)) {
		xdebug_log_ex(channel, XLOG_WARN, "NOTDIR", "The path '%s' is not a directory.", directory);
		return;
	}

	xdebug_log_ex(channel, XLOG_WARN, "PERM", "The path '%s' has the permissions: 0%03o.", directory, dir_info.st_mode & 0777);
#endif
}

static int xdebug_info_printf(const char *fmt, ...) /* {{{ */
{
	char *buf;
	size_t len, written;
	va_list argv;

	va_start(argv, fmt);
	len = vspprintf(&buf, 0, fmt, argv);
	va_end(argv);

	written = php_output_write(buf, len);
	efree(buf);
	return written;
}
/* }}} */

static void print_logo(void)
{
	if (!sapi_module.phpinfo_as_text) {
		PUTS("<tr><td colspan=\"2\" class=\"l\">");
		PUTS("<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"176\" height=\"91\" viewBox=\"0 0 351 181\"><title>Xdebug</title><g fill=\"none\" fill-rule=\"evenodd\" stroke=\"#FFF\"><g transform=\"translate(5 5)\"><path fill=\"#317E1E\" fill-rule=\"nonzero\" stroke-width=\"5\" d=\"M127.9 85.5l68.2 70.7.4 17.3h-38.6l-60.8-59.7-60 59.7h-39v-17.6l69.7-69.8-69.6-71.2V-2.5h38.7l60.2 60 61-60h38.3v16l-68.5 72z\"></path><circle cx=\"75.9\" cy=\"74.6\" r=\"3.5\" stroke-opacity=\".8\" stroke-width=\"4\"></circle><circle cx=\"105.1\" cy=\"85\" r=\"3.5\" stroke-opacity=\".8\" stroke-width=\"4\"></circle><circle cx=\"158.4\" cy=\"15\" r=\"3.5\" stroke-opacity=\".8\" stroke-width=\"4\"></circle><circle cx=\"50.9\" cy=\"140\" r=\"3.5\" stroke-opacity=\".8\" stroke-width=\"4\"></circle><circle cx=\"180.4\" cy=\"158.5\" r=\"3.5\" stroke-opacity=\".8\" stroke-width=\"4\"></circle><path stroke-linecap=\"square\" stroke-linejoin=\"round\" stroke-width=\"4\" d=\"M32.6 158.2L42 149M104.3 69.1l49.4-49.4\"></path><path stroke-linejoin=\"round\" stroke-width=\"4\" d=\"M102.3 82.7L30.3 10H11.2M167.4 10H183M11.2 160.7l79.2-78\"></path><path stroke-linejoin=\"round\" stroke-opacity=\".7\" stroke-width=\"4\" d=\"M116.3 74.6L181 10\"></path><path stroke-linejoin=\"round\" stroke-width=\"4\" d=\"M20.7 19.7l52.7 52.6\"></path><path stroke-linejoin=\"round\" stroke-opacity=\".7\" stroke-width=\"4\" d=\"M116.3 96.7l32 32\"></path><path stroke-linejoin=\"round\" stroke-width=\"4\" d=\"M155.2 133.6l22.1 22M53.6 137.5L97 94.9l67.1 66.6h4.9\"></path></g><g fill=\"#000\" fill-rule=\"nonzero\" stroke-width=\"4\"><path d=\"M158.3 107.7V72.9c0-.5 0-.5-.8-.5-.7 0-.7 0-.7.6v34.7c0 .6 0 .5.7.5.8 0 .7 0 .7-.5zm3.9 11.2a22.5 22.5 0 0 1-20-.7c-3.4-2-5.2-4.8-5.2-8.4V69.6c0-3 1.5-5.6 4.3-7.4a17 17 0 0 1 9.2-2.4c3 0 5.6.3 7.7 1V47H178v73h-15l-.8-1.1zM200.9 84V73.2c0-.8 0-.8-.8-.8s-.8 0-.8.8V84h1.6zm-1.6 12.5v11c0 .7 0 .7.8.7.7 0 .8 0 .8-.8V97.2h19.8v10.4c0 8.9-7.3 13.2-20.7 13.2-13.3 0-20.5-4.5-20.5-13.7V73.6c0-9.2 7.2-13.8 20.5-13.8 13.4 0 20.7 4.4 20.7 13.4v23.3h-21.4zM243.3 107.7V73c0-.6 0-.6-.7-.6-.8 0-.8 0-.8.5v34.8c0 .6 0 .5.8.5.7 0 .7 0 .7-.5zm-5.4 11.1l-.9 1.3h-15V47h19.8v13.8c2.1-.7 4.7-1 7.9-1 3.1 0 6.1.8 8.8 2.3 3 1.6 4.6 4.2 4.6 7.5v40.2c0 3.6-1.8 6.4-5.3 8.4a22.5 22.5 0 0 1-19.9.6zM285.8 119.7c-2.1.8-4.7 1.1-7.8 1.1a17 17 0 0 1-9.1-2.4c-2.8-1.7-4.4-4.2-4.4-7.2v-51h19.8v47.5c0 .6 0 .5.8.5.7 0 .7 0 .7-.4V60h19.8v60h-19.8v-.4zM328.3 108.3V73c0-.6 0-.6-.7-.6-.8 0-.8 0-.8.6V108c0 .6 0 .6.8.6.7 0 .7 0 .7-.4zm-.9 11.8c-2 .5-4.2.7-6.9.7a17 17 0 0 1-9.1-2.4c-2.9-1.7-4.4-4.2-4.4-7.2V71c0-7.4 5.2-11.2 14.6-11.2 2.3 0 5 .7 8.3 2l1.7-1.7H348v57.3c0 4.4-1 9.5-2.8 12-2.1 3.2-7.7 4.5-17 4.5h-18.4v-12h14.9c1.1 0 2-.7 2.7-1.8z\"></path></g></g></svg>");
		PUTS("</td></tr>\n");
	} else {
		PUTS("\33[1m__   __   _      _                 \n"
		     "\33[1m\\ \\ / /  | |    | |                \n"
		     "\33[1m \\ V / __| | ___| |__  _   _  __ _ \n"
		     "\33[1m  > < / _` |/ _ \\ '_ \\| | | |/ _` |\n"
		     "\33[1m / . \\ (_| |  __/ |_) | |_| | (_| |\n"
		     "\33[1m/_/ \\_\\__,_|\\___|_.__/ \\__,_|\\__, |\n"
		     "\33[1m                              __/ |\n"
		     "\33[1m                             |___/ \n\n\33[0m");

	}
}

void print_feature_row(const char *name, int flag, const char *doc_name)
{
	if (!sapi_module.phpinfo_as_text) {
		PUTS("<tr>");
		PUTS("<td class=\"e\">");
		PUTS(name);
		PUTS("</td><td class=\"v\">");
		PUTS(XDEBUG_MODE_IS(flag) ? "✔ enabled" : "✘ disabled");
		PUTS("</td><td class=\"d\"><a href=\"");
		PUTS(xdebug_lib_docs_base());
		PUTS(doc_name);
		PUTS("\">🖹</a></td></tr>\n");
	} else {
		php_info_print_table_row(2, name, XDEBUG_MODE_IS(flag) ? "✔ enabled" : "✘ disabled");
	}
}

void xdebug_print_info(void)
{
	/* Header block */
	php_info_print_table_start();

	print_logo();

	php_info_print_table_row(2, "Version", XDEBUG_VERSION);

	if (!sapi_module.phpinfo_as_text) {
		xdebug_info_printf("<tr><td colspan='2' style='background-color: white; text-align: center'>%s</td></tr>\n", "<a style='color: #317E1E; background-color: transparent; font-weight: bold; text-decoration: underline' href='https://xdebug.org/support'>Support Xdebug on Patreon, GitHub, or as a business</a>");
	} else {
		xdebug_info_printf("Support Xdebug on Patreon, GitHub, or as a business: https://xdebug.org/support\n");
	}
	php_info_print_table_end();

	/* Modes block */
	php_info_print_table_start();

	php_info_print_table_colspan_header(
		sapi_module.phpinfo_as_text ? 2 : 3,
		(char*) (XG_LIB(mode_from_environment) ? "Enabled Features<br/>(through 'XDEBUG_MODE' env variable)" : "Enabled Features<br/>(through 'xdebug.mode' setting)")
	);

	if (!sapi_module.phpinfo_as_text) {
		php_info_print_table_header(3, "Feature", "Enabled/Disabled", "Docs");
	} else {
		php_info_print_table_header(2, "Feature", "Enabled/Disabled");
	}

	print_feature_row("Development Aids", XDEBUG_MODE_DEVELOP, "develop");
	print_feature_row("Coverage", XDEBUG_MODE_COVERAGE, "code_coverage");
	print_feature_row("GC Stats", XDEBUG_MODE_GCSTATS, "garbage_collection");
	print_feature_row("Profiler", XDEBUG_MODE_PROFILING, "profiler");
	print_feature_row("Step Debugger", XDEBUG_MODE_STEP_DEBUG, "remote");
	print_feature_row("Tracing", XDEBUG_MODE_TRACING, "trace");

	php_info_print_table_end();
}

PHPAPI extern char *php_ini_opened_path;
PHPAPI extern char *php_ini_scanned_path;
PHPAPI extern char *php_ini_scanned_files;

static void xdebug_print_php_section(void)
{
	php_info_print_table_start();
	php_info_print_table_colspan_header(2, (char*) "PHP");

	php_info_print_table_colspan_header(2, (char*) "Build Configuration");
	php_info_print_table_row(2, "Version", PHP_VERSION);
#if ZEND_DEBUG
	php_info_print_table_row(2, "Debug Build", "yes");
#else
	php_info_print_table_row(2, "Debug Build", "no");
#endif

#ifdef ZTS
	php_info_print_table_row(2, "Thread Safety", "enabled");
# if PHP_VERSION_ID >= 70300
	php_info_print_table_row(2, "Thread API", tsrm_api_name());
# endif
#else
	php_info_print_table_row(2, "Thread Safety", "disabled");
#endif

	php_info_print_table_colspan_header(2, (char*) "Settings");
	php_info_print_table_row(2, "Configuration File (php.ini) Path", PHP_CONFIG_FILE_PATH);
	php_info_print_table_row(2, "Loaded Configuration File", php_ini_opened_path ? php_ini_opened_path : "(none)");
	php_info_print_table_row(2, "Scan this dir for additional .ini files", php_ini_scanned_path ? php_ini_scanned_path : "(none)");
	php_info_print_table_row(2, "Additional .ini files parsed", php_ini_scanned_files ? php_ini_scanned_files : "(none)");
	php_info_print_table_end();
}

static ZEND_COLD void php_ini_displayer_cb(zend_ini_entry *ini_entry, int type)
{
	if (ini_entry->displayer) {
		ini_entry->displayer(ini_entry, type);
	} else {
		const char *display_string;
		size_t display_string_length;
		int esc_html=0;

		if (type == ZEND_INI_DISPLAY_ORIG && ini_entry->modified) {
			if (ini_entry->orig_value && ZSTR_VAL(ini_entry->orig_value)[0]) {
				display_string = ZSTR_VAL(ini_entry->orig_value);
				display_string_length = ZSTR_LEN(ini_entry->orig_value);
				esc_html = !sapi_module.phpinfo_as_text;
			} else {
				if (!sapi_module.phpinfo_as_text) {
					display_string = "<i>no value</i>";
					display_string_length = sizeof("<i>no value</i>") - 1;
				} else {
					display_string = "no value";
					display_string_length = sizeof("no value") - 1;
				}
			}
		} else if (ini_entry->value && ZSTR_VAL(ini_entry->value)[0]) {
			display_string = ZSTR_VAL(ini_entry->value);
			display_string_length = ZSTR_LEN(ini_entry->value);
			esc_html = !sapi_module.phpinfo_as_text;
		} else {
			if (!sapi_module.phpinfo_as_text) {
				display_string = "<i>no value</i>";
				display_string_length = sizeof("<i>no value</i>") - 1;
			} else {
				display_string = "no value";
				display_string_length = sizeof("no value") - 1;
			}
		}

		if (esc_html) {
			zend_html_puts(display_string, display_string_length);
		} else {
			PHPWRITE(display_string, display_string_length);
		}
	}
}

static int if_overridden_xdebug_mode(char *name)
{
	if ((strcmp("xdebug.mode", name) == 0) && XG_LIB(mode_from_environment)) {
		return 1;
	}

	return 0;
}

static void xdebug_print_settings(void)
{
	zend_module_entry *module;
	zend_ini_entry *ini_entry;
	int module_number;
	zend_string *name = zend_string_init("xdebug", 6, 0);

	module = zend_hash_find_ptr(&module_registry, name);
	zend_string_release(name);

	if (!module) {
		return;
	}

	module_number = module->module_number;

	php_info_print_table_start();
	if (!sapi_module.phpinfo_as_text) {
		php_info_print_table_header(4, "Directive", "Local Value", "Master Value", "Docs");
	} else {
		php_info_print_table_header(3, "Directive", "Local Value", "Master Value");
	}

	ZEND_HASH_FOREACH_PTR(EG(ini_directives), ini_entry) {
		if (ini_entry->module_number != module_number) {
			continue;
		}

		/* Hack to not show changed and removed settings */
		if (ini_entry->value && strncmp(ZSTR_VAL(ini_entry->value), "This setting has", 16) == 0) {
			continue;
		}

		if (!sapi_module.phpinfo_as_text) {
			PUTS("<tr>");
			PUTS("<td class=\"e\">");
			PHPWRITE(ZSTR_VAL(ini_entry->name), ZSTR_LEN(ini_entry->name));
			if (if_overridden_xdebug_mode(ZSTR_VAL(ini_entry->name))) {
				PUTS(" (through XDEBUG_MODE)");
			}
			PUTS("</td><td class=\"v\">");
			if (if_overridden_xdebug_mode(ZSTR_VAL(ini_entry->name))) {
				PUTS(getenv("XDEBUG_MODE"));
			} else {
				php_ini_displayer_cb(ini_entry, ZEND_INI_DISPLAY_ACTIVE);
			}
			PUTS("</td><td class=\"v\">");
			php_ini_displayer_cb(ini_entry, ZEND_INI_DISPLAY_ORIG);
			PUTS("</td><td class=\"d\"><a href=\"");
			PUTS(xdebug_lib_docs_base());
			PUTS("all_settings#");
			PHPWRITE(ZSTR_VAL(ini_entry->name), ZSTR_LEN(ini_entry->name));
			PUTS("\">🖹</a></td></tr>\n");
		} else {
			PHPWRITE(ZSTR_VAL(ini_entry->name), ZSTR_LEN(ini_entry->name));
			if (if_overridden_xdebug_mode(ZSTR_VAL(ini_entry->name))) {
				PUTS(" (through XDEBUG_MODE)");
			}
			PUTS(" => ");
			if (if_overridden_xdebug_mode(ZSTR_VAL(ini_entry->name))) {
				PUTS(getenv("XDEBUG_MODE"));
			} else {
				php_ini_displayer_cb(ini_entry, ZEND_INI_DISPLAY_ACTIVE);
			}
			PUTS(" => ");
			php_ini_displayer_cb(ini_entry, ZEND_INI_DISPLAY_ORIG);
			PUTS("\n");
		}
	} ZEND_HASH_FOREACH_END();

	php_info_print_table_end();
}

static void print_html_header(void)
{
	if (sapi_module.phpinfo_as_text) {
		return;
	}

	PUTS("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"DTD/xhtml1-transitional.dtd\">\n");
	PUTS("<html xmlns=\"http://www.w3.org/1999/xhtml\">");
	PUTS("<head>\n");

	PUTS("<style type=\"text/css\">\n");
	PUTS("body {background-color: #fff; color: #222; font-family: sans-serif;}\n");
	PUTS("pre {margin: 0; font-family: monospace;}\n");
	PUTS("a:link, a:hover, a:visited {color: black; text-decoration: underline;}\n");
	PUTS("table {border-collapse: separate; border: 1px solid #666; width: 934px; box-shadow: 1px 2px 3px #ccc; border-bottom: none; border-right: none; border-spacing: 0;}\n");
	PUTS(".center {text-align: center;}\n");
	PUTS(".center table {margin: 1em auto; text-align: left;}\n");
	PUTS(".center th {text-align: center !important;}\n");
	PUTS("td, th {border: 1px solid #666; font-size: 75%; vertical-align: baseline; padding: 4px 5px; border-left: none; border-top: none;}\n");
	PUTS("th {top: 0; background: inherit; position: sticky;}\n");
	PUTS("h1 {font-size: 150%;}\n");
	PUTS("h2 {font-size: 125%;}\n");
	PUTS(".p {text-align: left;}\n");
	PUTS(".e {background-color: #e5f5d5; width: 300px; font-weight: bold;}\n");
	PUTS(".h {background-color: #bbde94; font-weight: bold;}\n");
	PUTS(".v {background-color: #ddd; max-width: 300px; overflow-x: auto; word-wrap: break-word;}\n");
	PUTS(".i {background-color: #ddd; text-align: center; font-size: 1em; font-family: serif; width: 1em;}\n");
	PUTS(".v i {color: #999;}\n");
	PUTS(".d {background-color: #ddd; width: 1em; text-align: center;}\n");
	PUTS(".l {background-color: #bbde94;}\n");
	PUTS("img {float: right; border: 0;}\n");
	PUTS("hr {width: 934px; background-color: #ccc; border: 0; height: 1px;}\n");
	PUTS("</style>\n");

	PUTS("<title>Xdebug ");
	PUTS(XDEBUG_VERSION);
	PUTS("</title>");
	PUTS("<meta name=\"ROBOTS\" content=\"NOINDEX,NOFOLLOW,NOARCHIVE\" />");
	PUTS("</head>\n");
	PUTS("<body><div class=\"center\">\n");
}

static void print_html_footer(void)
{
	if (sapi_module.phpinfo_as_text) {
		return;
	}

	php_output_write("</div></body></html>", strlen("</div></body></html>"));
}

static void print_diagnostic_log(void)
{
	php_info_print_table_start();
	if (!sapi_module.phpinfo_as_text) {
		php_info_print_table_colspan_header(3, (char*) "Diagnostic Log");
	} else {
		php_info_print_table_colspan_header(2, (char*) "Diagnostic Log");
	}
	if (XG_LIB(diagnosis_buffer) && XG_LIB(diagnosis_buffer)->l) {
		if (!sapi_module.phpinfo_as_text) {
			PUTS("<tr class=\"h\"><th colspan=\"2\">Message</th><th>Docs</th></tr>\n");
		}
		php_output_write(XG_LIB(diagnosis_buffer)->d, XG_LIB(diagnosis_buffer)->l);
	} else {
		if (!sapi_module.phpinfo_as_text) {
			PUTS("<tr><td class=\"v\" colspan=\"3\">No messages</td></tr>\n");
		} else {
			PUTS("No messages\n");
		}
	}
	php_info_print_table_end();
}

static void print_profile_information(void)
{
	char *file_name;

	if (!XDEBUG_MODE_IS(XDEBUG_MODE_PROFILING)) {
		return;
	}

	file_name = xdebug_get_profiler_filename();

	php_info_print_table_start();
	if (!sapi_module.phpinfo_as_text) {
		PUTS("<tr class=\"h\"><th colspan=\"2\">Profiler</th><th>Docs</th></tr>\n");
		if (file_name) {
			xdebug_info_printf("<tr><td class=\"e\">Profile File</td><td class=\"v\">%s</td><td class=\"d\"><a href=\"%sprofiler\">🖹</a></td></tr>\n",
				file_name, xdebug_lib_docs_base());
		} else {
			xdebug_info_printf("<tr><td colspan=\"2\" class=\"d\">Profiler is not active</td><td class=\"d\"><a href=\"%sprofiler\">🖹</a></td></tr>\n",
				xdebug_lib_docs_base());
		}
	} else {
		php_info_print_table_colspan_header(2, (char*) "Profiler");
		if (file_name) {
			php_info_print_table_row(2, "Profile File", file_name);
		} else {
			PUTS("Profiler is not active\n");
		}
	}
	php_info_print_table_end();
}

static void print_trace_information(void)
{
	char *file_name;

	if (!XDEBUG_MODE_IS(XDEBUG_MODE_TRACING)) {
		return;
	}

	file_name = xdebug_get_trace_filename();

	php_info_print_table_start();
	if (!sapi_module.phpinfo_as_text) {
		PUTS("<tr class=\"h\"><th colspan=\"2\">Function Tracing</th><th>Docs</th></tr>\n");
		if (file_name) {
			xdebug_info_printf("<tr><td class=\"e\">Trace File</td><td class=\"v\">%s</td><td class=\"d\"><a href=\"%strace\">🖹</a></td></tr>\n",
				file_name, xdebug_lib_docs_base());
		} else {
			xdebug_info_printf("<tr><td colspan=\"2\" class=\"d\">Function tracing is not active</td><td class=\"d\"><a href=\"%strace\">🖹</a></td></tr>\n",
				xdebug_lib_docs_base());
		}
	} else {
		php_info_print_table_colspan_header(2, (char*) "Function Tracing");
		if (file_name) {
			php_info_print_table_row(2, "Trace File", file_name);
		} else {
			PUTS("Function tracing is not active\n");
		}
	}
	php_info_print_table_end();
}

PHP_FUNCTION(xdebug_info)
{
	print_html_header();

	xdebug_print_info();

	print_diagnostic_log();
	print_profile_information();
	print_trace_information();

	xdebug_print_php_section();

	xdebug_print_settings();

	print_html_footer();
}

void xdebug_open_log(void)
{
	/* initialize remote log file */
	XG_LIB(log_file) = NULL;
	XG_LIB(log_opened_message_sent) = 0;
	XG_LIB(log_open_timestring) = NULL;

	if (XINI_LIB(log) && strlen(XINI_LIB(log))) {
		XG_LIB(log_file) = xdebug_fopen(XINI_LIB(log), "a", NULL, NULL);
	}
	if (XG_LIB(log_file)) {
		XG_LIB(log_open_timestring) = xdebug_nanotime_to_chars(xdebug_get_nanotime(), 6);
	} else if (strlen(XINI_LIB(log))) {
		xdebug_log_diagnose_permissions(XLOG_CHAN_LOGFILE, NULL, XINI_LIB(log));
	}
}

void xdebug_close_log()
{
	char *timestr;

	if (!XG_LIB(log_file)) {
		return;
	}

	if (XG_LIB(log_opened_message_sent)) {
		zend_ulong pid;

		pid = xdebug_get_pid();
		timestr = xdebug_nanotime_to_chars(xdebug_get_nanotime(), 6);

		fprintf(XG_LIB(log_file), "[" ZEND_ULONG_FMT "] Log closed at %s\n\n", pid, timestr);
		fflush(XG_LIB(log_file));
		xdfree(timestr);
	}

	if (XG_LIB(log_open_timestring)) {
		xdfree(XG_LIB(log_open_timestring));
		XG_LIB(log_open_timestring) = NULL;
	}

	fclose(XG_LIB(log_file));
	XG_LIB(log_file) = NULL;
}
