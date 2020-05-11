#ifndef __ARCH_H__
#define __ARCH_H__

#include <am.h>

#define PMEM_SIZE (128 * 1024 * 1024)
#define PGSIZE    4096    // Bytes mapped by a page

struct _RegSet {
  uintptr_t         // pushal
            edi,    // low addr
            esi,    //
            ebp,    //
            esp,    //
            ebx,    //
            edx,    //
            ecx,    //
            eax;    // high addr
  int       irq;    
  uintptr_t 
            error_code,
            eip, 
            CS,
            eflags;
};

#define SYSCALL_ARG1(r)  0 //r->eax
#define SYSCALL_ARG2(r)  0 //r->ebx
#define SYSCALL_ARG3(r)  0 //r->ecx
#define SYSCALL_ARG4(r)  0 //r->edx

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif
#endif
