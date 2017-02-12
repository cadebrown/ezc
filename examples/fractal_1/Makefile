###
# c 2017 chemicaldevelopment
#
# EZC Makefile for fractal_1
#
# more info at: chemicaldevelopment.us/ezc/
#
# cade@cade.site
#
###

# run with `EZCC=ezcc` to test a non-git version
EZCC=python ../../src/ezcc.py

EZC_SRC=fractal.ezc
C_SRC=

PREC=2500
FLAGS=-v1

X=0
Y=0

ZOOM=1

ITER=30


OUT=./fractal.o

default: build
	-

check: clean build run
	-

clean:
	-rm ${OUT}

build:
	${EZCC} ${C_SRC} ${EZC_SRC} ${FLAGS} -o ${OUT}

run:
	${OUT} ${X} ${Y} ${ZOOM} ${ITER} ${PREC}
