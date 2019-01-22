CC=gcc
SRC=$(wildcard  src/*.c)
LIBS =-lGLEW -lGLU -lGL -lSDL2 -lSDL2main 
BUILD_DIR=build
OUT=nixboy
COMPILE=$(CC) $(SRC) -o $(BUILD_DIR)/$(OUT) $(LIBS) 

all: $(BUILD_DIR)
	$(COMPILE) 
	@echo "Build Success"  

$(BUILD_DIR):
	mkdir $(BUILD_DIR)

clean:
	rm -r $(BUILD_DIR)
