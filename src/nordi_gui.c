/**
 * Copyright (c) 2023 Ayzurus
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

#include <gtk/gtk.h>
#include <stdio.h>
#include "nordi_app.h"
#include "nordi_gui.h"
#include "nordvpn_api.h"
#include "nordvpn_server.h"

struct _nordi_gui_t {
    GtkApplicationWindow parent;
    GtkDialog* dialog;
    // NordVPN API
    nordvpn_session_ptr nordvpn_session;
    nordvpn_host_ptr nordvpn_host;
    // template UI widget references
    // VPN page
    GtkComboBoxText* country_combo;
    GtkSpinner* spinner1;
    GtkSpinner* spinner2;
    GtkButton* connect_button;
    GtkButton* disconnect_button;
    GtkLabel* ip_label;
    GtkLabel* host_label;
    GtkStatusbar* status_bar;
    // Account page
    GtkLabel* version_label;
    GtkLabel* email_label;
    GtkLabel* expire_label;
    GtkButton* login_button;
    GtkButton* logout_button;
};

G_DEFINE_TYPE(nordi_gui_t, nordi_gui, GTK_TYPE_APPLICATION_WINDOW);

static nordi_gui_ptr
get_nordi_gui_from(GtkWidget* widget) {
    GtkWidget* main = gtk_widget_get_root(widget);
    if (main == NULL) {
        g_critical("Failed to fetch Nordi main window from widget: ", gtk_widget_get_name(widget));
        return NULL;
    }
    return NORDI_GUI(main);
}

static void
nordi_gui_update_vpn_data(nordi_gui_ptr window) {
    if (window->nordvpn_host->is_online) {
        gtk_statusbar_push(window->status_bar, 0, "Connected");
        gtk_label_set_label(window->ip_label, str_ptr(window->nordvpn_host->ip));
        gtk_label_set_label(window->host_label, str_ptr(window->nordvpn_host->name));
        gtk_widget_set_visible(GTK_WIDGET(window->disconnect_button), true);
        gtk_widget_set_visible(GTK_WIDGET(window->connect_button), false);
    } else {
        gtk_statusbar_push(window->status_bar, 0, "Disconnected");
        gtk_label_set_label(window->ip_label, "");
        gtk_label_set_label(window->host_label, "");
        gtk_widget_set_visible(GTK_WIDGET(window->disconnect_button), false);
        gtk_widget_set_visible(GTK_WIDGET(window->connect_button), true);
    }
}

static void
nordi_gui_update_account_data(nordi_gui_ptr window) {
    if (str_is_empty(window->nordvpn_session->user)) {
        gtk_label_set_label(window->email_label, "");
        gtk_label_set_label(window->expire_label, "");
        gtk_widget_set_visible(GTK_WIDGET(window->login_button), true);
        gtk_widget_set_visible(GTK_WIDGET(window->logout_button), false);
    } else {
        gtk_label_set_label(window->email_label, str_ptr(window->nordvpn_session->user));
        gtk_label_set_label(window->expire_label, str_ptr(window->nordvpn_session->expiry));
        gtk_widget_set_visible(GTK_WIDGET(window->login_button), false);
        gtk_widget_set_visible(GTK_WIDGET(window->logout_button), true);
    }
}

static void
nordi_gui_connect(GtkButton* button) {
    gtk_widget_set_sensitive(GTK_WIDGET(button), false);
    nordi_gui_ptr window = get_nordi_gui_from(GTK_WIDGET(button));
    gtk_statusbar_push(window->status_bar, 0, "Connecting...");
    str server = nordvpn_node_from_index(gtk_combo_box_get_active(GTK_COMBO_BOX(window->country_combo)));
    nordvpn_error_t result = nordvpn_server_connect(server);
    if (!window->nordvpn_host->is_online) {
        g_warning("Failed to connect to NordVPN");
        gtk_statusbar_push(window->status_bar, 0, "Failed to connect to the server");
    }
    nordi_gui_update_vpn_data(window);
    gtk_widget_set_sensitive(GTK_WIDGET(button), true);
    gtk_spinner_set_spinning(window->spinner1, false);
}

static void
nordi_gui_disconnect(GtkButton* button) {
    gtk_widget_set_sensitive(GTK_WIDGET(button), false);
    nordi_gui_ptr window = get_nordi_gui_from(GTK_WIDGET(button));
    gtk_spinner_set_spinning(window->spinner1, true);
    gtk_statusbar_push(window->status_bar, 0, "Disconnecting...");
    nordvpn_error_t result = nordvpn_disconnect();
    if (window->nordvpn_host->is_online) {
        g_warning("Failed to disconnect from NordVPN");
        gtk_statusbar_push(window->status_bar, 0, "Failed to disconnect from the server");
    }
    nordi_gui_update_vpn_data(window);
    gtk_widget_set_sensitive(GTK_WIDGET(button), true);
    gtk_spinner_set_spinning(window->spinner1, false);
}

static void
nordi_gui_login_refresh(GtkWindow* window) {
    nordvpn_refresh();
    nordi_gui_ptr nordi = NORDI_GUI(window);
    nordi_gui_update_account_data(nordi);
    gtk_window_destroy(nordi->dialog);
    nordi->dialog = NULL;
}

static void
nordi_gui_login(GtkButton* button) {
    nordi_gui_ptr window = get_nordi_gui_from(GTK_WIDGET(button));
    str login_link = str_null;
    nordvpn_error_t error = nordvpn_login(&login_link);
    if (error != OK || str_is_empty(login_link)) {
        gtk_statusbar_push(window->status_bar, 0, "Failed to get login link");
        return;
    }
    nordi_gui_update_account_data(window);
    GtkDialog* dialog = gtk_dialog_new_with_buttons("Login to NordVPN", GTK_WINDOW(window), 0, "Ok", GTK_RESPONSE_NONE, NULL);
    GtkBox* content = gtk_dialog_get_content_area(dialog);
    GtkLabel* tip = gtk_label_new("2. Hit 'Ok' after finishing.");
    GtkLinkButton* link = gtk_link_button_new_with_label(str_ptr(login_link), "1. Click to log in to NordVPN.");
    gtk_widget_set_margin_start(GTK_WIDGET(content), 10);
    gtk_widget_set_margin_end(GTK_WIDGET(content), 10);
    gtk_widget_set_margin_top(GTK_WIDGET(content), 10);
    gtk_widget_set_margin_bottom(GTK_WIDGET(content), 10);
    gtk_box_set_spacing(content, 10);
    gtk_box_append(content, link);
    gtk_box_append(content, tip);
    gtk_widget_set_halign(GTK_WIDGET(tip), GTK_ALIGN_START);
    gtk_widget_set_halign(GTK_WIDGET(link), GTK_ALIGN_START);
    window->dialog = dialog;
    g_signal_connect_swapped(dialog, "response", G_CALLBACK(nordi_gui_login_refresh), window);
    gtk_widget_show(dialog);
}

static void
nordi_gui_logout(GtkButton* button) {
    nordi_gui_ptr window = get_nordi_gui_from(GTK_WIDGET(button));
    nordvpn_logout();
    nordi_gui_update_account_data(window);
    nordi_gui_update_vpn_data(window);
}

static void
nordi_gui_init(nordi_gui_ptr window) {
    gtk_widget_init_template(GTK_WIDGET(window));
    // Setup NordVPN API
    window->nordvpn_session = nordvpn_get_session();
    window->nordvpn_host = nordvpn_get_host();
    if (window->nordvpn_session->is_active) {
        nordi_gui_update_vpn_data(window);
        nordi_gui_update_account_data(window);
        gtk_label_set_label(window->version_label, str_ptr(window->nordvpn_session->version));
    } else {
        gtk_statusbar_push(window->status_bar, 0, "Session failed to start");
        gtk_label_set_label(window->version_label, "NordVPN not found");
    }
    // Populate information on widgets
    for (int server = 0; server < COUNTRY_COUNT + GROUP_COUNT; server++) {
        str next_server = server < COUNTRY_COUNT ? NORDVPN_COUNTRY_STR[server] : NORDVPN_GROUP_STR[server - COUNTRY_COUNT];
        gtk_combo_box_text_append_text(GTK_WIDGET(window->country_combo), str_ptr(next_server));
    }
    // Associate callbacks
    g_signal_connect(window->connect_button, "clicked", G_CALLBACK(nordi_gui_connect), NULL);
    g_signal_connect(window->disconnect_button, "clicked", G_CALLBACK(nordi_gui_disconnect), NULL);
    g_signal_connect(window->login_button, "clicked", G_CALLBACK(nordi_gui_login), NULL);
    g_signal_connect(window->logout_button, "clicked", G_CALLBACK(nordi_gui_logout), NULL);
}

static void
nordi_gui_class_init(nordi_gui_class class) {
    gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(class), "/com/nordi/nordi.ui");
    GtkWidgetClass* widget_class = GTK_WIDGET_CLASS(class);
    // Populate template references
    gtk_widget_class_bind_template_child(widget_class, nordi_gui_t, country_combo);
    gtk_widget_class_bind_template_child(widget_class, nordi_gui_t, spinner1);
    gtk_widget_class_bind_template_child(widget_class, nordi_gui_t, connect_button);
    gtk_widget_class_bind_template_child(widget_class, nordi_gui_t, disconnect_button);
    gtk_widget_class_bind_template_child(widget_class, nordi_gui_t, status_bar);
    gtk_widget_class_bind_template_child(widget_class, nordi_gui_t, ip_label);
    gtk_widget_class_bind_template_child(widget_class, nordi_gui_t, host_label);
    gtk_widget_class_bind_template_child(widget_class, nordi_gui_t, version_label);
    gtk_widget_class_bind_template_child(widget_class, nordi_gui_t, email_label);
    gtk_widget_class_bind_template_child(widget_class, nordi_gui_t, expire_label);
    gtk_widget_class_bind_template_child(widget_class, nordi_gui_t, login_button);
    gtk_widget_class_bind_template_child(widget_class, nordi_gui_t, logout_button);
}

nordi_gui_ptr
nordi_gui_new(nordi_app_ptr app) {
    return g_object_new(NORDI_GUI_TYPE, "application", app, NULL);
}

void
nordi_gui_open(nordi_gui_ptr window, GFile* file) {}