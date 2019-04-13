all: simcpu

simcpu: simcpu.c
	gcc simcpu.c -g -ansi -Wall -std=c99 -o simcpu
clean:
	rm -f simcpu
	
