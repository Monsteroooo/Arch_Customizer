#include <gtk/gtk.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include "../include/parser.h"

static GtkRange *ui_scale_gaps_in;
static GtkColorDialogButton *ui_color_kitty;
static GtkColorDialogButton *ui_color_hypr;
static GtkColorDialogButton *ui_color_hypr_inactive;
static GtkEntry *ui_entry_preset_name;
static GtkDropDown *ui_combo_presets;

static GtkColorDialogButton *ui_color_wofi_bg;
static GtkColorDialogButton *ui_color_wofi_text;
static GtkColorDialogButton *ui_color_wofi_selected;
static GtkColorDialogButton *ui_color_wofi_border;

static GtkColorDialogButton *ui_color_waybar_workspace;

// Указатели для SwayNC
static GtkColorDialogButton *ui_color_swaync_bg;
static GtkColorDialogButton *ui_color_swaync_text;
static GtkColorDialogButton *ui_color_swaync_accent;
static GtkColorDialogButton *ui_color_swaync_border;

// Добавлены дефолтные цвета SwayNC
static AppState current_state = {5, "#ffffff", "#aaaaaa", "#000000", "", "#1e1e2e", "#cdd6f4", "#b4befe", "#89b4fa", "#cba6f7", "#1e1e2e", "#cdd6f4", "#89b4fa", "#cba6f7"};

void rgba_to_hex(const GdkRGBA *color, char *out_hex) {
    if (color) {
        snprintf(out_hex, 8, "#%02x%02x%02x", (int)(color->red*255), (int)(color->green*255), (int)(color->blue*255));
    }
}

void hex_to_rgba(const char *hex, GdkRGBA *color) {
    unsigned int r, g, b;
    if (sscanf(hex, "#%02x%02x%02x", &r, &g, &b) == 3) {
        color->red = r/255.0; color->green = g/255.0; color->blue = b/255.0; color->alpha = 1.0;
    }
}

static void sync_ui_with_state() {
    gtk_range_set_value(ui_scale_gaps_in, (double)current_state.gaps_in);
    
    GdkRGBA c_k, c_h, c_hi, c_wb, c_wt, c_ws, c_wborder, c_waybar_ws;
    GdkRGBA c_snc_bg, c_snc_txt, c_snc_acc, c_snc_brd;
    
    hex_to_rgba(current_state.kitty_bg, &c_k);
    hex_to_rgba(current_state.hypr_border, &c_h);
    hex_to_rgba(current_state.hypr_border_inactive, &c_hi);
    
    hex_to_rgba(current_state.wofi_bg, &c_wb);
    hex_to_rgba(current_state.wofi_text, &c_wt);
    hex_to_rgba(current_state.wofi_selected, &c_ws);
    hex_to_rgba(current_state.wofi_border, &c_wborder);
    hex_to_rgba(current_state.waybar_workspace_color, &c_waybar_ws);
    
    // Загружаем цвета SwayNC
    hex_to_rgba(current_state.swaync_bg, &c_snc_bg);
    hex_to_rgba(current_state.swaync_text, &c_snc_txt);
    hex_to_rgba(current_state.swaync_accent, &c_snc_acc);
    hex_to_rgba(current_state.swaync_border, &c_snc_brd);
    
    gtk_color_dialog_button_set_rgba(ui_color_kitty, &c_k);
    gtk_color_dialog_button_set_rgba(ui_color_hypr, &c_h);
    gtk_color_dialog_button_set_rgba(ui_color_hypr_inactive, &c_hi);
    
    gtk_color_dialog_button_set_rgba(ui_color_wofi_bg, &c_wb);
    gtk_color_dialog_button_set_rgba(ui_color_wofi_text, &c_wt);
    gtk_color_dialog_button_set_rgba(ui_color_wofi_selected, &c_ws);
    gtk_color_dialog_button_set_rgba(ui_color_wofi_border, &c_wborder);
    gtk_color_dialog_button_set_rgba(ui_color_waybar_workspace, &c_waybar_ws);
    
    // Устанавливаем кнопки SwayNC
    gtk_color_dialog_button_set_rgba(ui_color_swaync_bg, &c_snc_bg);
    gtk_color_dialog_button_set_rgba(ui_color_swaync_text, &c_snc_txt);
    gtk_color_dialog_button_set_rgba(ui_color_swaync_accent, &c_snc_acc);
    gtk_color_dialog_button_set_rgba(ui_color_swaync_border, &c_snc_brd);
}

static void refresh_presets_combo(GtkDropDown *drop_down) {
    GtkStringList *sl = gtk_string_list_new(NULL);
    char path[512];
    snprintf(path, sizeof(path), "%s/.config/hypr-tweaker/presets", getenv("HOME"));
    DIR *d = opendir(path);
    if (d) {
        struct dirent *de;
        while ((de = readdir(d))) {
            if (strstr(de->d_name, ".conf") && strncmp(de->d_name, "_last_state", 11) != 0) {
                char name[256];
                g_strlcpy(name, de->d_name, strlen(de->d_name)-4);
                gtk_string_list_append(sl, name);
            }
        }
        closedir(d);
    }
    gtk_drop_down_set_model(drop_down, G_LIST_MODEL(sl));
}

