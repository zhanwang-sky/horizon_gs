TARGET := horizon_gs.out

CC := gcc
CFLAGS := -Wall -Wextra -Wpedantic -g
LIB_DIR := ~/.local/lib/
INCLUDE_DIR := ~/.local/include/
LIBS := -lapue -lhorizonlink

OS := $(shell uname)
ifeq ($(OS), Linux)
LIBS += -lpthread
endif

$(TARGET): horizon_gs.c
	$(CC) $(CFLAGS) -o $@ $^ $(addprefix -I , $(INCLUDE_DIR)) $(addprefix -L , $(LIB_DIR)) $(LIBS)

.PHONY: test rebuild clean all
test:
	@echo $(OS)
	@echo $(LIBS)
rebuild: clean all
all: $(TARGET)
clean:
	rm -rf *.o *.out
