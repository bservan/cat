BINNAME = cat
SOURCES = main.c cat.c platform.c
CFLAGS = -std=iso9899:1990 -Wall -Wextra -fstack-protector-strong -march=native -s

$(BINNAME): $(SOURCES)
	@$(CC) $(CFLAGS) $(SOURCES) -o $(BINNAME)

clean:
	@rm -v $(BINNAME)
