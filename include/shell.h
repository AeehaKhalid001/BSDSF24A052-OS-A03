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
#include <fcntl.h>                // Feature-5

// Feature-4: GNU Readline
#include <readline/readline.h>
#include <readline/history.h>

#define MAX_LEN 512
#define MAXARGS 64             // Feature-5
#define ARGLEN 256             // Feature-5
#define MAX_PIPE 10            // Feature-5
#define MAX_JOBS 20            // Feature-6: zyada se zyada 20 background jobs
#define PROMPT "FCIT> "
#define HISTORY_SIZE 20        // Feature-3

// Function prototypes
char** tokenize(char* cmdline);
int execute(char** arglist, int background);   // Feature-6: background flag add hua
int handle_builtin(char** arglist);            // Feature-2

// Feature-3: history
void add_to_history(const char* cmdline);
void print_history(void);
char* get_history(int n);

// Feature-4: tab completion
void init_readline(void);

// Feature-6: background jobs aur chaining
void reap_jobs(void);
void print_jobs(void);
int  run_command_line(char* cmdline);

#endif // SHELL_H
