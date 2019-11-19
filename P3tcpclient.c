#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<string.h>
#include<stdlib.h>
#include<pthread.h>
#define SERVER_PORT 5432
#define MAX_LINE 256
#define MAXNAME 256
int main(int argc, char* argv[])
{
    
    struct hostent *hp;
    struct sockaddr_in sin;
    char *host;
    char buf[MAX_LINE];
    char *uName;
    int s;
    int len;
    int gnum;
    pthread_t threads[1];
    if(argc == 3){
        host = argv[1];
        uName = argv[2];
    }
    else{
        fprintf(stderr, "usage:newclient server   Username\n");
        exit(1);
    }
    /* translate host name into peer's IP address */
    hp = gethostbyname(host);
    if(!hp){
        fprintf(stderr, "unkown host: %s\n", host);
        exit(1);
    }
    /* active open */
    if((s = socket(PF_INET, SOCK_STREAM, 0)) < 0){
        perror("tcpclient: socket");
        exit(1);
    }
    //printf("socket: ");
    printf("local socket number %d\n", s);
    printf("\n");
    /* build address data structure */
    bzero((char*)&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
    sin.sin_port = htons(SERVER_PORT);
    printf("port number %d\n", ntohs(sin.sin_port));
    if(connect(s,(struct sockaddr *)&sin, sizeof(sin)) < 0){
        perror("tcpclient: connect");
        close(s);
        exit(1);
    }
    
    /* packet */
    struct packet {
        short type;
        char uName[MAXNAME];
        char mName[MAXNAME];
        char data[MAX_LINE];
        short seqNumber;
        int group;
    };
    struct packet packet_reg;
    //confirmation packet
        struct packet packet_con;
    //chat packet
    struct packet packet_chat;
    struct packet packet_chatRe;
    /*packet to register */
    //char *clientname = gethostname();
    char *clientname = "ubuntu";
    //packet type
    packet_reg.type = htons(121);
    //copy clientname to machine name
    strcpy(packet_reg.mName,clientname);
    //copy username to packet
    strcpy(packet_reg.uName,uName);
    //send registration packet

    printf ("Enter a group number: ");
    scanf("%d", &gnum);
    packet_reg.group = htons(gnum);
    packet_chat.group = htons(gnum);
    if(send(s,&packet_reg,sizeof(packet_reg),0) < 0)
    {
        printf("\n Send failed\n");
        exit(1);
    }else
    {
        //Let's us know if packet was sent
        printf("\nRegisture Packet Sent\n");
        
        //printf("\n Confirmation packet recieved \n");
        struct packet rg2;
        rg2.type = htons(121);
        strcpy(rg2.mName,clientname);
        strcpy(rg2.uName,uName);
        if(send(s,&rg2,sizeof(rg2),0) < 0){
                    printf("\n Send failed\n");
                    exit(1);
            }else{
            printf("\nRegisture Packet Sent\n");
    
        if(send(s,&packet_reg,sizeof(packet_reg),0) < 0){
                printf("\n Send failed\n");
                    exit(1);
            }else{
            if(recv(s,&packet_con,sizeof(packet_con),0) <0)
                {
                    printf("\n Could not recieve confirmation packet \n");
                        exit(1);
                }else{
                printf("Recieved confirmation packet");
            }
        }
    
    //}
        }
    }
    void *chat_reciever()
    {
        while(1)
        {
            while(len = recv(s,&packet_chatRe,sizeof(packet_chatRe),0))
                        {
                    printf("\n Recieved chat response packet. Printing chat data: ");
                    fputs(packet_chatRe.data, stdout);
                }close(s);
        }
        pthread_exit(NULL);
    }
    pthread_create(&threads[0],NULL,chat_reciever,NULL);
    int newsock;
    /* main loop: get and send lines of text */
    while(1)
    {
        while(fgets(buf, sizeof(buf), stdin))
        {
                //gets user input
                buf[MAX_LINE-1] = '\0';
                len = strlen(buf) + 1;
                //build chat packet
                packet_chat.type = htons(131);
                strcpy(packet_chat.mName, clientname);
                strcpy(packet_chat.uName, uName);
                //copies data from buffer to packet
                strcpy(packet_chat.data, buf);
                //send chat packet
                if(send(s,&packet_chat,sizeof(packet_chat),0)<0)
                {
                    printf("\n Chat packet send failed\n");
                }else
                {
                    printf("\nChat Packet send success\n");
                }
        }
    }
}