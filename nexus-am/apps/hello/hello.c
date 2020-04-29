#include <am.h>

#include "stdio.h"

void print(const char *s) {
  for (; *s; s ++) {
    printf("??\n");
    _putc(*s);
  }
}
int main() {
  for (int i = 0; i < 10; i ++) {
    print("Hello World!\n");
  }
}
