#include "../include/parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static void ensure_presets_dir() {
    char path[512];
    snprintf(path, sizeof(path), "%s/.config/hypr-tweaker", getenv("HOME"));
    mkdir(path, 0755);
    snprintf(path, sizeof(path), "%s/.config/hypr-tweaker/presets", getenv("HOME"));
    mkdir(path, 0755);
}

void save_preset(const char *preset_name, const AppState *state) {
    ensure_presets_dir();
    char path[512];
    snprintf(path, sizeof(path), "%s/.config/hypr-tweaker/presets/%s.conf", getenv("HOME"), preset_name);
    FILE *f = fopen(path, "w");
    if (!f) return;
    fprintf(f, "gaps_in=%d\n", state->gaps_in);
    fprintf(f, "hypr_border=%s\n", state->hypr_border);
    fprintf(f, "hypr_border_inactive=%s\n", state->hypr_border_inactive);
    fprintf(f, "kitty_bg=%s\n", state->kitty_bg);
    fprintf(f, "wallpaper=%s\n", state->wallpaper);
    fprintf(f, "wofi_bg=%s\n", state->wofi_bg);
    fprintf(f, "wofi_text=%s\n", state->wofi_text);
    fprintf(f, "wofi_selected=%s\n", state->wofi_selected);
    fprintf(f, "wofi_border=%s\n", state->wofi_border);
    fprintf(f, "waybar_workspace_color=%s\n", state->waybar_workspace_color);
    fprintf(f, "swaync_bg=%s\n", state->swaync_bg);
    fprintf(f, "swaync_text=%s\n", state->swaync_text);
    fprintf(f, "swaync_accent=%s\n", state->swaync_accent);
    fprintf(f, "swaync_border=%s\n", state->swaync_border);
    fclose(f);
}

gboolean load_preset(const char *preset_name, AppState *out_state) {
    char path[512];
    snprintf(path, sizeof(path), "%s/.config/hypr-tweaker/presets/%s.conf", getenv("HOME"), preset_name);
    FILE *f = fopen(path, "r");
    if (!f) return FALSE;
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        char key[256], val[512];
        if (sscanf(line, "%255[^=]=%511s", key, val) == 2) {
            val[strcspn(val, "\n")] = 0;
            if (strcmp(key, "gaps_in") == 0) out_state->gaps_in = atoi(val);
            else if (strcmp(key, "hypr_border") == 0) strcpy(out_state->hypr_border, val);
            else if (strcmp(key, "hypr_border_inactive") == 0) strcpy(out_state->hypr_border_inactive, val);
            else if (strcmp(key, "kitty_bg") == 0) strcpy(out_state->kitty_bg, val);
            else if (strcmp(key, "wallpaper") == 0) strcpy(out_state->wallpaper, val);
            else if (strcmp(key, "wofi_bg") == 0) strcpy(out_state->wofi_bg, val);
            else if (strcmp(key, "wofi_text") == 0) strcpy(out_state->wofi_text, val);
            else if (strcmp(key, "wofi_selected") == 0) strcpy(out_state->wofi_selected, val);
            else if (strcmp(key, "wofi_border") == 0) strcpy(out_state->wofi_border, val);
            else if (strcmp(key, "waybar_workspace_color") == 0) strcpy(out_state->waybar_workspace_color, val);
            else if (strcmp(key, "swaync_bg") == 0) strcpy(out_state->swaync_bg, val);
            else if (strcmp(key, "swaync_text") == 0) strcpy(out_state->swaync_text, val);
            else if (strcmp(key, "swaync_accent") == 0) strcpy(out_state->swaync_accent, val);
            else if (strcmp(key, "swaync_border") == 0) strcpy(out_state->swaync_border, val);
        }
    }
    fclose(f);
    return TRUE;
}

