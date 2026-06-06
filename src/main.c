#include <gtk/gtk.h>
#include <stdlib.h>
#include "../include/parser.h"

static void on_file_open_ready(GObject *source_object, GAsyncResult *res, gpointer data) {
    GtkFileDialog *dialog = GTK_FILE_DIALOG(source_object);
    GError *error = NULL;
    
    GFile *file = gtk_file_dialog_open_finish(dialog, res, &error);

    if (file != NULL) {
        char *path = g_file_get_path(file);
        g_print("Выбран файл: %s\n", path);
        char command[512];
        snprintf(command, sizeof(command), "killall swaybg; swaybg -i \"%s\" -m fill &", path);
        system(command);
        g_free(path);
        g_object_unref(file);
    } else {
        g_print("Файл не выбран: %s\n", error->message);
        g_error_free(error);
    }
}

static void on_btn_wallpaper_clicked(GtkButton *button, gpointer user_data) {
    GtkWindow *parent_window = GTK_WINDOW(user_data);
    GtkFileDialog *dialog = gtk_file_dialog_new();
    gtk_file_dialog_set_title(dialog, "Выберите обои");
    gtk_file_dialog_open(dialog, parent_window, NULL, on_file_open_ready, NULL);
    g_object_unref(dialog);
}

static void on_btn_apply_kitty_clicked(GtkButton *button, gpointer user_data) {
    GtkColorDialogButton *color_btn = GTK_COLOR_DIALOG_BUTTON(user_data);
    const GdkRGBA *selected_color = gtk_color_dialog_button_get_rgba(color_btn);
    if (selected_color != NULL) {
        save_kitty_bg_color(selected_color);
        system("killall -SIGUSR1 kitty");
    }
}

static void on_activate(GtkApplication *app, gpointer user_data) {
    GtkBuilder *builder = gtk_builder_new_from_file("ui/window.ui");
    GtkWindow *window = GTK_WINDOW(gtk_builder_get_object(builder, "main_window"));
    gtk_window_set_application(window, app);
    GtkButton *btn_wallpaper = GTK_BUTTON(gtk_builder_get_object(builder, "btn_wallpaper"));
    g_signal_connect(btn_wallpaper, "clicked", G_CALLBACK(on_btn_wallpaper_clicked), window);
    GtkColorDialogButton *color_btn_kitty_bg = GTK_COLOR_DIALOG_BUTTON(gtk_builder_get_object(builder, "color_btn_kitty_bg"));
    GtkButton *btn_apply_kitty = GTK_BUTTON(gtk_builder_get_object(builder, "btn_apply_kitty"));
    g_signal_connect(btn_apply_kitty, "clicked", G_CALLBACK(on_btn_apply_kitty_clicked), color_btn_kitty_bg);
    gtk_window_present(window);
    g_object_unref(builder);
}

int main(int argc, char **argv) {
    GtkApplication *app = gtk_application_new("org.danich.hyprtweaker", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
