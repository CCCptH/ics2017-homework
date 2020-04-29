#include <am.h>

#include "stdio.h"

void print(const char *s) {
  for (; *s; s ++) {
    _putc(*s);
    printf("??\n");
  }
}
int main() {
  for (int i = 0; i < 10; i ++) {
    print("Hello World!\n");
  }
}
