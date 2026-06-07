#!/usr/bin/env bash

GREEN='\e[1;32m'
BLUE='\e[1;34m'
YELLOW='\e[1;33m'
RED='\e[1;31m'
NC='\e[0m' # No Color

echo -e "${BLUE}=== Запуск умной установки Hyprland Tweaker ===${NC}"

echo -e "\n${BLUE}[1/5] Настройка директорий...${NC}"
STATE_DIR="$HOME/.config/hypr-tweaker"
PRESETS_DIR="$STATE_DIR/presets"

if [ ! -d "$PRESETS_DIR" ]; then
    mkdir -p "$PRESETS_DIR"
    echo -e "${GREEN}✓ Создана директория для шаблонов: $PRESETS_DIR${NC}"
else
    echo -e "${YELLOW}! Директория шаблонов уже существует${NC}"
fi

mkdir -p "$HOME/.config/waybar"
mkdir -p "$HOME/.config/swaync"
mkdir -p "$HOME/.config/hypr"
mkdir -p "$HOME/.local/bin"

echo -e "\n${BLUE}[2/5] Привязка к конфигу Hyprland...${NC}"
HYPR_CONF="$HOME/.config/hypr/hyprland.conf"
GEN_HYPR_CONF="$STATE_DIR/generated.conf"
SOURCE_LINE="source = $GEN_HYPR_CONF"

if [ ! -f "$GEN_HYPR_CONF" ]; then
    touch "$GEN_HYPR_CONF"
fi

if [ -f "$HYPR_CONF" ]; then
    if grep -q "hypr-tweaker/generated.conf" "$HYPR_CONF"; then
        echo -e "${YELLOW}! Hyprland уже импортирует конфигурацию твикера${NC}"
    else
        echo -e "" >> "$HYPR_CONF"
        echo "# Автоматическое подключение тем от Hyprland Tweaker" >> "$HYPR_CONF"
        echo "$SOURCE_LINE" >> "$HYPR_CONF"
        echo -e "${GREEN}✓ Строка 'source' успешно добавлена в $HYPR_CONF${NC}"
    fi
else
    echo -e "${RED}✗ Конфиг Hyprland не найден по пути $HYPR_CONF${NC}"
fi

echo -e "\n${BLUE}[3/5] Привязка к стилям Waybar...${NC}"
WAYBAR_STYLE="$HOME/.config/waybar/style.css"
WAYBAR_IMPORT="@import url(\"$STATE_DIR/waybar.css\");"

if [ -f "$WAYBAR_STYLE" ]; then
    if grep -q "hypr-tweaker/waybar.css" "$WAYBAR_STYLE"; then
        echo -e "${YELLOW}! Waybar уже импортирует стили твикера${NC}"
    else
        TEMP_STYLE=$(mktemp)
        echo "$WAYBAR_IMPORT" > "$TEMP_STYLE"
        cat "$WAYBAR_STYLE" >> "$TEMP_STYLE"
        mv "$TEMP_STYLE" "$WAYBAR_STYLE"
        echo -e "${GREEN}✓ Стили твикера успешно импортированы в начало $WAYBAR_STYLE${NC}"
    fi
else
    echo "$WAYBAR_IMPORT" > "$WAYBAR_STYLE"
    echo -e "${GREEN}✓ Создан новый файл $WAYBAR_STYLE с импортом стилей твикера${NC}"
fi

echo -e "\n${BLUE}[4/5] Настройка стилей Control Center (SwayNC)...${NC}"
SWAYNC_STYLE="$HOME/.config/swaync/style.css"

if [ -f "$SWAYNC_STYLE" ]; then
    if [ ! -f "${SWAYNC_STYLE}.bak" ]; then
        cp "$SWAYNC_STYLE" "${SWAYNC_STYLE}.bak"
        echo -e "${GREEN}✓ Создан бэкап оригинального файла стилей: ${SWAYNC_STYLE}.bak${NC}"
    else
        echo -e "${YELLOW}! Бэкап стилей SwayNC уже существует, пропускаем бэкап${NC}"
    fi
fi
echo -e "${GREEN}✓ Утилита готова напрямую управлять файлом $SWAYNC_STYLE${NC}"

echo -e "\n${BLUE}[5/5] Компиляция и установка программы...${NC}"
if make clean && make; then
    cp hypr-tweaker "$HOME/.local/bin/hypr-tweaker"
    echo -e "${GREEN}✓ Успешная компиляция! Бинарник установлен в: $HOME/.local/bin/hypr-tweaker${NC}"
    
    if [[ ":$PATH:" != *":$HOME/.local/bin:"* ]]; then
        echo -e "${YELLOW}! Внимание: Директория $HOME/.local/bin отсутствует в вашем PATH.${NC}"
        echo -e "${YELLOW}  Добавьте строку 'export PATH=\"\$HOME/.local/bin:\$PATH\"' в ваш ~/.zshrc или ~/.bashrc${NC}"
    fi
else
    echo -e "${RED}✗ Ошибка компиляции. Проверьте сообщения выше.${NC}"
    exit 1
fi

echo -e "\n${GREEN}=== Установка успешно завершена! ===${NC}"
echo -e "${BLUE}Теперь вы можете запустить твикер командой: hypr-tweaker (или .local/bin/hypr-tweaker)${NC}"
