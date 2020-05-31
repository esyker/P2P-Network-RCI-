CC=gcc -g
SOURCES= main.c iamroot.c network_communication.c  protocol.c list.c
OBJECTS= main.o iamroot.o network_communication.o protocol.o list.o

iamroot:$(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS)


network_communication: network_communication.h

protocol: protocol.h

iamroot: iamroot.h

list:list.h

clean::
	rm -f *.o core a.out zeruns *~

depend::
	makedepend $(SOURCES)
