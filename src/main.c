#include "shell.h"

/* Feature-6: EK simple command chalao (jis mein ';' nahi).
 * Aakhir mein "&" ho to background mein chalao. */
static int run_simple(char** args) {
    int n = 0;
    while (args[n] != NULL) n++;
    if (n == 0) return 0;

    int background = 0;
    if (strcmp(args[n - 1], "&") == 0) {        // aakhri token "&" hai?
        background = 1;
        args[n - 1] = NULL;                     // "&" execvp ko nahi dena
        if (n == 1) return 0;                   // sirf "&" likha tha
    }

    if (handle_builtin(args))                   // built-in -> shell mein hi
        return 0;
    return execute(args, background);           // external -> fork + exec
}

/* Feature-6: poori line tokenize karo, ";" par tukde karo, aur
 * har tukda baari baari chalao. Aakhri tukde ka status return karo. */
int run_command_line(char* cmdline) {
    char** arglist = tokenize(cmdline);
    if (arglist == NULL) return 0;

    int status = 0;
    char* segment[MAXARGS + 1];
    int n = 0;

    for (int i = 0; ; i++) {
        if (arglist[i] == NULL || strcmp(arglist[i], ";") == 0) {
            segment[n] = NULL;
            if (n > 0) status = run_simple(segment);   // '&' na ho to intezar karega
            n = 0;
            if (arglist[i] == NULL) break;
        } else {
            segment[n++] = arglist[i];          // sirf pointer copy
        }
    }

    for (int i = 0; arglist[i] != NULL; i++)   // tokenize ki memory free
        free(arglist[i]);
    free(arglist);
    return status;
}

int main() {
    char* cmdline;

    init_readline();                    // Feature-4

    while (1) {
        reap_jobs();                    // Feature-6: har prompt se pehle zombies saaf karo

        if ((cmdline = readline(PROMPT)) == NULL)
            break;                      // Ctrl+D

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

        // ---- history mein save karo
        if (cmdline[0] != '\0') {
            add_to_history(cmdline);    // hamari list (history, !n)
            add_history(cmdline);       // readline ki list (arrows)
        }

        run_command_line(cmdline);      // Feature-6: ';' aur '&' yahan handle hote hain
        free(cmdline);
    }

    printf("\nShell exited.\n");
    return 0;
}
