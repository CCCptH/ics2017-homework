#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

#define MAX_EXPR_LEN 100

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  uint32_t value;
  char expr[MAX_EXPR_LEN];

} WP;

WP* new_wp();
void free_wp(WP *wp);
void free_wp_by_no(uint32_t no);
void display_wp();

#endif
