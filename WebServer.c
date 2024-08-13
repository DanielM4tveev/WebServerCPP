#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define PORT 8080
#define BUFFER_SIZE 4096
#define USER_AGENT_HEADER "User-Agent: "

// Функция для определения MIME-типа в зависимости от расширения файла
const char* get_mime_type(const char* file_path) {
    if (strstr(file_path, ".html") != NULL) return "text/html";
    if (strstr(file_path, ".css") != NULL) return "text/css";
    if (strstr(file_path, ".js") != NULL) return "application/javascript";
    if (strstr(file_path, ".png") != NULL) return "image/png";
    if (strstr(file_path, ".jpg") != NULL) return "image/jpeg";
    if (strstr(file_path, ".gif") != NULL) return "image/gif";
    return "application/octet-stream";
}

// Функция для обработки запроса от клиента
void handle_request(int client_socket, struct sockaddr_in client_addr) {
    char buffer[BUFFER_SIZE];
    char response_header[BUFFER_SIZE];
    char *file_path = "index.html";
    char client_ip[INET_ADDRSTRLEN];
    
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
    printf("Request received from client IP: %s, Port: %d\n", client_ip, ntohs(client_addr.sin_port));
    
    ssize_t bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received <= 0) {
        perror("Error receiving data");
        close(client_socket);
        return;
    }
    
    buffer[bytes_received] = '\0';
    
    // Извлекаем путь к запрашиваемому ресурсу из строки запроса
    char *method_end = strstr(buffer, " ");
    if (method_end != NULL) {
        char *path_start = method_end + 1;
        char *path_end = strstr(path_start, " ");
        if (path_end != NULL) {
            *path_end = '\0';
            file_path = path_start;
            if (strcmp(file_path, "/") == 0) {
                file_path = "index.html";
            } else if (strcmp(file_path, "/admin") == 0) {
                file_path = "admin/admin.html"; // Перенаправляем на админ-панель
            } else if (strncmp(file_path, "/admin/", 7) == 0) {
                file_path += 7; // Убираем префикс /admin/
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
        perror("Error opening file");
        sprintf(response_header, "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nFile not found");
        send(client_socket, response_header, strlen(response_header), 0);
        close(client_socket);
        return;
    }

    // Отправляем заголовок ответа клиенту
    snprintf(response_header, sizeof(response_header), "HTTP/1.1 200 OK\r\nContent-Type: %s\r\n\r\n", mime_type);
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

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error binding socket");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 5) < 0) {
        perror("Error listening on socket");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("HTTP server running on port %d\n", PORT);

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_socket < 0) {
            perror("Error accepting connection");
            continue;
        }

        handle_request(client_socket, client_addr);
    }

    close(server_socket);
    return 0;
}
