FLAGS=-Wall -g `pkg-config --cflags --libs cairo pangocairo x11 xinerama xrender xrandr xdamage xcomposite imlib2 glib-2.0`
PROGNAME=tint2
FILES=*.c

ifndef DESTDIR
	ifndef PREFIX
		BINDIR=/usr/bin
		XDG_CONFIG_DIR=/etc/xdg
	else
		BINDIR=$(PREFIX)/bin
		XDG_CONFIG_DIR=/etc/xdg
	endif
else
	BINDIR=$(DESTDIR)/usr/bin
	XDG_CONFIG_DIR=$(DESTDIR)/etc/xdg
endif


$(PROGNAME): $(FILES)
	$(CC) $(CFLAGS) $(FLAGS) -o $(PROGNAME) $(FILES)
	strip $(PROGNAME)

install:
	mkdir -p $(BINDIR)
	mkdir -p $(XDG_CONFIG_DIR)/tint2
	install $(PROGNAME) $(BINDIR)
	cp -f ../sample.tint2rc $(XDG_CONFIG_DIR)/tint2/tint2rc

uninstall:
	rm -f $(BINDIR)/$(PROGNAME)

clean:
	rm -f $(PROGNAME)

