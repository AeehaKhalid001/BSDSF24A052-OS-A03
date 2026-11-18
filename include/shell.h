#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <dirent.h>               // Feature-4
#include <fcntl.h>                // Feature-5: open() ke flags (O_RDONLY waghera)

// Feature-4: GNU Readline
#include <readline/readline.h>
#include <readline/history.h>

#define MAX_LEN 512
#define MAXARGS 64             // Feature-5: ek line mein zyada se zyada 64 tokens
#define ARGLEN 256             // Feature-5: ek token zyada se zyada 256 characters
#define MAX_PIPE 10            // Feature-5: ek pipeline mein zyada se zyada 10 commands
#define PROMPT "FCIT> "
#define HISTORY_SIZE 20        // Feature-3

// Function prototypes
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
