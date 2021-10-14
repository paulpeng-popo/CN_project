#include <stdio.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <dirent.h>
#include "Library.h"

extern char* name;
extern char package[PACKET_HEAD+MSS];

int DirList(char* path)
{
    int notfirst = 0;
    DIR* dp = opendir(path);
    if (!dp)
    {
        strcat(((Segment*)package)->data, path);
        return 1;
    }
    struct dirent* filenames;
    while ((filenames = readdir(dp)))
    {
        if (!strcmp(filenames->d_name, "server")) continue;
        if (notfirst) strcat(((Segment*)package)->data, "  ");
        if(!strcmp(filenames->d_name, "..") || !strcmp(filenames->d_name,".")) continue;
        int pathLength = strlen(filenames->d_name);
        char* pathStr = (char*)malloc(sizeof(char)*pathLength);
        bzero(pathStr, sizeof(char)*pathLength);
        strcat(pathStr, filenames->d_name);
        notfirst = 1;
        DirList(pathStr);
    }
    closedir(dp);
    return 1;
}

int Hostname2ip(char* hostname, char* ip)
{
    struct hostent *he;
	struct in_addr **addr_list;
	int i;

	if ( (he = gethostbyname( hostname ) ) == NULL)
	{
		// get the host info
		herror("gethostbyname");
		return 1;
	}

	addr_list = (struct in_addr **) he->h_addr_list;

	for(i = 0; addr_list[i] != NULL; i++)
	{
		//Return the first one;
		strcpy(ip , inet_ntoa(*addr_list[i]) );
		return 0;
	}

	return 1;
}

void Server_action(LinkInfo* LKIF)
{
    int action;
    while(1)
    {
        ReceivePacket(LKIF, &(LKIF->mirror));
        action = atoi(((Segment*)package)->data);
        if (action == 1)
        {
            bzero(((Segment*)package)->data, sizeof(((Segment*)package)->data));
            DirList(".");
            SendPacket(LKIF, &(LKIF->mirror));
            ReceivePacket(LKIF, &(LKIF->mirror));

            char filename[20];
            strcpy(filename, ((Segment*)package)->data);
            FILE* fp = fopen(filename, "rb+");
            fseek(fp, 0L, SEEK_END);
            int size = ftell(fp);
            fclose(fp);
            sprintf(((Segment*)package)->data, "%d", size);
            SendPacket(LKIF, &(LKIF->mirror));
            TCPSendFile(LKIF, filename, size);
            bzero(((Segment*)package)->data, MSS);
        }
        else if (action == 2)
        {
            bzero(((Segment*)package)->data, MSS);
            char answer[100];
            char expression[100];
            SendPacket(LKIF, &(LKIF->mirror));
            ReceivePacket(LKIF, &(LKIF->mirror));
            strcpy(expression, ((Segment*)package)->data);

            float a = 0;
            float b = 0;
            float res = 0;
            char operator[10];
            char* substr = NULL;
            char* delim = "\n ";
            printf("\033[0;36m[%s]\033[0m: Math computing:", name);
            substr = strtok(expression, delim);
            printf(" %s", substr);
            a = atof(substr);
            substr = strtok(NULL, delim);
            printf(" %s", substr);
            sprintf(operator, "%s", substr);
            substr = strtok(NULL, delim);
            if (substr)
            {
                if (strcmp(substr, "\n"))
                {
                    printf(" %s", substr);
                    b = atof(substr);
                }
            }

            if (!strcmp(operator, "+"))
                res = a + b;
            else if (!strcmp(operator, "-"))
                res = a - b;
            else if (!strcmp(operator, "*"))
                res = a * b;
            else if (!strcmp(operator, "/"))
                res = a / b;
            else if (!strcmp(operator, "**"))
                res = pow(a, b);
            else if (!strcmp(operator, "sqrt2"))
                res = pow(a, 0.5);
            snprintf(answer, sizeof(answer), "%f", res);
            printf(" = %f\n", res);
            bzero(((Segment*)package)->data, MSS);
            strcpy(((Segment*)package)->data, answer);
            SendPacket(LKIF, &(LKIF->mirror));
        }
        else if (action == 3)
        {
            char hostname[100];
            char ip[100];
            SendPacket(LKIF, &(LKIF->mirror));
            ReceivePacket(LKIF, &(LKIF->mirror));
            strcpy(hostname, ((Segment*)package)->data);
            printf("\033[0;36m[%s]\033[0m: Resolving the hostname: %s\n", name, hostname);
            Hostname2ip(hostname, ip);
            strcpy(((Segment*)package)->data, ip);
            SendPacket(LKIF, &(LKIF->mirror));
        }
    }
}

