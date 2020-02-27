#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP* new_wp() {
  // when empty
  if (free_ == NULL) {
    return NULL;
  }
  WP* temp = free_;
  free_ = free_->next;
  if (head == NULL) {
    head = temp;
    temp->next = NULL;
  }
  else {
    temp->next = head;
    head = temp;
  }
  return temp;
}

void free_wp(WP *wp) {
  WP* target = head;
  if (target == NULL) return ;
  else if (target == wp) {
    head = target->next;
    target->next = free_;
    free_ = target;
  }
  else {
    WP* prev = target;
    target = target->next;
    while (target != NULL) {
      if (target == wp) {
        prev->next = target->next;
        target->next = free_;
        free_ = target;
        break;
      }
      prev = prev->next;
      target = target->next;
    }
  }
  
}
