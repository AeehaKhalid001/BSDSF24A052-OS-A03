#include "shell.h"

/* ---------- Feature-6: Background jobs ki list ----------
 * Har background job ka PID aur command ka text yahan save hota hai.
 */
typedef struct {
    pid_t pid;
    char  cmd[MAX_LEN];
} Job;

static Job jobs[MAX_JOBS];
static int njobs = 0;

static void add_job(pid_t pid, char** arglist) {
    if (njobs == MAX_JOBS) return;
    jobs[njobs].pid = pid;
    jobs[njobs].cmd[0] = '\0';
    for (int i = 0; arglist[i] != NULL; i++) {      // command ka text dobara jorho
        strncat(jobs[njobs].cmd, arglist[i], MAX_LEN - strlen(jobs[njobs].cmd) - 2);
        strcat(jobs[njobs].cmd, " ");
    }
    printf("[%d] %d\n", njobs + 1, pid);            // jaise bash: [1] 1234
    njobs++;
}

static void remove_job(pid_t pid) {
    for (int i = 0; i < njobs; i++) {
        if (jobs[i].pid == pid) {
            printf("[%d]+ Done    %s\n", i + 1, jobs[i].cmd);
            for (int j = i + 1; j < njobs; j++)     // baqi jobs ek jagah left shift
                jobs[j - 1] = jobs[j];
            njobs--;
            return;
        }
    }
}

/* Khatam ho chuke background children ko "reap" karo, taake zombie na banein.
 * WNOHANG = ruko mat: koi child khatam nahi hua to foran 0 return karo. */
void reap_jobs(void) {
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
        remove_job(pid);
}

void print_jobs(void) {
    if (njobs == 0) { printf("No background jobs.\n"); return; }
    for (int i = 0; i < njobs; i++)
        printf("[%d]  Running  PID %d  %s\n", i + 1, jobs[i].pid, jobs[i].cmd);
}

/* ---------- Feature-5: I/O redirection aur pipes ---------- */

/* CHILD mein: fork() ke BAAD aur execvp() se PEHLE.
 * "<" aur ">" ke liye stdin/stdout ko file se jorti hai. */
static int setup_redirection(char** args) {
    int j = 0;
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "<") == 0 || strcmp(args[i], ">") == 0) {
            if (args[i + 1] == NULL) {
                fprintf(stderr, "syntax error: missing file name after %s\n", args[i]);
                return -1;
            }
            int fd;
            if (args[i][0] == '<') {
                fd = open(args[i + 1], O_RDONLY);
                if (fd < 0) { perror(args[i + 1]); return -1; }
                dup2(fd, STDIN_FILENO);             // stdin (0) ab file se
            } else {
                fd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd < 0) { perror(args[i + 1]); return -1; }
                dup2(fd, STDOUT_FILENO);            // stdout (1) ab file mein
            }
            close(fd);
            i++;                                    // file ka naam bhi chhor do
        } else {
            args[j++] = args[i];
        }
    }
    args[j] = NULL;
    return 0;
}

int execute(char* arglist[], int background) {
    // ---- 1. "|" par tor kar cmds[0], cmds[1], ... banao
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
    int prev_read = -1;

    for (int k = 0; k < ncmds; k++) {
        int fd[2] = {-1, -1};
        if (k < ncmds - 1) {
            if (pipe(fd) < 0) { perror("pipe"); return 1; }
        }

        pid_t pid = fork();
        if (pid < 0) { perror("fork failed"); exit(1); }

        if (pid == 0) {                      // ===== CHILD =====
            if (prev_read != -1) {
                dup2(prev_read, STDIN_FILENO);
                close(prev_read);
            }
            if (k < ncmds - 1) {
                close(fd[0]);
                dup2(fd[1], STDOUT_FILENO);
                close(fd[1]);
            }
            if (setup_redirection(cmds[k]) < 0) exit(1);
            if (cmds[k][0] == NULL) exit(0);
            execvp(cmds[k][0], cmds[k]);
            perror("Command not found");
            exit(127);
        }

        // ===== PARENT ===== faltu pipe ends band karo (warna hang)
        pids[k] = pid;
        if (prev_read != -1) close(prev_read);
        if (k < ncmds - 1) {
            close(fd[1]);
            prev_read = fd[0];
        }
    }

    // ---- Feature-6: background hai to INTEZAR MAT KARO, sirf job save karo
    if (background) {
        add_job(pids[ncmds - 1], arglist);
        return 0;
    }

    // ---- 3. foreground: sab children ka intezar
    int status = 0, last_status = 0;
    for (int k = 0; k < ncmds; k++) {
        waitpid(pids[k], &status, 0);
        if (k == ncmds - 1 && WIFEXITED(status))
            last_status = WEXITSTATUS(status);
    }
    return last_status;
}
