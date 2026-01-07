#define _XOPEN_SOURCE 600

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
            "  %s [--id <id>] [--count <n>] [--payload <str>] [--hold]\n",
            prog);
}

int main(int argc, char **argv)
{
    int id = NS_LDISC_ID;
    int count = 1;
    const char *payload = "ns_test\n";
    int hold = 0;

    // Simple argument parsing (tiny helper, not a full CLI framework)
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--id") == 0) {
            if (i + 1 >= argc) {
                usage(argv[0]);
                return 1;
            }
            id = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--count") == 0) {
            if (i + 1 >= argc) {
                usage(argv[0]);
                return 1;
            }
            count = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--payload") == 0) {
            if (i + 1 >= argc) {
                usage(argv[0]);
                return 1;
            }
            payload = argv[++i];
        } else if (strcmp(argv[i], "--hold") == 0) {
            hold = 1;
        } else {
            usage(argv[0]);
            return 1;
        }
    }

    int master = posix_openpt(O_RDWR | O_NOCTTY);
    if (master < 0) {
        fprintf(stderr, "posix_openpt: %s\n", strerror(errno));
        return 1;
    }
    if (grantpt(master) < 0 || unlockpt(master) < 0) {
        fprintf(stderr, "grantpt/unlockpt: %s\n", strerror(errno));
        close(master);
        return 1;
    }

    // Create and open the slave side of the PTY
    char *slave_name = ptsname(master);
    if (!slave_name) {
        fprintf(stderr, "ptsname: %s\n", strerror(errno));
        close(master);
        return 1;
    }

    int slave = open(slave_name, O_RDWR | O_NOCTTY);
    if (slave < 0) {
        fprintf(stderr, "open %s: %s\n", slave_name, strerror(errno));
        close(master);
        return 1;
    }

    // Apply ldisc to the slave; writing to master will hit receive_buf()
    if (ioctl(slave, TIOCSETD, &id) < 0) {
        fprintf(stderr, "TIOCSETD: %s\n", strerror(errno));
        close(slave);
        close(master);
        return 1;
    }

    printf("slave: %s\n", slave_name);
    fflush(stdout);

    size_t len = strlen(payload);
    for (int i = 0; i < count; i++) {
        if (write(master, payload, len) < 0) {
            fprintf(stderr, "write: %s\n", strerror(errno));
            break;
        }
    }

    if (hold) {
        printf("press Enter to exit\n");
        (void)getchar();
    } else {
        usleep(200 * 1000);
    }

    close(slave);
    close(master);
    return 0;
}
