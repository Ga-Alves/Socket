all:
	gcc client.c common.c -o client
	gcc server.c common.c -o server

rm:
	rm server client

client_v4:
	./client 127.0.0.1 51511 
server_v4:
	./server v4 51511 -i input/in.txt

client_v6:
	./client ::1 51511 
server_v6:
	./server v6 51511 -i input/in.txt