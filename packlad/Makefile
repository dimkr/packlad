include ../Makefile.common

LIBS = -lpacklad-logic -lpacklad-core -led25519

PROG = packlad

all: $(PROG)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS) $(CURL_CFLAGS) $(LIBARCHIVE_CFLAGS) $(ZLIB_CFLAGS)

$(PROG): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS) $(CURL_LIBS) $(LIBARCHIVE_LIBS) $(ZLIB_LIBS)

clean:
	rm -f $(PROG) $(OBJS)
