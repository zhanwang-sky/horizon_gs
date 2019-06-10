TARGET := a.out

CC := g++
CFLAGS := -std=c++11 -Wall -Wextra -Wpedantic -g
LIB_DIR := ~/.local/lib/
INCLUDE_DIR := ~/.local/include/
LIBS := -lhorizonlink -lhidapi

$(TARGET): main.cpp shared_buffer.h horizon_joysticks.h
	$(CC) $(CFLAGS) -o $@ $< $(addprefix -I , $(INCLUDE_DIR)) $(addprefix -L , $(LIB_DIR)) $(LIBS)

.PHONY: rebuild clean all
rebuild: clean all
all: $(TARGET)
clean:
	rm -rf *.o *.out *.dSYM
