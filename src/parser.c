#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>

void save_kitty_bg_color(const GdkRGBA *color) {
    char path[512];
    const char *home = getenv("HOME");
    snprintf(path, sizeof(path), "%s/.config/kitty/colors.conf", home);
    FILE *f = fopen(path, "w");
    if (!f) {
        g_print("Ошибка: не могу открыть файл %s для записи.\n", path);
        return;
    }

    int r = (int)(color->red * 255);
    int g = (int)(color->green * 255);
    int b = (int)(color->blue * 255);
    fprintf(f, "background #%02x%02x%02x\n", r, g, b);
    fclose(f);
    g_print("Успешно записано: background #%02x%02x%02x в %s\n", r, g, b, path);
}
