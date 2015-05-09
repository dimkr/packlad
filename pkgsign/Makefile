include ../Makefile.common

LIBS = -led25519

PROG = pkgsign

all: $(PROG)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(PROG): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS)

clean:
	rm -f $(PROG) $(OBJS)
