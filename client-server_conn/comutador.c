
// Server side implementation of UDP client-server model 
#include <stdio.h> 
#include <stdlib.h> 
#include <stdbool.h>
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <netdb.h>
  

#define MAXLINE 1024 
  
// Driver code

int start_comutador()
{
    int sockfd;
        char *MYPORT="5001";
        struct addrinfo hints, *servinfo, *p;
        int rv;
        int numbytes;
        struct sockaddr_storage their_addr;
        socklen_t addr_len;
        char s[INET6_ADDRSTRLEN];

        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC; // configure com AF_INET para forçar IPv4
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_flags = AI_PASSIVE; // use meu IP

        if ((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0) {
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
            return 1;
        }

        // loop através de todos os resultados e fazer bind para o primeiro que pudermos
        for(p = servinfo; p != NULL; p = p->ai_next) {
            if ((sockfd = socket(p->ai_family, p->ai_socktype,
                    p->ai_protocol)) == -1) {
                perror("listener: socket");
                continue;
            }

            if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
                close(sockfd);
                perror("listener: bind");
                continue;
            }

            break;
        }

        if (p == NULL) {
            fprintf(stderr, "listener: falha ao fazer bind para o socket\n");
            return 2;
        }

        freeaddrinfo(servinfo);
        return sockfd;
}
int start_dest()
{
    char *host="127.0.0.1";
    char *service="5002";
    struct addrinfo addrCriteria;
    memset(&addrCriteria, 0, sizeof(addrCriteria));
    addrCriteria.ai_family = AF_UNSPEC;
    addrCriteria.ai_socktype = SOCK_DGRAM;
    addrCriteria.ai_protocol = IPPROTO_UDP;

    struct addrinfo *servAddr;
    int rtnVal = getaddrinfo(host, service, &addrCriteria, &servAddr);
    if (rtnVal != 0)
    {
        {
            perror("getaddrinfo() falhou\n");
            exit(EXIT_FAILURE);
        }
    }

    int sock = -1;
    for (struct addrinfo *addr = servAddr; addr != NULL; addr = addr->ai_next)
    {
        sock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        if (sock < 0)
            continue;

        if (connect(sock, addr->ai_addr, addr->ai_addrlen) == 0)
            break;

        close(sock);
        sock = -1;
    }
    freeaddrinfo(servAddr);
    return sock;
}
void Connect(int sockOri,int PORT,char *conn)
{
    int confirm=-1,len;
    char buffer[MAXLINE];
    struct sockaddr_in destino;

    memset(&destino, 0, sizeof(destino)); 
    destino.sin_family = AF_INET;
    destino.sin_addr.s_addr=INADDR_ANY; // usar em outro pc é só por inet_aton(ip,&destino.sin_addr);
    destino.sin_port = htons(PORT);

    printf("Conectando...\n");

    sendto(sockOri,(const char *)conn,strlen(conn),MSG_CONFIRM,(const struct sockaddr *)&destino,sizeof(destino));
    confirm=recvfrom(sockOri,(char *)buffer,MAXLINE,MSG_WAITALL,(struct sockaddr *)&destino,&len);
    
    buffer[confirm]='\0';
    printf("Conexão estabelecida! %s\n",buffer);
}
void SEND(int sock,int port,char *msg)
{
    struct sockaddr_in dest;
    memset(&dest,0,sizeof(dest));
    dest.sin_family=AF_INET;
    dest.sin_addr.s_addr=INADDR_ANY;
    dest.sin_port=htons(port);

    sendto(sock,(const char *)msg,strlen(msg),MSG_CONFIRM,(const struct sockaddr *)&dest,sizeof(dest));
}
char RECV(int sock)
{
    int len,confirm=-1;
    char buffer[MAXLINE];
    struct sockaddr_in ori;
    memset(&ori,0,sizeof(ori));
    len=sizeof(ori);

    //while (confirm<0)
        confirm=recvfrom(sock,(char *)buffer,MAXLINE,MSG_WAITALL,(struct sockaddr *)&ori,&len);
    buffer[confirm]='\0';
    return *buffer;
}
void encaminhar(int sockOC,int sockCD)
{
    int confirm=-1,lenOC,lenCD,cont=0;
    char buffer[MAXLINE];
    struct sockaddr_in origem,destino;
    memset(&origem, 0, sizeof(origem));

    memset(&destino,0,sizeof(destino));
    destino.sin_family=AF_INET;
    destino.sin_addr.s_addr=INADDR_ANY;
    destino.sin_port=htons(5002);
    
    lenOC=sizeof(origem);
    printf("Esperando conexão...\n");

    while (cont<2)
    {
        //while(confirm<0)
            confirm=recvfrom(sockOC,(char *)buffer,MAXLINE,MSG_WAITALL,(struct sockaddr *)&origem,&lenOC);
        buffer[confirm]='\0';
        confirm=-1;

        sendto(sockCD,buffer,strlen(buffer),MSG_CONFIRM,(const struct sockaddr *)&destino,sizeof(destino));
        printf("Pedindo conexão\n");
        

            confirm=recvfrom(sockCD,(char *)buffer,MAXLINE,MSG_WAITALL,(struct sockaddr *)&destino,&lenCD);
        buffer[confirm]='\0';
        confirm=-1;
        printf("Recv %s\n",buffer);

        sendto(sockOC,buffer,strlen(buffer),MSG_CONFIRM,(const struct sockaddr *)&origem,sizeof(origem));
        cont++;
    }
}

int main()
{
    int sockOC,sockCD;
    sockOC=start_comutador();
    sockCD=start_dest();
    encaminhar(sockOC,sockCD);
}
