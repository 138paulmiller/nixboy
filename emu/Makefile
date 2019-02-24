CC=gcc
SRC_DIR=src
# inlinable files
INL_DIR=$(SRC_DIR)/inl
BUILD_DIR=build
RES_DIR=res

SRC=$(wildcard  $(SRC_DIR)/*.c)
LIBS =-lGLEW -lGLU -lGL -lSDL2 -lSDL2main 
INCS = -I$(INL_DIR)
OUT=nixboy

all: $(BUILD_DIR)
	$(CC) $(INCS) $(SRC) -o $(BUILD_DIR)/$(OUT) $(LIBS) 
	cp -r $(RES_DIR) $(BUILD_DIR)

$(BUILD_DIR):
	mkdir $(BUILD_DIR)

clean:
	rm -r $(BUILD_DIR)
