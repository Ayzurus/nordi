/**
 * Copyright (c) 2023 Ayzurus
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

#include <gtk/gtk.h>
#include <stdlib.h>
#include "nordi_app.h"
#include "nordi_gui.h"
#include "nordvpn_api.h"

struct _nordi_app_t {
    GtkApplication parent;
};

G_DEFINE_TYPE(nordi_app_t, nordi_app, GTK_TYPE_APPLICATION);

static void
nordi_app_init(nordi_app_ptr app) {}

static void
nordi_app_activate(GApplication* app) {
    nordi_gui_ptr window = nordi_gui_new(NORDI_APP(app));
    gtk_window_present(GTK_WINDOW(window));
}

static void
nordi_app_open(GApplication* app, GFile** files, gint n_files, const gchar* hint) {
    GList* windows;
    nordi_gui_ptr window;
    windows = gtk_application_get_windows(GTK_APPLICATION(app));
    if (windows) {
        window = NORDI_GUI(windows->data);
    } else {
        window = nordi_gui_new(NORDI_APP(app));
    }
    for (int i = 0; i < n_files; i++) {
        nordi_gui_open(window, files[i]);
    }
    gtk_window_present(GTK_WINDOW(window));
}

// static void
// nordi_app_shutdown(GApplication* app) {
//     // close window, but move app to system tray
// }

static void
nordi_app_class_init(nordi_app_class class) {
    G_APPLICATION_CLASS(class)->activate = nordi_app_activate;
    G_APPLICATION_CLASS(class)->open = nordi_app_open;
    // G_APPLICATION_CLASS(class)->shutdown = nordi_app_shutdown;
}

nordi_app_ptr
nordi_app_new() {
    return g_object_new(NORDI_APP_TYPE, "application-id", "com.nordi", "flags", G_APPLICATION_HANDLES_OPEN, NULL);
}

int
nordi_app_run(int argc, char** argv) {
    nordvpn_error_t result = nordvpn_open();
    if (result != OK) {
        g_printerr("Couldn't start a NordVPN API session: %s\n", str_ptr(nordvpn_error(result)));
        nordvpn_close();
        return G_IO_ERROR_CANCELLED;
    }
    int status = g_application_run(G_APPLICATION(nordi_app_new()), argc, argv);
    nordvpn_close();
    return status;
}
