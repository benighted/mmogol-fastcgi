all:	cgi

cgi:
	gcc server.c -oserver -lfcgi -lm -lpthread

clean:
	rm server

