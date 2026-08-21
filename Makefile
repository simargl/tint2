CC      ?= cc
CFLAGS  ?= -O0 -g -Wall -Wextra -std=gnu99
CPPFLAGS += -Isrc

PKGS = cairo pangocairo x11 xinerama xrender xrandr xdamage xcomposite imlib2 glib-2.0

CFLAGS  += $(shell pkg-config --cflags $(PKGS))
LDLIBS  += $(shell pkg-config --libs $(PKGS))

TARGET = tint2

SRCS = \
	src/area.c \
	src/battery.c \
	src/clock.c \
	src/common.c \
	src/config.c \
	src/launcher.c \
	src/panel.c \
	src/server.c \
	src/systraybar.c \
	src/task.c \
	src/taskbar.c \
	src/taskbarname.c \
	src/timer.c \
	src/tint.c \
	src/tooltip.c \
	src/window.c \
	src/xsettings-client.c \
	src/xsettings-common.c

OBJS = $(SRCS:.c=.o)

PREFIX     ?= /usr
SYSCONFDIR ?= /etc
DATADIR    ?= $(PREFIX)/share

BINDIR     = $(PREFIX)/bin
TINT2_CONF = $(SYSCONFDIR)/xdg/tint2
TINT2_DATA = $(DATADIR)/tint2

.PHONY: all clean install uninstall

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDLIBS)

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

-include $(OBJS:.o=.d)

install: $(TARGET)
	install -Dm755 $(TARGET) $(DESTDIR)$(BINDIR)/$(TARGET)
	install -Dm644 tint2rc \
		$(DESTDIR)$(TINT2_CONF)/tint2rc
	install -Dm644 default_icon.png \
		$(DESTDIR)$(TINT2_DATA)/default_icon.png

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(TARGET)
	rm -f $(DESTDIR)$(TINT2_CONF)/tint2rc
	rm -f $(DESTDIR)$(TINT2_DATA)/default_icon.png

clean:
	rm -f $(TARGET) $(OBJS) $(OBJS:.o=.d)
