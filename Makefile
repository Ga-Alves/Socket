all:
	gcc client.c common.c -o client
	gcc server.c common.c -o server

rm:
	rm server client

client_test:
	./client 127.0.0.1 51511 
server_test:
	./server v4 51511 -i input/in.txt