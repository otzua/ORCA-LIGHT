#include <cmath>
#include "renderer.h"
#include <algorithm>
#include <cwchar>

namespace orca_light::ui {

Renderer::Renderer() {
}

Renderer::~Renderer() {
    discard_device_resources();

    if (font_search_) font_search_->Release();
    if (font_placeholder_) font_placeholder_->Release();
    if (font_title_) font_title_->Release();
    if (font_subtitle_) font_subtitle_->Release();
    if (font_badge_) font_badge_->Release();
    if (font_footer_) font_footer_->Release();

    if (dwrite_factory_) dwrite_factory_->Release();
    if (d2d_factory_) d2d_factory_->Release();
}

void Renderer::discard_device_resources() {
    icon_loader_.discard_resources();

    auto release_brush = [](ID2D1SolidColorBrush*& b) {
        if (b) {
            b->Release();
            b = nullptr;
        }
    };

    release_brush(brush_bg_);
    release_brush(brush_border_);
    release_brush(brush_search_bg_);
    release_brush(brush_divider_);
    release_brush(brush_row_hover_);
    release_brush(brush_row_selected_);
    release_brush(brush_row_accent_);
    release_brush(brush_text_primary_);
    release_brush(brush_text_secondary_);
    release_brush(brush_text_tertiary_);
    release_brush(brush_text_muted_);
    release_brush(brush_cursor_);
    release_brush(brush_badge_bg_);
    release_brush(brush_badge_border_);
    release_brush(brush_badge_text_);
    release_brush(brush_footer_bg_);
    release_brush(brush_footer_text_);
    release_brush(brush_key_bg_);
    release_brush(brush_key_border_);
    release_brush(brush_key_text_);

    if (render_target_) {
        render_target_->Release();
        render_target_ = nullptr;
    }
}

HRESULT Renderer::create_device_independent_resources() {
    HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &d2d_factory_);
    if (FAILED(hr)) return hr;

    hr = DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown**>(&dwrite_factory_)
    );
    if (FAILED(hr)) return hr;

    const wchar_t* font_family = L"Segoe UI Variable Display";

    auto create_format = [this, font_family](float size, DWRITE_FONT_WEIGHT weight, IDWriteTextFormat** fmt) -> HRESULT {
        HRESULT res = dwrite_factory_->CreateTextFormat(
            font_family,
            nullptr,
            weight,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            size * dpi_scale_,
            L"en-us",
            fmt
        );
        if (FAILED(res)) {
            // Fallback to classic Segoe UI
            res = dwrite_factory_->CreateTextFormat(
                L"Segoe UI",
                nullptr,
                weight,
                DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL,
                size * dpi_scale_,
                L"en-us",
                fmt
            );
        }
        if (SUCCEEDED(res)) {
            (*fmt)->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
            DWRITE_TRIMMING trimming = { DWRITE_TRIMMING_GRANULARITY_CHARACTER, 0, 0 };
            (*fmt)->SetTrimming(&trimming, nullptr);
        }
        return res;
    };

    create_format(Theme::FONT_SIZE_SEARCH, DWRITE_FONT_WEIGHT_NORMAL, &font_search_);
    create_format(Theme::FONT_SIZE_SEARCH, DWRITE_FONT_WEIGHT_NORMAL, &font_placeholder_);
    create_format(Theme::FONT_SIZE_TITLE, DWRITE_FONT_WEIGHT_SEMI_BOLD, &font_title_);
    create_format(Theme::FONT_SIZE_SUBTITLE, DWRITE_FONT_WEIGHT_NORMAL, &font_subtitle_);
    create_format(Theme::FONT_SIZE_BADGE, DWRITE_FONT_WEIGHT_BOLD, &font_badge_);
    if (font_badge_) {
        font_badge_->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
        font_badge_->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    }
    create_format(Theme::FONT_SIZE_FOOTER, DWRITE_FONT_WEIGHT_NORMAL, &font_footer_);

    return S_OK;
}

