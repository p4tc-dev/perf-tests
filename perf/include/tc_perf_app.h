// SPDX-License-Identifier: GPL-2.0
#ifndef __TC_PERF_APP_
#define __TC_PERF_APP_

/* Standard header files */
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <syslog.h>
#include <sys/stat.h>

/* User defined header files */
#include "tc_perf_app.h"
#include "libnetlink.h"

/* externs declerations */
extern int verbosity;

/* Logging related macros */
#define LOG_STDOUT      -1
#define LOG_STDERR      -2
#define MSG(l, ...)     do { if (LOG_##l < 0)                   \
                                printf(__VA_ARGS__);            \
                             else if (LOG_##l <= verbosity)     \
                                syslog(LOG_##l, __VA_ARGS__);   \
                        } while (0)


/* Macros to define static memory to preload netlink messages */
#define P4TC_PERF_MAX_PIPELINES		15
#define P4TC_PERF_MAX_TINST		32
#define P4TC_PERF_TC_PATH	"/home/ped07/gogineni/tc/tc "

#define  P4TC_PERF_EXECUTE_COMMAND(cmd) {	\
	printf("%s \n", P4TC_PERF_TC_PATH # cmd); \
	system(P4TC_PERF_TC_PATH  cmd);	\
}


/* Create pipeline */
#define PERF_CREATE_PIPELINE(pname, pid) { \
system("/home/ped07/gogineni/tc/tc p4template create pipeline/" # pname " pipeid " # pid  \
        " maxrules 1 numtclasses 1 preactions action gact index 1 postactions action gact index 2"); \
}

/* Move pipeline to READY state */
#define PERF_PIPELINE_READY(pname) { \
	system("/home/ped07/gogineni/tc/tc p4template update pipeline/" # pname " state ready"); \
}

/* Create table classes */
#define PERF_CREATE_TBCLASS(pname, tcname, tcid) { \
system("/home/ped07/gogineni/tc/tc p4template create tclass/" # pname "/cb/" # tcname  \
       " tbcid " # tcid " keysz 32 tcount 32 keys id 1 action gact index 3 postactions gact index 4"); \
}

