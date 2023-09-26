CC=gcc
CFLAGS=-Wall -g -O0
LIBS=
OBJS=array.o options.o

PROGS= array

all: $(PROGS)

%.o : %.c
	$(CC) $(CFLAGS) -c $<

array: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

clean:
	rm -f $(PROGS) *.o *~

