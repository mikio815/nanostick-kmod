#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "ns_ldisc.h"

static void usage(const char *prog)
{
    fprintf(stderr,
            "usage:\n"
            "  %s <tty> get\n"
            "  %s <tty> set [id]\n"
            "  %s <tty> reset\n"
            "  %s --id <id> <tty> set\n",
            prog, prog, prog, prog);
}

int main(int argc, char **argv)
{
    int id = NS_LDISC_ID;
    int argi = 1;

    if (argc < 3) {
        usage(argv[0]);
        return 1;
    }

    // Optional override: --id <num>
    if (strcmp(argv[argi], "--id") == 0) {
        if (argi + 2 >= argc) {
            usage(argv[0]);
            return 1;
        }
        id = atoi(argv[argi + 1]);
        argi += 2;
    }

    if (argc - argi < 2) {
        usage(argv[0]);
        return 1;
    }

    const char *tty_path = argv[argi++];
    const char *cmd = argv[argi++];

    int fd = open(tty_path, O_RDWR | O_NOCTTY);
    if (fd < 0) {
        fprintf(stderr, "open %s: %s\n", tty_path, strerror(errno));
        return 1;
    }

    if (strcmp(cmd, "get") == 0) {
        int cur = 0;
        // Query current line discipline
        if (ioctl(fd, TIOCGETD, &cur) < 0) {
            fprintf(stderr, "TIOCGETD: %s\n", strerror(errno));
            close(fd);
            return 1;
        }
        printf("%d\n", cur);
    } else if (strcmp(cmd, "set") == 0) {
        if (argi < argc)
            id = atoi(argv[argi]);
        // Apply our ldisc (or a custom id)
        if (ioctl(fd, TIOCSETD, &id) < 0) {
            fprintf(stderr, "TIOCSETD: %s\n", strerror(errno));
            close(fd);
            return 1;
        }
    } else if (strcmp(cmd, "reset") == 0) {
        int n_tty = 0;
        // Reset to default N_TTY (id 0)
        if (ioctl(fd, TIOCSETD, &n_tty) < 0) {
            fprintf(stderr, "TIOCSETD: %s\n", strerror(errno));
            close(fd);
            return 1;
        }
    } else {
        usage(argv[0]);
        close(fd);
        return 1;
    }

    close(fd);
    return 0;
}
