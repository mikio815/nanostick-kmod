CFLAGS ?= -O2 -Wall -Wextra
CPPFLAGS += -I..

BIN := ns_ldisc_ctl ns_pty_test

all: $(BIN)

ns_ldisc_ctl: ns_ldisc_ctl.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $<

ns_pty_test: ns_pty_test.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $<

clean:
	rm -f $(BIN)
