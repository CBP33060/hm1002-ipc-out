PRJ_CONFIG ?= ../../../.env

ifneq ($(wildcard $(PRJ_CONFIG)), )
include $(PRJ_CONFIG)
else
$(error "Please run source at top of zeratul project.")
endif

CONFIG_UCLIBC_BUILD	=y
ifeq ($(CONFIG_UCLIBC_BUILD), y)
BUILD_DIR			:= ./build/uclibc
else
BUILD_DIR			:= ./build/glibc
endif
MAKEFLAGS			+= --no-print-directory

APPS				:= RtspService

CPU ?= mips

.PHONY : all clean clean_build $(APPS)

all clean: $(APPS)

clean_build:
	-rm -rf build

$(APPS):
	@if [ -z "$(SUB)" ] || [ "$(SUB)" = "$@" ]; then \
		mkdir -p $(BUILD_DIR)/$@ ; \
		cd $(BUILD_DIR)/$@; \
		cmake -DCONFIG_UCLIBC_BUILD=${CONFIG_UCLIBC_BUILD} \
		-DAPP_DIR=${ZRT_ENV_TOP_DIR}/70mai -D70MAI_DIR=${ZRT_ENV_TOP_DIR}/70mai \
		-DTARGET_CPU=${CPU} -DSENSOR=${ZRT_ENV_SENSOR} -DHOST_NAME=t41\
		../../../$@ > /dev/null 2>&1;\
		make $(MAKECMDGOALS); \
	fi