HRESULT Renderer::create_device_resources(HWND hwnd) {
    if (render_target_) {
        return S_OK;
    }

    RECT rc = {};
    GetClientRect(hwnd, &rc);
    UINT width = (rc.right > rc.left) ? (rc.right - rc.left) : static_cast<UINT>(Theme::BASE_WINDOW_WIDTH * dpi_scale_);
    UINT height = (rc.bottom > rc.top) ? (rc.bottom - rc.top) : static_cast<UINT>((Theme::BASE_SEARCH_HEIGHT + Theme::BASE_FOOTER_HEIGHT) * dpi_scale_);

    D2D1_SIZE_U size = D2D1::SizeU(width, height);

    D2D1_RENDER_TARGET_PROPERTIES rtProps = D2D1::RenderTargetProperties(
        D2D1_RENDER_TARGET_TYPE_DEFAULT,
        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
    );
    D2D1_HWND_RENDER_TARGET_PROPERTIES hwndProps = D2D1::HwndRenderTargetProperties(hwnd, size);

    HRESULT hr = d2d_factory_->CreateHwndRenderTarget(rtProps, hwndProps, &render_target_);
    if (FAILED(hr)) return hr;

    render_target_->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
    render_target_->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE);

    auto create_brush = [this](uint32_t color, ID2D1SolidColorBrush** brush, float alpha = 1.0f) {
        return render_target_->CreateSolidColorBrush(Theme::to_d2d_color(color, alpha), brush);
    };

    create_brush(Theme::COLOR_BG, &brush_bg_);
    create_brush(Theme::COLOR_BORDER, &brush_border_);
    create_brush(Theme::COLOR_SEARCH_BG, &brush_search_bg_);
    create_brush(Theme::COLOR_DIVIDER, &brush_divider_);
    create_brush(Theme::COLOR_ROW_HOVER, &brush_row_hover_, 0.5f);
    create_brush(Theme::COLOR_ROW_SELECTED, &brush_row_selected_, 0.7f);
    create_brush(Theme::COLOR_ROW_ACCENT, &brush_row_accent_);
    create_brush(Theme::COLOR_TEXT_PRIMARY, &brush_text_primary_);
    create_brush(Theme::COLOR_TEXT_SECONDARY, &brush_text_secondary_);
    create_brush(Theme::COLOR_TEXT_TERTIARY, &brush_text_tertiary_);
    create_brush(Theme::COLOR_TEXT_MUTED, &brush_text_muted_);
    create_brush(Theme::COLOR_CURSOR, &brush_cursor_);
    create_brush(Theme::COLOR_BADGE_BG, &brush_badge_bg_);
    create_brush(Theme::COLOR_BADGE_BORDER, &brush_badge_border_);
    create_brush(Theme::COLOR_BADGE_TEXT, &brush_badge_text_);
    create_brush(Theme::COLOR_FOOTER_BG, &brush_footer_bg_);
    create_brush(Theme::COLOR_FOOTER_TEXT, &brush_footer_text_);
    create_brush(Theme::COLOR_KEY_BG, &brush_key_bg_, 0.5f);
    create_brush(Theme::COLOR_KEY_BORDER, &brush_key_border_, 0.5f);
    create_brush(Theme::COLOR_KEY_TEXT, &brush_key_text_);

    icon_loader_.initialize(render_target_);

    return S_OK;
}

bool Renderer::initialize(HWND hwnd) {
    if (FAILED(create_device_independent_resources())) {
        return false;
    }
    return SUCCEEDED(create_device_resources(hwnd));
}

void Renderer::resize(UINT width, UINT height) {
    current_width_ = static_cast<float>(width);
    current_height_ = static_cast<float>(height);
    if (render_target_) {
        render_target_->Resize(D2D1::SizeU(width, height));
    }
}

void Renderer::update_dpi(float dpi_scale) {
    dpi_scale_ = dpi_scale;
    // Recreate text formats with new DPI scale
    if (font_search_) font_search_->Release();
    if (font_placeholder_) font_placeholder_->Release();
    if (font_title_) font_title_->Release();
    if (font_subtitle_) font_subtitle_->Release();
    if (font_badge_) font_badge_->Release();
    if (font_footer_) font_footer_->Release();

    font_search_ = nullptr;
    font_placeholder_ = nullptr;
    font_title_ = nullptr;
    font_subtitle_ = nullptr;
    font_badge_ = nullptr;
    font_footer_ = nullptr;

    const wchar_t* font_family = L"Segoe UI Variable Display";

    auto create_format = [this, font_family](float size, DWRITE_FONT_WEIGHT weight, IDWriteTextFormat** fmt) {
        HRESULT res = dwrite_factory_->CreateTextFormat(
            font_family,
            nullptr,
            weight,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            size * dpi_scale_,
            L"en-us",
            fmt
        );
        if (FAILED(res)) {
            dwrite_factory_->CreateTextFormat(
                L"Segoe UI",
                nullptr,
                weight,
                DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL,
                size * dpi_scale_,
                L"en-us",
                fmt
            );
        }
        if (*fmt) {
            (*fmt)->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
            DWRITE_TRIMMING trimming = { DWRITE_TRIMMING_GRANULARITY_CHARACTER, 0, 0 };
            (*fmt)->SetTrimming(&trimming, nullptr);
        }
    };

    create_format(Theme::FONT_SIZE_SEARCH, DWRITE_FONT_WEIGHT_NORMAL, &font_search_);
    create_format(Theme::FONT_SIZE_SEARCH, DWRITE_FONT_WEIGHT_NORMAL, &font_placeholder_);
    create_format(Theme::FONT_SIZE_TITLE, DWRITE_FONT_WEIGHT_SEMI_BOLD, &font_title_);
    create_format(Theme::FONT_SIZE_SUBTITLE, DWRITE_FONT_WEIGHT_NORMAL, &font_subtitle_);
    create_format(Theme::FONT_SIZE_BADGE, DWRITE_FONT_WEIGHT_BOLD, &font_badge_);
    if (font_badge_) {
        font_badge_->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
        font_badge_->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    }
    create_format(Theme::FONT_SIZE_FOOTER, DWRITE_FONT_WEIGHT_NORMAL, &font_footer_);
}

float Renderer::calculate_window_height(size_t result_count) const {
    float search_h = Theme::BASE_SEARCH_HEIGHT * dpi_scale_;
    float item_h = Theme::BASE_ITEM_HEIGHT * dpi_scale_;
    float footer_h = Theme::BASE_FOOTER_HEIGHT * dpi_scale_;
    return search_h + (static_cast<float>(result_count) * item_h) + footer_h;
}

void Renderer::draw_search_glyph(float x, float y, float size) {
    float radius = size * 0.35f;
    D2D1_ELLIPSE circle = D2D1::Ellipse(D2D1::Point2F(x + radius, y + radius), radius, radius);
    render_target_->DrawEllipse(circle, brush_text_secondary_, 1.5f * dpi_scale_);

    float handle_start_x = x + radius + radius * 0.7071f;
    float handle_start_y = y + radius + radius * 0.7071f;
    float handle_end_x = x + size;
    float handle_end_y = y + size;

    render_target_->DrawLine(
        D2D1::Point2F(handle_start_x, handle_start_y),
        D2D1::Point2F(handle_end_x, handle_end_y),
        brush_text_secondary_,
        1.5f * dpi_scale_
    );
}

