# =============================================================
#  Makefile — Calcul du Gradient d'Images
#  Compilation modulaire en C99
# =============================================================

CC      = gcc
CFLAGS  = -std=c99 -Wall -Wextra -Wpedantic -O2 -g \
           -I./include
LDFLAGS = -lm

SRC_DIR = src
OBJ_DIR = obj

SRCS = $(SRC_DIR)/main.c     \
       $(SRC_DIR)/image_io.c \
       $(SRC_DIR)/gradient.c \
       $(SRC_DIR)/stats.c

OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

TARGET = gradient_images

# ----------------------------------------------------------
#  Règles
# ----------------------------------------------------------

all: $(OBJ_DIR) $(TARGET)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo ">>> Compilation réussie : $(TARGET)"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(TARGET)
	@echo ">>> Nettoyage effectué"

run: all
	./$(TARGET) ../assets ./output

.PHONY: all clean run
