CC = gcc 
CCFlags = -Wall
BUILD_DIR = build

SRC = example-buffer.c example-codelock.c example-small.c main.c
OBJ = $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRC))

all : $(OBJ)
	$(CC) $^ -o $(BUILD_DIR)/example

$(BUILD_DIR)/%.o : %.c async.h
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CCFlags) -c -o $@ $<

.PHONY : clean
clean :
	rm -f $(BUILD_DIR)/*