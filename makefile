all: tp1.c arquivos checksum
	gcc tp1.c arquivos.o checksum.o -Wall

arquivos: arquivos.h arquivos.c
	gcc -o arquivos.o -c arquivos.c -Wall

checksum: checksum.h checksum.c
	gcc -o checksum.o -c checksum.c -Wall

clean:
	rm -rf *.o
