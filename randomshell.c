#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <glob.h>
#include <dirent.h>
#include <errno.h>
#include <signal.h>
#include <ctype.h>
#include <time.h>

#define MAX_ARGS 512
#define MAX_CMDS 64
#define MAX_LINE 8192

char *last_command = NULL;

struct command {
    char *argv[MAX_ARGS];
    char *input;
    char *output;
    int append;
    int background;
};

// ===== DRUNK OUTPUT =====
void drunk_print(const char *s) {
    for (int i = 0; s[i]; i++) {
        char c = s[i];
        if (isalnum(c)) {
            int r = rand() % 50;
            if (r < 3 && s[i+1]) { // swap with next char
                char tmp = c;
                c = s[i+1];
                i++; 
                putchar(c);
                putchar(tmp);
                continue;
            } else if (r == 3 && isalpha(c)) {
                c ^= 32; // flip case
            }
        }
        putchar(c);
    }
}

// ===== SIGNAL HANDLER =====
void sigint_handler(int signo) {
    (void)write(STDOUT_FILENO, "\n", 1);
}

// ===== ENV VAR EXPANSION =====
void expand_vars(char **arg) {
    if ((*arg)[0] == '$') {
        char *env = getenv((*arg) + 1);
        if (env) {
            free(*arg);
            *arg = strdup(env);
        }
    }
}

// ===== GLOB EXPANSION =====
void expand_glob(struct command *cmd) {
    glob_t g;
    char *newargv[MAX_ARGS];
    int argc = 0;

    for (int i = 0; cmd->argv[i]; i++) {
        if (strchr(cmd->argv[i], '*') || strchr(cmd->argv[i], '?')) {
            if (glob(cmd->argv[i], GLOB_NOCHECK | GLOB_MARK | GLOB_TILDE, NULL, &g) == 0) {
                for (size_t j = 0; j < g.gl_pathc; j++)
                    newargv[argc++] = strdup(g.gl_pathv[j]);
                globfree(&g);
            }
        } else {
            newargv[argc++] = strdup(cmd->argv[i]);
        }
    }
    newargv[argc] = NULL;
    for (int i = 0; cmd->argv[i]; i++) free(cmd->argv[i]);
    for (int i = 0; i <= argc; i++)
        cmd->argv[i] = newargv[i];
}

// ===== PARSE COMMAND =====
void parse_command(char *cmd, struct command *c) {
    int argc = 0;
    c->input = NULL;
    c->output = NULL;
    c->append = 0;
    c->background = 0;

    char *token = strtok(cmd, " \t\n");
    while (token) {
        if (strcmp(token, "<") == 0) {
            char *next = strtok(NULL, " \t\n");
            if (next) c->input = strdup(next);
            else drunk_print("syntax error: expected file after <\n");
        } else if (strcmp(token, ">") == 0) {
            char *next = strtok(NULL, " \t\n");
            if (next) { c->output = strdup(next); c->append = 0; }
            else drunk_print("syntax error: expected file after >\n");
        } else if (strcmp(token, ">>") == 0) {
            char *next = strtok(NULL, " \t\n");
            if (next) { c->output = strdup(next); c->append = 1; }
            else drunk_print("syntax error: expected file after >>\n");
        } else if (strcmp(token, "&") == 0) {
            c->background = 1;
        } else {
            c->argv[argc++] = strdup(token);
            expand_vars(&c->argv[argc-1]);
        }
        token = strtok(NULL, " \t\n");
    }
    c->argv[argc] = NULL;
    expand_glob(c);
}

// ===== SPLIT PIPELINE =====
int split_pipeline(char *line, char *cmds[]) {
    int n = 0;
    cmds[n] = strtok(line, "|");
    while (cmds[n] && n < MAX_CMDS - 1) {
        n++;
        cmds[n] = strtok(NULL, "|");
    }
    return n;
}

