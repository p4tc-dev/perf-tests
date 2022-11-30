THIS_DIR := $(abspath $(lastword $(MAKEFILE_LIST)/../))

OBJ ?= src
SRC ?= src
INC ?= include
CC := /usr/bin/gcc
AR ?= ar

__cflags += -O0 -g -no-pie
__cflags += -Wformat=2 -Wmissing-prototypes -Wstrict-prototypes -Wwrite-strings
__cflags += -MMD -MP -fpic -fstack-protector-strong -DFORTIFY_SOURCE=2 -DGENHDR_STRUCT

# Flags Prevent compiler from optimizing out security checks
# -fno-strict-overflow - Dont assume strict overflow does not occure
# -fno-delete-null-pointer - Dont delete NULL pointer checks
# -fwrapv - Signed integers wrapping may occure
__cflags += -fno-strict-overflow -fno-delete-null-pointer-checks -fwrapv

__ldflags += -lm

__cflags += -I./src
__cflags += -I./include
__cflags += ${CFLAGS}
__cflags += ${EXTRA_CFLAGS}

__cflags += -USTATS_DEBUG

define cc_link
	${CC} ${__ldflags} -o ${1} ${2}
endef

define cc_comp
	${CC} ${__cflags} -o ${1} -c ${2}
endef

define lib_shared
        ${CC} -shared -Wl,-soname,${1} -o ${1} ${2} ${__ldflags}
endef

define lib_static
        ${AR} r ${1} ${2}
endef

${OBJ}/%.o: ${SRC}/%.c | ${OBJ}
	$(call cc_comp, $@, $<)

${OBJ}/%.o: %.c | ${OBJ}
	$(call cc_comp, $@, $<)

${OBJ}/%: | ${OBJ}

${OBJ}:
	mkdir -p $@

-include $(wildcard ${OBJ}/*.d)

.PHONY: all
.DEFAULT_GOAL := all
