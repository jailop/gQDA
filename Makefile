LDLIBS = `pkg-config --libs gtk+-3.0`
CFLAGS = -Wall -g -std=c99 -export-dynamic `pkg-config --cflags gtk+-3.0`

all: gqda 

clean:
	rm -f gqda