void Renderer::draw_calculator_glyph(float x, float y, float size) {
    float pad = size * 0.2f;
    render_target_->DrawRectangle(
        D2D1::RectF(x + pad, y + pad, x + size - pad, y + size - pad),
        brush_text_secondary_,
        1.2f * dpi_scale_
    );
    // Draw '=' inside
    float mid_y = y + size * 0.5f;
    float offset = size * 0.1f;
    render_target_->DrawLine(
        D2D1::Point2F(x + pad + 3.0f, mid_y - offset),
        D2D1::Point2F(x + size - pad - 3.0f, mid_y - offset),
        brush_text_primary_,
        1.2f * dpi_scale_
    );
    render_target_->DrawLine(
        D2D1::Point2F(x + pad + 3.0f, mid_y + offset),
        D2D1::Point2F(x + size - pad - 3.0f, mid_y + offset),
        brush_text_primary_,
        1.2f * dpi_scale_
    );
}

void Renderer::draw_command_glyph(float x, float y, float size) {
    // Sharp prompt '>'
    float pad = size * 0.2f;
    render_target_->DrawLine(
        D2D1::Point2F(x + pad, y + pad),
        D2D1::Point2F(x + size * 0.6f, y + size * 0.5f),
        brush_text_secondary_,
        1.5f * dpi_scale_
    );
    render_target_->DrawLine(
        D2D1::Point2F(x + size * 0.6f, y + size * 0.5f),
        D2D1::Point2F(x + pad, y + size - pad),
        brush_text_secondary_,
        1.5f * dpi_scale_
    );
}

void Renderer::draw_clipboard_glyph(float x, float y, float size) {
    float pad_x = size * 0.25f;
    float pad_y = size * 0.15f;
    // Clipboard board outline
    render_target_->DrawRectangle(
        D2D1::RectF(x + pad_x, y + pad_y + 4.0f, x + size - pad_x, y + size - pad_y),
        brush_text_secondary_,
        1.2f * dpi_scale_
    );
    // Clip on top
    render_target_->DrawRectangle(
        D2D1::RectF(x + size * 0.38f, y + pad_y, x + size * 0.62f, y + pad_y + 5.0f),
        brush_text_secondary_,
        1.2f * dpi_scale_
    );
}

void Renderer::draw_folder_glyph(float x, float y, float size) {
    float pad = size * 0.15f;
    // Minimalist folder outline
    D2D1_RECT_F body = D2D1::RectF(x + pad, y + pad + 5.0f, x + size - pad, y + size - pad);
    render_target_->DrawRectangle(body, brush_text_secondary_, 1.2f * dpi_scale_);
    render_target_->DrawLine(
        D2D1::Point2F(x + pad, y + pad + 5.0f),
        D2D1::Point2F(x + pad + 8.0f, y + pad),
        brush_text_secondary_,
        1.2f * dpi_scale_
    );
    render_target_->DrawLine(
        D2D1::Point2F(x + pad + 8.0f, y + pad),
        D2D1::Point2F(x + pad + 16.0f, y + pad),
        brush_text_secondary_,
        1.2f * dpi_scale_
    );
}

void Renderer::draw_file_glyph(float x, float y, float size) {
    float pad_x = size * 0.25f;
    float pad_y = size * 0.15f;
    render_target_->DrawRectangle(
        D2D1::RectF(x + pad_x, y + pad_y, x + size - pad_x, y + size - pad_y),
        brush_text_secondary_,
        1.2f * dpi_scale_
    );
}

void Renderer::draw_search_bar(float width, const std::wstring& query, size_t cursor_pos, bool show_cursor) {
    float search_h = Theme::BASE_SEARCH_HEIGHT * dpi_scale_;
    float padding_x = Theme::BASE_PADDING_X * dpi_scale_;
    float icon_size = 18.0f * dpi_scale_;

    // Search bar background
    render_target_->FillRectangle(D2D1::RectF(0, 0, width, search_h), brush_search_bg_);

    // Search vector glyph
    float icon_y = (search_h - icon_size) * 0.5f;
    draw_search_glyph(padding_x, icon_y, icon_size);

    float text_x = padding_x + icon_size + (12.0f * dpi_scale_);
    float text_w = width - text_x - padding_x;
    float text_y = (search_h - (Theme::FONT_SIZE_SEARCH * dpi_scale_ * 1.35f)) * 0.5f;

    D2D1_RECT_F text_rect = D2D1::RectF(text_x, text_y, text_x + text_w, text_y + 40.0f * dpi_scale_);

    if (query.empty()) {
        const wchar_t* placeholder = L"Search apps, files, math, or type > for commands...";
        render_target_->DrawText(
            placeholder,
            static_cast<UINT32>(wcslen(placeholder)),
            font_placeholder_,
            text_rect,
            brush_text_muted_
        );
    } else {
        render_target_->DrawText(
            query.c_str(),
            static_cast<UINT32>(query.size()),
            font_search_,
            text_rect,
            brush_text_primary_
        );
    }

    // Draw text cursor
    if (show_cursor) {
        float cursor_x = text_x;
        if (!query.empty() && cursor_pos > 0) {
            IDWriteTextLayout* layout = nullptr;
            dwrite_factory_->CreateTextLayout(
                query.c_str(),
                static_cast<UINT32>(std::min(cursor_pos, query.size())),
                font_search_,
                text_w,
                50.0f,
                &layout
            );
            if (layout) {
                DWRITE_TEXT_METRICS metrics;
                layout->GetMetrics(&metrics);
                cursor_x = text_x + metrics.widthIncludingTrailingWhitespace;
                layout->Release();
            }
        }

        float cursor_h = 22.0f * dpi_scale_;
        float cursor_y = (search_h - cursor_h) * 0.5f;
        render_target_->DrawLine(
            D2D1::Point2F(cursor_x + 1.0f, cursor_y),
            D2D1::Point2F(cursor_x + 1.0f, cursor_y + cursor_h),
            brush_cursor_,
            1.5f * dpi_scale_
        );
    }

    // Divider line below search bar
    render_target_->DrawLine(
        D2D1::Point2F(0, search_h),
        D2D1::Point2F(width, search_h),
        brush_divider_,
        1.0f * dpi_scale_
    );
}

