#pragma once

#include <d2d1.h>
#include <cstdint>

namespace orca_light::ui {

struct Theme {
    // Brutalist dark palette
    static constexpr uint32_t COLOR_BG                 = 0x0a0a0a;
    static constexpr uint32_t COLOR_BORDER             = 0x242424;
    static constexpr uint32_t COLOR_SEARCH_BG          = 0x111111;
    static constexpr uint32_t COLOR_DIVIDER            = 0x1e1e1e;
    static constexpr uint32_t COLOR_ROW_HOVER          = 0x141414;
    static constexpr uint32_t COLOR_ROW_SELECTED       = 0x1c1c1c;
    static constexpr uint32_t COLOR_ROW_ACCENT         = 0xffffff;
    
    // Typography colors
    static constexpr uint32_t COLOR_TEXT_PRIMARY       = 0xffffff;
    static constexpr uint32_t COLOR_TEXT_SECONDARY     = 0x8a8a8a;
    static constexpr uint32_t COLOR_TEXT_TERTIARY      = 0x555555;
    static constexpr uint32_t COLOR_TEXT_MUTED         = 0x404040;
    static constexpr uint32_t COLOR_CURSOR             = 0xffffff;

    // Badges & Tag colors
    static constexpr uint32_t COLOR_BADGE_BG           = 0x181818;
    static constexpr uint32_t COLOR_BADGE_BORDER       = 0x333333;
    static constexpr uint32_t COLOR_BADGE_TEXT         = 0x9e9e9e;

    // Footer & Action bar
    static constexpr uint32_t COLOR_FOOTER_BG          = 0x0d0d0d;
    static constexpr uint32_t COLOR_FOOTER_TEXT        = 0x666666;
    static constexpr uint32_t COLOR_KEY_BG             = 0x1a1a1a;
    static constexpr uint32_t COLOR_KEY_BORDER         = 0x333333;
    static constexpr uint32_t COLOR_KEY_TEXT           = 0xaaaaaa;

    // Base dimensions at 96 DPI (1.0 scale)
    static constexpr float BASE_WINDOW_WIDTH          = 680.0f;
    static constexpr float BASE_SEARCH_HEIGHT         = 54.0f;
    static constexpr float BASE_ITEM_HEIGHT           = 48.0f;
    static constexpr float BASE_FOOTER_HEIGHT         = 30.0f;
    static constexpr float BASE_ICON_SIZE             = 24.0f;
    static constexpr float BASE_BORDER_WIDTH          = 1.0f;
    static constexpr float BASE_ACCENT_WIDTH          = 2.0f;
    static constexpr float BASE_PADDING_X             = 16.0f;

    // Font Sizes (points)
    static constexpr float FONT_SIZE_SEARCH           = 19.0f;
    static constexpr float FONT_SIZE_TITLE            = 14.0f;
    static constexpr float FONT_SIZE_SUBTITLE         = 11.5f;
    static constexpr float FONT_SIZE_BADGE            = 10.0f;
    static constexpr float FONT_SIZE_FOOTER           = 11.0f;

    static inline D2D1_COLOR_F to_d2d_color(uint32_t hex, float alpha = 1.0f) {
        float r = ((hex >> 16) & 0xFF) / 255.0f;
        float g = ((hex >> 8) & 0xFF) / 255.0f;
        float b = (hex & 0xFF) / 255.0f;
        return D2D1::ColorF(r, g, b, alpha);
    }
};

} // namespace orca_light::ui
