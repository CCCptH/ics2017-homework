#include "common.h"
#include "syscall.h"



_RegSet* do_syscall(_RegSet *r) {
  uintptr_t a[4];
  a[0] = SYSCALL_ARG1(r);
  a[1] = SYSCALL_ARG2(r);
  a[2] = SYSCALL_ARG3(r);
  a[3] = SYSCALL_ARG1(r);

  switch (a[0]) {
    case SYS_none:
      SYSCALL_ARG1(r) = 1;
      break;
    case SYS_exit:
      _halt(SYSCALL_ARG2(r));
      break;
    case SYS_write:
    {
      int rs;
      int fd = a[1];
      void* buf = (void*)a[2];
      size_t len = a[3];
      if (fd == 1 || fd == 2) {
        char c;
        for (int i=0;i<len;i++) {
          memcpy(&c, buf+i, 1);
          _putc(c);
        }
        rs = len;
      }
      else {
        panic("sys_write error, invalid fd\n");
        rs = -1;
      }
      SYSCALL_ARG1(r) = rs;
    }
    break;

    case SYS_brk:
    {
      SYSCALL_ARG1(r) = 0;
    }
    break;
    
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}
