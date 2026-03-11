CC = gcc
CFLAGS = -Wall -Wextra -std=c11
PREFIX ?= /usr/local
DESTDIR ?=
INSTALL = install

TARGET = src/proctop
SRC = src/main.c src/ui.c src/system.c src/proc.c
MANPAGE = proctop.1
BINDIR = $(DESTDIR)$(PREFIX)/bin
MANDIR = $(DESTDIR)$(PREFIX)/share/man/man1

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

install: $(TARGET)
	$(INSTALL) -d $(BINDIR)
	$(INSTALL) -m 755 $(TARGET) $(BINDIR)/proctop

uninstall:
	rm -f $(BINDIR)/proctop

install-man:
	$(INSTALL) -d $(MANDIR)
	$(INSTALL) -m 644 $(MANPAGE) $(MANDIR)/$(MANPAGE)

uninstall-man:
	rm -f $(MANDIR)/$(MANPAGE)

clean:
	rm -f $(TARGET)
