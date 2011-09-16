#project's makefile

all: register_file.o memory.o processor.o mipsPipelined.o

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

msim: memory.o register_file.o
	$(CC) $(FLAGS) $^ -o $@


clean:
	rm *.o
