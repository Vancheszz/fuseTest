CC = gcc
CFLAGS = -Wall -Wextra -std=gnu11 -Iinclude
LDFLAGS = -lfuse3 -lsqlite3

SRC = \
    src/main.c \
    src/db.c \
    src/util.c \
    src/op_dir.c \
    src/op_file.c \
    src/op_meta.c

OBJ = $(SRC:.c=.o)

TARGET = myfs

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET) fs.db

.PHONY: all clean
