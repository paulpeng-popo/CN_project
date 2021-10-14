#ifndef __LIBRARY_H__
#define __LIBRARY_H__

#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

// color
#define NONE   "\033[0m"
#define RED    "\033[0;31m"
#define GREEN  "\033[1;32m"
#define YELLOW "\033[0;33m"
#define BLUE   "\033[1;34m"
#define PURPLE "\033[0;35m"
#define L_BLUE "\033[0;36m"
#define WHITE  "\033[1;37m"

// system variables
#define CLIENT 1
#define SERVER 0
#define RTT (15)
#define HEAD (20)
#define MSS (1024)
#define THRESHOLD (65536)
#define BUFFER_SIZE (512*1024)

#define MAX_CLIENTS (100)
#define MAX_DATA_SIZE (MSS-HEAD)
#define LOST_RATE (20)

// function marcos
#define ERROR(error_name) { perror(error_name); exit(1); }
#define SYSTEM(code, msg) { \
        char* name = code ? "Client" : "Server"; \
        printf(L_BLUE); printf("[%s]", name); \
        printf(NONE); printf(": %s\n", msg); \
    }

// process datagram
typedef struct LinkInfo {
    int user_code;
    int sockfd;
    struct sockaddr_in address;
    socklen_t address_len;
    struct sockaddr_in dest_addr;
    socklen_t dest_addr_len;
} LinkInfo;

// TCP Segment
// header size 20 bytes
typedef struct Segment {
    unsigned short src_port;	     // 2-bytes source port
	unsigned short dest_port;	     // 2-bytes destination port
	unsigned int   seq;		         // 4-bytes sequence number
	unsigned int   ack_seq;          // 4-bytes acknowledgement number
    unsigned short rwnd;             // 2-bytes receive window
    unsigned short head_len:4,       //  4-bit header length
                   not_use:6,        //  6-bit reserved
                   urg:1,            //  1-bit urgent flag
                   ack:1,            //  1-bit acknowledgement flag
                   psh:1,            //  1-bit push flag
                   rst:1,            //  1-bit reset flag
                   syn:1,            //  1-bit syn flag
                   fin:1;            //  1-bit finish flag
    unsigned short checksum;         // 2-bytes checksum
    unsigned short urg_ptr;          // 2-bytes urgent pointer
	char data[MAX_DATA_SIZE];        // application data
} Segment;

// SocketAPI.c
void CreateUDPsocket(LinkInfo* LKIF);
void Socket_bind(LinkInfo* LKIF);
void Server_listen(LinkInfo* LKIF);
void Client_connect(LinkInfo* LKIF);
void ThreeWayHandshake(LinkInfo* LKIF, struct sockaddr_in addr);
int SendPacket(LinkInfo* LKIF, struct sockaddr_in* recv_addr);
int ReceivePacket(LinkInfo* LKIF, struct sockaddr_in* send_addr);
void CloseSocket(LinkInfo* LKIF);
void FourWayHandshake(LinkInfo* LKIF, struct sockaddr_in addr);

// Packet.c
void Initialize(int code, int np, char* cmdl[], LinkInfo* LKIF);
Segment* get_packetHead(char* package, char* header);
void make_packet(char* package, Segment s);
Segment make_packetHeader(short srcp, short destp, unsigned seq, unsigned ack_num, int ack, int syn, int fin, unsigned cwnd, unsigned rwnd);
void ShowState(LinkInfo* LKIF);
int DirList(char* path);
void Server_action(LinkInfo* LKIF);
void Client_action(LinkInfo* LKIF);
void TCPSendFile(LinkInfo* LKIF, char filename[20], int size);
void TCPReceiveFile(LinkInfo* LKIF, char filename[20], int size);
int Hostname2ip(char* hostname, char* ip);

#endif
