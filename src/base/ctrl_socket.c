/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2024 Derick Rethans                               |
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

#include "php_xdebug.h"

#if HAVE_XDEBUG_CONTROL_SOCKET_SUPPORT

#include "ctrl_socket.h"
#include "lib/cmd_parser.h"
#include "lib/log.h"
#include "lib/xml.h"

ZEND_EXTERN_MODULE_GLOBALS(xdebug)

typedef struct {
	int         code;
	const char *message;
} xdebug_error_entry;

static xdebug_error_entry xdebug_error_codes[24] = {
	{   0, "no error" },
	{   1, "parse error in command" },
	{   2, "duplicate arguments in command" },
	{   3, "invalid or missing options" },
	{   4, "unimplemented command" },
	{   5, "command is not available" },
	{ 400, "step debugger is not enabled" },
	{ 401, "step debugger did not activate" },
	{  -1, NULL }
};

static const char *error_message_from_code(int code)
{
	xdebug_error_entry *error_entry = &xdebug_error_codes[0];

	while (error_entry->message) {
		if (code == error_entry->code) {
			return error_entry->message;
		}
		error_entry++;
	}
	return NULL;
}

#define ADD_REASON_MESSAGE(c) { \
	xdebug_xml_node *message = xdebug_xml_node_init("message"); \
	xdebug_xml_add_text(message, xdstrdup(error_message_from_code(c))); \
	xdebug_xml_add_child(error, message); \
}


#define CTRL_FUNC_PARAMETERS        xdebug_xml_node **retval, xdebug_dbgp_arg *args
#define CTRL_FUNC(name)             static void xdebug_ctrl_handle_##name(CTRL_FUNC_PARAMETERS)
#define CTRL_FUNC_ENTRY(name)       { #name, xdebug_ctrl_handle_##name },

typedef struct xdebug_ctrl_cmd {
	const char *name;
	void (*handler)(CTRL_FUNC_PARAMETERS);
	int  flags;
} xdebug_ctrl_cmd;

/* Command definition list */
CTRL_FUNC(ps);
CTRL_FUNC(pause);

static xdebug_ctrl_cmd ctrl_commands[] = {
	CTRL_FUNC_ENTRY(ps)
	CTRL_FUNC_ENTRY(pause)
	{ NULL, NULL }
};

/* Command handler selection */
static xdebug_ctrl_cmd* lookup_cmd(char *cmd)
{
	xdebug_ctrl_cmd *ptr = ctrl_commands;

	while (ptr->name) {
		if (strcmp(ptr->name, cmd) == 0) {
			return ptr;
		}
		ptr++;
	}
	return NULL;
}

