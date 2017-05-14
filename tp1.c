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
#define TIMEOUT_SECMAX 15
#define TIMEOUT_uSECMAX 0
#define ACK 0x80
#define END 0X40
#define MAX_COUNT 15

typedef struct {
    uint32_t sync1;
    uint32_t sync2;
    uint16_t chksum;
    uint16_t length;
    uint16_t id;
    uint8_t flags;
    uint8_t dados[INT16_MAX];
} block;

int s;

FILE* debbug_log;

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
    sendBlock.id = htons((sendBlock.id + 1) % 2);
    
    printf("antes memcpy: %d, %s\n", length, buf);
    memcpy(sendBlock.dados, buf, length);
    sendBlock.flags = length == 0 ? END : 0;

    sendBlock.chksum = checksum1((char const*) &sendBlock, length + 14);

    return 14 + length; //14 bytes de cabeçalho
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
    if(recv(s, &recvBlock, 14 + INT16_MAX, 0) < 0)
        return 0;

    recvBlock.length = ntohs(recvBlock.length);
    recvBlock.id = ntohs(recvBlock.id);

    if(recvBlock.sync1 != 0xDCC023C2 || recvBlock.sync2 != 0xDCC023C2) {
        puts("recvBlock.sync1 != 0xDCC023C2 || recvBlock.sync2 != 0xDCC023C2");
        return 0;
    }

    uint16_t chksum = recvBlock.chksum;
    recvBlock.chksum = 0;
    if(chksum != checksum1((char const*) &recvBlock, recvBlock.length + 14)) {
        puts("chksum != checksum1((char const*) &recvBlock, recvBlock.length + 14)");
        //return 0;
    }

    if(recvBlock.flags == ACK || recvBlock.flags == END) {
        printf("recvBlock.flags(%2x), ACK(%2x), END(%2x): %d\n", recvBlock.flags, ACK, END, recvBlock.flags == ACK || recvBlock.flags == END);
        return recvBlock.flags;
    }

    puts("receive Dados");
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

    blockACK.chksum = checksum1((char const*) &blockACK, blockACK.length + 14);

    if(send(s, &blockACK, blockACK.length + 14, 0) < 0){
        perror("error: send");
        close(s);
        exit(1);
    }
}

int main(int argc, char const *argv[]) {
    debbug_log = fopen(argv[5], "w");
    if(!openSocket(argv[1], argv[2]))
        exit(1);

    if(!arq_open(argv[3], argv[4]))
        exit(1);

    char sending = 1, readToSend = 0;
    char receiving = 1;
    char count;
    uint16_t lastIdReceived = -1;
    uint32_t lastCheckSum = 0x40000000;
    size_t length;
    while(sending) {
        /*Block Not Read*/
        if(!readToSend) {
            length = readBlock();
            printf("/*Block Not Read*/\n");
        }
        readToSend = 1;

        /*Read To Send*/
        printf("/*Read To Send*/\n");
        if(send(s, &sendBlock, length, 0) < 0){
            perror("error: send");
            close(s);
            exit(1);
        }
        if(sendBlock.flags & END) {
            /* Wait ACK */
            printf("/* Wait ACK */\n");
            for(count = 0; !(receive() & ACK); count++) { 
                if(MAX_COUNT > count) {
                    fprintf(stderr, "receive(): Tentativas excedidas: %dx\n", MAX_COUNT);
                    exit(1);
                }
                if(send(s, &sendBlock, length, 0) < 0){
                    perror("error: send");
                    close(s);
                    exit(1);
                }
            }
            sending = 0;
        } else {
            /* Wait ACK | Block | END */
            puts("/* Wait ACK | Block | END */");
            uint8_t r = receive();
            if(r == 1) {
                /* Block Received */
                puts("/* Block Received */");
                sendACK();

                /* Read To Write */
                puts("/* Read To Write */");
                if(lastIdReceived != recvBlock.id && lastCheckSum != recvBlock.chksum) {
                    puts("writeBlock();");
                    writeBlock();
                    lastIdReceived = recvBlock.id;
                    lastCheckSum = recvBlock.chksum;
                }
            } else if(r & ACK) {
                /* Block Not Read */
                puts("/* Block Not Read */");
                readToSend = 0;
            } else if(r & END) {
                /* END Received */
                puts("/* END Received */");
                sendACK();
                if(lastIdReceived != recvBlock.id && lastCheckSum != recvBlock.chksum) {
                    puts("writeBlock();");
                    writeBlock();
                    lastIdReceived = recvBlock.id;
                    lastCheckSum = recvBlock.chksum;
                }
                receiving = 0;
            } else if(r == 0) {
                /* Read To Send */
            } else {
                fprintf(stderr, "receive(): Retorno inválido %d\n", r);
                exit(1);
            }
        }
    }
    /* Not To Send */
    //Configurando timeout
    struct timeval time_str;
    time_str.tv_sec = TIMEOUT_SECMAX;
    time_str.tv_usec = TIMEOUT_uSECMAX;
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &time_str, sizeof(time_str));
    setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, &time_str, sizeof(time_str));
    puts("----------------------------/* Not To Send */------------------------------");
    while(receiving) {
        /* Wait Block | END */
        puts("/* Wait Block | END */");
        uint8_t r = receive();
        if(r == 1) {
            /* Block Received */
            puts("/* Block Received */");
            sendACK();

            /* Read To Write */
            puts("/* Read To Write */");
            if(lastIdReceived != recvBlock.id && lastCheckSum != recvBlock.chksum) {
                puts("writeBlock();");
                writeBlock();
                lastIdReceived = recvBlock.id;
                lastCheckSum = recvBlock.chksum;
            }
        } else if(r & END) {
            /* END Received */
            puts("/* END Received */");
            sendACK();
            if(lastIdReceived != recvBlock.id && lastCheckSum != recvBlock.chksum) {
                puts("writeBlock();");
                writeBlock();
                lastIdReceived = recvBlock.id;
                lastCheckSum = recvBlock.chksum;
            }
            receiving = 0;
        } else if(r == 0) {
            /* Wait Block | END */
        } else {
            fprintf(stderr, "receive(): Retorno inválido %d\n", r);
            exit(1);
        }
    }

    arq_close();
    fclose(debbug_log);
    exit(0);
}