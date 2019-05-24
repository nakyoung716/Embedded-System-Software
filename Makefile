CC =arm-none-linux-gnueabi-gcc -pthread -static

prj1:	main.o input.o output.o
		$(CC) -o prj1 main.o input.o output.o -lm

main.o: main.c
		$(CC) -c -o main.o main.c -lm

input.o: input.c
		$(CC) -c -o input.o input.c -lm

output.o: output.c
		$(CC) -c -o output.o output.c -lm

clean:
		rm -f main main.o input.o output.o prj1
