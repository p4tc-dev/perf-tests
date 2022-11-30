// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * tc_perf_app.c    P4TC control path performance application
 *
 * Copyright (c) 2022, Intel Corporation.
 */

/* Standard headers */
#include <inttypes.h>
#include <getopt.h>
#include <stdbool.h>

/* TC Specific headers */
#include "tc_perf_app.h"
#include "p4tc_perf_nl.h"

/* netlink message handler */
struct rtnl_handle rth;

extern struct nl_msg_s nl_req[P4TC_PERF_MAX_PIPELINES][P4TC_PERF_MAX_TINST][P4TC_PERF_NUM_MAX_MSGS + 8];
extern char *optarg;

/**
 * usage() - Prints the usage of the tool
 */
static void usage(char *prog_name)
{
        MSG(STDOUT, "%s: command options\n"
                "%s [-h|--help]\n"
                "%s [-l|--latency] to select latency test\n"
		"%s [-s|--synchronous] to select synchronous mode\n"
                "%s [-p|--rate] to select performance test]\n",
                prog_name, prog_name, prog_name, prog_name, prog_name);
}

void sig_handler(int sig)
{
	int ret = 0;

	/* dump stats and cleanup */
	printf("You have pressed ctrl+c\n");
	printf("Calling Handler for Signal %d\n", sig);
	printf("Calling cleanup crew\n");
	exit(ret ? 1 : 0);
}

int time_diff(struct timespec *a, struct timespec *b, struct timespec *result)
{
	long nsec_diff = a->tv_nsec - b->tv_nsec;
	long sec_diff = difftime(a->tv_sec, b->tv_sec);

	if (!sec_diff) {
		result->tv_sec = sec_diff;
		result->tv_nsec = a->tv_nsec - b->tv_nsec;
		return 0;
	}

	if (nsec_diff < 0) {
		result->tv_sec = sec_diff - 1;
		result->tv_nsec = (a->tv_nsec + 1e9) - b->tv_nsec;
		return 0;
	}
	result->tv_sec = sec_diff;
	result->tv_nsec = nsec_diff;
	return 0;
}


