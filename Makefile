all:
	gcc client.c common.c -o ./bin/client
	gcc server.c common.c -o ./bin/server

rm:
	rm ./bin/*

client_v4:
	./bin/client 127.0.0.1 51511 
server_v4:
	./bin/server v4 51511 -i input/in.txt

client_v6:
	./bin/client ::1 51511 
server_v6:
	./bin/server v6 51511 -i input/in.txt