/*
   +----------------------------------------------------------------------+
   | PHP Version 4                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002, 2003 The PHP Group                               |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.02 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | http://www.php.net/license/2_02.txt.                                 |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors:  Derick Rethans <d.rethans@jdimedia.nl>                     |
   |           Marco Canini <m.canini@libero.it>                          |
   +----------------------------------------------------------------------+
 */

#include <stdio.h>
#include <sys/types.h>
#ifndef WIN32
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

#define VERSION "0.7.0"

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
	int port = 17869;
	int ssocket = 0;
	struct sockaddr_in  server_in;
	int                 fd;
	struct sockaddr_in  client_in;
	int                 client_in_len;
	struct in_addr     *iaddr;
	char *buffer;
	const char *cmd;
	fd_buf cxt = { NULL, 0 };

#ifdef HAVE_LIBEDIT
	int num = 0;

	/* Add SIGTERM signal handler */
	signal (SIGTERM, handle_sigterm);
	initialize_libedit (argv[0]);
#else
	fd_buf std_in = { NULL, 0 };
#endif

#ifdef WIN32
	WORD               wVersionRequested;
	WSADATA            wsaData;

	wVersionRequested = MAKEWORD(2, 2);
	WSAStartup(wVersionRequested, &wsaData);
#endif

	printf("Xdebug GDB emulation client (%s)\n", VERSION);
	printf("Copyright 2002 by Derick Rethans, JDI Media Solutions.\n");

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
	
		while (bind(ssocket, (struct sockaddr *) &server_in, sizeof(struct sockaddr_in)) < 0) {
			fprintf (stderr, "bind: couldn't bind AF_INET socket?\n");
			sleep (5);
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
		while ((buffer = fd_read_line(fd, &cxt, FD_RL_SOCKET)) > 0) {
			if (buffer[0] == '?') {
				/* The server requires a new command */

#ifdef HAVE_LIBEDIT
				/* Copy the prompt string */
				sprintf(prompt, "(%s) ", &buffer[1]);
				if ((cmd = el_gets(el, &num)) != NULL && num != 0) {
					/* Add command to history */
					history(hist, &ev, H_ENTER, cmd);
#else
				printf("(%s) ", &buffer[1]);
				fflush(stdout);
				if ((cmd = fd_read_line(0, &std_in, FD_RL_FILE))) {
#endif

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
			} else if (buffer[0] == '-') {
				/* An error on the server happened, the error msg begins
				 * after 8 chars. */
				printf("%s\n", &buffer[8]);
			} else if (buffer[0] != '+') {
				/* The server has performed a command and the resulting output
				 * is in buffer. Let show it to the user. */
				printf ("%s\n", buffer);
			}
		}
		printf("Disconnect\n\n");
		close(fd);

		/* Sleep some time to reset the TCP/IP connection */
		sleep(1);
	}
}
