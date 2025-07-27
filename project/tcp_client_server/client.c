#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#define PORT 8181
#define SERVER_IP "127.0.0.1"

int main()
{
    int sock;
    struct sockaddr_in server_addr;
    char buffer[512];

    // Create the socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("socket");
        return -1;
    }

    // Initialize the server address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    // Convert IP address from text to binary
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0)
    {
        perror("inet_pton");
        close(sock);
        return -1;
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("connect");
        close(sock);
        return -1;
    }

    // Send something to the server (expects a read on server side)
    write(sock, "ping", 4);

    // Read the server's response
    int bytes = read(sock, buffer, sizeof(buffer) - 1);
    if (bytes < 0)
    {
        perror("read");
    }
    else
    {
        buffer[bytes] = '\0';  // Null-terminate the received string
        printf("Received: %s\n", buffer);
    }

    // Close the socket
    close(sock);
    return 0;
}