#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>

void error(char *message) {
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

int main(void) {

  int socket_id = 0;
  if ((socket_id = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    error("socket");
  }

  struct sockaddr_in address = {
      .sin_family = AF_INET,
      .sin_addr.s_addr = inet_addr("127.0.0.1"),
      .sin_port = htons(8080),
  };

  if (connect(socket_id, (struct sockaddr *)&address, sizeof(address)) < 0) {
    error("connect");
  }

  struct A a = {
      .a = 518230123159632841,
      .b = 59581231,
      .c = "123 hello world from a!",
  };
  char a_buffer[sizeof(a) + 1];
  a_buffer[0] = 1;
  char *a_ptr = (char *)&a;
  for (int i = 1; i < sizeof(a) + 1; i++) {
    a_buffer[i] = *a_ptr;
    a_ptr++;
  }

  struct B b = {
      .a = 12039123,
      .b = "321 321 321 321 hello world from b! 321 321 321",
  };

  char b_buffer[sizeof(b) + 1];
  b_buffer[0] = 2;
  char *b_ptr = (char *)&b;
  for (int i = 1; i < sizeof(b) + 1; i++) {
    b_buffer[i] = *b_ptr;
    b_ptr++;
  }

  if (send(socket_id, a_buffer, sizeof(a_buffer), 0) < 0) {
    error("send");
  }

  if (send(socket_id, b_buffer, sizeof(b_buffer), 0) < 0) {
    error("send");
  }

  close(socket_id);
}
