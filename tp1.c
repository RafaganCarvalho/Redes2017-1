#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

int main(int argc, char const *argv[]) {
	if(strcmp(argv[1], "-s") == 0) {
        //Rodando com abertura passiva
	} else if(strcmp(argv[1], "-c") == 0){
        //Rodando com abertura ativa
        int i;
        char host[15+1]; //000.000.000.000\0
        int server_port;

        struct hostent *hp;
    	struct sockaddr_in sin;

    	//Lendo apenas o endereço do servidor
        for(i = 0; argv[2][i] != ':'; i++)
        	host[i] = argv[2][i];
        host[i] = '\0';

        //Lendo e convertendo a porta do servidor
        server_port =  atoi(&argv[2][i+1]);

        hp = gethostbyname(host);
        if(!hp) {
        	fprintf(stderr, "Host desconhecido: %s\n", host);
        	exit(1);
    	}

    	bzero((char*) &sin, sizeof(sin));
    	sin.sin_family = AF_INET;
    	bcopy(hp->h_addr, (char*) &sin.sin_addr, hp->h_length);
    	sin.sin_port = htons(server_port);

    	

    }

    } else {
    	return -1;
    }

	return 0;
}