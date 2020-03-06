---
​---
title: 系统综合设计PA1实验报告
author: 1711326 郝润来
​---
---

<div>
	<p style="font-size:24px"><b>系统综合设计PA1实验报告</b></p>
    <p>1711326 郝润来</p>
</div>

---

[toc]



# 1 概述

## 1.1 实验目的

实现经过简化的x86全系统模拟器， 能够让超级玛丽、魂斗罗等程序在上面运行。

## 1.2 实验内容

1. 使用`git`切换到`pa1`分支
2. 尝试在虚拟机上运行`Hello World`、`超级玛丽`等程序
3. 实现正确的寄存器结构
4. 实现简易的调试器
   + `si` 单步执行
   + `info` 打印寄存器或者断点
   + `x`  扫描内存
   + `w`  创建监视点
   + `d` 删除监视点

# 2 阶段一

## 2. 1 简单计算机模型

### 2.1.1 NEMU 代码执行流程

 NEMU模拟了x86简单的寄存器结构

```
 31                23                15                7               0
+-----------------+-----------------+-----------------+-----------------+
|                                  EAX       AH       AX      AL        |
|-----------------+-----------------+-----------------+-----------------|
|                                  EDX       DH       DX      DL        |
|-----------------+-----------------+-----------------+-----------------|
|                                  ECX       CH       CX      CL        |
|-----------------+-----------------+-----------------+-----------------|
|                                  EBX       BH       BX      BL        |
|-----------------+-----------------+-----------------+-----------------|
|                                  EBP                BP                |
|-----------------+-----------------+-----------------+-----------------|
|                                  ESI                SI                |
|-----------------+-----------------+-----------------+-----------------|
|                                  EDI                DI                |
|-----------------+-----------------+-----------------+-----------------|
|                                  ESP                SP                |
+-----------------+-----------------+-----------------+-----------------+
```

CPU根据一个特殊的寄存器`EIP`保存现在执行的指令。

```
 31                23                15                7               0
+-----------------+-----------------+-----------------+-----------------+
|                       EIP (INSTRUCTION POINTER)                       |
+-----------------+-----------------+-----------------+-----------------+
```

NEMU的代码执行流程为：

1. 从`EIP`指示的位置取出指令
2. 执行指令
3. 更新`EIP`
4. 执行第第一步

### 2.1.2 代码： 实现正确的寄存器结构体

寄存器结构体的定义在`nemu/include/cpu/reg.h`中

正确的寄存器结构代码如下

```c
typedef struct {
  union {
    union {
      uint32_t _32;
      uint16_t _16;
      uint8_t _8[2];
    } gpr[8];

    /* Do NOT change the order of the GPRs' definitions. */

    /* In NEMU, rtlreg_t is exactly uint32_t. This makes RTL instructions
    * in PA2 able to directly access these registers.
    */
    struct {
      rtlreg_t eax, ecx, edx, ebx, esp, ebp, esi, edi;
    };
  };

  vaddr_t eip;

} CPU_state;
```

+ 由于`gpr`中的`_32` `_16` `_8` 分别代表寄存器的32位、低16位、低8位,所以`uint32_t __32, uint16_t 16, uint_8 _8[2]`共用同一个内存空间， 使用`union`
+ `rtlreg_t eax, ecx, edx, ebx, esp, ebp, esi, edi`和`gpr[8]`使用共同的内存空间， `eax`相当于`gpr[0]`、`ebx`相当于`gpr[1]`以此类推， 所以使用`union`。
+ 使用`struct`将`rtlreg_t eax, ecx, edx, ebx, esp, ebp, esi, edi`变为一个匿名结构体， 告诉编译器`eax`, `ecx`, `edx`,` ebx`, `esp`, `ebp`, `esi`, `edi`是一个整体， 占用连续的`8×32`位空间。 如果不加`struct`，`eax` `ebx`等寄存器将都和`gpr[0]`共用一个内存空间。

### 2.1.3 问题： 究竟要执行多久

> 在`cmd_c()`函数中, 调用`cpu_exec()`的时候传入了参数`-1`, 你知道这是什么意思吗?

`cpu_exec()`声明如下

```c
void cpu_exec(uint64_t n);
```

当我们传入`-1` 时， 由于参数的类型是`uint64_t`， 它是一个无符号类型。 `-1`会转换成他的补码形式`0xffff ffff ffff ffff`， 变成最大的`uint64_t`所支持的整数。可以理解为执行完所有指令。

### 2.1.4 问题： 谁来指示程序的结束

代码执行到一个特殊的指令`nemu_trap`停止。这个值令是为了在NEMU中让客户程序指示执行的结束而加入的.

> 在程序设计课上老师告诉你, 当程序执行到`main()`函数返回处的时候, 程序就退出了, 你对此深信不疑. 但你是否怀疑过, 凭什么程序执行到`main()`函数的返回处就结束了? 如果有人告诉你, 程序设计课上老师的说法是错的, 你有办法来证明/反驳吗? 如果你对此感兴趣, 请在互联网上搜索相关内容.

`main()`函数返回后还能继续执行代码。 他叫`atexit()` ，是一个特殊的函数。它是正常程序退出时调用的注册终止函数。

例如：

```c++
#include <stdlib.h>
#include <iostream>
#include <string>
using namespace std;
void foo() {
    cout<<"After Main..."<<endl;
}
int main(int argc, char*argv[]) {
    atexit(&foo);
    cout<<"End of Main"<<endl;
    return 0;
}
```

输出：

```cmd
$ End of Main
$ After Main...
```

可以看到并不是`return`之后就结束了程序的执行。在`return`之后， 会调用`atexit`这种注册终止函数。

## 2.2 基础设施：简易调试器

简易调试器所支持的命令

| 命令         | 格式          | 使用举例          | 说明                                                         |
| ------------ | ------------- | ----------------- | ------------------------------------------------------------ |
| 帮助         | `help`        | `help`            | 打印命令的帮助信息                                           |
| 继续运行     | `c`           | `c`               | 继续运行被暂停的程序                                         |
| 退出         | `q`           | `q`               | 退出NEMU                                                     |
| 单步执行     | `si [N]`      | `si 10`           | 让程序单步执行`N`条指令后暂停执行, 当`N`没有给出时, 缺省为`1` |
| 打印程序状态 | `info SUBCMD` | `info r` `info w` | 打印寄存器状态 打印监视点信息                                |
| 表达式求值   | `p EXPR`      | `p $eax + 1`      | 求出表达式`EXPR`的值, `EXPR`支持的 运算请见[调试中的表达式求值](https://nju-ics.gitbooks.io/ics2017-programming-assignment/content/1.5.html)小节 |
| 扫描内存     | `x N EXPR`    | `x 10 $esp`       | 求出表达式`EXPR`的值, 将结果作为起始内存 地址, 以十六进制形式输出连续的`N`个4字节 |
| 设置监视点   | `w EXPR`      | `w *0x2000`       | 当表达式`EXPR`的值发生变化时, 暂停程序执行                   |
| 删除监视点   | `d N`         | `d 2`             | 删除序号为`N`的监视点                                        |

支持的命令在`/src/monitor/ui.c`中定义

```c
static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },

  /* TODO: Add more commands */
  { "si", "step n instructions", cmd_si },
  { "info", "print info", cmd_info },
  { "x", "scan memory", cmd_x },
  { "p", "figure out expr", cmd_p },
  { "w", "watchpoint", cmd_w },
  { "d", "delete watchpoint", cmd_d }
};
```

### 2.2.1 代码： 实现单步执行、打印寄存器、扫描内存

为了能少打一点字， 定义了两个宏

```c++
#define __cr static int
#define __tk = strtok(NULL, " ")
```

**a) 单步执行:**

```c
__cr cmd_si(char *args) {
  char *arg __tk;
  if (arg == NULL)
    cpu_exec(1);
  else
    cpu_exec(atoi(arg));
  return 0;
}
```

通过`strtok(NULL, "")`获取命令参数。如果没有参数，则执行一条指令。如果有参数， 则通过`atoi(arg)`将参数转换成`int`类型， 再调用`cpu_exec()`

实验结果：

![2-1. si指令](./pa1/2-1.png)

**b) 打印寄存器**:

```c
__cr cmd_info (char *args) {
  char *arg __tk;
  if (arg == NULL) {
    printf("An arg is needed!\n");
    return 0;
  }
  if (strcmp(arg, "r") == 0)  {
    #define printER(r) printf("e"#r"\t\t0x%08x\t\t%u\n", cpu.e##r, cpu.e##r)
    printER(ax);
    printER(bx);
    printER(cx);
    printER(dx);
    printER(sp);
    printER(bp);
    printER(si);
    printER(di);
    printER(ip);
    #undef printER
  }
  else if (strcmp(arg, "w") == 0) {
    display_wp();
  }
  else {
    printf("Undefine Args. \n\t r: print registers\n\t w: print watchpoints\n");
  }
  return 0;
}
```

1. 首先通过`strtok(NULL, " ")`获取参数
2. 如果参数为空或者不是支持的参数，则在cli中打印出错误
3. 如果参数为`r`则打印寄存器，定义了一个宏能方便的打印出寄存器信息
4. 如果参数为`w`则打印所有监视点

实验结果：

![2-2. info指令](./pa1/2-2.png)

**c) 扫描内存**

```c
__cr cmd_x (char *args) {
  char *arg __tk;
  if (arg == NULL) {
    printf("Args are needed!\n");
    return 0;
  }
  int n = atoi(arg);
  arg __tk;
  if (arg == NULL) {
    printf("Args are needed!\n");
    return 0;
  }
  bool success_flag;
  // int result = expr(arg, &success_flag);
  int addr = expr(arg, &success_flag);
  if (!success_flag) {
    Log("\033[0;31m""Bad Expression!\n""\033[0m");
    return 0;
  }
  printf("Memory:");
  int i;
  for (i=0; i < n; i++)
  {
    int value = vaddr_read(addr+i, 1);
    if (i%4 == 0) {
      printf("\n0x%08x:  0x%02x", addr+i, value);
    }
    else {
      printf("  0x%02x", value);
    }
  }
  printf("\n");
  return 0;
}
```

1. 通过`strtok(NULL, " ")`获取第一个参数, 并且判断参数是否存在， 不存在则输出错误
2. 通过`atoi(arg)`将参数转换为整数
3. 获取第二个参数并且判断是否存在
4. 调用`expr()` 对参数进行求值， 根据`success_flag`的结果判断继续执行还是输出错误并且返回
5. 调用`vaddr_read()`，读取内存数据，并且将结果输出

实验结果：

![2-3. x命令](./pa1/2-3.png)

# 3 阶段二

## 3.1 词法分析

### 3.1.1 代码： 实现算数表达式的词法分析

通过正则表达式实现词法分析。

在`/src/monitor/debug/expr.c`中，定义了词法规则

```c
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
  {"!", '!'},           // not
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
```

+ 要先识别十六进制数，再识别十进制数， 也就是要把十六进制的正则写在十进制 的前面， 否则无法识别出十六进制数。
+ 要将`\`转义， 比如`“\\+”`才表示正则规则`\+`

词法单元定义：

```c
enum {
  TK_NOTYPE = 256, TK_EQ,

  /* TODO: Add more token types */
  TK_NUM, TK_NEQ, TK_HEX, TK_REG, TK_AND, TK_OR,
  TK_DEREF, TK_NEG
};
```

通过`init_regex()`初始化正则表达式的分析器。

```c
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
```

通过`make_token()`进行词法分析

```c
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
          case '!':
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
            for (i = 0; i < substr_len; i++) {
              if (i<substr_len) tokens[nr_token].str[i] = substr_start[i];
              else tokens[nr_token].str[i] = '\0';
            }
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
```

根据匹配到的词法规则识别出不同的词法单元

+ 空白字符`TK_NOTYPE`忽略
+ 设置运算符类的词法单元的`token.type`
+ 设置数字`TK_NUM`, 16进制`TK_HEX`和寄存器`TK_REG`的`token.type`和`token.str`

## 3.2 表达式求值

### 3.2.1 代码： 实现算数表达式的递归求值

表达式的递归求值为`/src/monitor/debug/expr.c`中的`int eval(uint32_t p, uint32_t q)`函数：

```c
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
          base = tokens[p].str[i] - 'A' + 10;
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
      if (expr == BAD_EXPR) return BAD_EXPR;
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
```

1. 我定义了一个宏：

   ```c
   #define BAD_EXPR INT32_MAX
   ```

   当表达式错误时， 返回这个值

2. 当`p > q`时， 表达式错误， 输出错误，返回`BAD_EXPR`

3. 当`p = q`时， 返回这个词素的值

   1. 如果这个token是`TK_REG`, 则返回寄存器的值。
      为了方便的对比字符串的值， 定义了一个宏`cmp_reg`
   2. 如果这个token是`TK_NUM`，转换为整数类型并且返回这个值
   3. 如果这个token是`TK_HEX`，转换为十进制并且返回

4. 如果`p < q`

   1. 首先检查括号，检查括号的函数是`check_parentheses()`

      ```c
      int8_t check_parentheses(uint32_t p, uint32_t q) {
        uint32_t i;
        uint8_t stack = 0;
        int8_t flag = 1;
        for(i = p; i <= q; i++) {
          if (tokens[i].type == '(') ++stack;
          else if (tokens[i].type == ')') -- stack;
          if (stack == 0 && i < q) flag = 0;  // 整个表达式没被括号包裹
          if (stack < 0) {
            flag = -1;           // 右括号多了， 表达式不对
            break;
          }
        }
        if (stack > 0) flag = -1;             // 左括号多了
        return flag;
      }
      ```

      + 当`(`出现，入栈
      + 当`)`出现， 出栈
      + 若循环未结束， `stack`为0, 则整个表达式没有被括号包裹， 返回`0`
      + 若循环未结束， `stack`小于0, 则整个表达式右括号未匹配， 返回`-1`
      + 若循环结束，`stack`大于0, 则整个表达式左括号未匹配，返回-1
      + 若整个表达式被括号包裹， 则返回`1`

   2. 如果整个表达式被括号包裹， 则忽略括号， 调用

      ```c
      eval(p+1, q-1);
      ```

   3. 如果整个表达式没有被括号包裹

      1. 先调用`get_dominant_op_index()`找到dominant operator。
         表达式中，不在括号内，并且优先级最低出现在最右边的运算符是dominant operator

         ```c
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
         ```

         通过`get_priority()`获取运算符的优先级， 优先级越低， 返回的数值越高

         ```c
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
         ```

      2. 判断dominant operator是一个一元运算符还是一个二元运算符

         + 一元运算符

           一元运算符有解引用， 取反和取负数。
           调用`eval(P + 1, q)`求出操作数的值，然后根据运算符的类型进行不同的操作

         + 二元运算符

           二元运算符调用`eval(p, dominant_op_index - 1)`和`eval(dominant_op_index + 1, q)`求出左运算数和右运算数，然后根据运算符进行不同的求值操作

         + 如果求操作数返回了`BAD_EXPR`那直接返回`BAD_EXPR`

然后实现表达式求值指令。

1. 首先实现`expr()`

   ```c
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
       && (i == 0 || is_operator(&tokens[i - 1]) || tokens[i - 1].type == '(')) {
         tokens[i].type = TK_DEREF;
       }
       else if (tokens[i].type == '-'
       && (i == 0 || is_operator(&tokens[i - 1]) || tokens[i - 1].type == '(')) {
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
   ```

   如果求值成功， `success_flag`为`true`，否则为`false`

2. 然后实现`cmd_p()`

   ```c
   __cr cmd_p (char *args) {
     char *arg __tk;
     if (arg == NULL) {
       Log("Need a expr!");
     }
     bool success_flag;
     int result = expr(arg, &success_flag);
     if (success_flag) printf("expr: %d\n", result);
     else Log("\033[0;31m""Bad Expression!\n""\033[0m");
     return 0;
   }
   ```

实验结果：

![3-1. p指令](./pa1/3-1.png)

### 3.2.2 代码：实现带有负数的算数表达式的求值

在词法单元的定义中， 定义了负数的符号`TK_NEG`

在`expr()`中

```c
  for (i = 0; i < nr_token; i++) {
    if (tokens[i].type == '*'
    && (i == 0 || is_operator(&tokens[i - 1]) || tokens[i - 1].type == '(')) {
      tokens[i].type = TK_DEREF;
    }
    else if (tokens[i].type == '-'
    && (i == 0 || is_operator(&tokens[i - 1]) || tokens[i - 1].type == '(')) {
      tokens[i].type = TK_NEG;
    }
  }
```

当`-`出现在表达式第一个或者它的前一个字符是运算符或者`(`时， 他表示负号

只需要将这个token的`type`从`-`转换成`TK_NEG`

实验结果：

![3-2. p-neg](./pa1/3-2.png)

### 3.2.3 代码： 实现更复杂的表达式求值

同*3.2.2*.

当`*`出现在表达式第一个或者它的前一个字符是运算符或者`(`时， 他表示负号

只需要将这个token的`type`从*转换成`TK_DEREF`

实验结果：

![3-3. p-deref](./pa1/3-3.png)

### 3.2.4 代码： 完善扫描内存功能

```c
__cr cmd_x (char *args) {
  char *arg __tk;
  if (arg == NULL) {
    printf("Args are needed!\n");
    return 0;
  }
  int n = atoi(arg);
  arg __tk;
  if (arg == NULL) {
    printf("Args are needed!\n");
    return 0;
  }
  bool success_flag;
  // int result = expr(arg, &success_flag);
  int addr = expr(arg, &success_flag);
  if (!success_flag) {
    Log("\033[0;31m""Bad Expression!\n""\033[0m");
    return 0;
  }
  printf("Memory:");
  int i;
  for (i=0; i < n; i++)
  {
    int value = vaddr_read(addr+i, 1);
    if (i%4 == 0) {
      printf("\n0x%08x:  0x%02x", addr+i, value);
    }
    else {
      printf("  0x%02x", value);
    }
  }
  printf("\n");
  return 0;
}
```

在`cmd_x()`中调用`expr()`即可

# 4 阶段三

## 4.1 监视点

### 4.1.1 代码：实现监视点池的管理

### 4.1.2 问题： static的使用

### 4.1.3 代码： 实现监视点

## 4.2 断点

### 4.2.1 断点的工作原理

### 4.2.2 问题： “一点也不能长？”

### 4.2.3 问题： 随心所欲的断点

### 4.2.4 问题： NEMU的前世今生

## 4.3 i386手册的学习

### 4.3.1  问题： 通过目录定位关注的问题

### 4.3.2 必答题

# 5 实验结论与感想

## 5.1 遇到的BUG

1. 十六进制转十进制公式搞错。。。

2. 词法分析`make_token()`中，将匹配到的词素存到`token.str`中，使用`strcpy()`，出现bug。`strcpy()`拷贝到`‘\0’`为止，然而正则匹配返回的是匹配的开始位置和长度, 所以应该使用循环

3. 在`nemu`中，输入

   ```shell
   (nemu)p 0xA
   ```

   结果正确，再输入

   ```shell
   (nemu)p $eax
   ```

   结果正确，再输入

   ```shell
   (nemu)p 0xA
   ```

   结果错误， 经过不懈查找，发现第三次输入时`token.str`为`0xAx`，原因为在`make_token()`函数中：

   ```c
             ......
   		  case TK_NUM:
             case TK_REG:
             case TK_HEX:
             {
          		...
               for (i = 0; i < sunstr_len; i++) {
                 tokens[nr_token].str[i] = substr_start[i];
               }
               ++nr_token;
               break;
             }
   		  ...
   ```

   上一次进行词法分析的`token.str`没有被这一次词法分析的`token.str`完全覆盖。应该改为

   ```c
               for (i = 0; i < 32; i++) {
                 if (i<substr_len) tokens[nr_token].str[i] = substr_start[i];
                 else tokens[nr_token].str[i] = '\0';
               }
   ```

   

## 5.2 感想

1. C++好多语法在C中不能用， 比如引用，参数`const`等等
2. 出了很多bug，然而没有发现的bug一定会在未来某个时候找到我，我很慌。