void preload_perf_test(void)
{

	const char *cbname, *tbcname, *tbiname;
	char pname[256] = "ptables";
	uint32_t pidx = 0;
	uint32_t param1 = 0;
	uint32_t intrvl = 0;
	uint32_t cnt = 0;
	uint32_t cintrvl = 0;
	uint32_t avg = 0;
	uint32_t prio = 1;
	uint32_t tcnt = 0;
	uint32_t tbid = 27;
	struct timespec start_time;
	struct timespec end_time;
	struct timespec diff;
	
        cbname = "cb";
        tbcname = "tname";
	tbiname = "tinst1";

	/* Cleanup for performance testing */
	P4TC_PERF_EXECUTE_COMMAND("p4template del pipeline/ptables")
	P4TC_PERF_EXECUTE_COMMAND("actions flush action gact")
	P4TC_PERF_EXECUTE_COMMAND("actions add action pass index 1")
	P4TC_PERF_EXECUTE_COMMAND("actions add action drop index 2")
	P4TC_PERF_EXECUTE_COMMAND("actions add action ok index 3")
	P4TC_PERF_EXECUTE_COMMAND("actions add action reclassify index 4")

	/* create pipeline */
	PERF_CREATE_PIPELINE(ptables1, 22)
	PERF_CREATE_TBCLASS(ptables1, tname, 2)
	PERF_CREATE_TBINST(ptables1, tname)
	PERF_PIPELINE_READY(ptables1)

	PERF_CREATE_PIPELINE(ptables2, 23)
	PERF_CREATE_TBCLASS(ptables2, tname, 2)
	PERF_CREATE_TBINST(ptables2, tname)
	PERF_PIPELINE_READY(ptables2)

	PERF_CREATE_PIPELINE(ptables3, 24)
	PERF_CREATE_TBCLASS(ptables3, tname, 2)
	PERF_CREATE_TBINST(ptables3, tname)
	PERF_PIPELINE_READY(ptables3)

	PERF_CREATE_PIPELINE(ptables4, 25)
	PERF_CREATE_TBCLASS(ptables4, tname, 2)
	PERF_CREATE_TBINST(ptables4, tname)
	PERF_PIPELINE_READY(ptables4)

#ifdef STATS_DEBUG
	P4TC_PERF_EXECUTE_COMMAND("-j p4template get tinst/ptables/cb/tname/tinst1");
#endif
	for(pidx = 0; pidx < P4TC_PERF_MAX_PIPELINES; pidx++) { 
		for(tcnt = 0; tcnt < P4TC_PERF_MAX_TINST; tcnt++) { 
			sprintf(pname, "ptables%u", pidx + 1);
			preload_nl_messages(pidx, pname, tbcname, cbname, tbiname, prio, tbid + tcnt, tcnt);
		}
	}
	printf("Internval\t Count\t Cumulative Interval\t Average \n");
	clock_gettime(CLOCK_MONOTONIC, &start_time);
	for(pidx = 0; pidx < P4TC_PERF_MAX_PIPELINES; pidx++) {
		for(tcnt = 0; tcnt < P4TC_PERF_MAX_TINST; tcnt++) {
			param1 = 0;
			while (1)
			{
				param1++;
				if (param1 >= P4TC_PERF_NUM_MAX_MSGS)
					break;

				cnt++;
				tbid++;
				send_nl_msg(pidx, tcnt, param1);
				clock_gettime(CLOCK_MONOTONIC, &end_time);
				time_diff(&end_time, &start_time, &diff);
				if (diff.tv_sec > 1) {
					intrvl++;
					cintrvl = cintrvl + cnt;
					avg = param1 / intrvl;

					printf("%u\t\t %u\t %u\t\t\t %u \n", intrvl, cnt, param1, avg);
					clock_gettime(CLOCK_MONOTONIC, &start_time);
					cnt = 0;
				}
			}
		}
	}
	clock_gettime(CLOCK_MONOTONIC, &end_time);
	time_diff(&end_time, &start_time, &diff);
			

#ifdef STATS_DEBUG
	printf("Diff: sec:%lu nsec:%llu count:%u\n", diff.tv_sec, diff.tv_nsec, param1);
	P4TC_PERF_EXECUTE_COMMAND("-j p4template get tinst/ptables1/cb/tname/tinst1");
	P4TC_PERF_EXECUTE_COMMAND("-j p4template get tinst/ptables1/cb/tname/tinst2");
	P4TC_PERF_EXECUTE_COMMAND("-j p4template get tinst/ptables1/cb/tname/tinst3");
	P4TC_PERF_EXECUTE_COMMAND("-j p4template get tinst/ptables1/cb/tname/tinst4");
	P4TC_PERF_EXECUTE_COMMAND("-j p4template get tinst/ptables1/cb/tname/tinst5");
	P4TC_PERF_EXECUTE_COMMAND("-j p4template get tinst/ptables1/cb/tname/tinst6");
	P4TC_PERF_EXECUTE_COMMAND("-j p4template get tinst/ptables1/cb/tname/tinst9");
	P4TC_PERF_EXECUTE_COMMAND("-j p4template get tinst/ptables1/cb/tname/tinst10");
	P4TC_PERF_EXECUTE_COMMAND("-j p4template get tinst/ptables1/cb/tname/tinst11");
	P4TC_PERF_EXECUTE_COMMAND("-j p4template get tinst/ptables1/cb/tname/tinst12");
	P4TC_PERF_EXECUTE_COMMAND("-j p4template get tinst/ptables1/cb/tname/tinst13");
	P4TC_PERF_EXECUTE_COMMAND("-j p4template get tinst/ptables1/cb/tname/tinst14");
	P4TC_PERF_EXECUTE_COMMAND("-j p4template get tinst/ptables1/cb/tname/tinst15");
	P4TC_PERF_EXECUTE_COMMAND("-j p4template get tinst/ptables1/cb/tname/tinst16");
	P4TC_PERF_EXECUTE_COMMAND("-j p4template get tinst/ptables1/cb/tname/tinst17");
	P4TC_PERF_EXECUTE_COMMAND("-j p4template get tinst/ptables1/cb/tname/tinst18");
	P4TC_PERF_EXECUTE_COMMAND("-j p4template get tinst/ptables1/cb/tname/tinst19");
	P4TC_PERF_EXECUTE_COMMAND("-j p4template get tinst/ptables1/cb/tname/tinst20");
	P4TC_PERF_EXECUTE_COMMAND("-j p4template get tinst/ptables1/cb/tname/tinst21");
	P4TC_PERF_EXECUTE_COMMAND("-j p4template get tinst/ptables1/cb/tname/tinst22");
	P4TC_PERF_EXECUTE_COMMAND("-j p4template get tinst/ptables1/cb/tname/tinst23");
	P4TC_PERF_EXECUTE_COMMAND("-j p4template get tinst/ptables1/cb/tname/tinst24");
	P4TC_PERF_EXECUTE_COMMAND("-j p4template get tinst/ptables1/cb/tname/tinst25");
	P4TC_PERF_EXECUTE_COMMAND("-j p4template get tinst/ptables1/cb/tname/tinst26");
	P4TC_PERF_EXECUTE_COMMAND("-j p4template get tinst/ptables1/cb/tname/tinst27");
	P4TC_PERF_EXECUTE_COMMAND("-j p4template get tinst/ptables1/cb/tname/tinst28");
	P4TC_PERF_EXECUTE_COMMAND("-j p4template get tinst/ptables1/cb/tname/tinst29");
	P4TC_PERF_EXECUTE_COMMAND("-j p4template get tinst/ptables1/cb/tname/tinst30");
	P4TC_PERF_EXECUTE_COMMAND("-j p4template get tinst/ptables1/cb/tname/tinst31");
	P4TC_PERF_EXECUTE_COMMAND("-j p4template get tinst/ptables1/cb/tname/tinst32");
#endif
}

