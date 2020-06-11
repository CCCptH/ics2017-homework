#include "common.h"
#include "syscall.h"

#include "fs.h"

int mm_brk(uint32_t);

_RegSet* do_syscall(_RegSet *r) {
  uintptr_t a[4];
  a[0] = SYSCALL_ARG1(r);
  a[1] = SYSCALL_ARG2(r);
  a[2] = SYSCALL_ARG3(r);
  a[3] = SYSCALL_ARG4(r);

  switch (a[0]) {
    case SYS_none:
      SYSCALL_ARG1(r) = 1;
      break;
    case SYS_exit:
      _halt(SYSCALL_ARG2(r));
      break;
    case SYS_write:
    {
      //Log("Syscall write");
      int rs;
      int fd = a[1];
      void* buf = (void*)a[2];
      size_t len = a[3];
      if (fd == 1 || fd == 2) {
        char c;
        //Log("buffer:%s", (char*)buf);
        for (int i=0;i<len;i++) {
          memcpy(&c, buf+i, 1);
          _putc(c);
        }
        rs = len;
      }
      else if (fd >= 3) {
        rs = fs_write(fd, buf, len);
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
      SYSCALL_ARG1(r) = mm_brk(a[1]);
    }
    break;

    case SYS_open:
      SYSCALL_ARG1(r) = fs_open((char*)a[1], 0, 0);
      break;
    
    case SYS_read:
      SYSCALL_ARG1(r) = fs_read(a[1], (void*)a[2], a[3]);
      break;

    case SYS_close:
      SYSCALL_ARG1(r) = fs_close(a[1]);
      break;
    
    case SYS_lseek:
      SYSCALL_ARG1(r) = fs_lseek(a[1], a[2], a[3]);
      break;

    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}
