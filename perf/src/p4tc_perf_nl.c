/*
 * p4_tc_perf_nl.c	P4 TC Performance evaluation
 *
 *		This program is free software; you can distribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Copyright (c) 2022, Intel Corporation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <dlfcn.h>
#include <linux/tc_act/tc_gact.h>

#include "libnetlink.h"
#include "p4tc_perf_nl.h"
#include "p4tc.h"
#include "rtnetlink.h"

#define MAX_MSG 2048
#define ACTNAMSIZ 64
//#undef  ASYNC_MODE
#define ASYNC_MODE 

extern struct rtnl_handle rth;

struct nl_msg_s nl_req[32][32][P4TC_PERF_NUM_MAX_MSGS + 8];

static int try_strncpy(char *dest, const char *src, size_t max_len)
{
	if (strnlen(src, max_len) == max_len)
		return -1;

	strcpy(dest, src);

	return 0;
}

#define PATH_SEPARATOR "/"

/* PATH SYNTAX: tc p4template objtype/pname/...  */
void parse_path(char *path, char **p4tcpath)
{
	int i = 0;
	char *component;

	component = strtok(path, PATH_SEPARATOR);
	while (component) {
		p4tcpath[i++] = component;
		component = strtok(NULL, PATH_SEPARATOR);
	}
}

int concat_cb_name(char *full_name, const char *cbname,
			   const char *objname, size_t sz)
{
	return snprintf(full_name, sz, "%s/%s", cbname, objname) >= sz ? -1 : 0;
}

struct {
		struct nlmsghdr n;
		struct p4tcmsg t;
		char buf[MAX_MSG];
} req = {
		.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct p4tcmsg)),
		.n.nlmsg_flags = NLM_F_REQUEST | NLM_F_EXCL | NLM_F_CREATE,
		.n.nlmsg_type = RTM_CREATEP4TBENT,
};

int preload_nl_messages(int pidx, char *pname, const char *tbcname,
		 	const char *cbname, const char *tbiname,
		   	uint32_t prio, uint32_t tbid, uint32_t tcnt)
{
	int ret = 0;
	struct rtattr *root;
	struct rtattr *count;
	struct rtattr *parm;
	struct rtattr *tail, *tail2, *tail3;
	uint32_t offset = 32, key = 0, mask = 0xffffffff;// 0xff;
	char *p4tcpath[MAX_PATH_COMPONENTS] = {};
	__u32 ids[2];
	uint32_t msg_count = 0, rulec;
	struct nl_msg_s *req;
	
	unsigned int flags;
	char full_tbcname[256] = {0};
	for (msg_count = 0; msg_count < P4TC_PERF_NUM_MAX_MSGS + 1; msg_count++) {
		struct nl_msg_s *req = &nl_req[pidx][tcnt][msg_count];
		req->n.nlmsg_len = NLMSG_LENGTH(sizeof(struct p4tcmsg)),
		req->n.nlmsg_flags = NLM_F_REQUEST | NLM_F_EXCL | NLM_F_CREATE,
		req->n.nlmsg_type = RTM_CREATEP4TBENT,

		req->t.obj = P4TC_OBJ_TABLE_ENTRY;

		//sprintf(cmd, "$TC p4 create ptables/table/cb/tname2/tinst1
		//srcPort bit16 %u dstPort bit16 443 prio 16", sport);
		addattrstrz(&req->n, MAX_MSG, P4TC_ROOT_PNAME, pname);
		root = addattr_nest(&req->n, MAX_MSG * P4TC_PERF_DEF_BATCH_SIZE, P4TC_ROOT);
		for (rulec = 1; rulec < P4TC_PERF_DEF_BATCH_SIZE + 1; rulec++) {
			count = addattr_nest(&req->n, MAX_MSG, rulec);
			parm = addattr_nest(&req->n, MAX_MSG, P4TC_PARAMS);

			ret = concat_cb_name(full_tbcname, cbname, tbcname,
					TCLASSNAMSIZ);
			addattrstrz(&req->n, MAX_MSG, P4TC_ENTRY_TBCNAME, full_tbcname);
			addattrstrz(&req->n, MAX_MSG, P4TC_ENTRY_TINAME, tbiname);
			addattr_l(&req->n, MAX_MSG, P4TC_ENTRY_KEY_BLOB, &key, 4);
			addattr_l(&req->n, MAX_MSG, P4TC_ENTRY_MASK_BLOB, &mask, 4);
			addattr8(&req->n, MAX_MSG, P4TC_ENTRY_WHODUNNIT, 2); // 2 - P4TC_ENTITY_TC
			addattr32(&req->n, MAX_MSG, P4TC_ENTRY_PRIO, prio);
			key++;

			addattr_nest_end(&req->n, parm);
			ids[0] = 2;
			ids[1] = tbid;
			addattr_l(&req->n, MAX_MSG, P4TC_PATH, ids, 2 * sizeof(__u32));
			addattr_nest_end(&req->n, count);
		}
		addattr_nest_end(&req->n, root);
	}
}

