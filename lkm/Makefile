PRJ_CONFIG ?= ../../.env

ifneq ($(wildcard $(PRJ_CONFIG)), )
include $(PRJ_CONFIG)
else
$(error "Please run source at top of zeratul project.")
endif

MAKEFLAGS	+= --no-print-directory
KDIR 		:= $(ZRT_ENV_KERNEL_DIR)

export KDIR

MODULES 	:= global-event env hx3205 ltr311als

.PHONY : all clean release $(MODULES)

all clean release: $(MODULES)

$(MODULES):
	@if [ -z "$(SUB)" ] || [ "$(SUB)" = "$@" ]; then \
		make -C $@ $(MAKECMDGOALS); \
	fi
