CC = clang++
CFLAGS = -Wall -std=c++20 -I/usr/include/freetype2 -lglfw -lGL
SRC_DIR = src
OUT_DIR = bin
OBJ_DIR = obj

_OBJ = main.o editor.o
OBJ = $(patsubst %,$(OBJ_DIR)/%,$(_OBJ))

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

$(OUT_DIR)/cgull: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: clean

clean:
	rm -f $(OBJ_DIR)/*.o
