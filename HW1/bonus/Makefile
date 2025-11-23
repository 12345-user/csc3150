CC = gcc
CFLAGS = -Wall -Wextra -std=c99
TARGETS = pstree myfork

all: $(TARGETS)

pstree: pstree.c
	$(CC) $(CFLAGS) -o pstree pstree.c

myfork: myfork.c
	$(CC) $(CFLAGS) -o myfork myfork.c

clean:
	rm -f $(TARGETS)

install: all
	@echo "Bonus programs compiled successfully!"
	@echo "Usage:"
	@echo "  ./pstree       - Show process tree"
	@echo "  ./pstree -p    - Show process tree with PIDs"
	@echo "  ./pstree -h    - Show help"
	@echo "  ./myfork       - Demo fork functionality"

.PHONY: all clean install
