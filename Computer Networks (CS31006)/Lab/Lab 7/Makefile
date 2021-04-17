rum: user1 user2 
	rm rsocket.o

user1: user1.c libsocket.a
	gcc user1.c -o user1 -L. -lsocket -lpthread

user2: user2.c libsocket.a
	gcc user2.c -o user2 -L. -lsocket -lpthread

libsocket.a: rsocket.o
	ar -rcs libsocket.a rsocket.o

socket.o: rsocket.h
	gcc -Wall -c rsocket.h

clean:
	rm user1 user2 libsocket.a