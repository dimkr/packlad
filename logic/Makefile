include ../Makefile.common

LIB = libpacklad-logic.a

all: $(LIB)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS) $(CURL_CFLAGS) $(LIBARCHIVE_CFLAGS) $(ZLIB_CFLAGS)

$(LIB): $(OBJS)
	ar rcs $@ $^

clean:
	rm -f $(LIB) $(OBJS)
