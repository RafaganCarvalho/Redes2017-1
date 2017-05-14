all: tp1.c arquivos checksum
	gcc tp1.c exec/arquivos.o exec/checksum.o -Wall

arquivos: arquivos.h arquivos.c
	gcc -o exec/arquivos.o -c arquivos.c -Wall

checksum: checksum.h checksum.c
	gcc -o exec/checksum.o -c checksum.c -Wall

clean:
	rm -rf *.o
