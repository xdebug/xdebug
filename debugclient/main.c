/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002, 2003 Derick Rethans                              |
   +----------------------------------------------------------------------+
   | This source file is subject to version 1.0 of the Xdebug license,    |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | http://xdebug.derickrethans.nl/license.php                           |
   | If you did not receive a copy of the Xdebug license and are unable   |
   | to obtain it through the world-wide-web, please send a note to       |
   | xdebug@derickrethans.nl so we can mail you a copy immediately.       |
   +----------------------------------------------------------------------+
   | Authors:  Derick Rethans <derick@xdebug.org>                         |
   |           Marco Canini <m.canini@libero.it>                          |
   +----------------------------------------------------------------------+
 */

#include <stdio.h>
#include <sys/types.h>
#ifndef WIN32
#include "config.h"
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/un.h>
#else
#include <winsock2.h>
#endif
#include <stdlib.h>
#include <sys/stat.h>

#ifdef HAVE_LIBEDIT
#include <signal.h>
#include <histedit.h>
#endif

#include "usefulstuff.h"

#ifdef WIN32
#define MSG_NOSIGNAL 0
#define sleep(t)  Sleep((t)*1000)
#define close(fd) closesocket(fd)
#endif

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

#define DEBUGCLIENT_VERSION "0.8.0"
#define DEFAULT_PORT 17869

#ifdef HAVE_LIBEDIT

static char prompt[8];
static EditLine *el = NULL;
static History *hist = NULL;
static HistEvent ev;

void initialize_libedit(const char *prog);
void deinitialize_libedit();

static char *get_prompt(EditLine *el);

void handle_sigterm(int i);

void initialize_libedit(const char *prog)
{
	/* Init the builtin history */
	hist = history_init();

	/* Remember 100 events */
	history(hist, &ev, H_SETSIZE, 100);

	/* Initialize editline */
	el = el_init(prog, stdin, stdout, stderr);

	el_set(el, EL_EDITOR, "emacs");    /* Default editor is emacs   */
	el_set(el, EL_SIGNAL, 1);          /* Handle signals gracefully */
	el_set(el, EL_PROMPT, get_prompt); /* Set the prompt function   */

	/* Tell editline to use this history interface */
	el_set(el, EL_HIST, history, hist);

	/*
	 * Source the user's defaults file.
	 */
	/* el_source(el, "xdebug"); */
}

void deinitialize_libedit(void)
{
	el_end(el);
	history_end(hist);
}

static char *get_prompt(EditLine *el)
{
	return prompt;
}

void handle_sigterm(int i)
{
	deinitialize_libedit();
}

#endif


