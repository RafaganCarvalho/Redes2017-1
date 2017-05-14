/* 
 * Usuário desta biblioteca não deve se preocupar em como arquivos são manipulados internamente.  
 */
#ifndef _ARQUIVOS_H
#define _ARQUIVOS_H

/* Abre arquivos de [input] e [output]. Retorna 0 em caso de falha. */
int arq_open(char const* input, char const* output);

/* Fecha arquivos abertos via arq_open(). */
void arq_close();

/* Lê o arquivo de entrada aberto via arq_open(). Retorna o número de bytes lidos. Se leitura atingir o fim do arquivo, retorna zero. */
size_t arq_read(char* buf);

/* Escreve [length] bytes no arquivo de saída aberto via arq_open(). Retorna zero em caso de erro.*/
int arq_write(char const* buf, int length);

#endif