void Renderer::draw_results_list(float width,
                                const std::vector<search::SearchResult>& results,
                                int selected_index,
                                int hovered_index) {
    float start_y = Theme::BASE_SEARCH_HEIGHT * dpi_scale_;
    float item_h = Theme::BASE_ITEM_HEIGHT * dpi_scale_;
    float padding_x = Theme::BASE_PADDING_X * dpi_scale_;
    float icon_size = Theme::BASE_ICON_SIZE * dpi_scale_;

    for (size_t i = 0; i < results.size(); ++i) {
        const auto& item = results[i];
        float row_y = start_y + (static_cast<float>(i) * item_h);
        D2D1_RECT_F row_rect = D2D1::RectF(0, row_y, width, row_y + item_h);

        bool is_selected = (static_cast<int>(i) == selected_index);
        bool is_hovered = (static_cast<int>(i) == hovered_index);

        if (is_selected) {
            render_target_->FillRectangle(row_rect, brush_row_selected_);
            // 2px Brutalist accent bar on the left
            render_target_->FillRectangle(
                D2D1::RectF(0, row_y, 2.5f * dpi_scale_, row_y + item_h),
                brush_row_accent_
            );
        } else if (is_hovered) {
            render_target_->FillRectangle(row_rect, brush_row_hover_);
        }

        // Icon Rendering
        float icon_x = padding_x;
        float icon_y = row_y + (item_h - icon_size) * 0.5f;

        ID2D1Bitmap* bmp = nullptr;
        if (!item.path.empty()) {
            bmp = icon_loader_.get_icon_for_path(render_target_, item.path, item.type == search::ResultType::Folder);
        }

        if (bmp) {
            D2D1_RECT_F dest_rect = D2D1::RectF(icon_x, icon_y, icon_x + icon_size, icon_y + icon_size);
            render_target_->DrawBitmap(bmp, dest_rect);
        } else {
            // Draw crisp vector glyph based on result type
            switch (item.type) {
                case search::ResultType::Calculator:
                    draw_calculator_glyph(icon_x, icon_y, icon_size);
                    break;
                case search::ResultType::Command:
                    draw_command_glyph(icon_x, icon_y, icon_size);
                    break;
                case search::ResultType::Clipboard:
                    draw_clipboard_glyph(icon_x, icon_y, icon_size);
                    break;
                case search::ResultType::Folder:
                    draw_folder_glyph(icon_x, icon_y, icon_size);
                    break;
                default:
                    draw_file_glyph(icon_x, icon_y, icon_size);
                    break;
            }
        }

        // Badge pill on right side
        float badge_w = 46.0f * dpi_scale_;
        float badge_h = 18.0f * dpi_scale_;
        float badge_x = width - padding_x - badge_w;
        float badge_y = row_y + (item_h - badge_h) * 0.5f;
        D2D1_RECT_F badge_rect = D2D1::RectF(badge_x, badge_y, badge_x + badge_w, badge_y + badge_h);

        render_target_->FillRectangle(badge_rect, brush_badge_bg_);
        render_target_->DrawRectangle(badge_rect, brush_badge_border_, 1.0f * dpi_scale_);

        D2D1_RECT_F badge_text_rect = D2D1::RectF(badge_x, badge_y + 1.5f * dpi_scale_, badge_x + badge_w, badge_y + badge_h);
        render_target_->DrawText(
            item.badge.c_str(),
            static_cast<UINT32>(item.badge.size()),
            font_badge_,
            badge_text_rect,
            brush_badge_text_
        );

        // Title and Subtitle text
        float content_x = icon_x + icon_size + (12.0f * dpi_scale_);
        float max_content_w = badge_x - content_x - (10.0f * dpi_scale_);

        if (!item.subtitle.empty()) {
            float title_y = row_y + 6.0f * dpi_scale_;
            D2D1_RECT_F title_rect = D2D1::RectF(content_x, title_y, content_x + max_content_w, title_y + 20.0f * dpi_scale_);
            render_target_->DrawText(
                item.title.c_str(),
                static_cast<UINT32>(item.title.size()),
                font_title_,
                title_rect,
                is_selected ? brush_text_primary_ : brush_text_primary_
            );

            float sub_y = row_y + 24.0f * dpi_scale_;
            D2D1_RECT_F sub_rect = D2D1::RectF(content_x, sub_y, content_x + max_content_w, sub_y + 18.0f * dpi_scale_);
            render_target_->DrawText(
                item.subtitle.c_str(),
                static_cast<UINT32>(item.subtitle.size()),
                font_subtitle_,
                sub_rect,
                brush_text_secondary_
            );
        } else {
            // Single line centered title
            float title_y = row_y + (item_h - (Theme::FONT_SIZE_TITLE * dpi_scale_ * 1.2f)) * 0.5f;
            D2D1_RECT_F title_rect = D2D1::RectF(content_x, title_y, content_x + max_content_w, title_y + 24.0f * dpi_scale_);
            render_target_->DrawText(
                item.title.c_str(),
                static_cast<UINT32>(item.title.size()),
                font_title_,
                title_rect,
                brush_text_primary_
            );
        }
    }
}

