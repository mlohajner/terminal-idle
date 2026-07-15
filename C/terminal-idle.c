/*
 * terminal-idle.c
 *
 * C rewrite of terminal-idle.sh. Same logic, zero fork/exec:
 *   - scan /dev/pts/[0-9]+
 *   - atime via stat()
 *   - foreground pgid via tcgetpgrp() on the pts fd
 *   - comm via /proc/<pid>/comm
 *   - SigCgt bitmask via /proc/<pid>/status
 *   - signal via kill(), no sudo needed (same-user process)
 *
 * Build: gcc -O2 -Wall -Wextra -o terminal-idle terminal-idle.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/stat.h>
#include <ctype.h>

#define PTS_DIR "/dev/pts"
#define DEFAULT_INACTIVITY_PERIOD 300

/* Read /proc/<pid>/comm into buf. Returns 0 on success. */
static int read_comm(pid_t pid, char *buf, size_t buflen) {
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/comm", (int)pid);

    FILE *f = fopen(path, "r");
    if (!f) return -1;

    if (!fgets(buf, (int)buflen, f)) {
        fclose(f);
        return -1;
    }
    fclose(f);

    /* strip trailing newline */
    size_t len = strlen(buf);
    if (len && buf[len - 1] == '\n') buf[len - 1] = '\0';
    return 0;
}

/* Returns 1 if comm matches one of bash/zsh/sh (with or without leading '-'
 * for login shells), 0 otherwise. */
static int is_target_shell(const char *comm) {
    const char *c = comm;
    if (c[0] == '-') c++;  /* login shell prefix */
    return (strcmp(c, "bash") == 0 ||
            strcmp(c, "zsh")  == 0 ||
            strcmp(c, "sh")   == 0);
}

/* Parse /proc/<pid>/status for the SigCgt: hex mask.
 * Returns 0 on success, mask written to *mask. */
static int read_sigcgt(pid_t pid, unsigned long long *mask) {
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/status", (int)pid);

    FILE *f = fopen(path, "r");
    if (!f) return -1;

    char line[256];
    int found = 0;
    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "SigCgt:", 7) == 0) {
            char *val = line + 7;
            while (isspace((unsigned char)*val)) val++;
            *mask = strtoull(val, NULL, 16);
            found = 1;
            break;
        }
    }
    fclose(f);
    return found ? 0 : -1;
}

int main(void) {
    int inactivity_period = DEFAULT_INACTIVITY_PERIOD;
    const char *env_period = getenv("INACTIVITY_PERIOD");
    if (env_period && *env_period) {
        int v = atoi(env_period);
        if (v > 0) inactivity_period = v;
    }

    DIR *d = opendir(PTS_DIR);
    if (!d) {
        perror("opendir " PTS_DIR);
        return 1;
    }

    time_t now = time(NULL);
    struct dirent *ent;

    while ((ent = readdir(d)) != NULL) {
        /* only numeric entries, e.g. "0", "1", "12" */
        const char *name = ent->d_name;
        if (!isdigit((unsigned char)name[0])) continue;

        char pts_path[280];
        snprintf(pts_path, sizeof(pts_path), PTS_DIR "/%s", name);

        struct stat st;
        if (stat(pts_path, &st) != 0) continue;

        time_t idle = now - st.st_atime;
        if (idle < inactivity_period) continue;  /* recent activity */

        int fd = open(pts_path, O_RDONLY | O_NOCTTY);
        if (fd < 0) continue;

        pid_t tpgid = tcgetpgrp(fd);
        close(fd);
        if (tpgid <= 0) continue;

        char comm[64];
        if (read_comm(tpgid, comm, sizeof(comm)) != 0) continue;
        if (!is_target_shell(comm)) continue;

        unsigned long long sigcgt;
        if (read_sigcgt(tpgid, &sigcgt) != 0) continue;

        /* SIGALRM = 14, bit index 13 (bit 0 == signal 1) */
        if ((sigcgt >> (SIGALRM - 1)) & 1ULL) {
            kill(tpgid, SIGALRM);
        }
    }

    closedir(d);
    return 0;
}
