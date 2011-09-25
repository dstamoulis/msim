#project's makefile

all: main

CC=g++
FLAGS=-Wall -O3 -g


register_file.o: register_file.cpp
	$(CC) $(FLAGS) $^ -c 

memory.o: memory.cpp
	$(CC) $(FLAGS) $^ -c

processor.o: processor.cpp
	$(CC) $(FLAGS) $^ -c

mipsPipelined.o: mipsPipelined.cpp
	$(CC) $(CFLAGS) $^ -c

safeops.o: safeops.cpp
	$(CC) $(FLAGS) $^ -c

main.o: main.cpp
	$(CC) $(FLAGS) $^ -c

main: memory.o register_file.o mipsPipelined.o processor.o safeops.o main.o
	$(CC) $(FLAGS) $^ -o $@




clean:
	rm *.o
