CC = g++
CFLAGS = -pthread -std=c++11
CPPFLAGS = -pthread

main: main.o bazaar.o
	$(CC) $(CFLAGS) -o main main.o bazaar.o

main.o: main.cpp bazaar.h
	$(CC) $(CFLAGS) -c main.cpp

bazaar.o: bazaar.cpp bazaar.h
	$(CC) $(CFLAGS) -c bazaar.cpp

