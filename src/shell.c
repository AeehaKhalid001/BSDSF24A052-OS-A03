#include "shell.h"

char* read_cmd(char* prompt, FILE* fp) {
    printf("%s", prompt);
    char* cmdline = (char*) malloc(sizeof(char) * MAX_LEN);
    int c, pos = 0;

    while ((c = getc(fp)) != EOF) {
        if (c == '\n') break;
        cmdline[pos++] = c;
    }

    if (c == EOF && pos == 0) {
        free(cmdline);
        return NULL; // Handle Ctrl+D
    }

    cmdline[pos] = '\0';
    return cmdline;
}

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

/* ---------- Feature-3: Command history ----------
 * cmd_history[0] sabse purani command, cmd_history[hist_count-1] sabse nayi.
 * Array full ho jaye to sabse purani free karke baqi sab ek jagah
 * left shift kar do, taake numbering hamesha 1..20 rahe.
 */
static char* cmd_history[HISTORY_SIZE];
static int   hist_count = 0;

void add_to_history(const char* cmdline) {
    if (hist_count == HISTORY_SIZE) {            // full: purani hatao
        free(cmd_history[0]);
        for (int i = 1; i < HISTORY_SIZE; i++)
            cmd_history[i - 1] = cmd_history[i];
        hist_count--;
    }
    cmd_history[hist_count++] = strdup(cmdline); // apni copy save karo
}

void print_history(void) {
    for (int i = 0; i < hist_count; i++)
        printf("%3d  %s\n", i + 1, cmd_history[i]);
}

// n-th command (1 se shuru) wapas do, ghalat n ho to NULL
char* get_history(int n) {
    if (n < 1 || n > hist_count)
        return NULL;
    return cmd_history[n - 1];
}
