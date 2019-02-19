CC=gcc
SRC=$(wildcard  src/*.c)
LIBS =-lGLEW -lGLU -lGL -lSDL2 -lSDL2main 
BUILD_DIR=build
RES_DIR=res
OUT=nixboy

all: $(BUILD_DIR)
	$(CC) $(SRC) -o $(BUILD_DIR)/$(OUT) $(LIBS) 
	cp -r $(RES_DIR) $(BUILD_DIR)

$(BUILD_DIR):
	mkdir $(BUILD_DIR)

clean:
	rm -r $(BUILD_DIR)