int main(int argc, char *argv[])
{
	int                 port = DEFAULT_PORT; /* Port number to listen for connections */
	int                 ssocket = 0;         /* Socket file descriptor */
	struct sockaddr_in  server_in;
	struct sockaddr_in  client_in;
	int                 client_in_len;
	int                 fd;                  /* Filedescriptor for userinput */
	fd_buf              cxt = { NULL, 0 };
	struct in_addr     *iaddr;
	char               *buffer;              /* Buffer with data from the server */
	const char         *cmd;                 /* Command to send to the server */
	char               *prev_cmd = NULL;     /* Last send command to the server */
	int                 opt;                 /* Current option during parameter parsing */
	int                 length;              /* Length of read buffer */

#ifdef HAVE_LIBEDIT
	int num = 0;

	/* Add SIGTERM signal handler */
	signal (SIGTERM, handle_sigterm);
	initialize_libedit (argv[0]);
#else
	fd_buf std_in = { NULL, 0 };
#endif

#ifdef WIN32
	/* Initialise Windows' WinSock library */
	WORD               wVersionRequested;
	WSADATA            wsaData;

	wVersionRequested = MAKEWORD(2, 2);
	WSAStartup(wVersionRequested, &wsaData);
#endif

	/* Display copyright notice and version number */
	printf("Xdebug GDB emulation client (%s)\n", DEBUGCLIENT_VERSION);
	printf("Copyright 2002-2003 by Derick Rethans.\n");

	/* Option handling */
	while (1) {
		opt = getopt(argc, argv, "hp:v");

		if (opt == -1) {
			break;
		}

		switch (opt) {
			case 'h':
				printf("\nUsage:\n");
				printf("\tdebugclient [-h] [-p port] [-v]\n");
				printf("\t-h\tShow this help\n");
				printf("\t-p\tSpecify the port to listen on (default = 17869)\n");
				printf("\t-v\tShow version number and exit\n");
				exit(0);
				break;
			case 'v':
				exit(0);
				break;
			case 'p':
				port = atoi(optarg);
				printf("Listening on TCP port %d.\n", port);
				break;
		}
	}

	/* Main loop that listens for connections from the debug client and that
	 * does all the communications handling. */
	while (1) {
		ssocket = socket(AF_INET, SOCK_STREAM, 0);
		if (ssocket < 0) {
			fprintf(stderr, "socket: couldn't create socket\n");
			exit(-1);
		}

		memset(&server_in, 0, sizeof(struct sockaddr));
		server_in.sin_family      = AF_INET;
		server_in.sin_addr.s_addr = htonl(INADDR_ANY);
		server_in.sin_port        = htons((unsigned short int) port);

		/* Loop until we can bind to the listening socket. */
		while (bind(ssocket, (struct sockaddr *) &server_in, sizeof(struct sockaddr_in)) < 0) {
			fprintf(stderr, "bind: couldn't bind AF_INET socket?\n");
			sleep(5);
		}

		if (listen(ssocket, 0) == -1) {
			fprintf(stderr, "listen: listen call failed\n");
			exit(-2);
		}
		printf("\nWaiting for debug server to connect.\n");

#ifdef WIN32
		fd = accept(ssocket, (struct sockaddr *) &client_in, NULL);
		if (fd == -1) {
			printf("accept: %d\n", WSAGetLastError());
			exit(-3);
		}
#else
		fd = accept(ssocket, (struct sockaddr *) &client_in, &client_in_len);
		if (fd == -1) {
			perror("accept");
			exit(-3);
		}
#endif
		close(ssocket);

		iaddr = &client_in.sin_addr;

		printf("Connect\n");

		/* Get the message from the server*/
		while ((buffer = fd_read_line_delim(fd, &cxt, FD_RL_SOCKET, '\0', &length)) > 0) {
			buffer = fd_read_line_delim(fd, &cxt, FD_RL_SOCKET, '\0', &length);
			printf ("%s\n", buffer);
			{
				/* The server requires a new command */

#ifdef HAVE_LIBEDIT
				/* Copy the prompt string */
				sprintf(prompt, "(cmd) ");
				if ((cmd = el_gets(el, &num)) != NULL && num != 0) {
					/* Add command to history */
					history(hist, &ev, H_ENTER, cmd);
#else
				printf("(cmd) ");
				fflush(stdout);
				if ((cmd = fd_read_line(0, &std_in, FD_RL_FILE))) {
#endif

					/* If there is a 'previous' command, and when the command
					 * just consists of an "enter", then we set the command to
					 * the previous command. */
					if (prev_cmd && strlen(cmd) == 0 || (strlen(cmd) == 1 && cmd[0] == '\n')) {
						cmd = prev_cmd;
					} else {
						if (prev_cmd) {
							free(prev_cmd);
						}
						prev_cmd = strdup(cmd);
					}

					/* Send the command to the debug server */
					if (send(fd, cmd, strlen(cmd), MSG_NOSIGNAL) == -1) {
						break;
					}
#ifndef HAVE_LIBEDIT
					/* el_gets already put a trailing \n in cmd */
					if (send(fd, "\n", 1, MSG_NOSIGNAL) == -1) {
						break;
					}
#endif
					/* If cmd is quit exit from while */
					if (strncmp(cmd, "quit", 4) == 0) {
						break;
					}
				}
			}
		}
		printf("Disconnect\n\n");
		close(fd);
		if (prev_cmd) {
			free(prev_cmd);
			prev_cmd = NULL;
		}

		/* Sleep some time to reset the TCP/IP connection */
		sleep(1);
	}
}
