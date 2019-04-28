C = gcc
CFLAGS= -std=c99
LDLIBS=  -lpthread

all: hash hashint debug_hashint lhi lh parse

hash.o: hash.c
	$(C) $(CFLAGS) -c hash.c $(LDLIBS)

hash: hash.o

hashint.o: hashint.c
	$(C) $(CFLAGS) -c hashint.c $(LDLIBS)

hashint: hashint.o

debug_hashint.o: debug_hashint.c
	$(C) $(CFLAGS) -c debug_hashint.c $(LDLIBS)

debug_hashint: debug_hashint.o

lhi.o: lhi.c 
	$(C) $(CFLAGS) -c lhi.c $(LDLIBS)

lhi: lhi.o

lh.o: lh.c 
	$(C) $(CFLAGS) -c lh.c $(LDLIBS)

lh: lh.o

parse.o: parse.c 
	$(C) $(CFLAGS) -c parse.c $(LDLIBS)

parse: parse.o



clean:
	rm -f *~ *.o hash hashint debug_hashint lh lhi parse
