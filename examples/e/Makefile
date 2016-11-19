COMP=../../ezcc.py

default: compile test

compile: main.ezc inversefactorial.ezc
	$(COMP) main.ezc inversefactorial.ezc -o e.o

test: e.o
	./e.o