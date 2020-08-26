#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <inttypes.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

void logexit(const char *str){
	perror(str);
	exit(EXIT_FAILURE);
}

void addrtostr(const struct sockaddr *addr, char *str, size_t strsize){
	char addrstr[INET_ADDRSTRLEN + 1] = "";
	uint16_t port;
	if (addr->sa_family == AF_INET){
			struct sockaddr_in *addr = (struct sockaddr_in *)addr;
			if(!inet_ntop(AF_INET, &(addr->sin_addr), addrstr, INET_ADDRSTRLEN + 1)){
				logexit("ntop");
			}
			port = ntohs(addr->sin_port);
	}
	if (str){
			snprintf(str, strsize, "%s %hu", addrstr, port);
	}
}

int main(int argc, char *argv[])
{
	int s;
	struct in_addr inaddr;
	inet_pton(AF_INET, argv[1],&inaddr);
	char addrstr[512];
	
	uint16_t port = (uint16_t)atoi(argv[2]);

	struct sockaddr_in server;
	struct sockaddr *serverptr = (struct sockaddr *)&server;
	serverptr = (struct sockaddr*) malloc(sizeof(struct sockaddr));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr = inaddr;

	s = socket(AF_INET, SOCK_STREAM, 0);
	if(s == -1) logexit("TIMEOUT: SOCKET");

	if(connect(s, (struct sockaddr *)&server, sizeof(struct sockaddr_in))) logexit("TIMEOUT: CONNECT");

    addrtostr(serverptr, addrstr, 512);
	printf("connected to %s\n", addrstr);

	char buffer[512];
	memset(buffer, 0, 512);

	int message_len = recv(s, buffer, 512, 0); //espera mensagem READY
	if(strcmp(buffer, "READY") != 0) logexit("TIMEOUT: SERVER NOT READY");

	memset(buffer, 0x0, 512);
	ssize_t count;
	count = send(s, argv[3], strlen(argv[3])+1, 0); // envia senha do professor
	if(count != strlen(argv[3])+1) logexit("TIMEOUT: SEND");

    do{		//recebe cada um dos numeros de matricula e imprime na tela
        memset(buffer, 0x0, 512);
        message_len = recv(s, buffer, 512, 0);
		puts(buffer);
    } while (strcmp(buffer, "\0"));

    //envia OK para o servidor para fechar a conexao
    memset(buffer, 0x0, 512);
    send(s, "OK", strlen("OK")+2, 0);

    close(s);
	exit(EXIT_SUCCESS);
}