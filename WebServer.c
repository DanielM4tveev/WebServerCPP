#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    int bytes_read = read(client_socket, buffer, sizeof(buffer) - 1);
    
    if (bytes_read <= 0) {
        perror("read");
        close(client_socket);
        return;
    }

    // Null-terminate the buffer to make it a proper string
    buffer[bytes_read] = '\0';

    // Print the request (for debugging purposes)
    printf("Request:\n%s\n", buffer);

    // HTML content to serve
    const char *html_content = "HTTP/1.1 200 OK\r\n"
                               "Content-Type: text/html\r\n"
                               "Content-Length: 91\r\n"
                               "Connection: close\r\n\r\n"
                               "<!DOCTYPE html>\n"
                               "<html>\n"
                               "<head><title>Simple HTTP Server</title></head>\n"
                               "<body>\n"
                               "<h1>Hello from the Simple HTTP Server!</h1>\n"
                               "<p>This is a simple HTML page served by a C HTTP server.</p>\n"
                               "</body>\n"
                               "</html>\n";

    // Send the response
    write(client_socket, html_content, strlen(html_content));

    // Close the client socket
    close(client_socket);
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    // Create a socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Configure the server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind the socket
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_socket, 10) < 0) {
        perror("listen");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    // Accept incoming connections and handle them
    while (1) {
        if ((client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len)) < 0) {
            perror("accept");
            continue;
        }

        handle_client(client_socket);
    }

    // Close the server socket (this point will never be reached in this example)
    close(server_socket);
    return 0;
}
