# TODO create a build system that compiles all dirs into dyn libs. 
CC=gcc
SRC_DIR=src
# inlinable files
INL_DIR=$(SRC_DIR)/inl
BUILD_DIR=build
RES_DIR=res

SRC_LIST=$(shell find $(SRC_DIR) -type f -name "*.c" )
OUT_LIST= 
LIBS =-lm -lGLEW -lGLU -lGL -lSDL2 -lSDL2main 
INCS = -I$(INL_DIR)
OUT=nixboy

all: $(SRC_LIST)
	$(CC) $(INCS) \
		$(OUT_LIST) \
		$(LIBS)\
		-o $(BUILD_DIR)/$(OUT)
	
.PHONY : $(SRC_LIST)
$(SRC_LIST): $(BUILD_DIR)
	-$(eval OUTFILE:=$(BUILD_DIR)/$(notdir $(@:.c=.o)))
	$(CC) $(INCS) -c 	$@ 	-o $(OUTFILE) 
	-$(eval OUT_LIST:=$(OUT_LIST) $(OUTFILE))
	

$(BUILD_DIR):
	mkdir $(BUILD_DIR)

clean:
	-rm -r $(BUILD_DIR)