static xdebug_str *make_message(xdebug_xml_node *message)
{
	xdebug_str  xml_message = XDEBUG_STR_INITIALIZER;
	xdebug_str *ret = xdebug_str_new();

	xdebug_xml_return_node(message, &xml_message);

	xdebug_str_add_literal(ret, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	xdebug_str_add(ret, xml_message.d, 0);
	xdebug_str_addc(ret, '\0');
	xdebug_str_destroy(&xml_message);

	return ret;
}


static void handle_command(int fd, const char *line)
{
	char *cmd = NULL;
	xdebug_dbgp_arg *args;
	int res = 0;
	xdebug_ctrl_cmd *command;
	xdebug_str *message;
	xdebug_xml_node *retval;
	res = xdebug_cmd_parse(line, (char**) &cmd, (xdebug_dbgp_arg**) &args);

	retval = xdebug_xml_node_init("ctrl-response");
	xdebug_xml_add_attribute(retval, "xmlns:xdebug-ctrl", "https://xdebug.org/ctrl/xdebug");

	command = lookup_cmd(cmd);
	if (command) {
		command->handler(&retval, args);
	} else {
		xdebug_xml_node *error;
		error = xdebug_xml_node_init("error");
		xdebug_xml_add_attribute_ex(error, "code", xdebug_sprintf("%lu", XDEBUG_ERROR_COMMAND_UNAVAILABLE), 0, 1);
		ADD_REASON_MESSAGE(XDEBUG_ERROR_COMMAND_UNAVAILABLE);
		xdebug_xml_add_child(retval, error);
	}

	message = make_message(retval);
	write(fd, message->d, message->l);

	xdfree(cmd);
	xdebug_cmd_arg_dtor(args);
}

CTRL_FUNC(ps)
{
	xdebug_xml_node *response, *engine, *file, *pid, *time, *memory;
	char *pid_str, *time_str, *memory_str;
	function_stack_entry *fse = XDEBUG_VECTOR_HEAD(XG_BASE(stack));
	double time_elapsed = XDEBUG_SECONDS_SINCE_START(xdebug_get_nanotime());

	response = xdebug_xml_node_init("ps");
	xdebug_xml_add_attribute(response, "success", "1");

	engine = xdebug_xml_node_init("engine");
	xdebug_xml_add_attribute(engine, "version", XDEBUG_VERSION);
	xdebug_xml_add_text(engine, xdstrdup(XDEBUG_NAME));
	xdebug_xml_add_child(response, engine);

	file = xdebug_xml_node_init("fileuri");
	xdebug_xml_add_text(file, ZSTR_VAL(fse->filename));
	xdebug_xml_add_child(response, file);

	pid = xdebug_xml_node_init("pid");
	pid_str = xdebug_sprintf("%lu", xdebug_get_pid());
	xdebug_xml_add_text(pid, pid_str);
	xdebug_xml_add_child(response, pid);

	time = xdebug_xml_node_init("time");
	time_str = xdebug_sprintf("%f", time_elapsed);
	xdebug_xml_add_text(time, time_str);
	xdebug_xml_add_child(response, time);

	memory = xdebug_xml_node_init("memory");
	memory_str = xdebug_sprintf("%ld", zend_memory_usage(0) / 1024);
	xdebug_xml_add_text(memory, memory_str);
	xdebug_xml_add_child(response, memory);

	xdebug_xml_add_child(*retval, response);
}

CTRL_FUNC(pause)
{
	xdebug_xml_node *response, *pid, *action;
	char *pid_str;

	response = xdebug_xml_node_init("pause");
	xdebug_xml_add_attribute(response, "success", "1");

	pid = xdebug_xml_node_init("pid");
	pid_str = xdebug_sprintf("%lu", xdebug_get_pid());
	xdebug_xml_add_text(pid, pid_str);
	xdebug_xml_add_child(response, pid);

	if (!XDEBUG_MODE_IS(XDEBUG_MODE_STEP_DEBUG)) {
		xdebug_xml_node *error;

		error = xdebug_xml_node_init("error");
		xdebug_xml_add_attribute_ex(error, "code", xdebug_sprintf("%lu", XDEBUG_ERROR_STEP_DEBUG_MODE_NOT_ENABLED), 0, 1);
		ADD_REASON_MESSAGE(XDEBUG_ERROR_STEP_DEBUG_MODE_NOT_ENABLED);
		xdebug_xml_add_child(*retval, error);

		xdebug_xml_add_child(*retval, response);
		return;
	}

	if (!xdebug_is_debug_connection_active()) {
		action = xdebug_xml_node_init("action");
		xdebug_xml_add_text(action, xdstrdup("IDE Connection Signalled"));

		XG_DBG(context).do_connect_to_client = 1;
	} else {
		action = xdebug_xml_node_init("action");
		xdebug_xml_add_text(action, xdstrdup("Breakpoint Signalled"));

		XG_DBG(context).do_break = 1;
	}
	xdebug_xml_add_child(response, action);
	xdebug_xml_add_child(*retval, response);
}

static void xdebug_control_socket_handle(void)
{
	char           buffer[256];
	int            bytes_read;
	int            rc;
	struct timeval timeout;
	fd_set         master_set, working_set;

	/* Update last trigger */
	XG_BASE(control_socket_last_trigger) = xdebug_get_nanotime();

	/* Initialize the master fd_set */
	FD_ZERO(&master_set);
	FD_SET(XG_BASE(control_socket_fd), &master_set);

	/* non blocking */
	timeout.tv_sec  = 0;
	timeout.tv_usec = 0;

	memcpy(&working_set, &master_set, sizeof(master_set));
	rc = select(XG_BASE(control_socket_fd) + 1, &working_set, NULL, NULL, &timeout);

	if (rc < 0) {
		xdebug_log_ex(XLOG_CHAN_CONFIG, XLOG_WARN, "CTRL-SELECT", "Select failed: %s", strerror(errno));
		return;
	}

	if (rc == 0) {
		return;
	}

	if (FD_ISSET(XG_BASE(control_socket_fd), &working_set)) {
		int new_sd = accept(XG_BASE(control_socket_fd), NULL, NULL);
		if (new_sd < 0) {
			if (errno != EWOULDBLOCK) {
				fprintf(stdout, "  accept() failed: %d: %s", errno, strerror(errno));
			}
			return;
		}

		memset(buffer, 0, sizeof(buffer));
		bytes_read = read(new_sd, buffer, sizeof(buffer));
		if (bytes_read == -1) {
			xdebug_log_ex(XLOG_CHAN_CONFIG, XLOG_WARN, "CTRL-RECV", "Can't receive from socket: %s", strerror(errno));
		} else {
			xdebug_log_ex(XLOG_CHAN_CONFIG, XLOG_INFO, "CTRL-RECV", "Received: '%s'", buffer);
			handle_command(new_sd, buffer);
		}
		close(new_sd);
	}
}

void xdebug_control_socket_dispatch(void)
{
	if (!XG_BASE(control_socket_path)) {
		return;
	}

	switch (XINI_BASE(control_socket_granularity)) {
		case XDEBUG_CONTROL_SOCKET_OFF:
			return;

		case XDEBUG_CONTROL_SOCKET_DEFAULT:
		case XDEBUG_CONTROL_SOCKET_TIME:
			if (xdebug_get_nanotime() < (XG_BASE(control_socket_last_trigger) + (XINI_BASE(control_socket_threshold_ms) * 1000000))) {
				return;
			}

			break;
	}

	xdebug_control_socket_handle();
}

void xdebug_control_socket_setup(void)
{
	struct sockaddr_un *servaddr = NULL;

	/* Initialise control socket globals */
	XG_BASE(control_socket_fd) = -1;
	XG_BASE(control_socket_path) = NULL;
	XG_BASE(control_socket_last_trigger) = xdebug_get_nanotime();

	/* Part 1 – create the socket */
	if (0 > (XG_BASE(control_socket_fd) = socket(AF_UNIX, SOCK_STREAM, 0))) {
		xdebug_log_ex(XLOG_CHAN_CONFIG, XLOG_WARN, "CTRL-SOCKET", "Can't create control socket");
		return;
	}

	XG_BASE(control_socket_path) = xdebug_sprintf("xdebug-ctrl." ZEND_ULONG_FMT, xdebug_get_pid());

	/* Part 2b — Configure socket */
	servaddr = (struct sockaddr_un *)xdmalloc(sizeof(struct sockaddr_un));

	if (servaddr == NULL) {
		xdebug_log_ex(XLOG_CHAN_CONFIG, XLOG_WARN, "CTRL-ALLOC", "Can't allocate memory");
		xdfree(XG_BASE(control_socket_path));
		XG_BASE(control_socket_path) = NULL;
		close(XG_BASE(control_socket_fd));
		return;
	}

	memset(servaddr, 'x', sizeof(struct sockaddr_un));
	servaddr->sun_family = AF_UNIX;
	snprintf(servaddr->sun_path + 1, strlen(XG_BASE(control_socket_path)) + 1, "%s", XG_BASE(control_socket_path));
	servaddr->sun_path[0] = '\0';
	servaddr->sun_path[strlen(XG_BASE(control_socket_path)) + 1] = 'y';

	if (0 != (bind(XG_BASE(control_socket_fd), (struct sockaddr *)servaddr, sizeof(struct sockaddr_un)))) {
		xdebug_log_ex(XLOG_CHAN_CONFIG, XLOG_WARN, "CTRL-BIND", "Can't bind control socket");
		xdfree(servaddr);
		xdfree(XG_BASE(control_socket_path));
		XG_BASE(control_socket_path) = NULL;
		close(XG_BASE(control_socket_fd));
		return;
	}

	/* Part 3 — Listen */
	if (listen(XG_BASE(control_socket_fd), 32) < 0)
	{
		xdebug_log_ex(XLOG_CHAN_CONFIG, XLOG_WARN, "CTRL-LISTEN", "Listen failed: %s", strerror(errno));
		xdfree(servaddr);
		xdfree(XG_BASE(control_socket_path));
		XG_BASE(control_socket_path) = NULL;
		close(XG_BASE(control_socket_fd));
		return;
	}

	xdebug_log_ex(XLOG_CHAN_CONFIG, XLOG_INFO, "CTRL-OK", "Control socket set up successfully: '@%s'", XG_BASE(control_socket_path));
	xdfree(servaddr);
}

void xdebug_control_socket_teardown(void)
{
	if (XG_BASE(control_socket_path)) {
		close(XG_BASE(control_socket_fd));
		xdfree(XG_BASE(control_socket_path));
		XG_BASE(control_socket_path) = NULL;
	}
}

#endif
