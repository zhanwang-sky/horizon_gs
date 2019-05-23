TARGET := horizon_gs.out

CC := gcc
CFLAGS := -std=c11 -Wall -Wextra -Wpedantic -g

$(TARGET): horizon_gs.c
	$(CC) $(CFLAGS) -o $@ $^ -lapue -lhorizonlink

.PHONY: rebuild clean all
rebuild: clean all
all: $(TARGET)
clean:
	rm -rf *.o *.out
