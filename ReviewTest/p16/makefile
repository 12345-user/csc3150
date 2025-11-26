CC := gcc
CFLAGS := -O2 -Wall -Wextra -std=c11
TARGET := affinity_test
SRC := affinity_test.c

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

