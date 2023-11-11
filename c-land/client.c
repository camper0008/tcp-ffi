#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void error(char* message)
{
    printf("%s\n", message);
    exit(1);
}

struct A {
    unsigned long a;
    unsigned int b;
    char c[24];
};

struct B {
    unsigned int a;
    char b[48];
};

int connection_fd(void)
{
    int socket_fd = 0;
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        error("could not get socket");
    }

    struct sockaddr_in address = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = inet_addr("127.0.0.1"),
        .sin_port = htons(8080),
    };

    if (connect(socket_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        error("could not connect, is the server running?");
    }

    return socket_fd;
}

void copy_struct(
    char variant, char* buffer, char* instance, size_t instance_size
)
{
    *buffer = variant;
    buffer++;
    memcpy(buffer, instance, instance_size);
}

int main(void)
{
    struct A a = {
        .a = 0xCAFECAFE,
        .b = 0xDEADBEEF,
        .c = "123 hello world from a!",
    };

    char a_buffer[sizeof(a) + 1];
    copy_struct(1, a_buffer, (char*)&a, sizeof(a));

    struct B b = {
        .a = 0x180504,
        .b = "321 321 321 321 hello world from b! 321 321 321",
    };

    char b_buffer[sizeof(b) + 1];
    copy_struct(2, b_buffer, (char*)&b, sizeof(b));

    int socket_fd = connection_fd();
    if (send(socket_fd, a_buffer, sizeof(a_buffer), 0) < 0) {
        error("could not send");
    }
    if (send(socket_fd, b_buffer, sizeof(b_buffer), 0) < 0) {
        error("could not send");
    }
    printf("sent!\n");
    close(socket_fd);
}
