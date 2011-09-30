#project's makefile
CC=g++
FLAGS= -Wall -O3 -g

all: register_file.o processor.o mipsPipelined.o safeops.o

register_file.o: register_file.cpp
	$(CC) $(FLAGS) $^ -c 

processor.o: processor.cpp
	$(CC) $(FLAGS) $^ -c

mipsPipelined.o: mipsPipelined.cpp
	$(CC) $(CFLAGS) $^ -c

safeops.o: safeops.cpp
	$(CC) $(FLAGS) $^ -c

clean:
	rm *.o
