#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<string.h>
#include<stdlib.h>
#include<pthread.h>
#include<fcntl.h>
#include<unistd.h>
#define SERVER_PORT 5432
#define MAX_LINE 256
#define MAXNAME 256
#define MAX_PENDING 5
#define MAX_QUEUE 100
int main(int argc, char* argv[])
{
    
    struct sockaddr_in sin;
    struct sockaddr_in clientAddr;
    char buf[MAX_LINE];
    int s, new_s;
    int len;
    int index = 0;
    
    /* setup passive open */
    if((s = socket(PF_INET, SOCK_STREAM, 0)) < 0){
        perror("tcpserver: socket");
        exit(1);
    }
    /* build address data structure */
    bzero((char*)&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(SERVER_PORT);
    /* registration table */
    struct registrationTable{
        int port;
        int sockid;
        int group;
        char mName[MAXNAME];
        char uName[MAXNAME];
    };
    struct registrationTable table[10];
    //packet structure
    struct packet 
    {
        short type;
        char uName[MAXNAME];
        char mName[MAXNAME];
        char data[MAX_LINE];
        short seqNumber;
        int group;
    };
    //register packet construction
    struct packet packet_reg;
    struct packet rg2;
    //confirmation packet construction
    struct packet packet_con;
    // chat packet
    struct packet packet_chat;
    char buffer[100];
    //chat response packet
    struct packet packet_chatRe;
    //threads for server
    pthread_t threads[25];
    int threadI = 1;
    //global mutex variable
    pthread_mutex_t my_mutex = PTHREAD_MUTEX_INITIALIZER;
    typedef struct Queue
{
        int capacity;
        int size;
        int front;
        int rear;
        struct packet packets[10];
}Queue;
Queue * createQueue(int maxElements, struct packet *p1)
{
        /* Create a Queue */
        Queue *Q;
        Q = (Queue *)malloc(sizeof(Queue));
        /* Initialise its properties */
        Q->packets;
        //Q->elements = (int *)malloc(sizeof(p1)*maxElements);
        Q->size = 0;
        Q->capacity = maxElements;
        Q->front = 0;
        Q->rear = -1;
        /* Return the pointer */
        return Q;
}
void Dequeue(Queue *Q)
{
        /* If Queue size is zero then it is empty. So we cannot pop */
        if(Q->size==0)
        {
                printf("Queue is Empty\n");
                return;
        }
        /* Removing an element is equivalent to incrementing index of front by one */
        else
        {
                Q->size--;
                Q->front++;
                /* As we fill elements in circular fashion */
                if(Q->front==Q->capacity)
                {
                        Q->front=0;
                }
        }
        return;
}
struct packet front(Queue *Q)
{
        if(Q->size==0)
        {
                printf("Queue is Empty\n");
                exit(0);
        }
        /* Return the element which is at the front*/
        return Q->packets[Q->front];
}
void Enqueue(Queue *Q,struct packet p1)
{
    /* If the Queue is full, we cannot push an element into it as there is no space for it.*/
    if(Q->size == Q->capacity)
    {
        printf("Queue is Full\n");
    }
    else
    {
        Q->size++;
        Q->rear = Q->rear + 1;
        /* As we fill the queue in circular fashion */
        if(Q->rear == Q->capacity)
        {
            Q->rear = 0;
        }
        /* Insert the element in its rear side */ 
        Q->packets[Q->rear] = p1;
    }
    return;
}
//create queue structure and construction packet.
struct packet constructPacket;
constructPacket.type = htons(111);
strcpy(constructPacket.mName,"clientname");
strcpy(constructPacket.uName,"uName");
strcpy(constructPacket.data, "Hello World");
constructPacket.group = htons(42);
Queue *Q = createQueue(10, &constructPacket);
//join threads method
void *join_handler(struct registrationTable *clientData)
{
    int newsock;
        
    //struct packet packet_reg;
    newsock = clientData->sockid;
    printf("sockid %d\n", newsock);
    packet_con.type = htons(221);
    if(send(newsock,&packet_con,sizeof(packet_con),0) < 0){
        printf("\n Send failed\n");
        exit(1);
    }
    if(recv(newsock,&rg2,sizeof(rg2),0)<0){
        printf("\n Could not receive. Line 84\n");
        exit(1);
    }else 
    {
        //wait for another reg packet from client
        if(recv(newsock,&packet_reg,sizeof(packet_reg),0)<0){
            printf("\n Could not receive. Line 90.\n");
            exit(1);
        }else
        {
            pthread_mutex_lock(&my_mutex);
            printf("\n===============================Register Table=========================\n");
            //update table
            table[index].port = clientData->port; 
            table[index].sockid = newsock;
            table[index].group = clientData->group;
            strcpy(table[index].uName, clientData->uName);
            printf("UserName is :");
            printf(table[index].uName);
            strcpy(table[index].mName, clientData->mName);
            printf("\nMachine name is :");
            printf(table[index].mName);
            printf("\nGroup number is :%d\n", table[index].group);
            int i;
            for(i=0; i < index; i++)
            {
                printf("table index is: %d\n", i);
                printf(table[i].mName);
                printf(table[i].uName);
                printf("sockid: %d \n", table[i].sockid);
            //send data packets to each client in table
            }
            pthread_mutex_unlock(&my_mutex);
            //send ack/con
            if(send(newsock,&packet_con,sizeof(packet_con),0) < 0)
            {
                printf("\n Send failed\n");
                exit(1);
            }
            printf("Confirmation packet sent");
        }
            
    }
        index ++;
        while(1)
        {
            if(recv(newsock,&packet_chat,sizeof(packet_chat),0)<0)
            {
            perror("chat: fail");
            exit(1);
            }else
            {
                
                struct packet qPacket;
                pthread_mutex_lock(&my_mutex);
                strcpy(qPacket.data, packet_chat.data);
                strcpy(qPacket.mName, packet_chat.mName);
                strcpy(qPacket.uName, packet_chat.uName);
                qPacket.group=packet_chat.group;
                printf(packet_chat.data);
                
                Enqueue(Q, packet_chat);
                pthread_mutex_unlock(&my_mutex);
            }
        }
        pthread_exit(NULL);
        
}
void *chat_multicaster()
{   
    //printf("inside multicast\n");
    int fd;
    int seqNum = 0;
    while(1){
        //check of any client is listed on table
        pthread_mutex_lock(&my_mutex);
        if(Q->size>0){
            //if there is at least one message in Queue
            if(index >0){
                printf("inside locked multicast\n");
                //if at least 1 client is listed read 100 bytes of data from the file and store in text
                strcpy(packet_chatRe.data, front(Q).data);
                packet_chatRe.group = front(Q).group;
                int i;
                for(i=0; i < index; i++)
                {
                    if(ntohs(packet_chatRe.group) == table[i].group)
                    {
                        //construct data packet
                        strcpy(packet_chatRe.mName, front(Q).mName);
                        //printf(packet_chatRe.mName);
                        strcpy(packet_chatRe.uName, front(Q).uName);
                        //printf(packet_chatRe.uName);
                        strcpy(packet_chatRe.data, front(Q).data);
                        //printf(packet_chatRe.data);
                        //packet_chatRe.seqNumber = htons(seqNum);
                        send(table[i].sockid, &packet_chatRe,sizeof(packet_chatRe),0);
                        //send data packets to each client in table
                    }
                }
                seqNum +=1;
                Dequeue(Q);
            }
        }
        pthread_mutex_unlock(&my_mutex);
        //sleep(3);
        }
}
    
    if(bind(s,(struct sockaddr *)&sin, sizeof(sin)) < 0){
        perror("tcpclient: bind");
        exit(1);
    }
    listen(s, MAX_PENDING);
    pthread_create(&threads[0],NULL,chat_multicaster,NULL);
    /* wait for connection, then receive and print text */
    while(1)
    {
        if((new_s = accept(s, (struct sockaddr *)&clientAddr, &len)) < 0)
        {
            perror("tcpserver: accept");
            exit(1);
        }
        /* recieve register packet*/
        if(recv(new_s,&packet_reg,sizeof(packet_reg),0) <0)
        {
                printf("\n Could not recieve first registration packet \n");
                exit(1);
        }else
        {
            //recieved packet. Add data to register table
            struct registrationTable client_info;
            client_info.port = ntohs(clientAddr.sin_port);
            client_info.sockid = new_s;
            client_info.group = ntohs(packet_reg.group);
            printf("group is : %d\n", client_info.group);
            strcpy(client_info.uName, packet_reg.uName);
            printf("UserName is :");
            printf(client_info.uName);
            strcpy(client_info.mName,packet_reg.mName);
            printf("\nMachine name is :");
            printf(client_info.mName);
            //join_handler thread
            if(threadI<=25)
            {
                pthread_create(&threads[threadI],NULL,join_handler,&client_info);
                threadI++;
            }else
            {
                printf("\nServer Full\n");
            }
                
        }
        
    
    }
}