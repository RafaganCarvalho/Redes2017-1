#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "arquivos.h"
#include "checksum.h"

#define MAX_PENDING 1
#define TIMEOUT_SEC 1
#define TIMEOUT_uSEC 0

int s, other_s;

/* Retorna o um descritor de arquivo para o socket aberto. */
int openSocket(char const* tipo, char const* addr) {
    int i;
    int server_port;
    struct hostent *hp;
    struct sockaddr_in sin;
    
    bzero((char*) &sin, sizeof(sin));
    sin.sin_family = AF_INET;
    
    //Configurando timeout
    struct timeval time_str;
    time_str.tv_sec = TIMEOUT_SEC;
    time_str.tv_usec = TIMEOUT_uSEC;
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &time_str, sizeof(time_str));
    setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, &time_str, sizeof(time_str));

    //Abertura do socket
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("error: socket");
        return 0;
    }

    if(strcmp(tipo, "-s") == 0) { //Abertura passiva
        server_port = atoi(addr); //Lendo e convertendo a porta do servidor

        sin.sin_addr.s_addr = INADDR_ANY;
        sin.sin_port = htons(server_port);

        if(bind(s, (struct sockaddr*) &sin, sizeof(sin)) < 0) {
            perror("error: bind");
            close(s);
            return 0;
        }
        listen(s, MAX_PENDING);

        int len = sizeof(sin);
        if((other_s = accept(s, (struct sockaddr*) &sin, &len)) < 0) {
            perror("error: accept");
            close(s);
            return 0;
        }
    } else if(strcmp(tipo, "-c") == 0) { //Abertura ativa
        //Recuperando endereço do servidor
        char host[15+1]; //000.000.000.000\0
        for(i = 0; addr[i] != ':'; i++)
            host[i] = addr[i];
        host[i] = '\0';
        server_port = atoi(&addr[i+1]); //Lendo e convertendo a porta do servidor

        if(!(hp = gethostbyname(host))) {
            perror("error: gethostbyname");
            fprintf(stderr, "Host: %s\n", host);
            close(s);
            return 0;
        }

        bcopy(hp->h_addr, (char*) &sin.sin_addr, hp->h_length);
        sin.sin_port = htons(server_port);

        if(connect(s, (struct sockaddr *) &sin, sizeof(sin)) < 0) {
            perror("error: connect");
            close(s);
            return 0;
        }
    } else {
        return 0;
    }
    return 1;
}

int main(int argc, char const *argv[]) {
    if(!openSocket(argv[1], argv[2]))
        exit(1);
	
	exit(0);
}
