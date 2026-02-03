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
  uint32_t old_data;
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

  head = NULL;
  head -> NO = 0;
  free_ = wp_pool;
}
WP* new_wp() {
  if(free_ == NULL) {
    printf("NO more watchpoint\n");
    return NULL;
  }
  WP *p = free_;
  free_ = free_->next;
  p->next = NULL;
  p->expr = NULL;
  p->data = 0;
  return p;
}
/* TODO: Implement the functionality of watchpoint */
void add(char *e) {
  WP *p = new_wp();
  p->next = head;
  head = p;
  p->data = e;
  p->NO = head->NO;
  head ->NO++;
  p->expr = strdup(e);
  bool success = false;
  p->old_data = expr(e, &success);
  if (!success) {
    printf("Invalid expression: %s\n", e);
  }
}
void free_wp(WP *wp) {
  wp->next = free_;
  wp->expr = NULL;
  free_ = wp;
}
void delete(int NO) {
  WP *pre = head;
  WP *cur = head->next;
  int f = 0;
  while(cur != NULL) {
    if(cur->NO == NO) {
      pre->next = cur->next;
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
void list(){
  WP *p = head;
  while(p != NULL) {
    printf("watchpoint %d: %s = %s\n", p->NO,p->expr,p->data);
    p = p->next;
  }
}
