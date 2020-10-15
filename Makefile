p4client.o: p4client.c
	gcc -o p4client p4client.c -w -std=c99
	gcc -o p4server p4server.c -w -std=c99

clean:
	rm *.o output