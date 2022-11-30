# SPDX-License-Identifier: GPL-2.0

__objs= $(addprefix ${OBJ}/,tc_perf_app.o tc_perf_utils.o libnetlink.o \
	p4tc_perf_nl.o)

TARGET=p4tc_perf

all: ${TARGET}

include ./common.mk
include ./env.mk
__cflags +=  -I./ -DIFC_GEN_STRUCT -I./include

__ldflags += -lpthread

${TARGET}: ${__objs}
	$(info __objs)
	$(call cc_link,$@,$^)
clean:
	rm -rf ${TARGET} ${__objs}

.PHONY: all clean
