#ifndef PARSER_H
#define PARSER_H

#include <gtk/gtk.h>

typedef struct {
    int gaps_in;
    char hypr_border[8];
    char hypr_border_inactive[8];
    char kitty_bg[8];
    char wallpaper[512];
    char wofi_bg[8];
    char wofi_text[8];
    char wofi_selected[8];
    char wofi_border[8];
    char waybar_workspace_color[8];
    // НОВЫЕ ПАРАМЕТРЫ SWAYNC
    char swaync_bg[8];
    char swaync_text[8];
    char swaync_accent[8];
    char swaync_border[8];
} AppState;

void save_preset(const char *preset_name, const AppState *state);
gboolean load_preset(const char *preset_name, AppState *out_state);
void apply_state_to_system(const AppState *state);
void delete_preset(const char *preset_name);

#endif
