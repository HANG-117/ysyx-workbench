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

#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include <memory/vaddr.h>
#define ERROR UINT32_MAX
enum {
  TK_NOTYPE = 256, TK_EQ,TK_NEQ, TK_AND ,TK_NUMBER , TK_HEX , TK_REG  ,TK_DEREF

  /* TODO: Add more token types */

};
               
static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},                 
  {"0x[0-9a-fA-F]+", TK_HEX},           
  {"\\$[0-9a-zA-Z]+", TK_REG},            
  {"[0-9]+", TK_NUMBER},              
  {"\\*", '*'},
  {"\\+", '+'},
  {"-", '-'},
  {"/", '/'},
  {"\\(", '('},
  {"\\)", ')'},
  {"==", TK_EQ},
  {"!=", TK_NEQ},
  {"&&", TK_AND},
  
};
static int get_priority(int type) {
  switch (type) {
    case TK_DEREF:          return 12;  
    case '*': case '/':     return 10;
    case '+': case '-':     return 9;
    case TK_EQ: case TK_NEQ: return 7;
    case TK_AND:            return 2;    
    default:                return 0;
  }
}
#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[65336] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;




static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;
  //printf("make_token: %s\n",e);
  nr_token = 0;

  while (e[position] != '\0') {
    
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;
        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          case TK_NOTYPE: break;
          case TK_REG: {
            tokens[nr_token].type = TK_REG;
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0';
            nr_token++;
            break;
          }
          case TK_HEX: {
            tokens[nr_token].type = TK_HEX;
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0';
            nr_token++;
            break;
          }
          
          case '+': tokens[nr_token].type = '+'; nr_token++; break;
          case '-': tokens[nr_token].type = '-'; nr_token++; break;
          case '/': tokens[nr_token].type = '/'; nr_token++; break;
          case '*': tokens[nr_token].type = '*'; nr_token++; break;
          case '(': tokens[nr_token].type = '('; nr_token++; break;
          case ')': tokens[nr_token].type = ')'; nr_token++; break;
          case TK_EQ: tokens[nr_token].type = TK_EQ; nr_token++; break;
          case TK_NUMBER: {
            tokens[nr_token].type = TK_NUMBER;
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0';
            nr_token++;
            break;
          }
          
          case TK_NEQ: tokens[nr_token].type = TK_NEQ; nr_token++; break;
          case TK_AND: tokens[nr_token].type = TK_AND; nr_token++; break;
          default: TODO();
        }

        break;
      }
    }
    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

int eval(int p,int q);
int check_parentheses(int p,int q);




word_t expr(char *e, bool *success) {
  //printf("expr(%s)\n", e);
  if (!make_token(e)) {
    printf("make_token failed\n");
    *success = false;
    return 0;
  }
  *success = true;
  /* TODO: Insert codes to evaluate the expression. */
  word_t res = (word_t)eval(0,nr_token-1);
  if(res == ERROR) TODO();
  return res;
}


int eval(int p,int q){
  for(int i = p; i <= q; i++){
    if(tokens[i].type == '*'&&(i==0 || (tokens[i-1].type != TK_NUMBER && tokens[i-1].type != TK_REG && tokens[i-1].type != TK_HEX && tokens[i-1].type != ')' ))){
      tokens[i].type = TK_DEREF;      
    }
  }  
    if (p > q) {
    /* Bad expression */
    return ERROR;
  }
  else if (p == q) {
    if (tokens[p].type == TK_NUMBER) {
      return strtoul(tokens[p].str, NULL, 10);
    } else if (tokens[p].type == TK_HEX) {
      return strtoul(tokens[p].str + 2, NULL, 16);
    } else if (tokens[p].type == TK_REG) {
      bool ok;
      return isa_reg_str2val(tokens[p].str + 1, &ok);
      assert (ok == true);
    }
    return 0;
  }
  else if (check_parentheses(p, q) == true) {
    /* The expression is surrounded by a matched pair of parentheses.
     * If that is the case, just throw away the parentheses.
     */
    assert(check_parentheses(p, q) >= 0);
    return eval(p + 1, q - 1);
  }
  int op = -1;
  int op_pri =999;

  int in = 0;
  for (int i = p; i <= q; i++) {
    if (tokens[i].type == '(') in++;
    else if (tokens[i].type == ')') in--;
    else if (in == 0) {
      int pri = get_priority(tokens[i].type);
      if (pri > 0 && pri <= op_pri) {   
        op = i;
        op_pri = pri;
      }
    }
  }

  if (op == -1) {
    return 0;
  }

  int left = eval(p, op - 1);
  int right = eval(op + 1, q);

  switch (tokens[op].type) {
    case '+': return left + right;
    case '-': return left - right;
    case '*': return left * right;
    case '/':
      assert(right != 0);
      return left / right;
    case TK_EQ:   return left == right;
    case TK_NEQ:  return left != right;
    case TK_AND:  return left && right;   
    case TK_DEREF:
      return vaddr_read(right, 4);  
    default:
      return ERROR;
  }
}




int check_parentheses(int p, int q) {
    if (p > q) return -1;

    if (tokens[p].type != '(' || tokens[q].type != ')') {
        return 0;
    }

    int count = 0;
    for (int i = p+1; i <= q-1; i++) {
        if (tokens[i].type == '(') {
            count++;
        } else if (tokens[i].type == ')') {
            count--;
            if (count < 0) {
                return -1;
            }
        }
    }

    return (count == 0) ? 1 : -1;
}