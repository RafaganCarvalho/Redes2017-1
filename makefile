all: tp1.c arquivos
	gcc tp1.c arquivos.o -Wall

arquivos: arquivos.h arquivos.c
	gcc -o arquivos.o -c arquivos.c -Wall

clean:
	rm -rf *.o
