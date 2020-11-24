.PHONY: main

main: toy

toy: main.o queue.o
	g++ -g main.o queue.o -lpthread -o toy

main.o: main.cpp src/WorkQueue.hpp
	g++ -g -c main.cpp -o main.o

queue.o: src/WorkQueue.cpp src/WorkQueue.hpp
	g++ -g -c src/WorkQueue.cpp -o queue.o