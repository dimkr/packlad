include ../Makefile.common

LIBS = -led25519

PROG = keygen
KEYS = pub_key priv_key

all: $(KEYS)

$(KEYS): $(PROG)
	./keygen $(KEYS)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(PROG): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS)

clean:
	rm -f $(KEYS) $(PROG) $(OBJS)
