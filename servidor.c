#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <inttypes.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#define ESTUDANTES_MAX 1024

void logexit(const char *str)
{
    perror(str);
    exit(EXIT_FAILURE);
}

void addrtostr(const struct sockaddr *addr, char *str, size_t strsize)
{
    char addrstr[INET_ADDRSTRLEN + 1] = "";
    uint16_t port;
    if (addr->sa_family == AF_INET)
    {
        struct sockaddr_in *addr = (struct sockaddr_in *)addr;
        if (!inet_ntop(AF_INET, &(addr->sin_addr), addrstr, INET_ADDRSTRLEN + 1))
        {
            logexit("ntop");
        }
        port = ntohs(addr->sin_port);
    }
    if (str)
    {
        snprintf(str, strsize, "%s %hu", addrstr, port);
    }
}

int main(int argc, char *argv[])
{
    int i, s, ret = 1;
    char addrstr[512];
    int matriculas[ESTUDANTES_MAX];
    int num_estudantes;
    char* matricula;    
    //criando o socket do servidor
    int serverfd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverfd == -1) logexit("TIMEOUT: SOCKET");
    
    struct in_addr inaddr;
    inet_pton(AF_INET, "127.0.0.1", &inaddr);
    uint16_t port = (uint16_t)atoi(argv[1]);
    printf("Senha do aluno: %s\n", argv[2]);
    printf("Senha do professor: %s\n", argv[3]);

    struct sockaddr_in server;
    struct sockaddr *serverptr = (struct sockaddr *)&server;
    serverptr = (struct sockaddr *)malloc(sizeof(struct sockaddr));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr = inaddr;
    memset(server.sin_zero, 0x0, 8);

    if (bind(serverfd, (struct sockaddr *)&server, sizeof(struct sockaddr_in)) == -1) logexit("TIMEOUT: BIND");
    if (listen(serverfd, 10)) logexit("TIMEOUT: LISTEN");

    addrtostr(serverptr, addrstr, 512);
    printf("bound to %s, waiting connections\n", addrstr);

    while (1){
        //criando o socket do client
        struct sockaddr_in client;
        struct sockaddr *clientptr = (struct sockaddr *)&client;
        clientptr = (struct sockaddr *)malloc(sizeof(struct sockaddr));
        socklen_t client_len = sizeof(struct sockaddr_in);

        int clientfd = accept(serverfd, (struct sockaddr *)&client, &client_len);
        if (clientfd == -1)
            logexit("TIMEOUT: ACCEPT");

        char caddrstr[512];
        addrtostr(clientptr, caddrstr, 512);
        printf("connection from %s\n", &caddrstr);

        char buffer[512];
        memset(buffer, 0x0, 512);
        char ipcliente[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client.sin_addr), ipcliente, 512);

        ssize_t count = send(clientfd, "READY", 6, 0);
        int message_len = recv(clientfd, buffer, 512, 0); //recebe senha
        if (strcmp(buffer, argv[2]) == 0){ //case aluno: senha dos alunos eh passada pela linha de comando
            
            send(clientfd, "OK", strlen("OK") + 2, 0);
            send(clientfd, "MATRICULA", strlen("MATRICULA") + 2, 0);

            int32_t ret;
            char *data = (char *)&ret; //cast de data para int
            int left = sizeof(ret);

            memset(buffer, 0x0, 512);
            message_len = recv(clientfd, data, left, MSG_WAITALL); //recebe matricula

            send(clientfd, "OK", 2, 0);
            matriculas[num_estudantes] = ntohl(ret);
            //printf("Matricula do aluno: %d", matriculas[num_estudantes]);
            num_estudantes++;

        }
        else if (strcmp(buffer, argv[3]) == 0){   //case professor: senha do professor eh passada pela linha de comando
            
            for (i = 0; i < num_estudantes; i++){
                //printf("%d", matriculas[i]);
                sprintf(matricula, "%d", matriculas[i]);
                //strcat(matricula, "\n");
                //puts(matricula);
                send(clientfd, matricula, strlen(matricula)+1, 0);
            }
            send(clientfd, "\0", 2, 0);
            do{
                memset(buffer, 0x0, 512);
                recv(clientfd, buffer, 512, 0);
            } while (strcmp(buffer, "OK"));

        }
        else if (strcmp(buffer, "END") == 0){ //case end: fechar o servidor e apagar a lista
            close(clientfd);
            break;
        }
        //encerra a comunicacao com o cliente
        close(clientfd);
    }
    return 0;
}