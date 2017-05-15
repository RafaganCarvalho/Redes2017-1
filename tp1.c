#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>

#include "arquivos.h"
#include "checksum.h"

#define MAX_PENDING 1
#define TIMEOUT_SEC 1
#define TIMEOUT_uSEC 0
#define TIMEOUT_SECMAX 15
#define TIMEOUT_uSECMAX 0
#define ACK 0x80
#define END 0X40
#define MAX_COUNT 15

typedef struct {
    uint32_t sync1; /*4bytes*/
    uint32_t sync2; /*4bytes*/
    uint16_t chksum; /*2bytes*/
    uint16_t length; /*2bytes*/
    uint16_t id; /*2bytes*/
    uint8_t flags; /*1byte*/
    uint8_t dados[INT16_MAX];
} block;

int s;

/* Retorna o um descritor de arquivo para o socket aberto. */
int openSocket(char const* tipo, char const* addr) {
    int i;
    int server_port;
    struct hostent *hp;
    struct sockaddr_in sin;
    
    bzero((char*) &sin, sizeof(sin));
    sin.sin_family = AF_INET;

    //Abertura do socket
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("error: socket");
        return 0;
    }

    //Configurando timeout
    struct timeval time_str;
    time_str.tv_sec = TIMEOUT_SECMAX;
    time_str.tv_usec = TIMEOUT_uSECMAX;
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &time_str, sizeof(time_str));
    setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, &time_str, sizeof(time_str));

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

        int other_s;
        int len = sizeof(sin);
        if((other_s = accept(s, (struct sockaddr*) &sin, (socklen_t*) &len)) < 0) {
            perror("error: accept");
            close(s);
            return 0;
        }
        s = other_s;
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

        int len = sizeof(sin);
        if(connect(s, (struct sockaddr *) &sin, len) < 0) {
            perror("error: connect");
            close(s);
            return 0;
        }
    } else {
        return 0;
    }

    time_str.tv_sec = TIMEOUT_SEC;
    time_str.tv_usec = TIMEOUT_uSEC;
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &time_str, sizeof(time_str));
    setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, &time_str, sizeof(time_str));

    return 1;
}

block sendBlock;
block recvBlock;

size_t readBlock() {
    char buf[INT16_MAX];
    size_t length = arq_read(buf);

    sendBlock.sync1 = sendBlock.sync2 = 0xDCC023C2;
    sendBlock.length = htons(length);
    sendBlock.chksum = 0;
    sendBlock.id = htons((ntohs(sendBlock.id) + 1) % 2);
    
    memcpy(sendBlock.dados, buf, length);
    sendBlock.flags = arq_read_end() ? END : 0;

    /*sendBlock.chksum = checksum1((char const*) &sendBlock, length + 15);*/

    return 15 + length; //15 bytes de cabeçalho
}

void writeBlock() {
    arq_write((char const*) recvBlock.dados, recvBlock.length);
}

/* 
 * Monta o bloco de recebimento.
 * Retorna ACK(0x80), END(0x40) ou dado(1), dependendo do valor recebido. 
 * Retorna 0 em caso de erro ou timeout.
 */
uint8_t receive() {
    //Recebimento do cabeçalho
    if(recv(s, &recvBlock, 15, 0) <= 0) {
        return 0;
    }
    recvBlock.length = ntohs(recvBlock.length);
    recvBlock.id = ntohs(recvBlock.id);

    //Recebimento dos dados após saber o length dos dados
    if(recv(s, &recvBlock.dados, recvBlock.length, 0) < 0) {
        return 0;    
    }

    if(recvBlock.sync1 != 0xDCC023C2 || recvBlock.sync2 != 0xDCC023C2) {
        return 0;
    }

    /*uint16_t chksum = recvBlock.chksum;
    recvBlock.chksum = 0;
    if(chksum != checksum1((char const*) &recvBlock, recvBlock.length + 15)) {
        //return 0;
    }*/

    if(recvBlock.flags == ACK || recvBlock.flags == END) {
        return recvBlock.flags;
    }

    return 1;
}

void sendACK() {
    block blockACK;
    blockACK.sync1 = blockACK.sync2 = 0xDCC023C2;
    blockACK.chksum = 0;
    blockACK.length = htons(0);
    blockACK.id = htons(recvBlock.id);
    blockACK.flags = ACK;
    //blockACK.dados[];

    /*blockACK.chksum = checksum1((char const*) &blockACK, blockACK.length + 15);*/

    if(send(s, &blockACK, blockACK.length + 15, 0) < 0){
        perror("error: send");
        close(s);
        exit(1);
    }
}

int main(int argc, char const *argv[]) {
    if(!openSocket(argv[1], argv[2]))
        exit(1);

    if(!arq_open(argv[3], argv[4]))
        exit(1);

    char sending = 1, blockNotRead = 1;
    char receiving = 1;
    uint16_t lastIdReceived = -1;
    uint32_t lastCheckSum = 0x40000000;
    size_t length;
    while(sending || receiving) {
        if(sending) {
            if(blockNotRead) {
                /*Block Not Read*/
                length = readBlock();
                blockNotRead = 0;
            }

            /*Read To Send*/
            if(send(s, &sendBlock, length, 0) < 0){
                perror("error: send");
                close(s);
                exit(1);
            }
        }

        /* Wait ACK | Block | END */
        uint8_t r = receive();
        if(r == 1 || r == END) {
            /* Block Received */
            sendACK();
            if(r == END)
                receiving = 0;

            /* Read To Write */
            if(lastIdReceived != recvBlock.id || lastCheckSum != recvBlock.chksum) {
                writeBlock();
                lastIdReceived = recvBlock.id;
                lastCheckSum = recvBlock.chksum;
            }
        } else if(r == ACK) {
            if(sendBlock.flags == END) {
                sending = 0;
            } else {
                /* Block Not Read */
                blockNotRead = 1;
            }
        }
    }

    arq_close();
    exit(0);
}