// Client side implementation of UDP client-server model 
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

struct socket_data
{
    int sock;
    struct sockaddr_in destino;
};
typedef struct socket_data socket_data;

int meuSocket(_Bool destino,char *servip)
{
    if(destino==0)
    {
        int sockfd;
        char *SERVERPORT="5001";
        struct addrinfo hints, *servinfo, *p;
        int rv;
        int numbytes;

        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_DGRAM;

        if ((rv = getaddrinfo(servip, SERVERPORT, &hints, &servinfo)) != 0) {
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
            return 1;
        }

        //  Percorrer todos os resultados e criar socket
        for(p = servinfo; p != NULL; p = p->ai_next) {
            if ((sockfd = socket(p->ai_family, p->ai_socktype,
                    p->ai_protocol)) == -1) {
                perror("talker: socket");
                continue;
            }

            break;
        }

        if (p == NULL) {
            fprintf(stderr, "talker: falha ao criar o socket\n");
            return 2;
        }
        return sockfd;
    }
    else
    {
        int sockfd;
        char *MYPORT="5002";
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
    
}
void meuListen()
{
    printf("Iniciando escuta\n");
}
struct sockaddr_in meuConnect(int sockOri,char *ip,int PORT)
{
    int confirm=-1,len;
    char buffer[MAXLINE];
    char *conn="SYN";
    struct sockaddr_in comutador;

    memset(&comutador, 0, sizeof(comutador)); 
    comutador.sin_family = AF_INET;
    //comutador.sin_addr.s_addr=INADDR_ANY;
    inet_aton(ip,&comutador.sin_addr);
    comutador.sin_port = htons(PORT);

    printf("Conectando...\n");

    sendto(sockOri,(const char *)conn,strlen(conn),MSG_CONFIRM,(const struct sockaddr *)&comutador,sizeof(comutador));
    
    //while (confirm<0)
        confirm=recvfrom(sockOri,(char *)buffer,MAXLINE,MSG_WAITALL,(struct sockaddr *)&comutador,&len);
    
    buffer[confirm]='\0';
    printf("Conexão estabelecida! %s\n",buffer);
    return comutador;
}
struct sockaddr_in meuAccept(int sockDest)
{
    int confirm=-1,len;
    char buffer[MAXLINE];
    char *conn="SYNACK";
    struct sockaddr_in origem;
    memset(&origem, 0, sizeof(origem));
    len=sizeof(origem);
    printf("Esperando conexão\n");

        confirm=recvfrom(sockDest,(char *)buffer,MAXLINE,MSG_WAITALL,(struct sockaddr *)&origem,&len);
    buffer[confirm]='\0';
    printf("Conexão pedida %s\n",buffer);

    sendto(sockDest,(const char *)conn,strlen(conn),MSG_CONFIRM,(const struct sockaddr *)&origem,sizeof(origem));

    printf("Conexão aceita\n");
    return origem;
}
void meuSend(int sock,struct sockaddr_in dest,char *msg)
{
    sendto(sock,(const char *)msg,strlen(msg),MSG_CONFIRM,(const struct sockaddr *)&dest,sizeof(dest));
}

void meuRecv(int sock)
{
    int len,confirm=-1;
    char buffer[MAXLINE];
    struct sockaddr_in ori;
    memset(&ori,0,sizeof(ori));
    len=sizeof(ori);

    //while (confirm<0)
        confirm=recvfrom(sock,(char *)buffer,MAXLINE,MSG_WAITALL,(struct sockaddr *)&ori,&len);
    buffer[confirm]='\0';
    printf("Recebido: %s\n",buffer);
}
void origem()
{
    int sock;
    char *ip="192.168.1.6";
    struct sockaddr_in dest;
    char *msg="TESTE";
    sock=meuSocket(0,ip);
    dest=meuConnect(sock,ip,5001);
    meuSend(sock,dest,msg);
    meuRecv(sock);
    close(sock);
}

void destino()
{
    int sock;
    struct sockaddr_in ori;
    
    char *msg="TESTADO";
    sock=meuSocket(1,"");
    meuListen();
    ori=meuAccept(sock);
    meuRecv(sock);
    meuSend(sock,ori,msg);
    close(sock);
}

int main()
{
    int opt;
    printf("Digite 1 para origem ou 2 para destino:\n");
    scanf(" %d",&opt);
    if(opt==1)
        origem();
    else
    {
        destino();
    }

}
