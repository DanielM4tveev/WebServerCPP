#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define PORT 8088
#define BUFFER_SIZE 1024
#define USER_AGENT_HEADER "User-Agent: "

// Функция для обработки запроса
void handle_request(int client_socket, struct sockaddr_in client_addr) {
    char buffer[BUFFER_SIZE];
    char *response_header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
    char *file_path = "index.html";
    char client_ip[INET_ADDRSTRLEN];
    
    // Преобразуем IP-адрес клиента в строку
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
    
    // Выводим информацию о клиенте
    printf("Получен запрос от клиента IP: %s, Порт: %d\n", client_ip, ntohs(client_addr.sin_port));
    
    // Читаем запрос и ищем заголовок User-Agent
    ssize_t bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received <= 0) {
        perror("Ошибка при получении данных");
        close(client_socket);
        return;
    }
    
    buffer[bytes_received] = '\0'; // Нуль-терминатор для корректного отображения строки

    // Ищем заголовок User-Agent в запросе
    char *user_agent = strstr(buffer, USER_AGENT_HEADER);
    if (user_agent != NULL) {
        user_agent += strlen(USER_AGENT_HEADER); // Пропускаем "User-Agent: "
        char *end_of_line = strstr(user_agent, "\r\n");
        if (end_of_line != NULL) {
            *end_of_line = '\0'; // Нуль-терминатор для корректного отображения строки
        }
        printf("User-Agent: %s\n", user_agent);
    } else {
        printf("User-Agent: Не найдено\n");
    }

    // Открываем HTML файл
    int file = open(file_path, O_RDONLY);
    if (file < 0) {
        perror("Ошибка при открытии файла");
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
        perror("Ошибка создания сокета");
        exit(EXIT_FAILURE);
    }

    // Настраиваем адрес и порт
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Привязываем сокет к адресу и порту
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Ошибка привязки сокета");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Слушаем входящие соединения
    if (listen(server_socket, 5) < 0) {
        perror("Ошибка прослушивания сокета");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("HTTP сервер запущен на порту %d\n", PORT);

    // Основной цикл обработки запросов
    while (1) {
        // Принимаем входящее соединение
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_socket < 0) {
            perror("Ошибка приема соединения");
            continue;
        }

        // Обрабатываем запрос
        handle_request(client_socket, client_addr);
    }

    // Закрываем серверный сокет
    close(server_socket);
    return 0;
}
