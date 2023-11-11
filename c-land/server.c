#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

void error(char* message)
{
    printf("%s (errno=%d)\n", message, errno);
    exit(errno);
}

typedef struct {
    unsigned long a;
    unsigned int b;
    char c[24];
} A;

typedef struct {
    unsigned int a;
    char b[48];
} B;

int listener_fd(void)
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

    if (bind(socket_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        error("could not bind");
    }

    if (listen(socket_fd, 0) < 0) {
        error("could not connect, is the server running?");
    }

    return socket_fd;
}

int accept_connection(int socket_fd)
{
    struct sockaddr_in address = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = inet_addr("127.0.0.1"),
        .sin_port = htons(8080),
    };

    unsigned int size = sizeof(address);

    int socket = accept(socket_fd, (struct sockaddr*)&address, &size);
    if (socket < 0) {
        error("could not accept");
    }
    printf("connected with fd %d\n", socket);
    return socket;
}

void copy_struct(
    char variant, char* buffer, char* instance, size_t instance_size
)
{
    *buffer = variant;
    buffer++;
    memcpy(buffer, instance, instance_size);
}

int min(int a, int b)
{
    if (a > b) {
        return b;
    }
    return a;
}

typedef enum {
    Padding = 0,
    StructA = 1,
    StructB = 2,
} Variant;

#define loop while (1)

int main(void)
{
    printf("starting...\n");
    int listener = listener_fd();
    const int BUFFER_SIZE = 32;
    char* io_buffer_start = malloc(BUFFER_SIZE);
    char* a_buffer_start = malloc(sizeof(A));
    char* b_buffer_start = malloc(sizeof(B));
    printf("running on 127.0.0.1:8080\n");
    // connection loop
    loop
    {
        printf("listening...\n");
        int client = accept_connection(listener);
        int size_remaining = 0;
        Variant variant = 0;
        char* a_buffer = a_buffer_start;
        char* b_buffer = b_buffer_start;
        int struct_buffer_idx = 0;

        // reading loop
        loop
        {
            char* io_buffer = io_buffer_start;
            int io_buffer_size = recv(client, io_buffer, BUFFER_SIZE, 0);

            if (io_buffer_size == 0) {
                break;
            }

            if (io_buffer_size < 0) {
                error("unable to read from buffer");
            };

            // parsing loop
            loop
            {
                if (io_buffer_size <= 0) {
                    break;
                }
                if (size_remaining == 0) {
                    printf("i=%d", io_buffer_size);
                    char next_variant = *io_buffer;
                    switch (next_variant) {
                        case Padding:
                            continue;
                        case StructA:
                            variant = StructA;
                            size_remaining = sizeof(A);
                            break;
                        case StructB:
                            variant = StructB;
                            size_remaining = sizeof(B);
                            break;
                        default:
                            printf("received variant %d\n", next_variant);
                            error("unknown variant");
                    }
                    io_buffer_size -= 1;
                    io_buffer++;
                    if (io_buffer_size <= 0) {
                        break;
                    }
                }

                switch (variant) {
                    case StructA:
                        a_buffer[struct_buffer_idx] = *io_buffer;
                    case StructB:
                        b_buffer[struct_buffer_idx] = *io_buffer;
                        break;
                    default:
                        error("unreachable: unknown variant");
                }

                io_buffer++;
                struct_buffer_idx++;
                size_remaining -= 1;
                io_buffer_size -= 1;

                if (size_remaining > 0) {
                    continue;
                }

                struct_buffer_idx = 0;
                switch (variant) {
                    case StructA: {
                        A a = *((A*)a_buffer);
                        printf(
                            "received A {\n    value_0=%lx,\n    "
                            "value_1=%x,\n    message='%s',\n}\n",
                            a.a,
                            a.b,
                            a.c
                        );
                        break;
                    }
                    case StructB: {
                        B b = *((B*)b_buffer);
                        printf(
                            "received B {\n    value_0=%x,\n    "
                            "message='%s',\n}\n",
                            b.a,
                            b.b
                        );
                        break;
                    }
                    default:
                        error("unreachable: unknown variant");
                }
            }
        }
    }
    free(io_buffer_start);
    free(a_buffer_start);
    free(b_buffer_start);
}