void latency_test(void)
{
	const char *cbname, *tbcname, *tbiname;
	struct timespec start_time;
	struct timespec end_time;
	struct timespec diff;
	uint32_t cnt;

	char pname[256] = "ptables";
	uint32_t pidx = 0;
	uint32_t param1 = 0;
	uint32_t prio = 1;
#if 0
	uint32_t time_values[32756];
#endif
	uint32_t tbl_idx;
	uint32_t tbid = 27;
	uint32_t tcnt = 1;
	uint64_t nsec_sum = 0;
	
        cbname = "cb";
        tbcname = "tname";
	tbiname = "tinst1";


	/* Cleanup */
	P4TC_PERF_EXECUTE_COMMAND("p4template del pipeline/ptables");
	P4TC_PERF_EXECUTE_COMMAND("actions flush action gact");
	P4TC_PERF_EXECUTE_COMMAND("actions add action pass index 1");
	P4TC_PERF_EXECUTE_COMMAND("actions add action drop index 2");
	P4TC_PERF_EXECUTE_COMMAND("actions add action ok index 3");
	P4TC_PERF_EXECUTE_COMMAND("actions add action reclassify index 4");

	/* create pipeline */
	P4TC_PERF_EXECUTE_COMMAND("p4template create pipeline/ptables pipeid 22 maxrules 1 numtclasses 1 preactions action gact index 1 postactions action gact index 2");
	P4TC_PERF_EXECUTE_COMMAND("p4template create tclass/ptables/cb/tname tbcid 2 keysz 32 tcount 2 keys id 1 action gact index 3 postactions gact index 4");
	P4TC_PERF_EXECUTE_COMMAND("p4template create tinst/ptables/cb/tname/tinst1 tinstid 27 maxentries 1");
	P4TC_PERF_EXECUTE_COMMAND("p4template update tinst/ptables/cb/tname/tinst1 tinstid 27 maxentries 60000");
	P4TC_PERF_EXECUTE_COMMAND("-j p4template get tinst/ptables/cb/tname/tinst1");
	P4TC_PERF_EXECUTE_COMMAND("p4template update pipeline/ptables state ready");
 
	/* Load netlink messages to avoid NL build latencies */
	preload_nl_messages(pidx, pname, tbcname, cbname, tbiname, prio, tbid, tcnt);


	while (1)
	{
		if (param1 >= P4TC_PERF_NUM_MAX_MSGS)
			break;
		
		param1++;
		cnt++;
		clock_gettime(CLOCK_MONOTONIC, &start_time);
		
		struct nl_msg_s *nl_msg = &nl_req[0][tcnt][param1];
#ifdef ASYNC_MODE
		if (rtnl_send(&rth, nl_msg, nl_msg->n.nlmsg_len) < 0) {
			perror("Failed to send dump request");
		}
#else
		if (rtnl_talk(&rth, &(nl_msg->n), NULL) < 0) {
			fprintf(stderr, "We have an error talking to the kernel\n");
		}
#endif

		clock_gettime(CLOCK_MONOTONIC, &end_time);
		time_diff(&end_time, &start_time, &diff);
#ifdef P4TC_PERF_LAT_REPORT_FILE
		fprintf(stderr, "%lu\n", diff.tv_nsec);
#endif
#if 0
		time_values[tbl_idx++] = diff.tv_nsec;
#endif
		nsec_sum += diff.tv_nsec;
		if (tbl_idx == 32756) {
	 		tbl_idx = 0;
		}
	}

#if P4TC_PERF_LAT_FILE
	P4TC_PERF_EXECUTE_COMMAND("-j p4template get tinst/ptables/cb/tname/tinst1");
	printf("Latency for %u sum:%lu avg per batch : %lu, avg per rule:%lu\n",
		param1,
		nsec_sum,
		nsec_sum/param1,
		nsec_sum/param1/BATCH_SIZE);
#endif
}

