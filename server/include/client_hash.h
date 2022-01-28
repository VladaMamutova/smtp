#ifndef CLIENT_MAP_H
#define CLIENT_MAP_H

#include "client.h"

#include <string.h>
#include <arpa/inet.h>

#define EQUALS(key1, key2) (key1 == key2)

typedef int key;

typedef struct client_hash_t {
    struct client_hash_t *next;
    client client_info;
} client_hash;


client_hash **clients;
int client_hash_size;

// Инициализация списка клиентов.
void init_client_hash(int size);

// Получение хэш-значения от ключа клиента.
key hash(key client_socket);

// Добавление нового клиента в ассоциативный список клиентов.
client_hash *insert_client(client client_info);

// Удаление существующего клиента из списка.
int remove_client(key client_socket);

// Поиск клиента в списке.
client_hash* find_client(key client_socket);

#endif // CLIENT_MAP_H
