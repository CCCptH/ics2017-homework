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

void free_wp_by_no(uint32_t no) {
  WP* iterator;
  for (iterator = head; iterator != NULL; iterator = iterator->next) {
    if (iterator->NO == no) {
      free_wp(iterator);
      printf("watchpoint %u is deleted!\n", no);
      break;
    }
  }
}

void display_wp() {
  WP* iterator;
  printf("No\tHex\t\tDec\t\tExpr\n");
  for(iterator = head; iterator != NULL; iterator = iterator->next) {
    printf("%u\t0x%08x\t%u\t\t%s\n",
    iterator->NO, iterator->value,
    iterator->value, iterator->expr);
  }
}

// if changed, return 1
bool update_wp () {
  WP* iterator;
  bool flag = false;
  for(iterator = head; iterator != NULL; iterator = iterator->next) {
    bool success_flag;
    int new_val = expr(iterator->expr, &success_flag);
    if (new_val != iterator->value) {
      printf("watchpoint %u has changed! previous value is %d, current value is %d, expr is %s\n",
      iterator->NO, iterator->value, new_val, iterator->expr);
      flag = true;
      iterator->value = new_val;
    }
  }
  return flag;
}
