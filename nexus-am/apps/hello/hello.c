#include <am.h>

void print(const char *s) {
  for (; *s; s ++) {
    // _putc(*s);
    _putc('a');
  }
}
int main() {
  for (int i = 0; i < 10; i ++) {
    print("Hello World!\n");
  }
}
