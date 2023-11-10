SOURCE_DIR = ./src
INCLUDE_DIR  = ./include
BIN_DIR = ./bin

all:
	gcc -I ${INCLUDE_DIR} ${SOURCE_DIR}/client.c ${SOURCE_DIR}/common.c -o ./bin/client
	gcc -I ${INCLUDE_DIR} ${SOURCE_DIR}/server.c ${SOURCE_DIR}/common.c ${SOURCE_DIR}/lista_de_topicos.c -o ${BIN_DIR}/server

rm:
	rm ./bin/*

client_v4:
	./bin/client 127.0.0.1 51511 
server_v4:
	./bin/server v4 51511

client_v6:
	./bin/client ::1 51511 
server_v6:
	./bin/server v6 51511