// ===== EXECUTE PIPELINE WITH DRUNK OUTPUT =====
void execute_pipeline(struct command cmds[], int n) {
    int pipes[MAX_CMDS][2];
    for (int i = 0; i < n - 1; i++)
        if (pipe(pipes[i]) < 0) { perror("pipe"); exit(1); }

    for (int i = 0; i < n; i++) {
        int out_pipe[2];
        pipe(out_pipe);

        pid_t pid = fork();
        if (pid < 0) { perror("fork"); exit(1); }
        if (pid == 0) { // child
            signal(SIGINT, SIG_DFL);
            if (i > 0) dup2(pipes[i-1][0], STDIN_FILENO);
            if (i < n-1) dup2(pipes[i][1], STDOUT_FILENO);

            dup2(out_pipe[1], STDOUT_FILENO);
            dup2(out_pipe[1], STDERR_FILENO);
            close(out_pipe[0]);
            close(out_pipe[1]);

            for (int j = 0; j < n-1; j++) { close(pipes[j][0]); close(pipes[j][1]); }

            if (cmds[i].input) {
                int fd = open(cmds[i].input, O_RDONLY);
                if (fd < 0) { perror(cmds[i].input); exit(1); }
                dup2(fd, STDIN_FILENO);
                close(fd);
            }
            if (cmds[i].output) {
                int fd;
                if (cmds[i].append)
                    fd = open(cmds[i].output, O_WRONLY|O_CREAT|O_APPEND, 0644);
                else
                    fd = open(cmds[i].output, O_WRONLY|O_CREAT|O_TRUNC, 0644);
                if (fd < 0) { perror(cmds[i].output); exit(1); }
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }

            execvp(cmds[i].argv[0], cmds[i].argv);
            perror("exec");
            exit(1);
        } else { // parent
            close(out_pipe[1]); // close write end
            char buf[1024];
            ssize_t nread;
            while ((nread = read(out_pipe[0], buf, sizeof(buf)-1)) > 0) {
                buf[nread] = 0;
                drunk_print(buf);
            }
            close(out_pipe[0]);
        }
    }

    for (int i = 0; i < n-1; i++) { close(pipes[i][0]); close(pipes[i][1]); }

    if (!cmds[n-1].background)
        for (int i = 0; i < n; i++) wait(NULL);
}

// ===== PROMPT =====
void get_prompt(char *buf, size_t size) {
    char cwd[512], hostname[256];
    getcwd(cwd, sizeof(cwd));
    gethostname(hostname, sizeof(hostname));
    char *user = getenv("USER");
    if (!user) user = "user";
    char end_char = (getuid() == 0) ? '#' : '$';
    snprintf(buf, size, "%s@%s:%s %c ", user, hostname, cwd, end_char);
}

// ===== FREE MEMORY =====
void free_commands(struct command cmds[], int n) {
    for (int i=0;i<n;i++){
        for(int j=0; cmds[i].argv[j]; j++) free(cmds[i].argv[j]);
        if(cmds[i].input) free(cmds[i].input);
        if(cmds[i].output) free(cmds[i].output);
    }
}

// ===== MAIN =====
int main() {
    srand(time(NULL));
    signal(SIGINT, SIG_IGN);

    char line[MAX_LINE];
    while(1){
        char prompt[1024];
        get_prompt(prompt, sizeof(prompt));
        drunk_print(prompt);
        fflush(stdout);

        if (!fgets(line, sizeof(line), stdin)) break;
        line[strcspn(line, "\n")] = 0;  // remove newline

        if(strcmp(line,"!!")==0){
            if(!last_command){
                drunk_print("no history\n");
                continue;
            }
            strcpy(line, last_command);
            drunk_print(line); putchar('\n');
        }

        if(strlen(line)==0) continue;

        free(last_command);
        last_command = strdup(line);

        if(strcmp(line,"exit")==0) break;

        if(strncmp(line,"cd ",3)==0){
            if(chdir(line+3)<0) perror("cd");
            continue;
        }
        if(strcmp(line, "clear") == 0){
            write(STDOUT_FILENO, "\033[3J\033[H\033[2J", 11);
            continue;
        }
        char *cmd_strings[MAX_CMDS];
        struct command cmds[MAX_CMDS];
        int n = split_pipeline(line, cmd_strings);
        for(int i=0;i<n;i++)
            parse_command(cmd_strings[i], &cmds[i]);
        execute_pipeline(cmds,n);
        free_commands(cmds,n);
    }

    free(last_command);
    return 0;
}