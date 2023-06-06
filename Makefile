CC = gcc
CFLAGS = -Wall

SRCS = main.c stats_functions.c
OBJS = $(SRCS:.c=.o)
EXEC = a3

all:    $(EXEC)

$(EXEC):    $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

%.o:    %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(EXEC)
