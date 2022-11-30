/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __LIBNETLINK_H__
#define __LIBNETLINK_H__ 1

#include <stdio.h>
#include <string.h>
#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/if_link.h>
#include <linux/if_addr.h>
#include <linux/neighbour.h>
#include <linux/netconf.h>
#include <arpa/inet.h>

#include "rtnetlink.h"

struct rtnl_handle {
	int			fd;
	struct sockaddr_nl	local;
	struct sockaddr_nl	peer;
	__u32			seq;
	__u32			dump;
	int			proto;
	FILE		       *dump_fp;
#define RTNL_HANDLE_F_LISTEN_ALL_NSID		0x01
#define RTNL_HANDLE_F_SUPPRESS_NLERR		0x02
#define RTNL_HANDLE_F_STRICT_CHK		0x04
	int			flags;
};

int rtnl_open(struct rtnl_handle *rth, unsigned int subscriptions)
	__attribute__((warn_unused_result));

int rtnl_open_byproto(struct rtnl_handle *rth, unsigned int subscriptions,
			     int protocol)
	__attribute__((warn_unused_result));
void rtnl_close(struct rtnl_handle *rth);
typedef int (*nl_ext_ack_fn_t)(const char *errmsg, uint32_t off,
			       const struct nlmsghdr *inner_nlh);

int rtnl_talk(struct rtnl_handle *rtnl, struct nlmsghdr *n,
	      struct nlmsghdr **answer)
	__attribute__((warn_unused_result));
int rtnl_talk_iov(struct rtnl_handle *rtnl, struct iovec *iovec, size_t iovlen,
		  struct nlmsghdr **answer)
	__attribute__((warn_unused_result));
int rtnl_send(struct rtnl_handle *rth, const void *buf, int)
	__attribute__((warn_unused_result));

int addattr(struct nlmsghdr *n, int maxlen, int type);
int addattr8(struct nlmsghdr *n, int maxlen, int type, __u8 data);
int addattr16(struct nlmsghdr *n, int maxlen, int type, __u16 data);
int addattr32(struct nlmsghdr *n, int maxlen, int type, __u32 data);
int addattr64(struct nlmsghdr *n, int maxlen, int type, __u64 data);
int addattrstrz(struct nlmsghdr *n, int maxlen, int type, const char *data);

int addattr_l(struct nlmsghdr *n, int maxlen, int type,
	      const void *data, int alen);
int addraw_l(struct nlmsghdr *n, int maxlen, const void *data, int len);
struct rtattr *addattr_nest(struct nlmsghdr *n, int maxlen, int type);
int addattr_nest_end(struct nlmsghdr *n, struct rtattr *nest);
struct rtattr *addattr_nest_compat(struct nlmsghdr *n, int maxlen, int type,
				   const void *data, int len);
int addattr_nest_compat_end(struct nlmsghdr *n, struct rtattr *nest);
int rta_addattr8(struct rtattr *rta, int maxlen, int type, __u8 data);
int rta_addattr16(struct rtattr *rta, int maxlen, int type, __u16 data);
int rta_addattr32(struct rtattr *rta, int maxlen, int type, __u32 data);
int rta_addattr64(struct rtattr *rta, int maxlen, int type, __u64 data);
int rta_addattr_l(struct rtattr *rta, int maxlen, int type,
		  const void *data, int alen);

struct rtattr *rta_nest(struct rtattr *rta, int maxlen, int type);
int rta_nest_end(struct rtattr *rta, struct rtattr *nest);

#define RTA_TAIL(rta) \
		((struct rtattr *) (((void *) (rta)) + \
				    RTA_ALIGN((rta)->rta_len)))

#define NLMSG_TAIL(nmsg) \
	((struct rtattr *) (((void *) (nmsg)) + NLMSG_ALIGN((nmsg)->nlmsg_len)))

#endif /* __LIBNETLINK_H__ */
