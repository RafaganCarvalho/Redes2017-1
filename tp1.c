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
        //Rodando como servidor
	} else if(strcmp(argv[1], "-c") == 0){
        //Rodando como cliente
    } else {
    	return -1;
    }

	return 0;
}