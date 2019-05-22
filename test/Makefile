TARGET := test.out

CC := g++
CFLAGS := -std=c++11 -Wall -Wextra -Wpedantic -g

$(TARGET): test.cpp
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: rebuild clean all
rebuild: clean all
all: $(TARGET)
clean:
	rm -rf *.o *.out
