APP := demoscene
SRC := src/main.cpp src/shapes.cpp
BIN_DIR := build
OUT := $(BIN_DIR)/$(APP)

CXX := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra
RAYLIB_FLAGS := $(shell pkg-config --cflags --libs raylib 2>/dev/null)
ifeq ($(strip $(RAYLIB_FLAGS)),)
RAYLIB_FLAGS := -I/usr/local/include -L/usr/local/lib -lraylib
endif
PLATFORM_LIBS := -lm -lpthread -ldl -lrt -lX11

.PHONY: all setup run clean

all: $(OUT)

setup:
	./getting_started.sh

$(OUT): $(SRC) METRO_FINAL_TRUSTMEBRO.mp3
	mkdir -p $(BIN_DIR)
	objcopy -I binary -O elf64-x86-64 -B i386:x86-64 METRO_FINAL_TRUSTMEBRO.mp3 $(BIN_DIR)/METRO_FINAL_TRUSTMEBRO.o
	$(CXX) $(CXXFLAGS) $(SRC) $(BIN_DIR)/METRO_FINAL_TRUSTMEBRO.o -o $@ $(RAYLIB_FLAGS) $(PLATFORM_LIBS)

run: $(OUT)
	./$(OUT)

clean:
	rm -rf $(BIN_DIR)
