CC=gcc
APP=test_tcpserver
SRCS= test_tcpserver.c tcpserver.c
OBJS=  $(patsubst %.c,%.o,$(SRCS))
CFLAGS= -Wall -O2 -pipe
LIBS=-lpthread #-lnsl -lsocket -lresolv

all: $(APP)

$(OBJS): %.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

$(APP): $(OBJS)
	$(CC) $(CFLAGS) $(LIBS) -o $(APP) $(OBJS)

clean:
	@-rm *.o *.core $(APP)
