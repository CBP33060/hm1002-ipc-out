PRJ_CONFIG ?= ../.env

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

APPS				:= libosp libcore libcomproxy platform-media libcrypt

CPU ?= mips

ifeq ($(CPU) , mips)
APPS += ai_manage_subsystem media_source_subsystem audio_manage_subsystem ipc_manage_subsystem speaker_manage_subsystem dev_manage_subsystem
APPS += event_manage_subsystem center_manage_subsystem log_manage_subsystem cli_agent_client
APPS += ota_manage_subsystem
else
BUILD_DIR := ./build/x86
endif

.PHONY : all clean clean_build $(APPS)

all clean: $(APPS)

clean_build:
	-rm -rf build

$(APPS):
	@if [ -z "$(SUB)" ] || [ "$(SUB)" = "$@" ]; then \
		mkdir -p $(BUILD_DIR)/$@ ; \
		cd $(BUILD_DIR)/$@; \
		if [ "$@" = "libosp" ]; then \
			cmake -DCONFIG_UCLIBC_BUILD=${CONFIG_UCLIBC_BUILD} \
			-DAPP_DIR=${ZRT_ENV_TOP_DIR}/70mai -D70MAI_DIR=${ZRT_ENV_TOP_DIR}/70mai \
			-DTARGET_CPU=${CPU} -DSENSOR=${ZRT_ENV_SENSOR}\
			../../../platform-osp/$@ > /dev/null 2>&1;\
		elif [ "$@" = "platform-media" ]; then \
			cmake -DCONFIG_UCLIBC_BUILD=${CONFIG_UCLIBC_BUILD} \
			-DAPP_DIR=${ZRT_ENV_TOP_DIR}/70mai -D70MAI_DIR=${ZRT_ENV_TOP_DIR}/70mai \
			-DTARGET_CPU=${CPU} -DSENSOR=${ZRT_ENV_SENSOR}\
			../../../platform-media/mips32/zeratul_interface_T41 > /dev/null 2>&1;\
		else \
			cmake -DCONFIG_UCLIBC_BUILD=${CONFIG_UCLIBC_BUILD} \
			-DAPP_DIR=${ZRT_ENV_TOP_DIR}/70mai -D70MAI_DIR=${ZRT_ENV_TOP_DIR}/70mai \
			-DTARGET_CPU=${CPU} -DSENSOR=${ZRT_ENV_SENSOR} -DHOST_NAME=t41\
			../../../$@ > /dev/null 2>&1;\
		fi; \
		make $(MAKECMDGOALS); \
	fi
