#include "common.h"
#include "fs.h"

#define DEFAULT_ENTRY ((void *)0x4000000)

void ramdisk_read(void *, off_t, size_t);
void ramdisk_write(const void*, off_t, size_t);
size_t get_ramdisk_size();
extern uint8_t ramdisk_start;

uintptr_t loader(_Protect *as, const char *filename) {
  // TODO();
  ramdisk_read(DEFAULT_ENTRY, 0, get_ramdisk_size());
  //int fd = fs_open(filename, 0, 0);
  //Log("filename=%s, fd=%d", filename, fd);
  //fs_read(fd, DEFAULT_ENTRY, fs_filesz(fd));
  //fs_close(fd);
  return (uintptr_t)DEFAULT_ENTRY;
}
