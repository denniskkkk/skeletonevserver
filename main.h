/* 
 * File:   main.h
 * Author: de
 *
 * Created on
 */
#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <stdint.h>
#include <ftd2xx.h>
#include <string.h>
#include <string>
#include <vector>
#include <cstdlib>
#include <unistd.h>
#include <sys/time.h>

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <syslog.h>
#include <string.h>
#include <errno.h>

using namespace std;

#ifndef MAIN_H
#define	MAIN_H

#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif

#endif	/* MAIN_H */

FT_HANDLE ftHandle;

pthread_mutex_t hmutex;

time_t t;
struct tm * now;
typedef void (*t_sighup_handler)(void);
static t_sighup_handler user_sighup_handler = NULL;
static int got_sighup = 0;
int daemonized = 0;

#define MAX_LINE 16384
#define MAX_HOSTNAME_LEN 1024

struct in_addr localip;
int pid;

struct fd_state {
    char buffer[MAX_LINE];
    struct in_addr mcast_ip;
    u_int16_t mcast_port;
    struct event *read_event;
};

// multicasting
int sock;
int port; 
struct sockaddr_in multicastAddr;
unsigned char multicastTTL;
unsigned int sendStringLen;

// localhost
int lsock;
struct sockaddr_in loaddr;
