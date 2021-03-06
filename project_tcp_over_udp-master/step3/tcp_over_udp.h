#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/errno.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

//Parameters
#define RTT		    (150)
#define MSS     	(1024)
#define THRESHOLD	(65535)
#define BUFF_SIZE	(32*1024)
#define SERVER_ADDR "127.0.0.1"

//Code
#define FIN	1
#define SYN	2
#define ACK	16

#define LOST_R              (1)
#define MAX_NUM_CLIENTS     (200)
#define HEADER_SIZE         (sizeof(TCP_header))
#define MAX_DATA_SIZE       (MSS-HEADER_SIZE)
#define MAX_FILE_NAME_LEN   (100)
#define MAX_FILE_LEN        (300*1024)   
#define ERR_EXIT(m) \
    do { \
    perror(m); \
    exit(EXIT_FAILURE); \
    } while (0)

typedef struct {
    unsigned short head_len:5;
    unsigned short file_request_c:4;
    unsigned short final_seg:1;
    unsigned short U:1;
    unsigned short A:1;
    unsigned short P:1;
    unsigned short R:1;
    unsigned short S:1;
    unsigned short F:1;
}flag_code;

//Header structure
typedef struct {
        uint16_t source_port,des_port;
        uint32_t seq_num;
        uint32_t ack_num;
        flag_code code;
        uint16_t rcv_win;
        uint16_t checksum,urg_data_p;
}TCP_header;

//Segement structure
typedef struct{
        TCP_header header;
        char data[MAX_DATA_SIZE];
}seg;

//Structure that is pushed into queue
typedef struct{
    struct sockaddr_in addr;
    seg s;
}dgram;

//For the establishment of connection
typedef struct{
    dgram the_dgram;
    int sockfd;
}new_connection_p;



