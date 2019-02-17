.PHONY: all clean

CC := cc
CFLAGS := -Wextra -Wall -g

TARGET = tinyvm
SRCS = \
	$(shell find ./ -name "*.c") \
	$(shell find ./sds -name "*.c")
OBJS = $(shell find ./ -name "*.o")

GENERATED = generated

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) -o $(addprefix $(GENERATED)/, $@) $^ $(CFLAGS)

clean:
	$(RM) $(OBJS) $(addprefix $(GENERATED)/, $(TARGET))
