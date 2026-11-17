#include "shell.h"

int main() {
    char* cmdline;
    char** arglist;

    init_readline();                    // Feature-4: tab completion set karo

    // Feature-4: readline() prompt dikhata hai, line parhta hai
    // (arrows, Tab sab ke saath) aur malloc ki hui string deta hai.
    // Ctrl+D par NULL deta hai.
    while ((cmdline = readline(PROMPT)) != NULL) {

        // ---- Feature-3: !n
        if (cmdline[0] == '!') {
            int n = atoi(cmdline + 1);
            char* old = get_history(n);
            if (old == NULL) {
                printf("!%d: event not found\n", n);
                free(cmdline);
                continue;
            }
            free(cmdline);
            cmdline = strdup(old);
            printf("%s\n", cmdline);
        }

        // ---- har non-empty command save karo
        if (cmdline[0] != '\0') {
            add_to_history(cmdline);    // hamari list (history aur !n ke liye)
            add_history(cmdline);       // readline ki list (Up/Down arrows ke liye)
        }

        if ((arglist = tokenize(cmdline)) != NULL) {
            if (handle_builtin(arglist) == 0) {
                execute(arglist);
            }

            for (int i = 0; arglist[i] != NULL; i++) {
                free(arglist[i]);
            }
            free(arglist);
        }
        free(cmdline);
    }

    printf("\nShell exited.\n");
    return 0;
}
