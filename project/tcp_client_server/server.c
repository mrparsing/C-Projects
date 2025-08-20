#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>

#define PORT 8181

int main()
{
    int s, c;
    socklen_t addrlen;
    struct sockaddr_in server, client;
    char buffer[512];
    const char *response = "HELLO";

    memset(&server, 0, sizeof(server));
    memset(&client, 0, sizeof(client));

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0)
    {
        perror("socket");
        return -1;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    if (bind(s, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("bind");
        close(s);
        return -1;
    }

    if (listen(s, 5) < 0)
    {
        perror("listen");
        close(s);
        return -1;
    }

    printf("Listening on port %d...\n", PORT);

    addrlen = sizeof(client);

    c = accept(s, (struct sockaddr *)&client, &addrlen);
    if (c < 0)
    {
        perror("accept");
        close(s);
        return -1;
    }

    printf("Client connected: %s\n", inet_ntoa(client.sin_addr));

    ssize_t bytes_read = read(c, buffer, sizeof(buffer) - 1);
    if (bytes_read > 0)
    {
        buffer[bytes_read] = '\0';
        printf("Received: %s\n", buffer);

        write(c, response, strlen(response));
    }
    else
    {
        printf("Read failed or client disconnected.\n");
    }

    close(c);
    close(s);

    return 0;
}