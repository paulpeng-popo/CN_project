CC = gcc
CFLAG = -c -Wall -O2 -g
PACKET = Packet
API = SocketAPI
SERVER = Server
CLIENT = Client
SERV = ../Server_edge/server
CLI = ../Client_edge/client

all: $(SERVER).c $(CLIENT).c $(SERV) $(CLI)
	@ echo "Compiled done."
	@ echo "Try \"./server --help\" for help"
	@ echo "Try \"./client --help\" for help"
	@ cp $(CLI) ../Client_edge2/
#	@ ../Server_edge/server
#	@ ../Client_edge/client 12345
#	@ ../Client_edge2/client 23456

$(SERV): $(SERVER).o $(API).o $(PACKET).o
	@ $(CC) $(SERVER).o $(API).o $(PACKET).o -lm -o $@

$(CLI): $(CLIENT).o $(API).o $(PACKET).o
	@ $(CC) $(CLIENT).o $(API).o $(PACKET).o -lm -o $@

.c.o:
	@ $(CC) $(CFLAG) $< -o $@

dep:
	@ $(CC) -M *.c > .depend

clean:
	@ rm -rf $(SERV) $(CLI) ../Client_edge*/client ../Client_edge*/*.mp4 *.o .depend
