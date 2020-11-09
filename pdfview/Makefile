LDLIBS = `pkg-config --libs gtk+-3.0 poppler-glib`
CFLAGS = -Wall -g -export-dynamic `pkg-config --cflags gtk+-3.0 poppler-glib`

all: pdfview

install:
	cp pdfview /usr/local/bin

clean:
	rm -f pdfview
