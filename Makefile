# Makefile - basic rules for building ezc, and ec, the command line interface
#
# 
#
#

# prefix to install in. EZC will install in `$(PREFIX)/bin`, `$(PREFIX)/lib`, 
#   `$(PREFIX)/include`
PREFIX     ?= /usr/local

# C compiler, should support basic stuff like -fPIC, -std, etc
CC         ?= cc
CFLAGS     ?= -O3 -std=c99


# always use -fPIC
CFLAGS     += -fPIC

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


# -*- auto-generated outputs used

ezc_o      := $(patsubst %.c,%.o,$(ezc_src_c))
ec_o       := $(patsubst %.c,%.o,$(ec_src_c))

# only used for removing/cleaning
all_o      := $(ezc_o) $(ec_o) $(ezc_SHARED) $(ezc_STATIC) $(ec_EXE)


.PHONY: clean default install

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
	install -m 655 $(ec_EXE) $(PREFIX)/bin

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




