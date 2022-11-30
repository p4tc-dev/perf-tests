/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _TC_PERF_UTILS_H_
#define _TC_PERF_UTILS_H_
#include <dirent.h>

int tc_perf_strncpy(char *d, int dlen,  const char *s, int slen);

int tc_perf_get_mtime(const char *file_name, struct timespec *mtime);

int tc_perf_open(const char *file_name, int mode);

FILE* tc_perf_fopen(const char *file_name, const char *mode);

DIR *tc_perf_opendir(const char *dir_name);

int tc_perf_strsplit(char *string, int stringlen,
             char **tokens, int maxtokens, char delim);

#endif /* _TC_PERF_UTILS_H_ */
