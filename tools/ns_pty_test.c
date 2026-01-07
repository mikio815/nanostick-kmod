#define _XOPEN_SOURCE 600

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdint.h>

#include "ns_ldisc.h"

static void usage(const char *prog)
{
    fprintf(stderr,
            "usage:\n"
            "  %s [--id <id>] [--count <n>] [--payload <str>] [--hold]\n"
            "  %s --frame [--seq <n>] [--lx <n>] [--ly <n>] [--rx <n>] [--ry <n>]\n"
            "     [--buttons <n>] [--flags <n>] [--count <n>] [--hold] [--id <id>]\n",
            prog);
}

static uint16_t crc16_ccitt_false(const uint8_t *data, size_t len)
{
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < len; i++) {
        crc ^= (uint16_t)data[i] << 8;
        for (int b = 0; b < 8; b++) {
            if (crc & 0x8000)
                crc = (uint16_t)((crc << 1) ^ 0x1021);
            else
                crc <<= 1;
        }
    }
    return crc;
}

static void put_le16(uint8_t *dst, int v)
{
    uint16_t u = (uint16_t)v;
    dst[0] = (uint8_t)(u & 0xFF);
    dst[1] = (uint8_t)((u >> 8) & 0xFF);
}

int main(int argc, char **argv)
{
    int id = NS_LDISC_ID;
    int count = 1;
    const char *payload = "ns_test\n";
    int hold = 0;
    int use_frame = 0;
    int seq = 1;
    int lx = 0, ly = 0, rx = 0, ry = 0;
    int buttons = 0;
    int flags = 0;

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
        } else if (strcmp(argv[i], "--frame") == 0) {
            use_frame = 1;
        } else if (strcmp(argv[i], "--seq") == 0) {
            if (i + 1 >= argc) { usage(argv[0]); return 1; }
            seq = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--lx") == 0) {
            if (i + 1 >= argc) { usage(argv[0]); return 1; }
            lx = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--ly") == 0) {
            if (i + 1 >= argc) { usage(argv[0]); return 1; }
            ly = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--rx") == 0) {
            if (i + 1 >= argc) { usage(argv[0]); return 1; }
            rx = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--ry") == 0) {
            if (i + 1 >= argc) { usage(argv[0]); return 1; }
            ry = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--buttons") == 0) {
            if (i + 1 >= argc) { usage(argv[0]); return 1; }
            buttons = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--flags") == 0) {
            if (i + 1 >= argc) { usage(argv[0]); return 1; }
            flags = atoi(argv[++i]);
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

    if (use_frame) {
        uint8_t frame[16];
        frame[0] = 0xA5;
        frame[1] = 0x5A;
        put_le16(&frame[2], seq);
        put_le16(&frame[4], lx);
        put_le16(&frame[6], ly);
        put_le16(&frame[8], rx);
        put_le16(&frame[10], ry);
        frame[12] = (uint8_t)buttons;
        frame[13] = (uint8_t)flags;
        put_le16(&frame[14], crc16_ccitt_false(frame, 14));

        for (int i = 0; i < count; i++) {
            if (write(master, frame, sizeof(frame)) < 0) {
                fprintf(stderr, "write: %s\n", strerror(errno));
                break;
            }
        }
    } else {
        size_t len = strlen(payload);
        for (int i = 0; i < count; i++) {
            if (write(master, payload, len) < 0) {
                fprintf(stderr, "write: %s\n", strerror(errno));
                break;
            }
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
