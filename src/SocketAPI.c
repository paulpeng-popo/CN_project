#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "Library.h"

extern char* name;
extern char package[PACKET_HEAD+MSS];

// socket create
void CreateUDPsocket(LinkInfo* LKIF)
{
    (LKIF->sockfd) = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if ((LKIF->sockfd) == -1 || (LKIF->sockfd) < 0)
	{
		perror("\033[0;31mSocket creation failed\033[0m\n");
		exit(0);
	}
    (LKIF->address).sin_family = AF_INET;
    LKIF->address_len = sizeof(LKIF->address);
}

// bind socket with ip address and port
void Socket_bind(LinkInfo* LKIF)
{
    if ( bind( LKIF->sockfd,
               (const struct sockaddr*) &(LKIF->address),
               sizeof(LKIF->address) ) < 0 )
	{
        perror("\033[0;31mSocket bind failed\033[0m");
		exit(0);
	}
}

// make server to listen on certain port
void Server_listen(LinkInfo* LKIF)
{
    printf("\033[0;36m[%s]\033[0m: Listening\n", name);
    ThreeWayHandshake(LKIF, LKIF->mirror);
}

void Client_connect(LinkInfo* LKIF)
{
    char serverIP[20] = "127.0.0.1";
    unsigned serverPort = 8080;

    // user input destination ip and port
    // printf("\033[0;36m[%s]\033[0m: The server's ip: ", name);
    // while (scanf("%s", serverIP) != 1)
    // {
    //     fprintf(stderr, "ERROR:\t \033[0;31mInvalid ip\033[0m\n");
    //     printf("\033[0;36m[%s]\033[0m: The server's ip: ", name);
    // }
    // printf("\033[0;36m[%s]\033[0m: The server's port: ", name);
    // while (scanf("%u", &serverPort) != 1)
    // {
    //     fprintf(stderr, "ERROR:\t \033[0;31mInvalid port\033[0m\n");
    //     printf("\033[0;36m[%s]\033[0m: The server's port: ", name);
    // }

    LKIF->mirror.sin_addr.s_addr = inet_addr(serverIP);
    LKIF->mirror.sin_port = htons(serverPort);
    printf("\033[0;36m[%s]\033[0m: Connecting to \033[0;32m%s\033[0m : \033[0;32m%hu\033[0m\n",
                name, serverIP, serverPort);
    ThreeWayHandshake(LKIF, LKIF->mirror);
}

void ThreeWayHandshake(LinkInfo* LKIF, struct sockaddr_in addr)
{
    if (LKIF->user_code) // client
    {
        // make SYN packet
        srand(time(NULL));
        Segment segment = make_packetHeader(LKIF->address.sin_port, addr.sin_port, (short)(rand()%10000)+1, 0, 0, 1, 0, 0, 0);
        Segment* psegment;
        char header[PACKET_HEAD];
        make_packet(package, segment);
        SendPacket(LKIF, &addr);
        printf("Send a packet(SYN) to \033[0;32m%s\033[0m : \033[0;32m%hu\033[0m\n",
                    inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
        ReceivePacket(LKIF, &addr);
        printf("Received a packet(SYN/ACK) from \033[0;32m%s\033[0m : \033[0;32m%hu\033[0m\n",
                    inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
        psegment = get_packetHead(package, header);
        if (psegment->syn && psegment->ack && psegment->ack_seq == segment.seq + 1)
        {
            // make ACK packet
            segment = make_packetHeader(LKIF->address.sin_port, addr.sin_port, psegment->ack_seq, psegment->seq+1, 1, 0, 0, 0, 0);
            make_packet(package, segment);
            SendPacket(LKIF, &addr);
            printf("Send a packet(ACK) to \033[0;32m%s\033[0m : \033[0;32m%hu\033[0m\n",
                        inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
        }
        else
        {
            fprintf(stderr, "ERROR:\t \033[0;31mSYN or ACK bit not set\033[0m\n");
    		exit(0);
        }
        printf("\033[0;36m[%s]\033[0m: Connect successfully\n", name);
    }
    else // server
    {
        Segment segment;
        Segment* psegment;
        char header[PACKET_HEAD];
        ReceivePacket(LKIF, &addr);
        printf("Received a packet(SYN) from \033[0;32m%s\033[0m : \033[0;32m%hu\033[0m\n",
                    inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
        psegment = get_packetHead(package, header);
        srand(time(NULL)*79);
        if (psegment->syn)
        {
            // make SYN/ACK packet
            segment = make_packetHeader(LKIF->address.sin_port, psegment->src_port, (short)(rand()%10000)+1, psegment->seq+1, 1, 1, 0, 0, 0);
            make_packet(package, segment);
            SendPacket(LKIF, &addr);
            printf("Send a packet(SYN/ACK) to \033[0;32m%s\033[0m : \033[0;32m%hu\033[0m\n",
                        inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
        }
        else
        {
            fprintf(stderr, "ERROR:\t \033[0;31mSYN bit not set\033[0m\n");
    		exit(0);
        }
        ReceivePacket(LKIF, &addr);
        printf("Received a packet(ACK) from \033[0;32m%s\033[0m : \033[0;32m%hu\033[0m\n",
                    inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
        psegment = get_packetHead(package, header);
        printf("\033[0;36m[%s]\033[0m: Client enqueue \033[0;32m%s\033[0m : \033[0;32m%hu\033[0m\n\n",
                    name, inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
    }
    LKIF->mirror = addr;
}

int SendPacket(LinkInfo* LKIF, struct sockaddr_in* recv_addr)
{
    int bytes;
    bytes = sendto( LKIF->sockfd,
            (const char*) package,
            PACKET_HEAD+MSS,
            MSG_CONFIRM,
            (const struct sockaddr*) recv_addr,
            sizeof(*recv_addr) );
    return bytes;
}

int ReceivePacket(LinkInfo* LKIF, struct sockaddr_in* send_addr)
{
    int bytes;
    LKIF->mirror_len = sizeof(*send_addr);
    bytes = recvfrom( LKIF->sockfd,
              (char*) package,
              PACKET_HEAD+MSS,
              MSG_WAITALL,
              (struct sockaddr*) send_addr,
              &LKIF->mirror_len );
    return bytes;
}

// close socket file pointer
void CloseSocket(LinkInfo* LKIF)
{
    close(LKIF->sockfd);
}
