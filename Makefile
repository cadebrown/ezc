# Makefile - basic rules for building ezc, and ec, the command line interface
#
# You should copy (and possibly modify) `ezc-config.h.template` to `ezc-config.h`. If not, this makefile
#   will copy it for you, and emit a message
# Just run `make` to build the EZC library
#
#

# prefix to install in. EZC will install in `$(PREFIX)/bin`, `$(PREFIX)/lib`, 
#   `$(PREFIX)/include`
PREFIX     ?= /usr/local

# C compiler, should support basic stuff like -fPIC, -std, etc
CC         ?= cc
CFLAGS     ?= -O3 -std=c99

# the location of the config file for building
EZC_CONFIG ?= ezc-config.h

ifeq ($(wildcard $(EZC_CONFIG)),)
    $(warning No EZC config found, copying the default one)
    $(shell cp ezc-config.h.template ezc-config.h)
endif

# always use -fPIC
CFLAGS     += -fPIC

# now, figure out what we're building with:
HAVE_READLINE := $(shell grep '^\#define EZC_HAVE_READLINE' "$(EZC_CONFIG)")
HAVE_GMP   := $(shell grep '^\#define EZC_HAVE_GMP' "$(EZC_CONFIG)")

# -*- main ezc library, libezc
ezc_src_c  := $(addprefix ezc/,mem.c log.c str.c stk.c ezcp.c vm.c exec.c ezc.c ezc-std.c)
ezc_src_h  := $(addprefix ezc/,ezc-types.h ezc-funcs.h ezc.h ezc-impl.h ezc-module.h)

ezc_SHARED := ezc/libezc.so
ezc_STATIC := ezc/libezc.a
ezc_libs   := -lm

# -*- ec, a commandline usage for it

ec_src_c   := $(addprefix ec/,ec.c)
ec_src_h   := $(addprefix ec/,ec.h)

ec_EXE     := ec/ec
ec_libs    := -Lezc -lezc -lm

ifneq ($(HAVE_READLINE),)
  ec_libs  += -lreadline
endif

ifneq ($(HAVE_GMP),)
  ec_libs += -lgmp
endif

# -*- auto-generated outputs used

ezc_o      := $(patsubst %.c,%.o,$(ezc_src_c))
ec_o       := $(patsubst %.c,%.o,$(ec_src_c))

# only used for removing/cleaning
all_o      := $(ezc_o) $(ec_o) $(ezc_SHARED) $(ezc_STATIC) $(ec_EXE)

# all installation files
install_mf := $(wildcard \
    $(addprefix $(PREFIX)/bin/,$(notdir $(ec_EXE))) \
    $(addprefix $(PREFIX)/include/,$(notdir $(ezc_src_h) $(EZC_CONFIG))) \
    $(addprefix $(PREFIX)/lib/,$(notdir $(ezc_SHARED) $(ezc_STATIC))) \
)

.PHONY: clean default install uninstall

# by default, build the `ec` binary
default: $(ec_EXE)

# using wildcard means the message
clean:
	rm -rf $(wildcard $(all_o))


install: $(ec_EXE) $(ezc_SHARED)
	install -d $(PREFIX)/lib
	install -d $(PREFIX)/include
	install -d $(PREFIX)/bin
	install -m 644 $(ezc_SHARED) $(PREFIX)/lib
	install -m 644 $(ezc_STATIC) $(PREFIX)/lib
	install -m 644 $(ezc_src_h) $(PREFIX)/include
	install -m 644 $(EZC_CONFIG) $(PREFIX)/include
	install -m 655 $(ec_EXE) $(PREFIX)/bin

uninstall:
	rm -r $(install_mf)

ezc/%.o: ezc/%.c $(ezc_src_h)
	$(CC) -I./ $(CFLAGS) $< -c -o $@

ec/%.o: ec/%.c $(ec_src_h)
	$(CC) -I./ -Iezc $(CFLAGS) $< -c -o $@

$(ezc_SHARED): $(ezc_o)
	$(CC) $(CFLAGS) -shared $^ $(ezc_libs) -o $@

$(ezc_STATIC): $(ezc_o)
	$(AR) cr $@ $^

$(ec_EXE): $(ec_o) $(ezc_STATIC)
	$(CC) $(CFLAGS) $< $(ec_libs) -o $@


 
