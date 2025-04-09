
CC = clang
CFLAGS = -Wall -Wextra -ggdb -lm -pedantic

main: FORCE
	$(CC) $(CFLAGS) -o deglish deglish.c

clean:
	rm deglish

FORCE:
