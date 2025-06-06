/*
 * Copyright © 2001 Red Hat, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Red Hat not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  Red Hat makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * RED HAT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL RED HAT
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Owen Taylor, Red Hat, Inc.
 */
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/Xmd.h>		/* For CARD16 */

#include "xsettings-client.h"
#include "server.h"
#include "panel.h"
#include "launcher.h"

struct _XSettingsClient {
    Display *display;
    int screen;
    XSettingsNotifyFunc notify;
    XSettingsWatchFunc watch;
    void *cb_data;

    Window manager_window;
    XSettingsList *settings;
};


void xsettings_notify_cb (const char *name, XSettingsAction action, XSettingsSetting *setting, void *data) {
    (void)data;
    //printf("xsettings_notify_cb\n");
    if ((action == XSETTINGS_ACTION_NEW || action == XSETTINGS_ACTION_CHANGED) && name != NULL && setting != NULL) {
        if (!strcmp(name, "Net/IconThemeName") && setting->type == XSETTINGS_TYPE_STRING) {
            if (icon_theme_name) {
                if (strcmp(icon_theme_name, setting->data.v_string) == 0)
                    return;
                free(icon_theme_name);
            }
            icon_theme_name = strdup(setting->data.v_string);

            int i;
            for (i = 0 ; i < nb_panel ; i++) {
                Launcher *launcher = &panel1[i].launcher;
                cleanup_launcher_theme(launcher);
                launcher_load_themes(launcher);
                launcher_load_icons(launcher);
                launcher->area.resize = 1;
            }
        }
    }
}


static void notify_changes (XSettingsClient *client, XSettingsList *old_list) {
    XSettingsList *old_iter = old_list;
    XSettingsList *new_iter = client->settings;

    if (!client->notify)
        return;

    while (old_iter || new_iter) {
        int cmp;

        if (old_iter && new_iter)
            cmp = strcmp (old_iter->setting->name, new_iter->setting->name);
        else if (old_iter)
            cmp = -1;
        else
            cmp = 1;

        if (cmp < 0) {
            client->notify (old_iter->setting->name, XSETTINGS_ACTION_DELETED, NULL, client->cb_data);
        } else if (cmp == 0) {
            if (!xsettings_setting_equal (old_iter->setting, new_iter->setting))
                client->notify (old_iter->setting->name, XSETTINGS_ACTION_CHANGED, new_iter->setting, client->cb_data);
        } else {
            client->notify (new_iter->setting->name, XSETTINGS_ACTION_NEW, new_iter->setting, client->cb_data);
        }

        if (old_iter)
            old_iter = old_iter->next;
        if (new_iter)
            new_iter = new_iter->next;
    }
}


static int ignore_errors (Display *display, XErrorEvent *event) {
    (void)display;
    (void)event;
    return True;
}

static char local_byte_order = '\0';

#define BYTES_LEFT(buffer) ((buffer)->data + (buffer)->len - (buffer)->pos)

static XSettingsResult fetch_card16 (XSettingsBuffer *buffer, CARD16 *result) {
    CARD16 x;

    if (BYTES_LEFT (buffer) < 2)
        return XSETTINGS_ACCESS;

    x = *(CARD16 *)buffer->pos;
    buffer->pos += 2;

    if (buffer->byte_order == local_byte_order)
        *result = x;
    else
        *result = (x << 8) | (x >> 8);

    return XSETTINGS_SUCCESS;
}


static XSettingsResult fetch_ushort (XSettingsBuffer *buffer, unsigned short  *result) {
    CARD16 x;
    XSettingsResult r;

    r = fetch_card16 (buffer, &x);
    if (r == XSETTINGS_SUCCESS)
        *result = x;

    return r;
}


static XSettingsResult fetch_card32 (XSettingsBuffer *buffer, CARD32 *result) {
    CARD32 x;

    if (BYTES_LEFT (buffer) < 4)
        return XSETTINGS_ACCESS;

    x = *(CARD32 *)buffer->pos;
    buffer->pos += 4;

    if (buffer->byte_order == local_byte_order)
        *result = x;
    else
        *result = (x << 24) | ((x & 0xff00) << 8) | ((x & 0xff0000) >> 8) | (x >> 24);

    return XSETTINGS_SUCCESS;
}

static XSettingsResult fetch_card8 (XSettingsBuffer *buffer, CARD8 *result) {
    if (BYTES_LEFT (buffer) < 1)
        return XSETTINGS_ACCESS;

    *result = *(CARD8 *)buffer->pos;
    buffer->pos += 1;

    return XSETTINGS_SUCCESS;
}

#define XSETTINGS_PAD(n,m) ((n + m - 1) & (~(m-1)))

static XSettingsList *parse_settings (unsigned char *data, size_t len) {
    XSettingsBuffer buffer;
    XSettingsResult result = XSETTINGS_SUCCESS;
    XSettingsList *settings = NULL;
    CARD32 serial;
    CARD32 n_entries;
    CARD32 i;
    XSettingsSetting *setting = NULL;

    local_byte_order = xsettings_byte_order ();

    buffer.pos = buffer.data = data;
    buffer.len = len;

    result = fetch_card8 (&buffer, (CARD8*)&buffer.byte_order);
    if (buffer.byte_order != MSBFirst && buffer.byte_order != LSBFirst) {
        fprintf (stderr, "Invalid byte order %x in XSETTINGS property\n", buffer.byte_order);
        result = XSETTINGS_FAILED;
        goto out;
    }

    buffer.pos += 3;

    result = fetch_card32 (&buffer, &serial);
    if (result != XSETTINGS_SUCCESS)
        goto out;

    result = fetch_card32 (&buffer, &n_entries);
    if (result != XSETTINGS_SUCCESS)
        goto out;

    for (i = 0; i < n_entries; i++) {
        CARD8 type;
        CARD16 name_len;
        CARD32 v_int;
        size_t pad_len;

        result = fetch_card8 (&buffer, &type);
        if (result != XSETTINGS_SUCCESS)
            goto out;

        buffer.pos += 1;

        result = fetch_card16 (&buffer, &name_len);
        if (result != XSETTINGS_SUCCESS)
            goto out;

        pad_len = XSETTINGS_PAD(name_len, 4);
        if ((size_t)BYTES_LEFT (&buffer) < pad_len) {
            result = XSETTINGS_ACCESS;
            goto out;
        }

        setting = malloc (sizeof *setting);
        if (!setting) {
            result = XSETTINGS_NO_MEM;
            goto out;
        }
        setting->type = XSETTINGS_TYPE_INT; /* No allocated memory */

        setting->name = malloc (name_len + 1);
        if (!setting->name) {
            result = XSETTINGS_NO_MEM;
            goto out;
        }

        memcpy (setting->name, buffer.pos, name_len);
        setting->name[name_len] = '\0';
        buffer.pos += pad_len;

        result = fetch_card32 (&buffer, &v_int);
        if (result != XSETTINGS_SUCCESS)
            goto out;
        setting->last_change_serial = v_int;

        switch (type) {
        case XSETTINGS_TYPE_INT:
            result = fetch_card32 (&buffer, &v_int);
            if (result != XSETTINGS_SUCCESS)
                goto out;
            setting->data.v_int = (INT32)v_int;
            break;
        case XSETTINGS_TYPE_STRING:
            result = fetch_card32 (&buffer, &v_int);
            if (result != XSETTINGS_SUCCESS)
                goto out;

            pad_len = XSETTINGS_PAD (v_int, 4);
            if (v_int + 1 == 0 || /* Guard against wrap-around */
                    (size_t)BYTES_LEFT (&buffer) < pad_len) {
                result = XSETTINGS_ACCESS;
                goto out;
            }

            setting->data.v_string = malloc (v_int + 1);
            if (!setting->data.v_string) {
                result = XSETTINGS_NO_MEM;
                goto out;
            }

            memcpy (setting->data.v_string, buffer.pos, v_int);
            setting->data.v_string[v_int] = '\0';
            buffer.pos += pad_len;
            break;
        case XSETTINGS_TYPE_COLOR:
            result = fetch_ushort (&buffer, &setting->data.v_color.red);
            if (result != XSETTINGS_SUCCESS)
                goto out;
            result = fetch_ushort (&buffer, &setting->data.v_color.green);
            if (result != XSETTINGS_SUCCESS)
                goto out;
            result = fetch_ushort (&buffer, &setting->data.v_color.blue);
            if (result != XSETTINGS_SUCCESS)
                goto out;
            result = fetch_ushort (&buffer, &setting->data.v_color.alpha);
            if (result != XSETTINGS_SUCCESS)
                goto out;
            break;
        default:
            /* Quietly ignore unknown types */
            break;
        }

        setting->type = type;

        result = xsettings_list_insert (&settings, setting);
        if (result != XSETTINGS_SUCCESS)
            goto out;

        setting = NULL;
    }

out:

    if (result != XSETTINGS_SUCCESS) {
        switch (result) {
        case XSETTINGS_NO_MEM:
            fprintf(stderr, "Out of memory reading XSETTINGS property\n");
            break;
        case XSETTINGS_ACCESS:
            fprintf(stderr, "Invalid XSETTINGS property (read off end)\n");
            break;
        case XSETTINGS_DUPLICATE_ENTRY:
            fprintf (stderr, "Duplicate XSETTINGS entry for '%s'\n", setting->name);
        case XSETTINGS_FAILED:
        case XSETTINGS_SUCCESS:
        case XSETTINGS_NO_ENTRY:
            break;
        }

        if (setting)
            xsettings_setting_free (setting);

        xsettings_list_free (settings);
        settings = NULL;
    }

    return settings;
}


static void read_settings (XSettingsClient *client) {
    Atom type;
    int format;
    unsigned long n_items;
    unsigned long bytes_after;
    unsigned char *data;
    int result;

    int (*old_handler) (Display *, XErrorEvent *);

    XSettingsList *old_list = client->settings;
    client->settings = NULL;

    old_handler = XSetErrorHandler (ignore_errors);
    result = XGetWindowProperty (client->display, client->manager_window, server.atom._XSETTINGS_SETTINGS, 0, LONG_MAX, False, server.atom._XSETTINGS_SETTINGS, &type, &format, &n_items, &bytes_after, &data);
    XSetErrorHandler (old_handler);

    if (result == Success && type == server.atom._XSETTINGS_SETTINGS) {
        if (format != 8) {
            fprintf (stderr, "Invalid format for XSETTINGS property %d", format);
        } else
            client->settings = parse_settings (data, n_items);
        XFree (data);
    }

    notify_changes (client, old_list);
    xsettings_list_free (old_list);
}


static void check_manager_window (XSettingsClient *client) {
    if (client->manager_window && client->watch)
        client->watch (client->manager_window, False, 0, client->cb_data);

    XGrabServer (client->display);

    client->manager_window = XGetSelectionOwner (server.dsp, server.atom._XSETTINGS_SCREEN);
    if (client->manager_window)
        XSelectInput (server.dsp, client->manager_window, PropertyChangeMask | StructureNotifyMask);

    XUngrabServer (client->display);
    XFlush (client->display);

    if (client->manager_window && client->watch)
        client->watch (client->manager_window, True, PropertyChangeMask | StructureNotifyMask, client->cb_data);

    read_settings (client);
}


XSettingsClient *xsettings_client_new (Display *display, int screen, XSettingsNotifyFunc notify, XSettingsWatchFunc watch, void *cb_data) {
    XSettingsClient *client;

    client = malloc (sizeof *client);
    if (!client)
        return NULL;

    client->display = display;
    client->screen = screen;
    client->notify = notify;
    client->watch = watch;
    client->cb_data = cb_data;

    client->manager_window = None;
    client->settings = NULL;

    if (client->watch)
        client->watch (RootWindow (display, screen), True, StructureNotifyMask,	client->cb_data);

    check_manager_window (client);

    if (client->manager_window == None) {
        printf("NO XSETTINGS manager, tint2 use config 'launcher_icon_theme'.\n");
        free (client);
        return NULL;
    } else
        return client;
}


void xsettings_client_destroy (XSettingsClient *client) {
    if (client->watch)
        client->watch (RootWindow (client->display, client->screen), False, 0, client->cb_data);
    if (client->manager_window && client->watch)
        client->watch (client->manager_window, False, 0, client->cb_data);

    xsettings_list_free (client->settings);
    free (client);
}


XSettingsResult xsettings_client_get_setting (XSettingsClient *client, const char *name, XSettingsSetting **setting) {
    XSettingsSetting *search = xsettings_list_lookup (client->settings, name);
    if (search) {
        *setting = xsettings_setting_copy (search);
        return *setting ? XSETTINGS_SUCCESS : XSETTINGS_NO_MEM;
    } else
        return XSETTINGS_NO_ENTRY;
}


Bool xsettings_client_process_event (XSettingsClient *client, XEvent *xev) {
    /* The checks here will not unlikely cause us to reread
    * the properties from the manager window a number of
    * times when the manager changes from A->B. But manager changes
    * are going to be pretty rare.
    */
    if (xev->xany.window == RootWindow (server.dsp, server.screen)) {
        if (xev->xany.type == ClientMessage && xev->xclient.message_type == server.atom.MANAGER) {
            check_manager_window (client);
            return True;
        }
    } else if (xev->xany.window == client->manager_window) {
        if (xev->xany.type == DestroyNotify) {
            check_manager_window (client);
            return True;
        } else if (xev->xany.type == PropertyNotify) {
            read_settings (client);
            return True;
        }
    }

    return False;
}

