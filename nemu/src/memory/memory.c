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
  // Log("Page data cross page boundary");
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


paddr_t page_translate (vaddr_t vaddr, bool write) {
  // Log("cr0:%x", cpu.cr0.val);
  // if (cpu.cr3.val != 0)
  //   Log("cr3:%x", cpu.cr3.val);
  //Log("pgtrans");
  if (cpu.cr0.paging != 1) return vaddr;
  PgAddr addr;
  addr.addr = vaddr;
  paddr_t pde_base = cpu.cr3.page_directory_base << 12;
  paddr_t pd_addr = pde_base + ((uint32_t)(addr.dir) << 2);
  PDE pd;
  pd.val = paddr_read(pd_addr, 4);
  Assert(pd.present, "addr=%x, pde=%x", vaddr, pd.val);
  PTE pt;
  paddr_t pt_addr = (pd.page_frame<<12)+((uint32_t)(addr.page)<<2);
  pt.val = paddr_read(pt_addr, 4);
  Assert(pt.present, "addr=%x, pde=%x, pte=%x, ptaddr=%x", vaddr, pd.val, pt.val, pt_addr);
  pd.accessed = 1;
  pt.accessed = 1;
  if (write) pt.dirty = 1;
  paddr_write(pd_addr, 4, pd.val);
  paddr_write(pt_addr, 4, pt.val);
  return (pt.page_frame << 12)+ addr.offset;
}

uint32_t vaddr_read(vaddr_t addr, int len) {
  if (data_cross_page_boundary(addr, len)) {
		//Log("in the read!!!!!!!!!!!!!!!!!!!!!!!!");
		/* this is a special case, you can handle it later. */
		int point;
		paddr_t paddr, low, high;
		// calculate the split point
		point = (int)((addr & 0xfff) + len - 0x1000);
		// get the low address
		paddr = page_translate(addr, false);
		low = paddr_read(paddr, len - point);
		// get the low address
		paddr = page_translate(addr + len - point, false);
		high = paddr_read(paddr, point);
		paddr = (high << ((len - point) << 3)) + low;
		return paddr;
	}
	else {
		paddr_t paddr = page_translate(addr, false);
		return paddr_read(paddr, len);
	}
  // return paddr_read(addr, len);
}

void vaddr_write(vaddr_t addr, int len, uint32_t data) {
  if (data_cross_page_boundary(addr, len)) {
    // Assert(0, "Data cross the page boundary!");
    vaddr_t p;
    paddr_t  paddr;
    uint32_t low, high;
    p = (int)(addr & 0xffff) + len - 0x1000;
    low = (data << (p << 3)) >> (p << 3);
		high = data >> ((len - p) << 3);
    paddr = page_translate(addr, true);
		paddr_write(paddr, len - p, low);
    paddr = page_translate(addr + len - p, true);
		paddr_write(paddr, p, high);
  } else {
    paddr_t paddr = page_translate(addr, true);
    return paddr_write(paddr, len, data);
  }
  //paddr_write(addr, len, data);
}
