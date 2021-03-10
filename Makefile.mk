CC = g++
CFLAGS = -pthread -std=c++11

main: main.o bazaar.o
	$(CC) $(CFLAGS) -o main main.o bazaar.o

main.o: main.cpp bazaar.h
	$(CC) $(CFLAGS) -c main.cpp

bazaar.o: bazaar.h

