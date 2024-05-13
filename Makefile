# Variablendefinieren
# Welchen Compiler verwenden wir?
CC=gcc
# Welche Flags sollen gesetzt werden?
CFLAGS=-Wall -D_XOPEN_SOURCE -D_POSIX_SOURCE
# optional die harten Flags
CFLAGS_HARD=-Werror -ansi -pedantic

# Targets
bibliothek: bibliothek.c sync.c sync.h
	$(CC) $(CFLAGS) -o bibliothek bibliothek.c sync.c

bibliothek_hard: bibliothek.c sync.c sync.h
	$(CC) $(CFLAGS) $(CFLAGS_HARD) -o bibliothek bibliothek.c sync.c

clean:
	rm -f bibliothek

.PHONY: clean
