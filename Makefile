LDLIBS = -L/usr/lib/libxml2 -lxml2 `pkg-config --libs gtk+-3.0`
CFLAGS = -Wall -g -std=c99 -DDEBUG -export-dynamic -I/usr/include/libxml2 `pkg-config --cflags gtk+-3.0`

all: gqda 

clean:
	rm -f gqda