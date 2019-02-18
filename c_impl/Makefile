.PHONY: all clean

CC := cc
CFLAGS := -Wextra -Wall -g

TARGET = tinyvm
SRCS = \
	$(shell find ./ -name "*.c" -maxdepth 1) \
	$(shell find ./sds -name "*.c")
OBJS = $(shell find ./ -name "*.o")

GENERATED = generated

TEST_TARGET = tinyvm_test
TEST_SRCS = \
	$(shell find ./ ! -name "tinyvm.c" -name "*.c") \
	$(shell find ./sds -name "*.c") \
	$(shell find ./tests -name "*.c")

all: $(TARGET) $(TEST_TARGET)

test: $(TEST_TARGET)

$(TARGET): $(SRCS) | $(GENERATED)
	$(CC) -o $(addprefix $(GENERATED)/, $@) $^ $(CFLAGS)

$(TEST_TARGET): $(TEST_SRCS) | $(GENERATED)
	$(CC) -o $(addprefix $(GENERATED)/, $@) $^ $(CFLAGS)  -I .

$(GENERATED):
	mkdir -p $(GENERATED)

clean:
	$(RM) $(OBJS) $(addprefix $(GENERATED)/, $(TARGET))
