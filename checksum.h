/* 
 * Essa biblioteca contém duas implementações de checksums, uma consderendo uma extensão de 32 bits de addcarry 
 * e ao final tem uma compactação para os 16 bits. A outra já opera totalmente em 16 bits tratando do bit extra
 * durante o calculo.  
 */
#ifndef _CHECKSUM_H
#define _CHECKSUM_H

/* Primeira implementação, calcula-se um sum de 32bits e ao final se compacta em 16 bits além da utilização de registers para melhoria de desempenho. */
int in_cksum(register u_short  *ptr, register int    nbytes);

/* Implementação mais simples, operando em 16 bits.  */
unsigned short checksum1(const char *buf, unsigned size);

#endif
