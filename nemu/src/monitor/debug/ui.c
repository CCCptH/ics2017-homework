#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  return -1;
}

#define __cr static int
#define __tk = strtok(NULL, " ")
static int cmd_help(char *args);

__cr cmd_si(char *args) {
  char *arg __tk;
  if (arg == NULL)
    cpu_exec(1);
  else
    cpu_exec(atoi(arg));
  return 0;
}

__cr cmd_info (char *args) {
  char *arg __tk;
  if (arg == NULL) {
    printf("An arg is needed!\n");
    return 0;
  }
  if (strcmp(arg, "r") == 0)  {
    #define printER(r) printf(#r"\t\t%x\t\t%u\n", cpu.e##r, cpu.e##r)
    printER(ax);
    printER(bx);
    printER(cx);
    printER(dx);
    printER(sp);
    printER(bp);
    printER(si);
    printER(di);
    #undef printER
  }
  else if (strcmp(arg, "w") == 0) {

  }
  else {
    printf("Undefine Args. \n\t r: print registers\n\t w: print watchpoints\n");
  }
  return 0;
}

__cr cmd_x (char *args) {
  char *arg __tk;
  if (arg == NULL) {
    printf("Args are needed!\n");
    return 0;
  }
  int n = atoi(arg);
  if (arg == NULL) {
    printf("Args are needed!\n");
    return 0;
  }
  bool success_flag;
  // int result = expr(arg, &success_flag);
  int addr = expr(args, &success_flag);
  if (!success_flag) {
    printf("\033[0;31m""Bad Expression!\n""\033[0m");
  }
  int i;
  for (i=0; i < n; i++)
  {
    printf("0x%08x: %u\n", addr, vaddr_read(addr, 4));
    addr += 4;
  }
  return 0;
}

#undef __cr
#undef __tk

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
  { "x", "scan memory", cmd_x }

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  while (1) {
    char *str = rl_gets();
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}
