/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include "sdb.h"

#define NR_WP 32
char *strdup(const char *s){
  char *p = malloc(strlen(s)+1);
  strcpy(p,s);
  return p;
}


typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
  uint32_t data;
  char *expr;
  /* TODO: Add more members if necessary */

} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;


void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head  = NULL;
  free_ = wp_pool;
}


WP* new_wp() {
  if(free_ == NULL) {
    printf("NO more watchpoint\n");
    assert(0);
  }
  WP *p = free_;
  free_ = free_->next;
  p->next = NULL;
  p->expr = NULL;
  p->data = 0;
  return p;
}


/* TODO: Implement the functionality of watchpoint */
void add_point(char *e) {
  WP *p = new_wp();
  p->next = head;
  if(head != NULL) p->NO = head->NO + 1;
  else p->NO = 1;
  head = p;
  p->expr = strdup(e);
  bool success = false;
  p->data = expr(e, &success);
  if (!success) {
    printf("Invalid expression: %s\n", e);
    return;
  }
  printf("Add watchpoint %d: %s = %x\n", p->NO,p->expr,p->data);
}


void free_wp(WP *wp) {
  wp->next = free_;
  wp->expr = NULL;
  free_ = wp;
}


void delete_point(int NO) {
  WP *pre = NULL;
  WP *cur = head;
  int f = 0;
  while(cur != NULL) {
    if(cur->NO == NO) {
      printf("Delete watchpoint %d: %s \n", cur->NO,cur->expr);
      if(pre != NULL)pre->next = cur->next;
      else head = cur->next;
      free_wp(cur);
      f = 1;
      break;
    }
    pre = cur;
    cur = cur->next;
  }
  if(f == 0) {
    printf("NO such watchpoint\n");
  }
}


void watchpoint_display(){
  WP *p = head;
  while(p != NULL) {
    printf("watchpoint %d: %s = %x\n", p->NO,p->expr,p->data);
    p = p->next;
  }
}
void check_watchpoints(){
  WP *p = head;
  while(p != NULL) {
    bool success = false;
    uint32_t data_new = expr(p->expr, &success);
    if (!success) {
      printf("Invalid expression: %s\n", p->expr);
      return;
    }
    if(data_new != p->data) {
      printf("%d %s old_data= 0x%x\n", p->NO,p->expr, p->data);
      p->data = data_new;
      printf("%d %s new_data= 0x%x\n", p->NO,p->expr, p->data);
      if(nemu_state.state != NEMU_END && nemu_state.state != NEMU_STOP) {nemu_state.state = NEMU_STOP;}
    }
    p = p->next;
  }
}
