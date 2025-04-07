
CC = clang
CFLAGS = -Wall -Wextra -ggdb -lm -pedantic

main: FORCE
	$(CC) $(CFLAGS) -o main main.c

clean:
	rm main

FORCE:
