#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

#include<stdlib.h>

#define BAD_EXPR INT32_MAX

enum {
  TK_NOTYPE = 256, TK_EQ,

  /* TODO: Add more token types */
  TK_NUM, TK_NEQ, TK_HEX, TK_REG, TK_AND, TK_OR,
  TK_DEREF, TK_NEG
};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"==", TK_EQ},        // equal
  {"!=", TK_NEQ},       // not equal
  {"&&", TK_AND},       // l_and
  {"\\|\\|", TK_OR},    // l_or
  {"-", '-'},           // minus
  {"\\*", '*'},         // mul
  {"\\/", '/'},         // div
  {"\\(", '('},         // lb
  {"\\)", ')'},         // rb
  {"0[xX][0-9a-fA-F]+", TK_HEX},// hexnum
  {"[0-9]+", TK_NUM},   // decnum
  {"\\$e(([abcd]x)|[sd]i|[bsi]p)", TK_REG}   // reg
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

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

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

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
          case TK_NOTYPE:
            break;
          case '+':
          case '-':
          case '*':
          case '/':
          case TK_OR:
          case TK_AND:
          case TK_EQ:
          case TK_NEQ:
          case '(':
          case ')':
            tokens[nr_token].str[0]='\0';
            tokens[nr_token].type = rules[i].token_type;
            ++nr_token;
            break;
          case TK_NUM:
          case TK_REG:
          case TK_HEX:
          {
            tokens[nr_token].type = rules[i].token_type;
            if (substr_len >31) {
              printf("Number overflow\n");
              return false;
            }
            int i;
            for (i = 0; i < substr_len; i++) 
              tokens[nr_token].str[i] = substr_start[i];
            ++nr_token;
            break;
          }
          default: 
            break;
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

int eval(uint32_t, uint32_t);
int8_t check_parentheses(uint32_t, uint32_t);
bool is_operator(Token*);
uint32_t get_dominant_op_index(uint32_t, uint32_t);
int get_priority(uint32_t);

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  // TODO();

  // neg and deref
  int i;
  for (i = 0; i < nr_token; i++) {
    if (tokens[i].type == '*'
    && (i == 0 || is_operator(&tokens[i - 1]))) {
      tokens[i].type = TK_DEREF;
    }
    else if (tokens[i].type == '-'
    && (i == 0 || is_operator(&tokens[i - 1]))) {
      tokens[i].type = TK_NEG;
    }
  }

  int result = eval(0, nr_token-1);
  if (result == BAD_EXPR) {
    *success = false;
    printf("expr:%d\n", BAD_EXPR);
    return 0;
  }
  *success = true;
  return result;
}

int eval(uint32_t p, uint32_t q) {
  if (p > q) {
    printf("Bad expression!\n");
    return BAD_EXPR;
  }
  else if (p == q) {
    if (tokens[p].type == TK_REG) {
      #define cmp_reg(r) (strcmp(tokens[p].str, "$"#r)==0)
      if(cmp_reg(eax)) return cpu.eax;
      else if (cmp_reg(ebx)) return cpu.ebx;
      else if (cmp_reg(ecx)) return cpu.ecx;
      else if (cmp_reg(edx)) return cpu.edx;
      else if (cmp_reg(esi)) return cpu.esi;
      else if (cmp_reg(esp)) return cpu.esp;
      else if (cmp_reg(ebp)) return cpu.ebp;
      else if (cmp_reg(edi)) return cpu.edi;
      else return cpu.eip;
      #undef cmp_reg
    }
    else if (tokens[p].type == TK_NUM) {
      return atoi(tokens[p].str);
    }
    else if (tokens[p].type == TK_HEX) {
      uint32_t i = 2;
      uint32_t result = 0;
      while (tokens[p].str[i] != '\0') {
        int base;
        if (tokens[p].str[i] >= '0' && tokens[p].str[i] <= '9') {
          base = tokens[p].str[i] - '0';
        }
        else if (tokens[p].str[i] >='a' && tokens[p].str[i] <= 'f') {
          base = tokens[p].str[i] - 'a' + 10;
        }
        else {
          base = tokens[p].str[i] - 'A';
        }
        result = result * 16 + base;
        ++i;
      }
      return result;
    }
    else {
      printf("Unknown token\n");
      return BAD_EXPR;
    }
  }
  else if (check_parentheses(p, q) == 1) {
    return eval(p+1, q-1);
  }
  else if (check_parentheses(p, q) == -1) {
    printf("Bad expression, parentheses unmatched!\n");
    return BAD_EXPR;
  }
  else {
    uint32_t dominant_op_index = get_dominant_op_index(p,q);
    // 二元运算符
    if (dominant_op_index != p) {
      int expr1 = eval(p, dominant_op_index - 1);
      int expr2 = eval(dominant_op_index + 1, q);
      if (expr1 == BAD_EXPR || expr2 == BAD_EXPR) return BAD_EXPR;
      switch (tokens[dominant_op_index].type) {
      case '+':
        return expr1 + expr2;
      case '-':
        return expr1 - expr2;
      case '*':
        return expr1 * expr2;
      case '/':
        return expr1 / expr2;
      case TK_OR:
        return expr1 || expr2;
      case TK_AND:
        return expr1 && expr2;
      case TK_EQ:
        return expr1 == expr2;
      case TK_NEQ:
        return expr1 != expr2;
      default:
        assert(0);
        return BAD_EXPR;
      }
    }
    // 一元运算符
    else {
      int expr = eval(p + 1, q);
      switch (tokens[dominant_op_index].type) {
      case TK_NEG:
        return -expr;
      case '!':
        return !expr;
      case TK_DEREF:
        return vaddr_read(expr, 4);
      default:
        assert(0);
        return BAD_EXPR;
      }
    }
  }
}

int8_t check_parentheses(uint32_t p, uint32_t q) {
  uint32_t i;
  uint8_t stack = 0;
  int8_t flag = 1;
  for(i = p; i <= q; i++) {
    if (tokens[i].type == '(') ++stack;
    else if (tokens[i].type == ')') -- stack;
    if (stack == 0 && i < q) flag = 0;  // 整个表达式没被括号包裹
    if (stack < 0) flag = -1;           // 右括号多了， 表达式不对
  }
  if (stack > 0) flag = -1;             // 左括号多了
  return flag;
}

inline bool is_operator(Token *token) {
  switch (token->type)
  {
  case TK_EQ:
  case TK_NEQ:
  case TK_OR:
  case TK_AND:
  case '+':
  case '-':
  case '*':
  case '/':
    return true;
  default:
    return false;
  }
}

uint32_t get_dominant_op_index(uint32_t p, uint32_t q) {
  int i;
  uint32_t parentheses_flag = 0;
  uint32_t dominant = 0;
  uint32_t dominant_index = p;
  for (i = p; i <= q; i++) {
    if (!is_operator(&tokens[i])) continue;
    else if (parentheses_flag > 0) continue;
    else {
      if (tokens[i].type == '(') ++parentheses_flag;
      else if (tokens[i].type == ')') --parentheses_flag;
      else {
        if (get_priority(tokens[i].type) >= get_priority(dominant)) {
          dominant = tokens[i].type;
          dominant_index = i;
        }
      }
    }
  }
  return dominant_index;
}

inline int get_priority(uint32_t op) {
  const int base_priority = 100;
  // low to high
  switch (op)
  {
  case TK_OR:
    return base_priority-0;
  case TK_AND:
    return base_priority-1;
  case TK_EQ:
  case TK_NEQ:
    return base_priority-2;
  case '+':
  case '-':
    return base_priority-3;
  case '*':
  case '/':
    return base_priority-4;
  case TK_NEG:
  case TK_DEREF:
  case '!':
    return base_priority-5;
  default:
    return -1;
  }
}
