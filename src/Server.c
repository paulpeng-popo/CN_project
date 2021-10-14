#include <stdio.h>
#include "Library.h"

int main(int argc, char* argv[])
{
	LinkInfo* serverPack = (LinkInfo*)malloc(sizeof(LinkInfo));
    Initialize(SERVER, argc, argv, serverPack);
	ShowState(serverPack);
    CreateUDPsocket(serverPack);
    Socket_bind(serverPack);
	Server_listen(serverPack);
	Server_action(serverPack);
	return 0;
}
