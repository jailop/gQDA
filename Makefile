LDLIBS = -L/usr/lib/libxml2 -lxml2 `pkg-config --libs gtk+-3.0 gtksourceview-4 json-glib-1.0`
CFLAGS = -Wall -g  -std=gnu99 -DDEBUG -O2 -export-dynamic -I/usr/include/libxml2 `pkg-config --cflags gtk+-3.0 gtksourceview-4 json-glib-1.0`
OBJECT = extension.o base.o selection.o xmlio.o resources.o util.o search.o
BINARY = gqda goutliner

all: $(BINARY)

gqda: extension.o base.o selection.o xmlio.o resources.o util.o search.o

resources.o: resources.c

resources.c: resources.xml gqda.ui
	glib-compile-resources --generate-header resources.xml
	glib-compile-resources --generate-source resources.xml

install: gqda
	cp $(BINARY) /usr/local/bin

uninstall:
	rm -f /usr/local/bin/gqda

clean:
	rm -f $(BINARY)
	rm -f $(OBJECT)
	rm -f resources.c resources.h
