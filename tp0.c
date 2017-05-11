/* Matrícula 2010074259 -> ímpar -> cliente */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define SERVER_PORT 51515
#define MAX_PENDING 1

int main(int argc, char const *argv[]) {
    struct hostent *hp;
    struct sockaddr_in sin;
    char *host = "127.0.0.1";
    int32_t value;
    char s_value[4], c;
    int s;
    
    if(argc < 2) {
        fprintf(stderr, "Falta argumento de entrada\n");
        exit(1);
    }
	
    if(strcmp(argv[1], "inc") == 0)
        c = '+';
    else if(strcmp(argv[1], "dec") == 0)
        c = '-';
    else {
        fprintf(stderr, "Erro de entrada: %s\n", argv[1]);
        exit(1);
    }

    hp = gethostbyname(host);
    if(!hp) {
        fprintf(stderr, "Host desconhecido: %s\n", host);
        exit(1);
    }
    

    bzero((char*) &sin, sizeof(sin));
    sin.sin_family = AF_INET;
    bcopy(hp->h_addr, (char*) &sin.sin_addr, hp->h_length);
    sin.sin_port = htons(SERVER_PORT);

    /* Abertura ativa */
    s = socket(PF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        fprintf(stderr, "Erro ao criar socket.\n");
        exit(1);
    }

    struct timeval time_str;
    time_str.tv_sec = 2;
    time_str.tv_usec = 0;
    setsockopt (s, SOL_SOCKET, SO_RCVTIMEO, &time_str, sizeof(time_str));
    setsockopt (s, SOL_SOCKET, SO_SNDTIMEO, &time_str, sizeof(time_str));

    if(connect(s, (struct sockaddr *) &sin, sizeof(sin)) < 0) {
        fprintf(stderr, "Erro ao conectar ao socket.\n");
        close(s);
        exit(1);
    }

	if(send(s, &c, 1, 0) < 0) {
        fprintf(stderr, "Erro de envio 1 %c\n", c);
        close(s);
        exit(1);
    }

	//Recebimento, conversão e impressão
	if(recv(s, &value, sizeof(value), 0) < 0) {
	    fprintf(stderr, "Erro de recebimento %d\n", value);
        close(s);
        exit(1);
    }
	value = ntohl(value);
    fprintf(stdout, "%d\n", value);

    //Conversão e envio
	sprintf(s_value, "%03d", value);
	if(send(s, s_value, sizeof(s_value), 0) < 0) {
        fprintf(stderr, "erro de envio 2 %s\n", s_value);
        close(s);
        exit(1);
    }

    close(s);
    exit(0);
}