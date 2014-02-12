all: linkedlist.o
	gcc -Wall linkedlist.o client_PFS.c -o client1/client_PFS
	gcc -Wall linkedlist.o client_PFS.c -o client2/client_PFS
	gcc -Wall linkedlist.o client_PFS.c -o client3/client_PFS
	gcc server_PFS.c -o server_PFS

linkedlist.o: linkedlist.h

clean:
	rm -f client_PFS server_PFS linkedlist.o client1/client_PFS client2/client_PFS client3/client_PFS
