CC = gcc
CFLAGS = -Wall -Werror -std=gnu99 -ggdb3
LFLAGS = -lrt

INCLUDES = -I include -I ../common/include
SERVER_SRC = main.c server.c server_poll.c socket_utils.c ../common/log.c
OBJ_DIR = obj

OBJECTS = $(patsubst %.o,$(OBJ_DIR)/%.o, $(SERVER_SRC:.c=.o))

SERVER_EXE = server

all: server

server: $(SERVER_SRC)
	$(CC) $(CFLAGS) $(SERVER_SRC) $(INCLUDES) $(LFLAGS) -o $(SERVER_EXE)

clean:
	rm -rf *.o
