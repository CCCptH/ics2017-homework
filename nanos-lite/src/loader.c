#include "common.h"
#include "fs.h"
#include "memory.h"

#define DEFAULT_ENTRY ((void *)0x8048000)

void ramdisk_read(void *, off_t, size_t);
void ramdisk_write(const void*, off_t, size_t);
size_t get_ramdisk_size();
extern uint8_t ramdisk_start;

uintptr_t loader(_Protect *as, const char *filename) {
  // TODO();
  // ramdisk_read(DEFAULT_ENTRY, 0, get_ramdisk_size());
  int fd = fs_open(filename, 0, 0);
  Log("filename=%s, fd=%d", filename, fd);
  // fs_read(fd, DEFAULT_ENTRY, fs_filesz(fd));
  size_t fsz = fs_filesz(fd);
  void* vaddr = DEFAULT_ENTRY;
  void* pg;
  while(fsz > 0) {
    pg = new_page();
    _map(as, vaddr, pg);
    fs_read(fd, pg, PGSIZE );
    vaddr += PGSIZE;
    fsz -= PGSIZE;
  }
  fs_close(fd);
  return (uintptr_t)DEFAULT_ENTRY;
}
