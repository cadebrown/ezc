# Makefile - basic rules for building ezc, and ec, the command line interface

CC         ?= gcc
CFLAGS     ?= -O3 -fPIC -std=c99

PREFIX     ?= /usr/local

# -*- main ezc library, libezc

ezc_src_c  := $(addprefix ezc/,mem.c log.c str.c stk.c ctx.c compile.c exec.c ezc-std.c)
ezc_src_h  := $(addprefix ezc/,ezc.h ezc-module.h ezc-impl.h)

ezc_SHARED := ezc/libezc.so
ezc_libs   := -lm

# -*- ec, a commandline usage for it

ec_src_c   := $(addprefix ec/,ec.c)
ec_src_h   := $(addprefix ec/,ec.h)

ec_EXE     := ec/ec
ec_libs    := -Lezc -lezc


# -*- auto-generated outputs used

ezc_o      := $(patsubst %.c,%.o,$(ezc_src_c))
ec_o       := $(patsubst %.c,%.o,$(ec_src_c))

# only used for removing/cleaning
all_o      := $(ezc_o) $(ec_o) $(ezc_SHARED) $(ec_EXE)


.PHONY: clean default install

# by default, build the `ec` binary
default: $(ec_EXE)

# using wildcard means the message
clean:
	rm -rf $(wildcard $(ec_o) $(ezc_o) $(ec_EXE) $(ezc_SHARED))

install: $(ec_EXE) $(ezc_SHARED)
	install -d $(PREFIX)/lib
	install -d $(PREFIX)/include
	install -d $(PREFIX)/bin
	install -m 644 $(ezc_SHARED) $(PREFIX)/lib
	install -m 644 $(ezc_src_h) $(PREFIX)/include
	install -m 655 $(ec_EXE) $(PREFIX)/bin

ezc/%.o: ezc/%.c $(ezc_src_h)
	$(CC) $(CFLAGS) $< -c -o $@

ec/%.o: ec/%.c $(ec_src_h)
	$(CC) -Iezc $(CFLAGS) $< -c -o $@


$(ezc_SHARED): $(ezc_o)
	$(CC) $(CFLAGS) -shared $^ $(ezc_libs) -o $@

$(ec_EXE): $(ec_o) | $(ezc_SHARED)
	$(CC) $(CFLAGS) $^ $(ec_libs) -o $@




