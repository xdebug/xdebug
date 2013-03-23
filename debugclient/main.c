/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2013 Derick Rethans                               |
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
#define socklen_t int
#endif
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>

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

#define DEBUGCLIENT_VERSION "0.10.0"
#define DEFAULT_PORT        9000

#ifdef HAVE_LIBEDIT

static char prompt[8];
static EditLine *el = NULL;
static History *hist = NULL;
#ifndef XSC_OLD_LIBEDIT
static HistEvent ev;
#endif

void initialize_libedit(const char *prog);
void deinitialize_libedit();

static char *get_prompt(EditLine *el);

void handle_sigterm(int i);

void initialize_libedit(const char *prog)
{
	/* Init the builtin history */
	hist = history_init();

	/* Remember 100 events */
#ifdef XDC_OLD_LIBEDIT
	history(hist, XDC_H_SETSIZE, 100);
#else
	history(hist, &ev, XDC_H_SETSIZE, 100);
#endif

	/* Initialize editline */
#ifdef XDC_OLD_LIBEDIT
	el = el_init(prog, stdin, stdout);
#else
	el = el_init(prog, stdin, stdout, stderr);
#endif

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
	int                 debug_once = 1;      /* Whether to allow more than one debug session (1 = no) */
	struct sockaddr_in  server_in;
	struct sockaddr_in  client_in;
	socklen_t           client_in_len;
	int                 fd;                  /* Filedescriptor for userinput */
	fd_buf              cxt = { NULL, 0 };
	struct in_addr     *iaddr;
	char               *buffer;              /* Buffer with data from the server */
	char               *cmd;                 /* Command to send to the server */
	char               *prev_cmd = NULL;     /* Last send command to the server */
	int                 length;              /* Length of read buffer */
#ifndef WIN32
	int                 opt;                 /* Current option during parameter parsing */
#endif

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
	printf("Xdebug Simple DBGp client (%s)\n", DEBUGCLIENT_VERSION);
	printf("Copyright 2002-2007 by Derick Rethans.\n");
#ifdef HAVE_LIBEDIT
	printf("- libedit support: enabled\n");
#endif

#ifndef WIN32
	/* Option handling */
	while (1) {
		opt = getopt(argc, argv, "hp:v1");

		if (opt == -1) {
			break;
		}

		switch (opt) {
			case 'h':
				printf("\nUsage:\n");
				printf("\tdebugclient [-h] [-p port] [-v]\n");
				printf("\t-h\tShow this help\n");
				printf("\t-p\tSpecify the port to listen on (default = 9000)\n");
				printf("\t-v\tShow version number and exit\n");
				printf("\t-1\tDebug once and then exit\n");
				exit(0);
				break;
			case 'v':
				exit(0);
				break;
			case 'p':
				port = atoi(optarg);
				printf("Listening on TCP port %d.\n", port);
				break;
			case '1':
				debug_once = 0;
				break;
		}
	}
#endif

	/* Main loop that listens for connections from the debug client and that
	 * does all the communications handling. */
	do {
		ssocket = socket(AF_INET, SOCK_STREAM, 0);
		if (ssocket < 0) {
			fprintf(stderr, "socket: couldn't create socket\n");
			exit(-1);
		}

		memset(&server_in, 0, sizeof(struct sockaddr));
		server_in.sin_family      = AF_INET;
		server_in.sin_addr.s_addr = htonl(INADDR_ANY);
		server_in.sin_port        = htons((unsigned short int) port);

		memset(&client_in, 0, sizeof(client_in));
		client_in_len = sizeof(client_in);

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
			if (strstr(buffer, "<stream") == NULL)
			{
				/* The server requires a new command */

#ifdef HAVE_LIBEDIT
				/* Copy the prompt string */
				sprintf(prompt, "(cmd) ");
				if ((cmd = (char *) el_gets(el, &num)) != NULL && num != 0) {
					/* Add command to history */
#ifdef XDC_OLD_LIBEDIT
					history(hist, H_ENTER, cmd);
#else
					history(hist, &ev, H_ENTER, cmd);
#endif
					/* We overwrite the \n with \0 for libedit builds */
					cmd[strlen(cmd) - 1] = '\0';
#else
				printf("(cmd) ");
				fflush(stdout);
				if ((cmd = fd_read_line(0, &std_in, FD_RL_FILE))) {
#endif

					/* If there is a 'previous' command, and when the command
					 * just consists of an "enter", then we set the command to
					 * the previous command. */
					if (prev_cmd && ((strlen(cmd) == 0) || (strlen(cmd) == 1 && cmd[0] == '\n'))) {
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

					if (send(fd, "\0", 1, MSG_NOSIGNAL) == -1) {
						break;
					}

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
	} while (debug_once);
}
