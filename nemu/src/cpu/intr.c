#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */
  TODO();
  rtl_push(&cpu.flags);
  cpu.eflags.IF = 0;
  cpu.eflags.TF = 0;
  rtl_push(&ret_addr);
  uint32_t base = cpu.idtr.base;
  uint32_t low = vaddr_read(base + NO*8, 4) & 0x0000ffff;
  uint32_t high = vaddr_read(base + NO*8 + 4, 4) & 0xffff0000;
  uint32_t offset = low | high;
  decoding.jmp_eip = offset;
  decoding.is_jmp = true;
}

void dev_raise_intr() {
}