int send_nl_msg(int pidx, int tcnt, int msg_index)
{
	int ret = 0;
	//printf(" msg_index %d \n",msg_index);
	struct nl_msg_s *nl_msg = &nl_req[pidx][tcnt][msg_index];
#ifdef ASYNC_MODE
	if (rtnl_send(&rth, nl_msg, nl_msg->n.nlmsg_len) < 0) {
		perror("Failed to send dump request");
		exit(1);
	}
#else
	if (rtnl_talk(&rth, &(nl_msg->n), NULL) < 0) {
		fprintf(stderr, "We have an error talking to the kernel\n");
		return -1;
	}
#endif
}

int tc_perf_tbl_cmd(char *pname, char *tbcname, char *cbname, char *tbiname,
		   uint32_t prio)
{
	int ret = 0;
	struct rtattr *root;
	struct rtattr *count;
	struct rtattr *parm;
	struct rtattr *tail, *tail2, *tail3;
	uint32_t offset = 32, key = 0xff, mask = 0xff;
	char *p4tcpath[MAX_PATH_COMPONENTS] = {};
	__u32 ids[2];
	
	unsigned int flags;
	char full_tbcname[256] = {0};
	struct {
		struct nlmsghdr n;
		struct p4tcmsg t;
		char buf[MAX_MSG];
	} req = {
		.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct p4tcmsg)),
		.n.nlmsg_flags = NLM_F_REQUEST | NLM_F_EXCL | NLM_F_CREATE,
		.n.nlmsg_type = RTM_CREATEP4TBENT,
		//.n.nlmsg_type = RTM_NEWP4TEMPLATE,
	};

	req.t.obj = P4TC_OBJ_TABLE_ENTRY;

	//sprintf(cmd, "$TC p4 create ptables/table/cb/tname2/tinst1 srcPort bit16 %u dstPort bit16 443 prio 16", sport);
	addattrstrz(&req.n, MAX_MSG, P4TC_ROOT_PNAME, pname);
	root = addattr_nest(&req.n, MAX_MSG, P4TC_ROOT);

	count = addattr_nest(&req.n, MAX_MSG, 1);
	parm = addattr_nest(&req.n, MAX_MSG, P4TC_PARAMS);

	ret = concat_cb_name(full_tbcname, cbname, tbcname,
			TCLASSNAMSIZ);
	addattrstrz(&req.n, MAX_MSG, P4TC_ENTRY_TBCNAME, full_tbcname);
	addattrstrz(&req.n, MAX_MSG, P4TC_ENTRY_TINAME, tbiname);
	addattr_l(&req.n, MAX_MSG, P4TC_ENTRY_KEY_BLOB, &key, 0);
	addattr_l(&req.n, MAX_MSG, P4TC_ENTRY_MASK_BLOB, &mask, 0);
	addattr8(&req.n, MAX_MSG, P4TC_ENTRY_WHODUNNIT, 2); // 2 - P4TC_ENTITY_TC
	addattr32(&req.n, MAX_MSG, P4TC_ENTRY_PRIO, prio);

	addattr_nest_end(&req.n, parm);
	ids[0] = 2;
	ids[1] = 27;
	addattr_l(&req.n, MAX_MSG, P4TC_PATH, ids, 2 * sizeof(__u32));
	addattr_nest_end(&req.n, count);
	addattr_nest_end(&req.n, root);

	if (rtnl_talk(&rth, &req.n, NULL) < 0) {
		fprintf(stderr, "We have an error talking to the kernel\n");
		return -1;
	}
}

