/**
 * Copyright (c) 2023 Ayzurus
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

#ifndef NORDI_APP_H_
#define NORDI_APP_H_

#include <gtk/gtk.h>

#define NORDI_APP_TYPE (nordi_app_get_type())

G_DECLARE_FINAL_TYPE(nordi_app_t, nordi_app, NORDI, APP, GtkApplication)

typedef nordi_app_t* nordi_app_ptr;

typedef nordi_app_tClass* nordi_app_class; // Mask name generated from GTK

/**
 * @brief Creates a new instance of the Nordi GTK app.
 * @return nordi_app_ptr 
 */
nordi_app_ptr nordi_app_new();

/**
 * @brief Runs the Nordi app.
 * @param argc Argument count passed to the application.
 * @param argv The arguments passed to the application.
 * @return The resulting status of the GTK app. 
 */
int nordi_app_run(int, char**);

#endif /* NORDI_APP_H_ */