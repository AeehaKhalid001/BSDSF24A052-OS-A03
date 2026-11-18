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

/* ---------- Feature-7: if-then-else-fi ----------
 * first_line jaise: "if grep user /etc/passwd > /dev/null"
 * Hum "> " prompt ke saath lines parhte rehte hain jab tak "fi" na aaye.
 * "then" wali lines ek array mein, "else" wali doosre array mein.
 * Phir condition chalate hain aur exit status dekh kar ek block chalate hain.
 */
#define MAX_BLOCK 50

/* shuru aur aakhir ke spaces/tabs/newlines hata do ("  then " -> "then") */
static char* trim(char* s) {
    while (*s == ' ' || *s == '\t') s++;                 // shuru se
    int len = strlen(s);
    while (len > 0 && (s[len-1] == ' ' || s[len-1] == '\t' ||
                       s[len-1] == '\n' || s[len-1] == '\r'))
        s[--len] = '\0';                                 // aakhir se
    return s;
}

static void handle_if(char* first_line) {
    char* condition = strdup(trim(first_line) + 2);   // "if" ke baad ka hissa
    char* then_block[MAX_BLOCK];
    char* else_block[MAX_BLOCK];
    int n_then = 0, n_else = 0;
    int state = 0;      // 0 = "then" ka intezar, 1 = then-block, 2 = else-block
    char* line;

    while ((line = readline("> ")) != NULL) {
        char* t = trim(line);
        if (strcmp(t, "then") == 0 && state == 0) {
            state = 1;
        } else if (strcmp(t, "else") == 0 && state == 1) {
            state = 2;
        } else if (strcmp(t, "fi") == 0 && state != 0) {
            free(line);
            break;                              // if statement khatam
        } else if (state == 1 && n_then < MAX_BLOCK) {
            then_block[n_then++] = strdup(t);
        } else if (state == 2 && n_else < MAX_BLOCK) {
            else_block[n_else++] = strdup(t);
        } else if (state == 0) {
            printf("syntax error: expected 'then'\n");
            free(line);
            goto cleanup;
        }
        free(line);
    }

    if (line == NULL) {                         // "fi" se pehle Ctrl+D
        printf("syntax error: missing 'fi'\n");
        goto cleanup;
    }

    // Condition normal fork-exec-wait se chalao.
    // run_command_line() aakhri command ka WEXITSTATUS deta hai:
    // 0 = success -> then-block, non-zero = failure -> else-block
    if (run_command_line(condition) == 0) {
        for (int i = 0; i < n_then; i++) run_command_line(then_block[i]);
    } else {
        for (int i = 0; i < n_else; i++) run_command_line(else_block[i]);
    }

cleanup:
    free(condition);
    for (int i = 0; i < n_then; i++) free(then_block[i]);
    for (int i = 0; i < n_else; i++) free(else_block[i]);
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

        // ---- Feature-7: "if " se shuru hone wali line -> if-block
        char* start = trim(cmdline);
        if (strncmp(start, "if ", 3) == 0) {
            handle_if(start);
        } else {
            run_command_line(cmdline);  // Feature-6: ';' aur '&'
        }
        free(cmdline);
    }

    printf("\nShell exited.\n");
    return 0;
}
