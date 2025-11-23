CC = gcc
CFLAGS = -Wall -g

TARGET = pipe

all: $(TARGET) run

$(TARGET): pipe.c
	$(CC) $(CFLAGS) -o $@ $<

run: $(TARGET)
	./$(TARGET)
	@echo "\n===finish Run ==="
clean:
	rm -f $(TARGET)