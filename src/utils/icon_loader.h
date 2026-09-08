#pragma once

#include <windows.h>
#include <d2d1.h>
#include <wincodec.h>
#include <string>
#include <unordered_map>
#include <memory>

namespace orca_light::utils {

class IconLoader {
public:
    IconLoader();
    ~IconLoader();

    bool initialize(ID2D1RenderTarget* render_target);
    void discard_resources();

    // Get or load cached icon for a file or application path
    ID2D1Bitmap* get_icon_for_path(ID2D1RenderTarget* render_target, const std::wstring& path, bool is_directory = false);

    // Get default icon for specific item type
    ID2D1Bitmap* get_type_icon(ID2D1RenderTarget* render_target, const std::wstring& type_name);

private:
    ID2D1Bitmap* create_bitmap_from_hicon(ID2D1RenderTarget* render_target, HICON hicon);

    IWICImagingFactory* wic_factory_ = nullptr;
    std::unordered_map<std::wstring, ID2D1Bitmap*> cache_;
};

} // namespace orca_light::utils
