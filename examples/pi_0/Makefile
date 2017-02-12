###
# c 2017 chemicaldevelopment
#
# EZC Makefile for pi_0
#
# more info at: chemicaldevelopment.us/ezc/
#
# cade@cade.site
#
###

# run with `EZCC=ezcc` to test a non-git version
EZCC=python ../../src/ezcc.py

EZC_SRC=pi.ezc
C_SRC=

PREC=2500
FLAGS=-v1

OUT=./pi.o

default: build
	-

check: clean build run
	-

clean:
	-rm ${OUT}

build:
	${EZCC} ${C_SRC} ${EZC_SRC} ${FLAGS} -o ${OUT}

run:
	${OUT} ${PREC}
