# NanoStick Serial Protocol

Frame layout (16 bytes)

Offset  Size  Name     Type            Notes

0       2     magic    u16             Fixed sync marker

2       2     seq      u16             Incrementing sequence

4       2     lx       s16             Left stick X

6       2     ly       s16             Left stick Y

8       2     rx       s16             Right stick X

10      2     ry       s16             Right stick Y

12      1     buttons  u8              Button bitmask

13      1     flags    u8              reserved

14      2     crc16    u16             CRC-16/CCITT-FALSE

Constants

- magic: 0xA55A (bytes: 0xA5, 0x5A)
- flags: 0x00 (reserved)

CRC

- CRC-16/CCITT-FALSE
  - poly = 0x1021
  - init = 0xFFFF
  - refin = false
  - refout = false
  - xorout = 0x0000
