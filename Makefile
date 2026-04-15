# Space Shooter — build and run
CC      := gcc
CFLAGS  := -Wall -Wextra -std=c11 -O2
LDFLAGS :=
TARGET  := space_shooter
SRCS    := main.c game.c screen.c keyboard.c memory.c string.c math.c
OBJS    := $(SRCS:.c=.o)

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
