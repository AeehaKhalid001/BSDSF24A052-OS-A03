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

/* ---------- Feature-2: Built-in commands ----------
 * Agar arglist[0] built-in hai to yahin (shell ke process mein)
 * chala kar 1 return karo. Built-in nahi hai to 0 return karo,
 * taake main() usay fork/exec se chalaye.
 */
int handle_builtin(char** arglist) {
    char* cmd = arglist[0];

    if (strcmp(cmd, "exit") == 0) {
        printf("Exiting shell...\n");
        exit(0);                      // shell khud band ho jaye
    }

    if (strcmp(cmd, "cd") == 0) {
        char* dir = arglist[1];
        if (dir == NULL)              // sirf "cd" likha -> home folder
            dir = getenv("HOME");
        if (chdir(dir) != 0)          // chdir() shell ka apna folder badalta hai
            perror("cd");
        return 1;
    }

    if (strcmp(cmd, "help") == 0) {
        printf("Built-in commands:\n");
        printf("  cd <dir>   Change the current directory\n");
        printf("  exit       Exit the shell\n");
        printf("  help       Show this help message\n");
        printf("  jobs       List background jobs\n");
        return 1;
    }

    if (strcmp(cmd, "jobs") == 0) {
        printf("Job control not yet implemented.\n");
        return 1;
    }

    return 0;                         // built-in nahi hai
}
