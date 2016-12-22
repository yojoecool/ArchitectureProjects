OBJ = pipeSimulator.o
CC = g++
DEBUG = -g
CFLAGS = -Wall -c $(DEBUG)

pipeSimulator: GPRMem.h GPRMem.cpp pipeSim.cpp
	g++ -o pipeSimulator pipeSim.cpp

clean:
	rm -f pipeSimulator.o