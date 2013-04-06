# makefile

all: dataserver client

reqchannel.o: reqchannel.H reqchannel.C
	g++ -c -g reqchannel.C

dataserver: dataserver.C reqchannel.o 
	g++ -pthread -g -o dataserver dataserver.C reqchannel.o
	
client: client.c reqchannel.o
	g++ -pthread -g -o client client.c reqchannel.o