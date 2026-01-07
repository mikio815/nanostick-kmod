#ifndef NS_PROTO_H
#define NS_PROTO_H

#include <linux/types.h>

#define NS_PROTO_MAGIC0 0xA5
#define NS_PROTO_MAGIC1 0x5A
#define NS_PROTO_FRAME_SIZE 16

struct ns_proto_frame {
    u16 seq;
    s16 lx;
    s16 ly;
    s16 rx;
    s16 ry;
    u8 buttons;
    u8 flags;
};

struct ns_proto_state {
    u8 buf[NS_PROTO_FRAME_SIZE];
    u8 pos;
};

typedef void (*ns_proto_frame_cb)(const struct ns_proto_frame *f, void *ctx);

void ns_proto_init(struct ns_proto_state *st);
void ns_proto_feed(struct ns_proto_state *st, const u8 *data, size_t len,
                   ns_proto_frame_cb cb, void *ctx);
u16 ns_proto_crc16_ccitt_false(const u8 *data, size_t len);

#endif
