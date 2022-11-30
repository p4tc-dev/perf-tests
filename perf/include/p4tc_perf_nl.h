/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __P4TC_PERF_NL_H__
#define  __P4TC_PERF_NL_H__
#include "p4tc.h"

#define P4TC_PERF_NUM_MAX_MSGS 3750
#define P4TC_PERF_DEF_BATCH_SIZE 16
#define MAX_PATH_COMPONENTS 5
#define MAX_MSG 2048

struct nl_msg_s {
		struct nlmsghdr n;
		struct p4tcmsg t;
		char buf[MAX_MSG];
};

void parse_path(char *path, char **p4tcpath);
int get_obj_type(const char *str_obj_type);

/* Send config message to kernel via netlink socket */
int tc_perf_tbl_cmd(char *pname, char *tbcname, char *cbname, char *tbiname,
                   uint32_t prio);

int create_pipeline(char *pname);

int send_nl_msg(int pidx, int tcnt, int msg_index);

void preload_perf_test(void);

int preload_nl_messages(int pidx, char *pname,
			const char *tbcname,
			const char *cbname, const char *tbiname,
			uint32_t prio, uint32_t tbid, uint32_t tcnt);

int concat_cb_name(char *full_name, const char *cbname,
                   const char *objname, size_t sz);

#endif // __P4TC_PERF_NL_H__
