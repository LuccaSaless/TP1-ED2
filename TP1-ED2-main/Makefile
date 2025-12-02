
all: main.o arvoreb.o arvorebe.o arvorebin.o index.o geradorDeArquivos.o
	@gcc main.o arvoreb.o arvorebe.o arvorebin.o index.o geradorDeArquivos.o -o pesquisa -lm

	@rm -f main.o arvoreb.o arvorebe.o arvorebin.o index.o geradorDeArquivos.o

main.o: main.c
	@gcc -c main.c -Wall -g

arvoreb.o: arvoreb.c
	@gcc -c arvoreb.c -Wall -g

arvorebe.o: arvorebe.c
	@gcc -c arvorebe.c -Wall -g

arvorebin.o: arvorebin.c
	@gcc -c arvorebin.c -Wall -g

index.o: index.c
	@gcc -c index.c -Wall -g

geradorDeArquivos.o: geradorDeArquivos.c
	@gcc -c geradorDeArquivos.c -Wall -g

clean:
	@rm -f pesquisa *.o