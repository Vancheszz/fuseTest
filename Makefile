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

TEST_SRC = \
    tests/main.c \
    tests/test_helpers.c \
    tests/test_create.c \
    tests/test_dir.c \
    tests/test_rw.c \
    tests/test_truncate.c \
    tests/test_getattr.c \
    tests/test_meta.c \
    tests/test_err.c

TEST_OBJ = $(TEST_SRC:.c=.o)

TEST_TARGET = fs_tests

COVERAGE_CFLAGS = -O0 -g --coverage
COVERAGE_LDFLAGS = --coverage


all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)


test: CFLAGS += -DUNIT_TEST
test: $(OBJ) $(TEST_OBJ)
	$(CC) $(OBJ) $(TEST_OBJ) -o $(TEST_TARGET) \
	    $(LDFLAGS) -lcmocka

coverage: CFLAGS += $(COVERAGE_CFLAGS) -DUNIT_TEST
coverage: LDFLAGS += $(COVERAGE_LDFLAGS)
coverage: clean test


%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TEST_OBJ) \
	      $(TARGET) $(TEST_TARGET) \
	      coverage.info \
	      fs.db
	find . -name "*.gcno" -o -name "*.gcda" | xargs rm -f
	rm -rf coverage

.PHONY: all clean test coverage
