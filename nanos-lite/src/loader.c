#include "common.h"

#define DEFAULT_ENTRY ((void *)0x4000000)

void ramdisk_read(void *, off_t, size_t);
void ramdisk_write(const void*, off_t, size_t);
size_t get_ramdisk_size();
extern uint8_t ramdisk_start;

uintptr_t loader(_Protect *as, const char *filename) {
  // TODO();
  ramdisk_read(DEFAULT_ENTRY, 0, get_ramdisk_size());
  return (uintptr_t)DEFAULT_ENTRY;
}
