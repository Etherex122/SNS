CC = gcc
flags = -g -O3
source = Parser.c Tokenize.c transpilator.c main.c Preproccesor.c
target = m

make:
	$(CC) $(flags) $(source) -o $(target) -lm

win:
	nasm -f elf64 out.asm -o out.o
	ld out.o -o out
	./out

clean:
	rm -f m *.o out