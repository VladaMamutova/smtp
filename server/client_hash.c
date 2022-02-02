#include "client_hash.h"

#include <stdlib.h>

void init_client_hash(int size)
{
    client_hash_size = size;
    clients = malloc(client_hash_size * sizeof(client_hash*));
}

key hash(key client_socket)
{
    return client_socket % client_hash_size; // не больше, чем размер client_hash
}

client_hash *insert_client(client *client_info)
{
    client_hash *new_client = malloc(sizeof(client_hash));
    key key = hash(client_info->socket);

    // Вставляем клиента в начало списка по индексу.
    client_hash *next_client = clients[key];
    clients[key] = new_client;
    new_client->next = next_client; 
    new_client->client_info = client_info;
    return new_client;
}

int remove_client(key client_socket)
{
    client_hash *previous_client = NULL;
    key key = hash(client_socket);

    // Ищем клиента, которого необходимо удалить из списка.
    client_hash *client_to_remove = clients[key];
    while (client_to_remove && !EQUALS(client_to_remove->client_info->socket, client_socket)) {
        previous_client = client_to_remove;
        client_to_remove = client_to_remove->next;
    }
    if (!client_to_remove) { // клиент для удаления не найден
        // Удалять нечего, выходим.
        return -1;
    }

    if (previous_client) { // удаляемый клиент - не первый
        // Связываем предыдущего клиента со следующим после удаляемого.
        previous_client->next = client_to_remove->next;
    } else { // удаляемый клиент - первый в списке
        clients[key] = client_to_remove->next;
    }

    free_client(client_to_remove->client_info);
    free(client_to_remove);
    return 0;
}

client_hash* find_client(key client_socket) {
    client_hash *result;

    result = clients[hash(client_socket)];
    while (result && !EQUALS(result->client_info->socket, client_socket)) {
        result = result->next;
    }
    return result;
}
