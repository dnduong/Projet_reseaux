CC=gcc
CFLAGS= -std=c11 -Wall -g 
LDFLAGS= -pthread -lm
all: main

main : main.o ll.o message.o voisin.o data.o
main.o : main.c ll.h data.h voisin.h message.h
data.o : data.c data.h 
voisin.o : voisin.c voisin.h
ll.o : ll.c ll.h
message.o : message.c message.h

clean:
	rm -rf *.o
