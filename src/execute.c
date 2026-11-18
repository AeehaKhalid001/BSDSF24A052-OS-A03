#include "shell.h"

/* ---------- Feature-5: I/O redirection aur pipes ----------
 * execute() ko EK command line ke tokens milte hain, jaise:
 *     cat /etc/passwd | grep root > out.txt
 * Ye unhein har "|" par tor kar alag commands banata hai, har do
 * commands ke beech ek pipe banata hai, har command ke liye ek child
 * fork karta hai, aur sab ka intezar karta hai.
 */

/* CHILD mein chalti hai: fork() ke BAAD aur execvp() se PEHLE.
 * args mein "<" aur ">" dhoondti hai, stdin/stdout ko file se jorti hai,
 * aur operator + file ka naam list se hata deti hai (execvp ko na milein).
 * Kamyabi par 0, error par -1. */
static int setup_redirection(char** args) {
    int j = 0;                                  // saaf list ka index
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "<") == 0 || strcmp(args[i], ">") == 0) {
            if (args[i + 1] == NULL) {
                fprintf(stderr, "syntax error: missing file name after %s\n", args[i]);
                return -1;
            }
            int fd;
            if (args[i][0] == '<') {
                fd = open(args[i + 1], O_RDONLY);                       // file parhne ke liye kholo
                if (fd < 0) { perror(args[i + 1]); return -1; }
                dup2(fd, STDIN_FILENO);             // ab stdin (0) is file se parhega
            } else {
                fd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644); // likhne ke liye (nahi to bana do, hai to khali karo)
                if (fd < 0) { perror(args[i + 1]); return -1; }
                dup2(fd, STDOUT_FILENO);            // ab stdout (1) is file mein likhega
            }
            close(fd);                              // copy 0/1 par hai, asal fd ki zaroorat nahi
            i++;                                    // file ka naam bhi chhor do
        } else {
            args[j++] = args[i];                    // normal argument rakho
        }
    }
    args[j] = NULL;
    return 0;
}

int execute(char* arglist[]) {
    // ---- 1. arglist ko "|" par tor kar cmds[0], cmds[1], ... banao
    char* cmds[MAX_PIPE][MAXARGS + 1];
    int ncmds = 0, argc = 0;

    for (int i = 0; arglist[i] != NULL; i++) {
        if (strcmp(arglist[i], "|") == 0) {
            if (argc == 0) { fprintf(stderr, "syntax error near |\n"); return 1; }
            cmds[ncmds][argc] = NULL;
            ncmds++; argc = 0;
            if (ncmds == MAX_PIPE) { fprintf(stderr, "too many pipes\n"); return 1; }
        } else {
            cmds[ncmds][argc++] = arglist[i];
        }
    }
    if (argc == 0) { fprintf(stderr, "syntax error near |\n"); return 1; }
    cmds[ncmds][argc] = NULL;
    ncmds++;

    // ---- 2. har command ke liye ek child, pipes se jure hue
    pid_t pids[MAX_PIPE];
    int prev_read = -1;          // pichli command wale pipe ka read end

    for (int k = 0; k < ncmds; k++) {
        int fd[2] = {-1, -1};
        if (k < ncmds - 1) {                 // aakhri command nahi -> pipe chahiye
            if (pipe(fd) < 0) { perror("pipe"); return 1; }
        }

        pid_t pid = fork();
        if (pid < 0) { perror("fork failed"); exit(1); }

        if (pid == 0) {                      // ===== CHILD =====
            if (prev_read != -1) {           // pichli command se parho
                dup2(prev_read, STDIN_FILENO);
                close(prev_read);
            }
            if (k < ncmds - 1) {             // agli command ke liye likho
                close(fd[0]);                // child apna read end use nahi karta
                dup2(fd[1], STDOUT_FILENO);
                close(fd[1]);
            }
            if (setup_redirection(cmds[k]) < 0) exit(1);
            if (cmds[k][0] == NULL) exit(0); // sirf "> file" likha ho
            execvp(cmds[k][0], cmds[k]);
            perror("Command not found");     // execvp fail ho tabhi yahan aata hai
            exit(127);
        }

        // ===== PARENT ===== jo pipe ends ab nahi chahiye, band karo.
        // Warna parhne wale ko kabhi EOF nahi milega aur shell ruk jayega.
        pids[k] = pid;
        if (prev_read != -1) close(prev_read);
        if (k < ncmds - 1) {
            close(fd[1]);
            prev_read = fd[0];
        }
    }

    // ---- 3. sab children ka intezar; aakhri command ka exit code yaad rakho
    int status = 0, last_status = 0;
    for (int k = 0; k < ncmds; k++) {
        waitpid(pids[k], &status, 0);
        if (k == ncmds - 1 && WIFEXITED(status))
            last_status = WEXITSTATUS(status);
    }
    return last_status;
}
