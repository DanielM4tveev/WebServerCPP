#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define PORT 8088               // Порт, на котором сервер будет слушать входящие соединения
#define BUFFER_SIZE 1024       // Размер буфера для чтения данных
#define USER_AGENT_HEADER "User-Agent: " // Заголовок, который мы будем искать в запросе

// Функция для определения MIME-типа в зависимости от расширения файла
const char* get_mime_type(const char* file_path) {
    // Проверяем расширение файла и возвращаем соответствующий MIME-тип
    if (strstr(file_path, ".html") != NULL) return "text/html";
    if (strstr(file_path, ".css") != NULL) return "text/css";
    if (strstr(file_path, ".js") != NULL) return "application/javascript";
    if (strstr(file_path, ".png") != NULL) return "image/png";
    if (strstr(file_path, ".jpg") != NULL) return "image/jpeg";
    if (strstr(file_path, ".gif") != NULL) return "image/gif";
    return "application/octet-stream"; // MIME-тип по умолчанию для неизвестных файлов
}

// Функция для обработки запроса от клиента
void handle_request(int client_socket, struct sockaddr_in client_addr) {
    char buffer[BUFFER_SIZE];        // Буфер для хранения данных запроса и ответа
    char response_header[BUFFER_SIZE]; // Заголовок ответа
    char *file_path = "index.html";  // Путь к файлу, который будет отправлен клиенту по умолчанию
    char client_ip[INET_ADDRSTRLEN]; // Буфер для хранения IP-адреса клиента
    
    // Преобразуем IP-адрес клиента в строку для вывода в консоль
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
    
    // Выводим информацию о клиенте
    printf("Request received from client IP: %s, Port: %d\n", client_ip, ntohs(client_addr.sin_port));
    
    // Читаем запрос от клиента
    ssize_t bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received <= 0) {  // Проверяем, что данные успешно получены
        perror("Error receiving data");
        close(client_socket);
        return;
    }
    
    buffer[bytes_received] = '\0'; // Добавляем нуль-терминатор для корректного отображения строки

    // Ищем заголовок User-Agent в запросе
    char *user_agent = strstr(buffer, USER_AGENT_HEADER);
    if (user_agent != NULL) {
        user_agent += strlen(USER_AGENT_HEADER); // Перемещаем указатель после "User-Agent: "
        char *end_of_line = strstr(user_agent, "\r\n");
        if (end_of_line != NULL) {
            *end_of_line = '\0'; // Добавляем нуль-терминатор, чтобы завершить строку
        }
        printf("User-Agent: %s\n", user_agent); // Выводим User-Agent в консоль
    } else {
        printf("User-Agent: Not found\n"); // Если заголовок не найден
    }

    // Извлекаем путь к запрашиваемому ресурсу из строки запроса
    char *method_end = strstr(buffer, " ");
    if (method_end != NULL) {
        char *path_start = method_end + 1;
        char *path_end = strstr(path_start, " ");
        if (path_end != NULL) {
            *path_end = '\0'; // Завершаем строку
            file_path = path_start; // Устанавливаем путь к файлу
            if (strcmp(file_path, "/") == 0) {
                file_path = "index.html"; // По умолчанию отправляем index.html
            } else {
                file_path++; // Убираем начальный слэш из пути
            }
        }
    }

    // Определяем MIME-тип для ответа
    const char *mime_type = get_mime_type(file_path);

    // Открываем файл для отправки клиенту
    int file = open(file_path, O_RDONLY);
    if (file < 0) {
        // Если файл не найден, отправляем ошибку 404
        perror("Error opening file");
        sprintf(response_header, "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nFile not found");
        send(client_socket, response_header, strlen(response_header), 0);
        close(client_socket);
        return;
    }

    // Отправляем заголовок ответа клиенту
    sprintf(response_header, "HTTP/1.1 200 OK\r\nContent-Type: %s\r\n\r\n", mime_type);
    send(client_socket, response_header, strlen(response_header), 0);

    // Отправляем содержимое файла клиенту
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

    // Создаем сокет для сервера
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Настраиваем адрес и порт для сервера
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Привязываем сокет к указанному адресу и порту
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error binding socket");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Начинаем слушать входящие соединения
    if (listen(server_socket, 5) < 0) {
        perror("Error listening on socket");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("HTTP server running on port %d\n", PORT);

    // Основной цикл обработки запросов
    while (1) {
        // Принимаем входящее соединение
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_socket < 0) {
            perror("Error accepting connection");
            continue;
        }

        // Обрабатываем запрос клиента
        handle_request(client_socket, client_addr);
    }

    // Закрываем серверный сокет (это никогда не достигнется в текущей реализации)
    close(server_socket);
    return 0;
}
