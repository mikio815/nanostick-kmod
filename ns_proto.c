#include <linux/kernel.h>
#include <linux/unaligned/le_byteshift.h>

#include "ns_proto.h"

void ns_proto_init(struct ns_proto_state *st)
{
    st->pos = 0;
}

u16 ns_proto_crc16_ccitt_false(const u8 *data, size_t len)
{
    u16 crc = 0xFFFF;

    for (size_t i = 0; i < len; i++) {
        crc ^= (u16)data[i] << 8;
        for (int b = 0; b < 8; b++) {
            if (crc & 0x8000)
                crc = (crc << 1) ^ 0x1021;
            else
                crc <<= 1;
        }
    }

    return crc;
}

static bool ns_proto_try_emit(struct ns_proto_state *st,
                              ns_proto_frame_cb cb, void *ctx)
{
    u16 crc_calc;
    u16 crc_recv;
    struct ns_proto_frame f;

    crc_calc = ns_proto_crc16_ccitt_false(st->buf, 14);
    crc_recv = get_unaligned_le16(&st->buf[14]);
    if (crc_calc != crc_recv)
        return false;

    f.seq = get_unaligned_le16(&st->buf[2]);
    f.lx = (s16)get_unaligned_le16(&st->buf[4]);
    f.ly = (s16)get_unaligned_le16(&st->buf[6]);
    f.rx = (s16)get_unaligned_le16(&st->buf[8]);
    f.ry = (s16)get_unaligned_le16(&st->buf[10]);
    f.buttons = st->buf[12];
    f.flags = st->buf[13];

    if (cb)
        cb(&f, ctx);
    return true;
}

void ns_proto_feed(struct ns_proto_state *st, const u8 *data, size_t len,
                   ns_proto_frame_cb cb, void *ctx)
{
    for (size_t i = 0; i < len; i++) {
        u8 b = data[i];

        if (st->pos == 0) {
            if (b != NS_PROTO_MAGIC0)
                continue;
            st->buf[st->pos++] = b;
            continue;
        }

        if (st->pos == 1) {
            if (b == NS_PROTO_MAGIC1) {
                st->buf[st->pos++] = b;
                continue;
            }
            st->pos = (b == NS_PROTO_MAGIC0) ? 1 : 0;
            st->buf[0] = NS_PROTO_MAGIC0;
            continue;
        }

        st->buf[st->pos++] = b;
        if (st->pos < NS_PROTO_FRAME_SIZE)
            continue;

        ns_proto_try_emit(st, cb, ctx);
        st->pos = 0;
    }
}
