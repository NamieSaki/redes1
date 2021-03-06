#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#define LISTEN_ENQ 5

void sendAnswer(char *buffer, int fd, int newsockfd){
    int m, n;

    while(1) {
        int a = sizeof(buffer);
        memset(buffer, 0, a);
        n = read(fd, buffer, a);
        if(n < 0) {
            fprintf(stderr, "ERROR: %s\n", strerror(errno));
            exit(1);    
        } else if(n == 0) {
            close(fd);
            break;
        } else {
            m = send(newsockfd, buffer, n, 0);
            while(m < n) {
                m += send(newsockfd, buffer + m, n - m, 0);
            }
        }
        
    }
}

int main(int argc, char** argv) {
    int n;
    int fd;
    int sockfd;
    int clilen;
    int newsockfd;
    char buffer[256];
    char *srch;
    //man 7 ip
    //man unix
    struct sockaddr_in serv_addr;
    struct sockaddr_in cli_addr;
  
    if(argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }
  
    //man socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
        exit(1);
    }
   
    memset((char*) &serv_addr, 0, sizeof(serv_addr));
  
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(atoi(argv[1]));
    //man bind
    if(bind(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
        exit(1);
    }
  
    //man listen
    if(listen(sockfd, LISTEN_ENQ) < 0) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
        exit(1);
    }

    clilen = sizeof(cli_addr);
    //man accept
    newsockfd = accept(sockfd, (struct sockaddr*) &cli_addr, (unsigned int*) &clilen);
    if(newsockfd < 0) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
        exit(1);
    }
    memset(buffer, 0, sizeof(buffer));
  
    //man recv
    n = recv(newsockfd, buffer, sizeof(buffer), 0);
    if(n < 0) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
        exit(1);
    }

    printf("Mensagem recebida: %s\n", buffer);

    buffer[n - 1] = 0;
    srch = strtok(buffer, " /");
    srch = strtok(NULL, " /");
    strcpy(buffer, srch);
    
    printf("Buscando pelo arquivo: %s...\n", buffer);

    fd = open(buffer, O_RDONLY);
    if(fd < 0) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
        strcpy(buffer, "error404.html");
        sendAnswer(buffer, fd, newsockfd);
        exit(1);    
    }

    printf("Enviando o arquivo...\n");
       
    sendAnswer(buffer, fd, newsockfd);

    printf("\nArquivo enviado.\n");

    close(newsockfd);

    return 0;
}
# redes1
