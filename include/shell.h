#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <dirent.h>               // Feature-4: PATH folders parhne ke liye

// Feature-4: GNU Readline
#include <readline/readline.h>
#include <readline/history.h>

#define MAX_LEN 512
#define MAXARGS 10
#define ARGLEN 30
#define PROMPT "FCIT> "
#define HISTORY_SIZE 20        // Feature-3

// Function prototypes  (read_cmd hata diya, ab readline() use hoga)
char** tokenize(char* cmdline);
int execute(char** arglist);
int handle_builtin(char** arglist);   // Feature-2

// Feature-3: history
void add_to_history(const char* cmdline);
void print_history(void);
char* get_history(int n);

// Feature-4: tab completion
void init_readline(void);

#endif // SHELL_H
