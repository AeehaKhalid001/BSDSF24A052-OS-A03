#include "shell.h"

int main() {
    char* cmdline;
    char** arglist;

    while ((cmdline = read_cmd(PROMPT, stdin)) != NULL) {

        // ---- Feature-3: !n  (tokenize aur history se PEHLE check karo)
        if (cmdline[0] == '!') {
            int n = atoi(cmdline + 1);          // '!' ke baad wala number
            char* old = get_history(n);
            if (old == NULL) {
                printf("!%d: event not found\n", n);
                free(cmdline);
                continue;                       // wapas prompt par
            }
            free(cmdline);
            cmdline = strdup(old);              // "!n" ki jagah asal command
            printf("%s\n", cmdline);            // dikhao kaun si chal rahi hai
        }

        // ---- Feature-3: har non-empty command history mein save karo
        if (cmdline[0] != '\0') {
            add_to_history(cmdline);
        }

        if ((arglist = tokenize(cmdline)) != NULL) {
            // Feature-2: built-in hai to yahin chalao, warna fork/exec
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