void Client_action(LinkInfo* LKIF)
{
    char action[20];

    while(1)
    {
        printf("\n");
        printf("\033[0;36m[%s]\033[0m: What do you want to do?\n", name);
        printf("\033[0;36m\t[1]\033[0m: Download a video.\n");
        printf("\033[0;36m\t[2]\033[0m: Cloud Computing.\n");
        printf("\033[0;36m\t[3]\033[0m: DNS resolving.\n");
        printf("\033[0;36m[%s]\033[0m: Choose number: ", name);
        if (scanf("%s", action));
        if (!strcmp(action, "quit")) break;
        Segment segment;
        segment.ack_seq = ((Segment*)package)->ack_seq;
        segment.seq = ((Segment*)package)->seq;
        strcpy(segment.data, action);
        make_packet(package, segment);
        SendPacket(LKIF, &(LKIF->mirror));
        ReceivePacket(LKIF, &(LKIF->mirror));
        int select = atoi(action);
        if (select == 1)
        {
            char filename[20];
            printf("\033[0;36m[%s]\033[0m: File list\n", name);
            printf("%s\n", ((Segment*)package)->data);
            printf("\033[0;36m[%s]\033[0m: Choose one file: ", name);
            if (scanf("%s", filename));
            strcpy(((Segment*)package)->data, filename);
            SendPacket(LKIF, &(LKIF->mirror));
            ReceivePacket(LKIF, &(LKIF->mirror));
            int filesize = atoi(((Segment*)package)->data);
            TCPReceiveFile(LKIF, filename, filesize);
            bzero(((Segment*)package)->data, MSS);
        }
        else if (select == 2)
        {
            char buff[100];
            bzero(((Segment*)package)->data, MSS);
            printf("\033[0;36m[%s]\033[0m: Operator(+, -, *, /, **, sqrt2)\n", name);
            printf("\033[0;36m[%s]\033[0m: Equation: ", name);
            getchar();
            if (fgets(buff, 100, stdin));
            strcpy(segment.data, buff);
            make_packet(package, segment);
            SendPacket(LKIF, &(LKIF->mirror));
            ReceivePacket(LKIF, &(LKIF->mirror));
            printf("\033[0;36m[%s]\033[0m: Answer is: ", name);
            printf("%s\n", ((Segment*)package)->data);
        }
        else if (select == 3)
        {
            bzero(((Segment*)package)->data, MSS);
            char domain[100];
            printf("\033[0;36m[%s]\033[0m: Input the domain name: ", name);
            if (scanf("%s", domain));
            strcpy(segment.data, domain);
            make_packet(package, segment);
            SendPacket(LKIF, &(LKIF->mirror));
            ReceivePacket(LKIF, &(LKIF->mirror));
            printf("\033[0;36m[%s]\033[0m: It's IP address: ", name);
            printf("%s\n", ((Segment*)package)->data);
        }
    }
}

