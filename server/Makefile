CC = gcc
CFLAGS = -Wall -Werror -std=gnu99 -ggdb3
LFLAGS = -lrt -lconfig

INCLUDES = -I include -I ../common/include
SERVER_SRC = ../common/log.c config.c socket_utils.c status.c state.c letter.c client.c client_hash.c client_handler.c server_poll.c server.c maildir.c main.c
OBJ_DIR = obj

OBJECTS = $(patsubst %.o,$(OBJ_DIR)/%.o, $(SERVER_SRC:.c=.o))

SERVER_EXE = server

all: server

server: $(SERVER_SRC)
	$(CC) $(CFLAGS) $(SERVER_SRC) $(INCLUDES) $(LFLAGS) -o $(SERVER_EXE)

clean:
	rm -rf *.o
