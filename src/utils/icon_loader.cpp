#include "icon_loader.h"
#include "string_utils.h"
#include <shellapi.h>
#include <shlwapi.h>

namespace orca_light::utils {

IconLoader::IconLoader() {
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    CoCreateInstance(
        CLSID_WICImagingFactory,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&wic_factory_)
    );
}

IconLoader::~IconLoader() {
    discard_resources();
    if (wic_factory_) {
        wic_factory_->Release();
        wic_factory_ = nullptr;
    }
}

void IconLoader::discard_resources() {
    for (auto& pair : cache_) {
        if (pair.second) {
            pair.second->Release();
        }
    }
    cache_.clear();
}

bool IconLoader::initialize(ID2D1RenderTarget*) {
    return wic_factory_ != nullptr;
}

ID2D1Bitmap* IconLoader::create_bitmap_from_hicon(ID2D1RenderTarget* render_target, HICON hicon) {
    if (!render_target || !wic_factory_ || !hicon) {
        return nullptr;
    }

    IWICBitmap* wic_bitmap = nullptr;
    HRESULT hr = wic_factory_->CreateBitmapFromHICON(hicon, &wic_bitmap);
    if (FAILED(hr) || !wic_bitmap) {
        return nullptr;
    }

    IWICFormatConverter* converter = nullptr;
    hr = wic_factory_->CreateFormatConverter(&converter);
    if (FAILED(hr) || !converter) {
        wic_bitmap->Release();
        return nullptr;
    }

    hr = converter->Initialize(
        wic_bitmap,
        GUID_WICPixelFormat32bppPBGRA,
        WICBitmapDitherTypeNone,
        nullptr,
        0.0f,
        WICBitmapPaletteTypeCustom
    );

    ID2D1Bitmap* d2d_bitmap = nullptr;
    if (SUCCEEDED(hr)) {
        render_target->CreateBitmapFromWicBitmap(converter, nullptr, &d2d_bitmap);
    }

    converter->Release();
    wic_bitmap->Release();
    return d2d_bitmap;
}

ID2D1Bitmap* IconLoader::get_icon_for_path(ID2D1RenderTarget* render_target, const std::wstring& path, bool is_directory) {
    if (!render_target) {
        return nullptr;
    }

    // Determine cache key
    std::wstring key;
    if (is_directory) {
        key = L"::folder::";
    } else {
        std::wstring ext = to_lower(get_extension(path));
        if (ext == L".exe" || ext == L".lnk" || ext == L".ico" || ext.empty()) {
            key = to_lower(path);
        } else {
            // Cache by extension for general files to conserve memory and render target handles
            key = ext;
        }
    }

    auto it = cache_.find(key);
    if (it != cache_.end()) {
        return it->second;
    }

    // Extract icon using SHGetFileInfoW
    SHFILEINFOW sfi = {};
    DWORD flags = SHGFI_ICON | SHGFI_SMALLICON;
    if (!is_directory && (key.front() == L'.' && key != L".exe" && key != L".lnk")) {
        flags |= SHGFI_USEFILEATTRIBUTES;
    }
    DWORD attrs = is_directory ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL;

    DWORD_PTR res = SHGetFileInfoW(path.c_str(), attrs, &sfi, sizeof(SHFILEINFOW), flags);
    if (res && sfi.hIcon) {
        ID2D1Bitmap* bmp = create_bitmap_from_hicon(render_target, sfi.hIcon);
        DestroyIcon(sfi.hIcon);
        if (bmp) {
            cache_[key] = bmp;
            return bmp;
        }
    }

    return nullptr;
}

ID2D1Bitmap* IconLoader::get_type_icon(ID2D1RenderTarget* render_target, const std::wstring& type_name) {
    (void)render_target;
    auto it = cache_.find(type_name);
    if (it != cache_.end()) {
        return it->second;
    }
    return nullptr;
}

} // namespace orca_light::utils
