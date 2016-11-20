COMP=../../ezcc.py

default: compile test

compile: main.ezc ataninv.ezc
	$(COMP) main.ezc ataninv.ezc -o pi.o

test: pi.o
	./pi.o 128