/* main handler */
int main(int argc, char *argv[])
{
	bool latency = false;
	bool perf_test = false;
	bool synchronous = false;
	int ret = 0;

	/* command line options */
        static const struct option lopts[] = {
                { "help", no_argument, NULL, 'h' },
                { "latency", no_argument, NULL, 'l' },
                { "rate", no_argument, NULL, 'p' },
		{ "synchronous", no_argument, NULL, 's' },
        };

	/* command line options processing */
        for (;;) {
                int opt;

                opt = getopt_long(argc, argv, "hlp", lopts, NULL);
                if (opt == -1)
                        break;

                switch (opt) {
                case '?':
                        usage(argv[0]);
                        exit(0);
                case 'h':
                        usage(argv[0]);
                        exit(0);
                case 'l':
                        latency = true;
                        break;
		case 'p':
			perf_test = true;
			break;
		case 's':
			synchronous = true;
			break;

                }
        }

        if (!perf_test && !latency && !synchronous) {
                MSG(STDERR, "No option specified\n");
                usage(argv[0]);
                return 0;
        }

	/* Signal handlers */
	signal(SIGINT, sig_handler);
	signal(SIGUSR1, sig_handler);

	/* Create netlink socket to send P4TC netlink messages
	 * to kernel
	 */
	if (rtnl_open(&rth, 0) < 0) {
		fprintf(stderr, "Cannot open rtnetlink\n");
		return -1;
	}

	/* Evaluate table update latency */
	if (latency)
		latency_test();

	/* Evaluate table update latency */
	if (perf_test)
		preload_perf_test();

	return ret ? 1 : 0;
}
