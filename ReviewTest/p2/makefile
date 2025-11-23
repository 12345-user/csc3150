CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -lrt -pthread
TARGETS = producer consumer

all: $(TARGETS) run

producer: producer.c buffer.h
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

consumer: consumer.c buffer.h
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

run:
	@echo "=== producer: ==="
	./producer &
	@echo "=== waiter: ==="
	sleep 1
	@echo "=== consumer: ==="
	./consumer
	@echo "=== finish Test ==="

.PHONY: clean run
clean:
	rm -f $(TARGETS)