int create_pipeline(char *pname)
{
	char *p4tcpath[MAX_PATH_COMPONENTS] = {};
	char *pipelinename;
	char params[16][128] = {0};
	int param_cnt;
	int ret = 0;
	struct rtattr *root;
	struct rtattr *count;
	struct rtattr *nest;
	struct rtattr *tail, *tail2, *tail3;
	struct tc_gact p = {0, };
	struct tc_gact p1 = {0, };
	int obj_type;
	int pipeid;
	unsigned int flags;

	/*tc create pipeline/pname pname name1 
	 *pipedid 5 preactions gact actid 5 postactiosn gact actid 7 numtclasses 2 maxrules 5
	 define netlink message */

	struct {
		struct nlmsghdr		n;
		struct p4tcmsg		t;
		char			buf[MAX_MSG];
	} req = {
		.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct p4tcmsg)),
		.n.nlmsg_type = RTM_NEWP4TEMPLATE,
		.t.obj = P4TC_OBJ_PIPELINE,
	};

	/* populate netlink header */
	//pipelinename = p4tcpath[PATH_PNAME_IDX];
	if (pname)
		addattrstrz(&req.n, MAX_MSG, P4TC_ROOT_PNAME, pname);

	root = addattr_nest(&req.n, MAX_MSG, P4TC_ROOT);
	count = addattr_nest(&(req.n), MAX_MSG, 1);
	nest = addattr_nest(&(req.n), MAX_MSG, P4TC_PARAMS);


	/* populate pipelinet parameters */
	addattr32(&req.n, MAX_MSG, P4TC_PIPELINE_MAXRULES, 1);
	addattr16(&req.n, MAX_MSG, P4TC_PIPELINE_NUMTCLASSES, 2);

	tail = addattr_nest(&req.n, MAX_MSG, P4TC_PIPELINE_PREACTIONS);
	tail2 = addattr_nest(&req.n, MAX_MSG, 1);
	addattr_l(&req.n, MAX_MSG, TCA_ACT_KIND, "gact", strlen("gact") + 1);

	tail3 = addattr_nest(&req.n, MAX_MSG, TCA_ACT_OPTIONS | NLA_F_NESTED);
	p.index = 5;
	addattr_l(&req.n, MAX_MSG, TCA_GACT_PARMS, &p, sizeof(p));
	addattr_nest_end(&req.n, tail3);

	addattr_nest_end(&req.n, tail2);
	addattr_nest_end(&req.n, tail);

	tail = addattr_nest(&req.n, MAX_MSG, P4TC_PIPELINE_POSTACTIONS);
	tail2 = addattr_nest(&req.n, MAX_MSG, 1);
	addattr_l(&req.n, MAX_MSG, TCA_ACT_KIND, "gact", strlen("gact") + 1);

	tail3 = addattr_nest(&req.n, MAX_MSG, TCA_ACT_OPTIONS | NLA_F_NESTED);
	p1.index = 6;
	addattr_l(&req.n, MAX_MSG, TCA_GACT_PARMS, &p1, sizeof(p1));
	addattr_nest_end(&req.n, tail3);

	addattr_nest_end(&req.n, tail2);
	addattr_nest_end(&req.n, tail);

	addattr_nest_end(&req.n, nest);
	addattr_nest_end(&req.n, count);

	req.n.nlmsg_flags = NLM_F_REQUEST | flags,
	addattr_nest_end(&req.n, root);

	if (rtnl_talk(&rth, &req.n, NULL) < 0) {
		fprintf(stderr, "We have an error talking to the kernel\n");
		return -1;
	}
}

