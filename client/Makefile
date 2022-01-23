CC = gcc
CFLAGS = -Wall -Werror -std=gnu99 -ggdb3
LFLAGS = -lrt -lconfig -D__GNU_SOURCE -lresolv

INCLUDES = -I include -I ../common/include 
CLIENT_SRC = main.c util.c ../common/log.c  config.c directory.c smtp.c client.c
OBJ_DIR = obj

OBJECTS = $(patsubst %.o,$(OBJ_DIR)/%.o, $(CLIENT_SRC:.c=.o))

all: client

client: $(CLIENT_SRC)
	$(CC) $(CFLAGS) $(CLIENT_SRC) $(INCLUDES) $(LFLAGS) -o client

clean:
	rm -rf *.o