static void on_btn_save_preset_clicked(GtkButton *b, gpointer d) {
    const char *n = gtk_entry_buffer_get_text(gtk_entry_get_buffer(ui_entry_preset_name));
    if (*n) { save_preset(n, &current_state); refresh_presets_combo(ui_combo_presets); }
}

static void on_btn_apply_preset_clicked(GtkButton *b, gpointer d) {
    GObject *item = gtk_drop_down_get_selected_item(ui_combo_presets);
    if (item) {
        if (load_preset(gtk_string_object_get_string(GTK_STRING_OBJECT(item)), &current_state)) {
            apply_state_to_system(&current_state);
            sync_ui_with_state();
            save_preset("_last_state", &current_state);
        }
    }
}

static void on_btn_delete_preset_clicked(GtkButton *b, gpointer d) {
    GObject *item = gtk_drop_down_get_selected_item(ui_combo_presets);
    if (item) {
        delete_preset(gtk_string_object_get_string(GTK_STRING_OBJECT(item)));
        refresh_presets_combo(ui_combo_presets);
    }
}

static void on_gaps_in_changed(GtkRange *r, gpointer d) {
    current_state.gaps_in = (int)gtk_range_get_value(r);
    char cmd[128]; snprintf(cmd, 128, "hyprctl keyword general:gaps_in %d", current_state.gaps_in);
    system(cmd);
}

static void on_file_open_ready(GObject *so, GAsyncResult *res, gpointer d) {
    GFile *f = gtk_file_dialog_open_finish(GTK_FILE_DIALOG(so), res, NULL);
    if (f) {
        char *p = g_file_get_path(f);
        g_strlcpy(current_state.wallpaper, p, 512);
        char cmd[512]; 
        snprintf(cmd, 512, "killall swaybg; nohup swaybg -i \"%s\" -m fill > /dev/null 2>&1 &", p);
        system(cmd); 
        apply_state_to_system(&current_state);
        save_preset("_last_state", &current_state);
        g_free(p); g_object_unref(f);
    }
}

static void on_btn_wallpaper_clicked(GtkButton *b, gpointer d) {
    gtk_file_dialog_open(gtk_file_dialog_new(), GTK_WINDOW(d), NULL, on_file_open_ready, NULL);
}

static void on_btn_apply_kitty_clicked(GtkButton *b, gpointer d) {
    rgba_to_hex(gtk_color_dialog_button_get_rgba(ui_color_kitty), current_state.kitty_bg);
    apply_state_to_system(&current_state);
    save_preset("_last_state", &current_state);
}

static void on_btn_apply_hypr_border_clicked(GtkButton *b, gpointer d) {
    rgba_to_hex(gtk_color_dialog_button_get_rgba(ui_color_hypr), current_state.hypr_border);
    apply_state_to_system(&current_state);
    save_preset("_last_state", &current_state);
}

static void on_btn_apply_hypr_border_inactive_clicked(GtkButton *b, gpointer d) {
    rgba_to_hex(gtk_color_dialog_button_get_rgba(ui_color_hypr_inactive), current_state.hypr_border_inactive);
    apply_state_to_system(&current_state);
    save_preset("_last_state", &current_state);
}

static void on_btn_apply_wofi_clicked(GtkButton *b, gpointer d) {
    rgba_to_hex(gtk_color_dialog_button_get_rgba(ui_color_wofi_bg), current_state.wofi_bg);
    rgba_to_hex(gtk_color_dialog_button_get_rgba(ui_color_wofi_text), current_state.wofi_text);
    rgba_to_hex(gtk_color_dialog_button_get_rgba(ui_color_wofi_selected), current_state.wofi_selected);
    rgba_to_hex(gtk_color_dialog_button_get_rgba(ui_color_wofi_border), current_state.wofi_border);
    apply_state_to_system(&current_state);
    save_preset("_last_state", &current_state);
}

static void on_btn_apply_waybar_clicked(GtkButton *b, gpointer d) {
    rgba_to_hex(gtk_color_dialog_button_get_rgba(ui_color_waybar_workspace), current_state.waybar_workspace_color);
    apply_state_to_system(&current_state);
    save_preset("_last_state", &current_state);
}

// ОБРАБОТЧИК КНОПКИ SWAYNC
static void on_btn_apply_swaync_clicked(GtkButton *b, gpointer d) {
    rgba_to_hex(gtk_color_dialog_button_get_rgba(ui_color_swaync_bg), current_state.swaync_bg);
    rgba_to_hex(gtk_color_dialog_button_get_rgba(ui_color_swaync_text), current_state.swaync_text);
    rgba_to_hex(gtk_color_dialog_button_get_rgba(ui_color_swaync_accent), current_state.swaync_accent);
    rgba_to_hex(gtk_color_dialog_button_get_rgba(ui_color_swaync_border), current_state.swaync_border);
    apply_state_to_system(&current_state);
    save_preset("_last_state", &current_state);
}

