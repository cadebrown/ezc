COMP=../../ezcc.py

default: compile test

compile: main.ezc
	$(COMP) main.ezc -o e.o

test: e.o
	./e.o