void apply_state_to_system(const AppState *state) {
    ensure_presets_dir();
    char cmd[1024];

    // Hyprland
    snprintf(cmd, sizeof(cmd), "hyprctl keyword general:gaps_in %d", state->gaps_in);
    system(cmd);
    snprintf(cmd, sizeof(cmd), "hyprctl keyword general:col.active_border \"rgb(%s)\"", state->hypr_border + 1);
    system(cmd);
    snprintf(cmd, sizeof(cmd), "hyprctl keyword general:col.inactive_border \"rgb(%s)\"", state->hypr_border_inactive + 1);
    system(cmd);

    // Kitty
    char k_path[512];
    snprintf(k_path, sizeof(k_path), "%s/.config/kitty/colors.conf", getenv("HOME"));
    FILE *fk = fopen(k_path, "w");
    if (fk) { fprintf(fk, "background %s\n", state->kitty_bg); fclose(fk); system("killall -SIGUSR1 kitty"); }
    
    // Swaybg
    if (strlen(state->wallpaper) > 0) {
        snprintf(cmd, sizeof(cmd), "killall swaybg; nohup swaybg -i \"%s\" -m fill > /dev/null 2>&1 &", state->wallpaper);
        system(cmd);
    }

    // Hyprland Conf
    char gen_path[512];
    snprintf(gen_path, sizeof(gen_path), "%s/.config/hypr-tweaker/generated.conf", getenv("HOME"));
    FILE *fg = fopen(gen_path, "w");
    if (fg) {
        fprintf(fg, "general {\n");
        fprintf(fg, "    gaps_in = %d\n", state->gaps_in);
        fprintf(fg, "    col.active_border = rgb(%s)\n", state->hypr_border + 1);
        fprintf(fg, "    col.inactive_border = rgb(%s)\n", state->hypr_border_inactive + 1);
        fprintf(fg, "}\n\n");
        if (strlen(state->wallpaper) > 0) {
            fprintf(fg, "exec-once = swaybg -i \"%s\" -m fill\n", state->wallpaper);
        }
        fclose(fg);
    }

    // Wofi
    char wofi_dir[512];
    snprintf(wofi_dir, sizeof(wofi_dir), "%s/.config/wofi", getenv("HOME"));
    mkdir(wofi_dir, 0755); 
    char wofi_path[512];
    snprintf(wofi_path, sizeof(wofi_path), "%s/.config/wofi/style.css", getenv("HOME"));
    FILE *fw = fopen(wofi_path, "w");
    if (fw) {
        int r, g, b; sscanf(state->wofi_bg, "#%02x%02x%02x", &r, &g, &b);
        fprintf(fw, "window {\n    margin: 5px;\n    border: 3px solid %s;\n    border-radius: 12px;\n    background-color: rgba(%d, %d, %d, 0.8);\n}\n", state->wofi_border, r, g, b);
        fprintf(fw, "#outer-box {\n    padding: 12px;\n    background-color: transparent;\n}\n");
        fprintf(fw, "#inner-box, #scroll {\n    background-color: transparent;\n}\n");
        fprintf(fw, "#input {\n    margin-bottom: 12px;\n    padding: 6px;\n    border: 2px solid %s;\n    border-radius: 8px;\n    color: %s;\n    background-color: rgba(%d, %d, %d, 0.5);\n}\n", state->wofi_selected, state->wofi_text, r, g, b);
        fprintf(fw, "#text {\n    margin: 4px;\n    color: %s;\n}\n", state->wofi_text);
        fprintf(fw, "#entry {\n    padding: 4px;\n}\n");
        fprintf(fw, "#entry:selected {\n    background-color: %s;\n    border-radius: 8px;\n}\n", state->wofi_selected);
        fprintf(fw, "#entry:selected * {\n    color: %s;\n}\n", state->wofi_bg); 
        fclose(fw);
    }
    system("killall wofi > /dev/null 2>&1");

    // Waybar
    char waybar_path[512];
    snprintf(waybar_path, sizeof(waybar_path), "%s/.config/hypr-tweaker/waybar.css", getenv("HOME"));
    FILE *fwb = fopen(waybar_path, "w");
    if (fwb) {
        fprintf(fwb, "#workspaces button {\n    color: %s;\n}\n", state->waybar_workspace_color);
        fprintf(fwb, "#workspaces button label {\n    color: %s;\n}\n", state->waybar_workspace_color);
        fclose(fwb);
    }
    system("killall -SIGUSR2 waybar > /dev/null 2>&1");

    // --- ПРЯМА ГЕНЕРАЦІЯ ГОЛОВНОГО ФАЙЛУ SWAYNC ---
    char swaync_dir[512];
    snprintf(swaync_dir, sizeof(swaync_dir), "%s/.config/swaync", getenv("HOME"));
    mkdir(swaync_dir, 0755); 
    
    char swaync_path[512];
    snprintf(swaync_path, sizeof(swaync_path), "%s/.config/swaync/style.css", getenv("HOME"));
    FILE *fsnc = fopen(swaync_path, "w");
    if (fsnc) {
        int r, g, b;
        sscanf(state->swaync_bg, "#%02x%02x%02x", &r, &g, &b);
        
        fprintf(fsnc, "@define-color bg-color %s;\n", state->swaync_bg);
        fprintf(fsnc, "@define-color text-color %s;\n", state->swaync_text);
        fprintf(fsnc, "@define-color accent-color %s;\n", state->swaync_accent);
        fprintf(fsnc, "@define-color border-color %s;\n\n", state->swaync_border);
        
        fprintf(fsnc, ".control-center {\n    margin: 10px;\n    margin-top: 50px;\n    border-radius: 12px;\n    border: 2px solid @border-color;\n    background-color: rgba(%d, %d, %d, 0.85);\n    min-width: 400px;\n    max-height: 500px;\n}\n\n", r, g, b);
        
        fprintf(fsnc, ".control-center *, .widget-title {\n    color: @text-color;\n}\n\n");
        
        fprintf(fsnc, ".widget-volume, .widget-backlight {\n    background-color: rgba(255, 255, 255, 0.05);\n    padding: 10px;\n    margin: 5px;\n    border-radius: 8px;\n}\n\n");
        
        fprintf(fsnc, "scale trough {\n    background-color: rgba(255, 255, 255, 0.1);\n    border-radius: 4px;\n    min-height: 8px;\n}\n");
        fprintf(fsnc, "scale highlight {\n    background-color: @accent-color;\n    border-radius: 4px;\n}\n");
        
        // Рядок розбитий для безпечного копіювання
        fprintf(fsnc, "scale slider {\n    background-color: @text-color;\n");
        fprintf(fsnc, "    border-radius: 50%%;\n    min-width: 16px;\n    min-height: 16px;\n}\n\n");
        
        fprintf(fsnc, ".widget-buttons grid > button {\n    background-color: rgba(255, 255, 255, 0.05);\n    border-radius: 8px;\n    border: none;\n    box-shadow: none;\n    margin: 4px;\n}\n");
        fprintf(fsnc, ".widget-buttons grid > button:hover {\n    background-color: rgba(255, 255, 255, 0.15);\n}\n");
        fprintf(fsnc, ".widget-buttons grid > button:checked {\n    background-color: @accent-color;\n    color: @bg-color;\n}\n");
        fprintf(fsnc, ".widget-buttons grid > button:checked label {\n    color: @bg-color;\n}\n");
        
        fclose(fsnc);
    }
    system("swaync-client -rs > /dev/null 2>&1");
}

void delete_preset(const char *preset_name) {
    char path[512];
    snprintf(path, sizeof(path), "%s/.config/hypr-tweaker/presets/%s.conf", getenv("HOME"), preset_name);
    remove(path);
}
