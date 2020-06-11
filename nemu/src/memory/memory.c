#include "nemu.h"
#include "device/mmio.h"
#include "memory/mmu.h"

#define PMEM_SIZE (128 * 1024 * 1024)

#define pmem_rw(addr, type) *(type *)({\
    Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
    guest_to_host(addr); \
    })

uint8_t pmem[PMEM_SIZE];

/* Memory accessing interfaces */

uint32_t paddr_read(paddr_t addr, int len) {
  int map_NO = is_mmio(addr);
  if (map_NO!=-1) return mmio_read(addr, len, map_NO);
  else return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
}

void paddr_write(paddr_t addr, int len, uint32_t data) {
  int map_NO = is_mmio(addr);
  if (map_NO==-1) memcpy(guest_to_host(addr), &data, len);
  else mmio_write(addr, len, data, map_NO);
}

inline
bool data_cross_page_boundary (vaddr_t addr, int len) {
  return (addr & 0xfff) + len > 0x1000;
}

typedef union {
  vaddr_t addr;
  struct
  {
    vaddr_t offset: 12;
    vaddr_t page  : 10;
    vaddr_t dir   : 10;
  };
} PgAddr;

inline 
paddr_t page_translate (vaddr_t vaddr) {
  Log("cr0:%x", cpu.cr0.val);
  if (cpu.cr3.val != 0)
    Log("cr3:%x", cpu.cr3.val);
  if (cpu.cr0.paging != 1) return vaddr;
  PgAddr addr;
  addr.addr = vaddr;
  paddr_t pde_base = cpu.cr3.page_directory_base << 12;
  PDE pd;
  pd.val = paddr_read(pde_base + ((uint32_t)(addr.dir) << 2), 4);
  assert(pd.present);
  PTE pt;
  pt.val = paddr_read((pd.page_frame<<12)+(addr.page<<2), 4);
  assert(pt.present);
  return pt.page_frame + addr.offset;
}

uint32_t vaddr_read(vaddr_t addr, int len) {
  if (data_cross_page_boundary(addr, len)) {
    Assert(0, "Data cross the page boundary!");
  }
  else {
    paddr_t paddr = page_translate(addr);
    return paddr_read(paddr, len);
  }
  // return paddr_read(addr, len);
}

void vaddr_write(vaddr_t addr, int len, uint32_t data) {
  if (data_cross_page_boundary(addr, len)) {
    Assert(0, "Data cross the page boundary!");
  } else {
    paddr_t paddr = page_translate(addr);
    return paddr_write(paddr, len, data);
  }
  //paddr_write(addr, len, data);
}
