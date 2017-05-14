all: tp1.c arquivos.o checksum.o
	gcc -o tp1 tp1.c arquivos.o checksum.o -Wall

arquivos.o: arquivos.h arquivos.c
	gcc -o arquivos.o -c arquivos.c -Wall

checksum.o: checksum.h checksum.c
	gcc -o checksum.o -c checksum.c -Wall

clean:
	rm -rf *.o