/* Create table instances */
#define PERF_CREATE_TBINST(pname, tcname) { \
	system("/home/ped07/gogineni/tc/tc p4template create tinst/" # pname "/cb/" # tcname "/tinst1 tinstid 27 maxentries 60000"); \
	system("/home/ped07/gogineni/tc/tc p4template create tinst/" # pname "/cb/" # tcname "/tinst2 tinstid 28 maxentries 60000"); \
	system("/home/ped07/gogineni/tc/tc p4template create tinst/" # pname "/cb/" # tcname "/tinst3 tinstid 29 maxentries 60000"); \
	system("/home/ped07/gogineni/tc/tc p4template create tinst/" # pname "/cb/" # tcname "/tinst4 tinstid 30 maxentries 60000"); \
	system("/home/ped07/gogineni/tc/tc p4template create tinst/" # pname "/cb/" # tcname "/tinst5 tinstid 31 maxentries 60000"); \
	system("/home/ped07/gogineni/tc/tc p4template create tinst/" # pname "/cb/" # tcname "/tinst6 tinstid 32 maxentries 60000"); \
	system("/home/ped07/gogineni/tc/tc p4template create tinst/" # pname "/cb/" # tcname "/tinst7 tinstid 33 maxentries 60000"); \
	system("/home/ped07/gogineni/tc/tc p4template create tinst/" # pname "/cb/" # tcname "/tinst8 tinstid 34 maxentries 60000"); \
	system("/home/ped07/gogineni/tc/tc p4template create tinst/" # pname "/cb/" # tcname "/tinst9 tinstid 35 maxentries 60000"); \
	system("/home/ped07/gogineni/tc/tc p4template create tinst/" # pname "/cb/" # tcname "/tinst10 tinstid 36 maxentries 60000"); \
	system("/home/ped07/gogineni/tc/tc p4template create tinst/" # pname "/cb/" # tcname "/tinst11 tinstid 37 maxentries 60000"); \
	system("/home/ped07/gogineni/tc/tc p4template create tinst/" # pname "/cb/" # tcname "/tinst12 tinstid 38 maxentries 60000"); \
	system("/home/ped07/gogineni/tc/tc p4template create tinst/" # pname "/cb/" # tcname "/tinst13 tinstid 39 maxentries 60000"); \
	system("/home/ped07/gogineni/tc/tc p4template create tinst/" # pname "/cb/" # tcname "/tinst14 tinstid 40 maxentries 60000"); \
	system("/home/ped07/gogineni/tc/tc p4template create tinst/" # pname "/cb/" # tcname "/tinst15 tinstid 41 maxentries 60000"); \
	system("/home/ped07/gogineni/tc/tc p4template create tinst/" # pname "/cb/" # tcname "/tinst16 tinstid 42 maxentries 60000"); \
	system("/home/ped07/gogineni/tc/tc p4template create tinst/" # pname "/cb/" # tcname "/tinst17 tinstid 43 maxentries 60000"); \
	system("/home/ped07/gogineni/tc/tc p4template create tinst/" # pname "/cb/" # tcname "/tinst18 tinstid 44 maxentries 60000"); \
	system("/home/ped07/gogineni/tc/tc p4template create tinst/" # pname "/cb/" # tcname "/tinst19 tinstid 45 maxentries 60000"); \
	system("/home/ped07/gogineni/tc/tc p4template create tinst/" # pname "/cb/" # tcname "/tinst20 tinstid 46 maxentries 60000"); \
	system("/home/ped07/gogineni/tc/tc p4template create tinst/" # pname "/cb/" # tcname "/tinst21 tinstid 47 maxentries 60000"); \
	system("/home/ped07/gogineni/tc/tc p4template create tinst/" # pname "/cb/" # tcname "/tinst22 tinstid 48 maxentries 60000"); \
	system("/home/ped07/gogineni/tc/tc p4template create tinst/" # pname "/cb/" # tcname "/tinst23 tinstid 49 maxentries 60000"); \
	system("/home/ped07/gogineni/tc/tc p4template create tinst/" # pname "/cb/" # tcname "/tinst24 tinstid 50 maxentries 60000"); \
	system("/home/ped07/gogineni/tc/tc p4template create tinst/" # pname "/cb/" # tcname "/tinst25 tinstid 51 maxentries 60000"); \
	system("/home/ped07/gogineni/tc/tc p4template create tinst/" # pname "/cb/" # tcname "/tinst26 tinstid 52 maxentries 60000"); \
	system("/home/ped07/gogineni/tc/tc p4template create tinst/" # pname "/cb/" # tcname "/tinst27 tinstid 53 maxentries 60000"); \
	system("/home/ped07/gogineni/tc/tc p4template create tinst/" # pname "/cb/" # tcname "/tinst28 tinstid 54 maxentries 60000"); \
	system("/home/ped07/gogineni/tc/tc p4template create tinst/" # pname "/cb/" # tcname "/tinst29 tinstid 55 maxentries 60000"); \
	system("/home/ped07/gogineni/tc/tc p4template create tinst/" # pname "/cb/" # tcname "/tinst30 tinstid 56 maxentries 60000"); \
	system("/home/ped07/gogineni/tc/tc p4template create tinst/" # pname "/cb/" # tcname "/tinst31 tinstid 57 maxentries 60000"); \
	system("/home/ped07/gogineni/tc/tc p4template create tinst/" # pname "/cb/" # tcname "/tinst32 tinstid 58 maxentries 60000"); \
}


/* Declerations of user defined functions */
void preload_perf_test(void);

/* Latency evaluation */
void latency_test(void);

/* Preload netlink messages to evaluate update rate and latency */
int preload_nl_messages(int pidx, char *pname, const char *tbcname,
			const char *cbname,
		   	const char *tbiname, uint32_t prio,
			uint32_t tbid, uint32_t tcnt);

/* Send config message to kernel via netlink socket */
int send_nl_msg(int pidx, int tcnt, int msg_index);

void sig_handler(int sig);

int time_diff(struct timespec *a, struct timespec *b, struct timespec *result);

#endif
