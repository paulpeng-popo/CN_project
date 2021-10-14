#include <stdio.h>
#include "Library.h"

int main(int argc, char* argv[])
{
    LinkInfo* clientPack = (LinkInfo*)malloc(sizeof(LinkInfo));
    Initialize(CLIENT, argc, argv, clientPack);
    ShowState(clientPack);
    CreateUDPsocket(clientPack);
    Socket_bind(clientPack);
    Client_connect(clientPack);
    Client_action(clientPack);
    // CloseSocket(clientPack);
    return 0;
}
