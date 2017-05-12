/* 
 * Usuário desta biblioteca não deve se preocupar em como arquivos são manipulados internamente.  
 */
#ifndef _ARQUIVOS_H
#define _ARQUIVOS_H

/* Abre arquivos de [input] e [output]. */
int arq_open(char const* input, char const* output);

/* Fecha arquivos abertos via arq_open(). */
int arq_close();

/* Lê o arquivo de entrada aberto via arq_open(). Retorna o número de bytes lidos. Se leitura atingir o fim do arquivo, retorna EOF. */
int arq_read(char const* buf);

/* Escreve [length] bytes no arquivo de saída aberto via arq_open(). */
int arq_write(char const* buf, int length);

#endif
