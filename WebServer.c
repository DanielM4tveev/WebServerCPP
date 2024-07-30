#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define PORT 8080
#define BUFFER_SIZE 1024

// Функция для обработки запроса
void handle_request(int client_socket) {
    char buffer[BUFFER_SIZE];
    char *response_header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
    char *file_path = "index.html";
    
    // Открываем HTML файл
    int file = open(file_path, O_RDONLY);
    if (file < 0) {
        perror("ERROR 400. File not opened on doesn't exist");
        close(client_socket);
        return;
    }

    // Отправляем заголовок ответа
    send(client_socket, response_header, strlen(response_header), 0);
    
    // Отправляем содержимое файла
    ssize_t bytes_read;
    while ((bytes_read = read(file, buffer, sizeof(buffer))) > 0) {
        send(client_socket, buffer, bytes_read, 0);
    }

    // Закрываем файл и сокет
    close(file);
    close(client_socket);
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    // Создаем сокет
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Error creation socket");
        exit(EXIT_FAILURE);
    }

    // Настраиваем адрес и порт
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Привязываем сокет к адресу и порту
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error connection socket");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Слушаем входящие соединения
    if (listen(server_socket, 5) < 0) {
        perror("Error listening socket");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("HTTP Server loaded in port %d\n", PORT);

    // Основной цикл обработки запросов
    while (1) {
        // Принимаем входящее соединение
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_socket < 0) {
            perror("Error handshake connection");
            continue;
        }

        // Обрабатываем запрос
        handle_request(client_socket);
    }

    // Закрываем серверный сокет
    close(server_socket);
    return 0;
}
