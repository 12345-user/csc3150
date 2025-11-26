CC := gcc
CFLAGS := -Wall -Wextra -O2 -pthread
TARGET := RealTime
SRC := RealTime.c

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)