void TCPSendFile(LinkInfo* LKIF, char filename[20], int size)
{
    printf("\033[0;36m[%s]\033[0m: Start to send a file \033[0;35m%s\033[0m to \033[0;32m%s\033[0m : \033[0;32m%hu\033[0m\n",
                name, filename, inet_ntoa(LKIF->mirror.sin_addr), ntohs(LKIF->mirror.sin_port));
    printf("\033[0;36m[%s]\033[0m: File size is %d bytes\n", name, size);

    FILE* fp = fopen(filename, "rb");
    Segment* psegment;
    char header[PACKET_HEAD];
    Segment segment = make_packetHeader(LKIF->address.sin_port, LKIF->mirror.sin_port, 1, ((Segment*)package)->seq+1, 1, 0, 0, MSS, BUFFER_SIZE);
    bzero(((Segment*)package)->data, MSS);

    int ret;
    int total = size;
    while(!feof(fp))
    {
        printf("\033[0;36m[%s]\033[0m: cwnd = %u, rwnd = %u, threshold = %d\n",
                    name, segment.cwnd, segment.rwnd, THRESHOLD);
        if (total >= segment.cwnd)
        {
            ret = fread(segment.data, sizeof(char), segment.cwnd, fp);
            total -= ret;
        }
        else
        {
            ret = fread(segment.data, sizeof(char), total, fp);
            total -= ret;
        }
        make_packet(package, segment);
        SendPacket(LKIF, &(LKIF->mirror));
        printf("\033[0;36m[%s]\033[0m: Send a packet at : %d bytes\n", name, total);
        ReceivePacket(LKIF, &(LKIF->mirror));
        psegment = get_packetHead(package, header);
        psegment->rwnd += psegment->cwnd;
        if (total == 0) break;
        segment = make_packetHeader(LKIF->address.sin_port, LKIF->mirror.sin_port,
                                psegment->ack_seq, psegment->seq+1, 1, 0, 0, psegment->cwnd, psegment->rwnd);
    }
    printf("\033[0;36m[%s]\033[0m: Finish transmission\n", name);
    fclose(fp);
}

void TCPReceiveFile(LinkInfo* LKIF, char filename[20], int size)
{
    FILE* fp = fopen(filename, "wb");
    printf("\033[0;36m[%s]\033[0m: Receive a file \033[0;35m%s\033[0m from \033[0;32m%s\033[0m : \033[0;32m%hu\033[0m\n",
                name, filename, inet_ntoa(LKIF->mirror.sin_addr), ntohs(LKIF->mirror.sin_port));

    Segment segment;
    Segment* psegment;
    char header[PACKET_HEAD];

    int recv_bytes;
    int total = size;
    while(1)
    {
        ReceivePacket(LKIF, &(LKIF->mirror));
        psegment = get_packetHead(package, header);
        if (total >= psegment->cwnd)
        {
            recv_bytes = fwrite(((Segment*)package)->data, sizeof(char), psegment->cwnd, fp);
            total -= recv_bytes;
        }
        else
        {
            recv_bytes = fwrite(((Segment*)package)->data, sizeof(char), total, fp);
            total -= recv_bytes;
        }
        segment = make_packetHeader(LKIF->address.sin_port, LKIF->mirror.sin_port, psegment->ack_seq,
                        (psegment->seq)+1, 1, 0, 0, psegment->cwnd, (psegment->rwnd)-recv_bytes);
        make_packet(package, segment);
        SendPacket(LKIF, &(LKIF->mirror));
        if (total == 0) break;
    }
    printf("\033[0;36m[%s]\033[0m: Finish transmission\n", name);
    printf("\033[0;36m[%s]\033[0m: Total file size is %d bytes\n", name, size);
    fclose(fp);
}

Segment* get_packetHead(char* package, char* header)
{
    memcpy(header, package, PACKET_HEAD);
    printf("\t\033[0;33mReceived a packet (seq_num = %u , ack_num = %u)\033[0m\n",
                ((Segment*)header)->seq, ((Segment*)header)->ack_seq);
    return (Segment*)header;
}

void make_packet(char* package, Segment s)
{
    memcpy(package, &s, sizeof(s));
}

