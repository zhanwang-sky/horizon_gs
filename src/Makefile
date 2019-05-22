TARGET := horizon_gs.out

CC := g++
CFLAGS := -std=c++11 -Wall -Wextra -Wpedantic -g
INC := -I ~/.local/include/
LIB := -L ~/.local/lib/

$(TARGET): horizon_gs.cpp
	$(CC) $(CFLAGS) -o $@ $^ $(INC) $(LIB) -lapue -lhorizonlink

.PHONY: rebuild clean all
rebuild: clean all
all: $(TARGET)
clean:
	rm -rf *.o *.out