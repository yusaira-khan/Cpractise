$(shell mkdir -p bin )

CFLAGS = -g -Werror -Wall
LDFLAGS = -lrt -pthread


.PHONY: clean
clean:
	@rm -v *o

server: server.c info.h
	$(CC) $(CFLAGS)   -o server info.h server.c $(LDFLAGS)
	
client: client.c info.h
	$(CC) $(CFLAGS)   -o client info.h client.c $(LDFLAGS)
	


.PHONY: all
all:    server    client
