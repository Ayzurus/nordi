/**
 * Copyright (c) 2023 Ayzurus
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

#ifndef NORDI_GUI_H_
#define NORDI_GUI_H_

#include <gtk/gtk.h>
#include "nordi_app.h"

#define NORDI_GUI_TYPE (nordi_gui_get_type())

G_DECLARE_FINAL_TYPE(nordi_gui_t, nordi_gui, NORDI, GUI, GtkApplicationWindow)

typedef nordi_gui_t* nordi_gui_ptr;

typedef nordi_gui_tClass* nordi_gui_class; // Mask name generated from GTK

nordi_gui_ptr nordi_gui_new(nordi_app_ptr app);

void nordi_gui_open(nordi_gui_ptr win, GFile* file);

#endif /* NORDI_GUI_H_ */
