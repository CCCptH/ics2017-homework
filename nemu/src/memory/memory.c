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

uint32_t page_translate(vaddr_t addr, bool iswrite) {
	if (cpu.cr0.paging == 1) {
		paddr_t pde_base = cpu.cr3.val;
		paddr_t pde_address = pde_base + ((addr >> 22) << 2);
		paddr_t pde = paddr_read(pde_address, 4);
		if (!(pde & 0x1)) {
			Log("addr = 0x%x, iswrite = %d", addr, iswrite);
			Log("pde = 0x%x, pde_base = 0x%x, pde_address = 0x%x", pde, pde_base, pde_address);
			assert(0);
		}

		paddr_t pte_base = pde & 0xfffff000;
		//paddr_t pte_address = pte_base + (((addr & 0x003ff000) >> 12) << 2);
		paddr_t pte_address = pte_base + ((addr & 0x003ff000) >> 10);
		paddr_t pte = paddr_read(pte_address, 4);
		if (!(pte & 0x1)) {
			Log("addr = 0x%x, iswrite = %d", addr, iswrite);
			Log("pte = 0x%x", pte);
			assert(0);
		}
		paddr_t page_address = (pte & 0xfffff000) + (addr & 0xfff);
		
		// set the access and dirty
		pde = pde | 0x20;
		pte = pte | 0x20;
		if (iswrite) {
			pde = pde | 0x40;
			pte = pte | 0x40;
		}
		paddr_write(pde_address, 4, pde);
		paddr_write(pte_address, 4, pte);
	
		return page_address;
	}
	else {
		return addr;
	}
}


uint32_t vaddr_read(vaddr_t addr, int len) {
  if (data_cross_page_boundary(addr, len)) {
    // Assert(0, "Data cross the page boundary!");
    vaddr_t p;
    paddr_t  paddr;
    uint32_t low, high;
    p = (int)(addr & 0xffff) + len - 0x1000;
    paddr = page_translate(addr, false);
    low = paddr_read(paddr, len-p);
    paddr = page_translate(addr + len - p, false);
    high = paddr_read(paddr, p);
    return (high << ((len-p) << 3)) + low;
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
