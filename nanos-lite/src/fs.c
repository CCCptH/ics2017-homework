#include "fs.h"

typedef struct {
  char *name;
  size_t size;
  off_t disk_offset;
  off_t open_offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB, FD_EVENTS, FD_DISPINFO, FD_NORMAL};

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  {"stdin (note that this is not the actual stdin)", 0, 0},
  {"stdout (note that this is not the actual stdout)", 0, 0},
  {"stderr (note that this is not the actual stderr)", 0, 0},
  [FD_FB] = {"/dev/fb", 0, 0},
  [FD_EVENTS] = {"/dev/events", 0, 0},
  [FD_DISPINFO] = {"/proc/dispinfo", 128, 0},
#include "files.h"
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

size_t fs_filesz(int fd) {
  assert(fd>=0 && fd<NR_FILES);
  return file_table[fd].size;
}

off_t disk_offset(int fd) {
  assert(fd>=0 && fd<NR_FILES);
  return file_table[fd].disk_offset;
}

off_t get_open_offset(int fd) {
  assert(fd>=0 && fd<NR_FILES);
  return file_table[fd].open_offset;
}

void set_open_offset(int fd, off_t off) {
  assert(fd>=0 && fd<NR_FILES);
  assert(off>=0);
  if (off>file_table[fd].size) {
    off = file_table[fd].size;
  }
  file_table[fd].open_offset = off;
}

extern void ramdisk_read(void* buf, off_t offset, size_t len);
extern void ramdisk_write(void* buf, off_t offset, size_t len);

int fs_open(const char* name, int flags, int mode) {
  for (int i=0; i<NR_FILES; i++) {
    if (strcmp(name, file_table[i].name) == 0) {
      return i;
    }
  }
  //panic("file not exist");
  return -1;
}

extern void dispinfo_read(void *buf, off_t offset, size_t len);
extern size_t events_read(void *buf, size_t len) ;
ssize_t fs_read(int fd, void* buf, size_t len) {
  assert(fd>=0 && fd<NR_FILES);
  int n=fs_filesz(fd) - get_open_offset(fd);
  if (n<len) len = n;
  switch (fd)
  {
  case 0:
  case 1:
  case 2:
    Log("arg invalid: fd < 3");
    return 0;
  case FD_EVENTS:
    return events_read(buf, len);
  case FD_DISPINFO:
  //Log("reading....");
    dispinfo_read(buf, get_open_offset(fd), len);
    break;
  default:
    ramdisk_read(buf, disk_offset(fd) + get_open_offset(fd), len);
    break;
  }
  set_open_offset(fd, get_open_offset(fd) + len);
  return len;
}
extern void fb_write(const void *buf, off_t offset, size_t len);

ssize_t fs_write(int fd, void* buf, size_t len) {
  assert(fd>=0 && fd<NR_FILES);
  int n = fs_filesz(fd) - get_open_offset(fd);
  if (n<len) len = n;
  
  switch (fd)
  {
  case 0:
  case 1:
  case 2:
    Log("arg invalid: fd < 3");
    return 0;
  case FD_FB:
    fb_write(buf, get_open_offset(fd), len);
    break;
  
  default:
    ramdisk_write(buf, disk_offset(fd) + get_open_offset(fd), len);
    break;
  }
  set_open_offset(fd, get_open_offset(fd) + len);
  return len;
}

off_t fs_lseek(int fd, off_t offset, int whence) {
  switch (whence)
  {
  case SEEK_SET:
    set_open_offset(fd, offset);
    return get_open_offset(fd);
    break;
  case SEEK_CUR:
    set_open_offset(fd, get_open_offset(fd) + offset);
    return get_open_offset(fd);
    break;
  case SEEK_END:
    set_open_offset(fd, fs_filesz(fd) + offset);
    return get_open_offset(fd);
    break;
  default:
    panic("Unhandled whence ID = %d", whence);
    return -1;
    break;
  }
}

int fs_close(int fd) {
  assert(fd>=0 && fd<NR_FILES);
  return 0;
}

void init_fs() {
  // TODO: initialize the size of /dev/fb
  file_table[FD_FB].size = _screen.height * _screen.width * 4;
}
