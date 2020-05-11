#ifndef __ARCH_H__
#define __ARCH_H__

#include <am.h>

#define PMEM_SIZE (128 * 1024 * 1024)
#define PGSIZE    4096    // Bytes mapped by a page

struct _RegSet {
  uintptr_t 
            error_code,
            eip, 
            CS,
            eflags;
  int       irq;    
  uintptr_t         // pushal
            edi,    // low addr
            esi,    //
            ebp,    //
            esp,    //
            ebx,    //
            edx,    //
            ecx,    //
            eax;    // high addr
};

#define SYSCALL_ARG1(r) 0
#define SYSCALL_ARG2(r) 0
#define SYSCALL_ARG3(r) 0
#define SYSCALL_ARG4(r) 0

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif
#endif
