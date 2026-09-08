#pragma once

#include <windows.h>
#include <d2d1.h>
#include <dwrite.h>
#include <string>
#include <vector>
#include "../search/search_engine.h"
#include "../utils/icon_loader.h"
#include "theme.h"

namespace orca_light::ui {

class Renderer {
public:
    Renderer();
    ~Renderer();

    bool initialize(HWND hwnd);
    void discard_device_resources();
    void resize(UINT width, UINT height);
    void update_dpi(float dpi_scale);

    void render(HWND hwnd,
                const std::wstring& query,
                size_t cursor_pos,
                bool show_cursor,
                const std::vector<search::SearchResult>& results,
                int selected_index,
                int hovered_index,
                float anim_progress = 1.0f);

    float calculate_window_height(size_t result_count) const;
    float get_dpi_scale() const { return dpi_scale_; }

private:
    HRESULT create_device_independent_resources();
    HRESULT create_device_resources(HWND hwnd);

    void draw_search_bar(float width, const std::wstring& query, size_t cursor_pos, bool show_cursor);
    void draw_results_list(float width,
                           const std::vector<search::SearchResult>& results,
                           int selected_index,
                           int hovered_index);
    void draw_footer(float width, float y);

    // Vector iconography helpers
    void draw_search_glyph(float x, float y, float size);
    void draw_calculator_glyph(float x, float y, float size);
    void draw_command_glyph(float x, float y, float size);
    void draw_clipboard_glyph(float x, float y, float size);
    void draw_folder_glyph(float x, float y, float size);
    void draw_file_glyph(float x, float y, float size);

    ID2D1Factory* d2d_factory_ = nullptr;
    ID2D1HwndRenderTarget* render_target_ = nullptr;
    IDWriteFactory* dwrite_factory_ = nullptr;

    // Brushes
    ID2D1SolidColorBrush* brush_bg_ = nullptr;
    ID2D1SolidColorBrush* brush_border_ = nullptr;
    ID2D1SolidColorBrush* brush_search_bg_ = nullptr;
    ID2D1SolidColorBrush* brush_divider_ = nullptr;
    ID2D1SolidColorBrush* brush_row_hover_ = nullptr;
    ID2D1SolidColorBrush* brush_row_selected_ = nullptr;
    ID2D1SolidColorBrush* brush_row_accent_ = nullptr;
    ID2D1SolidColorBrush* brush_text_primary_ = nullptr;
    ID2D1SolidColorBrush* brush_text_secondary_ = nullptr;
    ID2D1SolidColorBrush* brush_text_tertiary_ = nullptr;
    ID2D1SolidColorBrush* brush_text_muted_ = nullptr;
    ID2D1SolidColorBrush* brush_cursor_ = nullptr;
    ID2D1SolidColorBrush* brush_badge_bg_ = nullptr;
    ID2D1SolidColorBrush* brush_badge_border_ = nullptr;
    ID2D1SolidColorBrush* brush_badge_text_ = nullptr;
    ID2D1SolidColorBrush* brush_footer_bg_ = nullptr;
    ID2D1SolidColorBrush* brush_footer_text_ = nullptr;
    ID2D1SolidColorBrush* brush_key_bg_ = nullptr;
    ID2D1SolidColorBrush* brush_key_border_ = nullptr;
    ID2D1SolidColorBrush* brush_key_text_ = nullptr;

    // Text Formats
    IDWriteTextFormat* font_search_ = nullptr;
    IDWriteTextFormat* font_placeholder_ = nullptr;
    IDWriteTextFormat* font_title_ = nullptr;
    IDWriteTextFormat* font_subtitle_ = nullptr;
    IDWriteTextFormat* font_badge_ = nullptr;
    IDWriteTextFormat* font_footer_ = nullptr;

    utils::IconLoader icon_loader_;
    float dpi_scale_ = 1.0f;
    float current_width_ = Theme::BASE_WINDOW_WIDTH;
    float current_height_ = 400.0f;
};

} // namespace orca_light::ui