static gboolean on_window_close(GtkWindow *w, gpointer d) { save_preset("_last_state", &current_state); return FALSE; }

static void on_activate(GtkApplication *app, gpointer d) {
    GtkBuilder *b = gtk_builder_new_from_file("ui/window.ui");
    GtkWindow *win = GTK_WINDOW(gtk_builder_get_object(b, "main_window"));
    gtk_window_set_application(win, app);
    g_signal_connect(win, "close-request", G_CALLBACK(on_window_close), NULL);

    ui_scale_gaps_in = GTK_RANGE(gtk_builder_get_object(b, "scale_gaps_in"));
    ui_color_kitty = GTK_COLOR_DIALOG_BUTTON(gtk_builder_get_object(b, "color_btn_kitty_bg"));
    ui_color_hypr = GTK_COLOR_DIALOG_BUTTON(gtk_builder_get_object(b, "color_btn_hypr_border"));
    ui_color_hypr_inactive = GTK_COLOR_DIALOG_BUTTON(gtk_builder_get_object(b, "color_btn_hypr_border_inactive"));
    ui_entry_preset_name = GTK_ENTRY(gtk_builder_get_object(b, "entry_preset_name"));
    ui_combo_presets = GTK_DROP_DOWN(gtk_builder_get_object(b, "combo_presets"));
    
    ui_color_wofi_bg = GTK_COLOR_DIALOG_BUTTON(gtk_builder_get_object(b, "color_btn_wofi_bg"));
    ui_color_wofi_text = GTK_COLOR_DIALOG_BUTTON(gtk_builder_get_object(b, "color_btn_wofi_text"));
    ui_color_wofi_selected = GTK_COLOR_DIALOG_BUTTON(gtk_builder_get_object(b, "color_btn_wofi_selected"));
    ui_color_wofi_border = GTK_COLOR_DIALOG_BUTTON(gtk_builder_get_object(b, "color_btn_wofi_border"));
    
    ui_color_waybar_workspace = GTK_COLOR_DIALOG_BUTTON(gtk_builder_get_object(b, "color_btn_waybar_workspace"));

    // Инициализация SwayNC UI
    ui_color_swaync_bg = GTK_COLOR_DIALOG_BUTTON(gtk_builder_get_object(b, "color_btn_swaync_bg"));
    ui_color_swaync_text = GTK_COLOR_DIALOG_BUTTON(gtk_builder_get_object(b, "color_btn_swaync_text"));
    ui_color_swaync_accent = GTK_COLOR_DIALOG_BUTTON(gtk_builder_get_object(b, "color_btn_swaync_accent"));
    ui_color_swaync_border = GTK_COLOR_DIALOG_BUTTON(gtk_builder_get_object(b, "color_btn_swaync_border"));

    refresh_presets_combo(ui_combo_presets);

    g_signal_connect(gtk_builder_get_object(b, "btn_save_preset"), "clicked", G_CALLBACK(on_btn_save_preset_clicked), NULL);
    g_signal_connect(gtk_builder_get_object(b, "btn_apply_preset"), "clicked", G_CALLBACK(on_btn_apply_preset_clicked), NULL);
    g_signal_connect(gtk_builder_get_object(b, "btn_delete_preset"), "clicked", G_CALLBACK(on_btn_delete_preset_clicked), NULL);
    g_signal_connect(gtk_builder_get_object(b, "btn_wallpaper"), "clicked", G_CALLBACK(on_btn_wallpaper_clicked), win);
    g_signal_connect(ui_scale_gaps_in, "value-changed", G_CALLBACK(on_gaps_in_changed), NULL);
    g_signal_connect(gtk_builder_get_object(b, "btn_apply_kitty"), "clicked", G_CALLBACK(on_btn_apply_kitty_clicked), NULL);
    g_signal_connect(gtk_builder_get_object(b, "btn_apply_hypr_border"), "clicked", G_CALLBACK(on_btn_apply_hypr_border_clicked), NULL);
    g_signal_connect(gtk_builder_get_object(b, "btn_apply_hypr_border_inactive"), "clicked", G_CALLBACK(on_btn_apply_hypr_border_inactive_clicked), NULL);
    g_signal_connect(gtk_builder_get_object(b, "btn_apply_wofi"), "clicked", G_CALLBACK(on_btn_apply_wofi_clicked), NULL);
    g_signal_connect(gtk_builder_get_object(b, "btn_apply_waybar"), "clicked", G_CALLBACK(on_btn_apply_waybar_clicked), NULL);
    
    // Сигнал SwayNC
    g_signal_connect(gtk_builder_get_object(b, "btn_apply_swaync"), "clicked", G_CALLBACK(on_btn_apply_swaync_clicked), NULL);

    if (load_preset("_last_state", &current_state)) sync_ui_with_state();
    gtk_window_present(win); 
    g_object_unref(b);
}

int main(int c, char **v) {
    GtkApplication *a = gtk_application_new("org.danich.hyprtweaker", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(a, "activate", G_CALLBACK(on_activate), NULL);
    int s = g_application_run(G_APPLICATION(a), c, v); g_object_unref(a); return s;
}
