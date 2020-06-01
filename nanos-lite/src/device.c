#include "common.h"

#define NAME(key) \
  [_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};

size_t events_read(void *buf, size_t len) {
  return 0;
}

static char dispinfo[128] __attribute__((used));

void dispinfo_read(void *buf, off_t offset, size_t len) {
  strncpy(buf, dispinfo+offset, len);
}

void fb_write(const void *buf, off_t offset, size_t len) {
  assert(offset%4 && len%4 == true);
  // int height = _screen.height;
  int width = _screen.width;
  int index = offset/4;
  int screen_y1 = index/width;
  int screen_x1 = index%width;
  int screen_x2 = (offset+len)/4;
  int screen_y2 = screen_x2/width;
  if (screen_y2 == screen_y1) {
    _draw_rect(buf, screen_x1, screen_y1, len/4, 1);
  }
  else if (screen_y2 - screen_y1 == 1) {
    _draw_rect(buf, screen_x1, screen_x1, width - screen_x1, 1);
    _draw_rect(buf+(width - screen_x1)*4, 0, screen_y2, len/4 - (width - screen_x1), 1);
  }
  else {
    int tw = (width - screen_x1);
    int ty = (screen_y2 - screen_y1 - 1);
    _draw_rect(buf, screen_x1, screen_y1, tw, 1);
    _draw_rect(buf+tw*4, 0, screen_y1+1, width, ty);
    _draw_rect(buf+tw*4+ty*width*4, 0, screen_y2, len/4-tw-ty*width,  1);
  }
}

void init_device() {
  _ioe_init();

  // TODO: print the string to array `dispinfo` with the format
  // described in the Navy-apps convention
  int width = _screen.width;
  int height = _screen.height;
  Log("Inditializing device...");
  sprintf(dispinfo, "WIDTH:%dHEIGHT:%d\n", width, height);
  Log("Inditializing device...Done");
}
