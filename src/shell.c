#include "shell.h"

char** tokenize(char* cmdline) {
    // Edge case: empty command line
    if (cmdline == NULL || cmdline[0] == '\0' || cmdline[0] == '\n') {
        return NULL;
    }

    char** arglist = (char**)malloc(sizeof(char*) * (MAXARGS + 1));
    for (int i = 0; i < MAXARGS + 1; i++) {
        arglist[i] = (char*)malloc(sizeof(char) * ARGLEN);
        bzero(arglist[i], ARGLEN);
    }

    char* cp = cmdline;
    char* start;
    int len;
    int argnum = 0;

    while (*cp != '\0' && argnum < MAXARGS) {
        while (*cp == ' ' || *cp == '\t') cp++; // Skip leading whitespace

        if (*cp == '\0') break; // Line was only whitespace

        start = cp;
        len = 1;
        while (*++cp != '\0' && !(*cp == ' ' || *cp == '\t')) {
            len++;
        }
        strncpy(arglist[argnum], start, len);
        arglist[argnum][len] = '\0';
        argnum++;
    }

    if (argnum == 0) { // No arguments were parsed
        for(int i = 0; i < MAXARGS + 1; i++) free(arglist[i]);
        free(arglist);
        return NULL;
    }

    arglist[argnum] = NULL;
    return arglist;
}

/* ---------- Feature-2: Built-in commands ---------- */
int handle_builtin(char** arglist) {
    char* cmd = arglist[0];

    if (strcmp(cmd, "exit") == 0) {
        printf("Exiting shell...\n");
        exit(0);
    }

    if (strcmp(cmd, "cd") == 0) {
        char* dir = arglist[1];
        if (dir == NULL)
            dir = getenv("HOME");
        if (chdir(dir) != 0)
            perror("cd");
        return 1;
    }

    if (strcmp(cmd, "help") == 0) {
        printf("Built-in commands:\n");
        printf("  cd <dir>   Change the current directory\n");
        printf("  exit       Exit the shell\n");
        printf("  help       Show this help message\n");
        printf("  jobs       List background jobs\n");
        printf("  history    Show command history\n");
        printf("  !n         Re-run command number n from history\n");
        return 1;
    }

    if (strcmp(cmd, "jobs") == 0) {
        printf("Job control not yet implemented.\n");
        return 1;
    }

    if (strcmp(cmd, "history") == 0) {   // Feature-3
        print_history();
        return 1;
    }

    return 0;
}

/* ---------- Feature-3: Command history ---------- */
static char* cmd_history[HISTORY_SIZE];
static int   hist_count = 0;

void add_to_history(const char* cmdline) {
    if (hist_count == HISTORY_SIZE) {
        free(cmd_history[0]);
        for (int i = 1; i < HISTORY_SIZE; i++)
            cmd_history[i - 1] = cmd_history[i];
        hist_count--;
    }
    cmd_history[hist_count++] = strdup(cmdline);
}

void print_history(void) {
    for (int i = 0; i < hist_count; i++)
        printf("%3d  %s\n", i + 1, cmd_history[i]);
}

char* get_history(int n) {
    if (n < 1 || n > hist_count)
        return NULL;
    return cmd_history[n - 1];
}

/* ---------- Feature-4: Tab completion (GNU Readline) ----------
 * Agar line ka PEHLA word complete ho raha hai -> command ka naam
 * complete karo (hamare built-ins + $PATH ke saare programs).
 * Baqi words ke liye NULL do -> readline khud file ka naam complete karega.
 */
static const char* builtin_names[] = { "cd", "exit", "help", "jobs", "history", NULL };

// Readline isay baar baar call karta hai: pehli baar state==0, phir 1,2,3...
// Har call par ek matching naam (malloc kiya hua) do, khatam ho to NULL.
static char* command_generator(const char* text, int state) {
    static int    b_index;
    static char*  path_copy;
    static char*  dir_name;
    static DIR*   dir;
    static size_t len;

    if (state == 0) {                       // pehli call: sab reset
        b_index = 0;
        len = strlen(text);
        free(path_copy);
        path_copy = getenv("PATH") ? strdup(getenv("PATH")) : NULL;
        dir_name  = path_copy ? strtok(path_copy, ":") : NULL;
        if (dir) closedir(dir);
        dir = NULL;
    }

    // 1) built-in commands ke naam
    while (builtin_names[b_index] != NULL) {
        const char* name = builtin_names[b_index++];
        if (strncmp(name, text, len) == 0)
            return strdup(name);
    }

    // 2) $PATH ke har folder ke programs
    while (dir_name != NULL) {
        if (dir == NULL)
            dir = opendir(dir_name);
        if (dir != NULL) {
            struct dirent* entry;
            while ((entry = readdir(dir)) != NULL) {
                if (strncmp(entry->d_name, text, len) == 0 &&
                    strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
                    return strdup(entry->d_name);
            }
            closedir(dir);
            dir = NULL;
        }
        dir_name = strtok(NULL, ":");       // PATH ka agla folder
    }
    return NULL;                            // aur koi match nahi
}

static char** shell_completion(const char* text, int start, int end) {
    (void)end;                              // use nahi ho raha (warning se bachne ke liye)
    if (start == 0)                         // pehla word -> command
        return rl_completion_matches(text, command_generator);
    return NULL;                            // baqi -> default file completion
}

void init_readline(void) {
    rl_attempted_completion_function = shell_completion;
}