void Renderer::draw_footer(float width, float y) {
    float footer_h = Theme::BASE_FOOTER_HEIGHT * dpi_scale_;
    float padding_x = Theme::BASE_PADDING_X * dpi_scale_;

    D2D1_RECT_F footer_rect = D2D1::RectF(0, y, width, y + footer_h);
    render_target_->FillRectangle(footer_rect, brush_footer_bg_);

    // Divider line above footer
    render_target_->DrawLine(
        D2D1::Point2F(0, y),
        D2D1::Point2F(width, y),
        brush_divider_,
        1.0f * dpi_scale_
    );

    // Render key hints
    struct KeyHint {
        const wchar_t* key;
        const wchar_t* action;
    };

    const KeyHint hints[] = {
        { L"Enter", L"Open" },
        { L"Ctrl+Enter", L"Open Folder" },
        { L"Ctrl+Shift+C", L"Copy Path" },
        { L"Esc", L"Dismiss" }
    };

    float current_x = padding_x;
    float key_y = y + (footer_h - 16.0f * dpi_scale_) * 0.5f;
    float key_h = 16.0f * dpi_scale_;

    for (const auto& hint : hints) {
        float key_text_len = static_cast<float>(wcslen(hint.key));
        float key_w = (key_text_len * 6.5f + 10.0f) * dpi_scale_;

        D2D1_RECT_F krect = D2D1::RectF(current_x, key_y, current_x + key_w, key_y + key_h);
        render_target_->FillRectangle(krect, brush_key_bg_);
        render_target_->DrawRectangle(krect, brush_key_border_, 1.0f * dpi_scale_);

        D2D1_RECT_F ktext_rect = D2D1::RectF(current_x, key_y + 1.0f * dpi_scale_, current_x + key_w, key_y + key_h);
        render_target_->DrawText(
            hint.key,
            static_cast<UINT32>(wcslen(hint.key)),
            font_footer_,
            ktext_rect,
            brush_key_text_
        );

        current_x += key_w + (6.0f * dpi_scale_);

        float act_text_len = static_cast<float>(wcslen(hint.action));
        float act_w = (act_text_len * 6.5f + 4.0f) * dpi_scale_;
        D2D1_RECT_F atext_rect = D2D1::RectF(current_x, key_y + 1.0f * dpi_scale_, current_x + act_w, key_y + key_h);
        render_target_->DrawText(
            hint.action,
            static_cast<UINT32>(wcslen(hint.action)),
            font_footer_,
            atext_rect,
            brush_footer_text_
        );

        current_x += act_w + (14.0f * dpi_scale_);
    }
}

void Renderer::render(HWND hwnd,
                      const std::wstring& query,
                      size_t cursor_pos,
                      bool show_cursor,
                      const std::vector<search::SearchResult>& results,
                      int selected_index,
                      int hovered_index,
                      float anim_progress) {
    if (FAILED(create_device_resources(hwnd))) {
        return;
    }

    render_target_->BeginDraw();
    render_target_->Clear(Theme::to_d2d_color(Theme::COLOR_BG, 0.35f));

    float ease = 1.0f - std::pow(1.0f - anim_progress, 4.0f);
    float offset_y = (1.0f - ease) * (15.0f * dpi_scale_);
    render_target_->SetTransform(D2D1::Matrix3x2F::Translation(0.0f, offset_y));

    draw_search_bar(current_width_, query, cursor_pos, show_cursor);
    draw_results_list(current_width_, results, selected_index, hovered_index);

    float footer_y = Theme::BASE_SEARCH_HEIGHT * dpi_scale_ + (static_cast<float>(results.size()) * Theme::BASE_ITEM_HEIGHT * dpi_scale_);
    draw_footer(current_width_, footer_y);

    // 1px Border around entire window
    render_target_->DrawRectangle(
        D2D1::RectF(0.5f, 0.5f, current_width_ - 0.5f, current_height_ - 0.5f),
        brush_border_,
        1.0f * dpi_scale_
    );

    render_target_->SetTransform(D2D1::Matrix3x2F::Identity());
    HRESULT hr = render_target_->EndDraw();
    if (hr == D2DERR_RECREATE_TARGET) {
        discard_device_resources();
    }
}

} // namespace orca_light::ui
