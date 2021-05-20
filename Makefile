CFLAGS = -Wall -Werror
TARGET = programa1
OBJECTFILES = veiculos.o arquivos.o main.o
CFILES = main.c veiculos.c arquivos.c

all: $(TARGET)

$(TARGET): $(OBJECTFILES)
	gcc $(CFLAGS) -o $(TARGET) $(OBJECTFILES)

clean:
	rm $(TARGET) $(OBJECTFILES)

debug:
	gcc $(CFLAGS) -g $(CFILES)

run:
	./$(TARGET) < ./Tests/1.in

runMem:
	valgrind ./$(TARGET)

avaliarBin:
	hexdump -c -n1000 veiculo1.bin > veiculo1.txt
	hexdump -c -n1000 ./Tests/veiculo1_saida_esperada.bin > saidaEsperada.txt
	diff -s --color veiculo1.txt saidaEsperada.txt
	
zip:
	@zip -r Operacoes.zip *.c *.h Makefile
	