Segment make_packetHeader(short srcp, short destp, unsigned seq, unsigned ack_num, int ack, int syn, int fin, unsigned cwnd, unsigned rwnd)
{
    Segment temp;
    temp.src_port = srcp;
    temp.dest_port = destp;
    temp.seq = seq;
    temp.ack_seq = ack_num;
    temp.ack = ack;
    temp.syn = syn;
    temp.fin = fin;
    temp.cwnd = cwnd;
    temp.rwnd = rwnd;
    return temp;
}

// confirm the parameters to build socket
void ShowState(LinkInfo* LKIF)
{
    int i = 0;
    printf("\n");
    SYSTEM(LKIF->user_code, "setting up parameters");
    printf(WHITE);
    for (i = 0; i < 10; i++) printf("=");
    printf(" parameter ");
    for (i = 0; i < 10; i++) printf("=");
    printf("\n");
    printf(YELLOW); printf(" The RTT delay:     "); printf(NONE); printf("%d ms\n", RTT);
    printf(YELLOW); printf(" The Threshold:     "); printf(NONE); printf("%d Kbytes\n", THRESHOLD/1024);
    printf(YELLOW); printf(" The MSS:           "); printf(NONE); printf("%d bytes\n", MSS);
    printf(YELLOW); printf(" The Buffer size:   "); printf(NONE); printf("%d Kbytes\n", BUFFER_SIZE/1024);
    printf(YELLOW); printf(" The IP address:    "); printf(NONE); printf("%s\n", inet_ntoa(LKIF->address.sin_addr));
    if (LKIF->user_code) // client
        printf("\033[0;33m You're using port: \033[0m%hu\n", ntohs(LKIF->address.sin_port));
    else // server
        printf("\033[0;33m Listening on port: \033[0m%hu\n", ntohs(LKIF->address.sin_port));
    for (i = 0; i < 31; i++) printf("=");
    printf("\n");
}

void Initialize(int code, int np, char* cmdl[], LinkInfo* LKIF)
{
    // initialization
    struct sockaddr_in temp;
    LKIF->user_code = code;
    char* exefile = cmdl[0];
    if (code) // client
    {
        temp.sin_port = htons(8700);
        temp.sin_addr.s_addr = inet_addr("127.0.0.1");
    }
    else // server
    {
        temp.sin_port = htons(8080);
        temp.sin_addr.s_addr = INADDR_ANY;
    }

    if (np < 2) // 1 argument
    {
        SYSTEM(code, "using default ip & port");
        LKIF->address.sin_port = temp.sin_port;
        LKIF->address.sin_addr.s_addr = temp.sin_addr.s_addr;
    }
    else if (np == 2) // 2 arguments
    {
        char* arg = cmdl[1];
        // if argument is "--help"
        if (!strcmp(arg, "--help"))
        {
            // print usage
            fprintf(stderr, "\n\033[0;36musage:\t \033[0;35m%s [ ip_addr ] [ port ]\033[0m\n\n", exefile);
            fprintf(stderr, "\t\033[0;33mip_addr:\033[0m you can specify an ipv4 address\n");
            fprintf(stderr, "\t\033[0;33mport:   \033[0m you can specify your own port\n\n");
            exit(0);
        }
        else if (strchr(arg, '.') != NULL)
        {
            LKIF->address.sin_port = temp.sin_port;
            LKIF->address.sin_addr.s_addr = inet_addr(arg);
        }
        else
        {
            LKIF->address.sin_port = htons(atoi(arg));
            LKIF->address.sin_addr.s_addr = temp.sin_addr.s_addr;
        }
    }
    else if (np == 3) // 3 arguments
    {
        char* arg1 = cmdl[1];
        char* arg2 = cmdl[2];
        LKIF->address.sin_port = htons(atoi(arg2));
        LKIF->address.sin_addr.s_addr = inet_addr(arg1);
    }
    else // 4 arguments above
    {
        fprintf(stderr, "%s:\t \033[0;31mToo many arguments, try '%s --help' for help\033[0m\n", exefile, exefile);
        exit(0);
    }
}
