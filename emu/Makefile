CC=gcc
SRC=src
LIB_DIR= external/lib
INC_DIR= external/inc
FLAGS = -lGL -lGLEW -lSDL2 -lSDL2main -march=native -O3
BUILD_DIR = build
OUT=nixboy

COMPILE = $(CC) $(SRC)/* -o $(BUILD_DIR)/$(OUT) -I$(INC_DIR) -L$(LIB_DIR) $(FLAGS)

all: 
	$(COMPILE) 
	@echo "Success"  

clean:
	rm $(BUILD_DIR)/*
