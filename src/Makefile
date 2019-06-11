TARGET := a.out

CC := g++
LD := g++
CFLAGS := -std=c++11 -Wall -Wextra -Wpedantic -g
LIB_DIR := ~/.local/lib/
INCLUDE_DIR := ~/.local/include/
LIBS := -lhorizonlink -lhidapi

$(TARGET): main.o horizon_joysticks.o
	$(LD) -o $@ $^ $(addprefix -L , $(LIB_DIR)) $(LIBS)

main.o: main.cpp horizon_joysticks.h shared_buffer.h
	$(CC) $(CFLAGS) -c -o $@ $< $(addprefix -I , $(INCLUDE_DIR))

horizon_joysticks.o: horizon_joysticks.cpp horizon_joysticks.h
	$(CC) $(CFLAGS) -c -o $@ $< $(addprefix -I , $(INCLUDE_DIR))

.PHONY: rebuild clean all
rebuild: clean all
all: $(TARGET)
clean:
	rm -rf *.o *.out *.dSYM
