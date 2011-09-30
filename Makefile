#project's makefile

all: main

GUI_DIR= ./gui/
PROC_DIR= ./processor/
MEM_DIR= ./memory/

CC=g++
FLAGS=-Wall -O3 -g

main: $(MEM_DIR)memory.o $(PROC_DIR)register_file.o $(PROC_DIR)mipsPipelined.o $(PROC_DIR)processor.o $(PROC_DIR)safeops.o main.o
	$(CC) $(FLAGS) $^ -o $@

main.o: main.cpp
	$(CC) $(FLAGS) $^ -c

clean:
	cd $(PROC_DIR); make clean
	cd $(MEM_DIR); make clean
	rm main
