# Direct2D & DirectWrite Render Pipeline

This document details the graphics pipeline, hardware acceleration architecture, and glyph rasterization implemented in Orca Light.

---

## 1. Graphics Subsystem Overview

Orca Light bypasses legacy Windows GDI and bulky third-party UI engines (such as Chromium or Skia) in favor of the native Windows Direct2D 1.1 graphics API and DirectWrite.

```
+-------------------------------------------------------------+
|                     DIRECT2D PIPELINE                       |
+-------------------------------------------------------------+
                              |
       +----------------------+----------------------+
       |                                             |
       v                                             v
[ID2D1Factory]                              [IDWriteFactory]
CreateHwndRenderTarget                      CreateTextFormat
       |                                             |
       v                                             v
[ID2D1HwndRenderTarget]                     [IDWriteTextLayout]
- Hardware GPU Surface                      - ClearType Font Metrics
- Double-Buffered Present                   - Natural Subpixel Positioning
       |                                             |
       +----------------------+----------------------+
                              |
                              v
                [Direct2D Immediate Draw Pass]
                - Background Fill (Alpha Blended)
                - Selection Geometry & Accent Lines
                - Shell Icon Direct2D Bitmaps
                - DirectWrite Glyph Run Render
                              |
                              v
                [DWM Present / VSync Display]
```

---

## 2. Render Target Lifecycle & Device Management

### 2.1 Target Creation
The HWND render target is instantiated during window creation:
```cpp
D2D1_RENDER_TARGET_PROPERTIES rtProps = D2D1::RenderTargetProperties(
    D2D1_RENDER_TARGET_TYPE_DEFAULT,
    D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
);

D2D1_HWND_RENDER_TARGET_PROPERTIES hwndProps = D2D1::HwndRenderTargetProperties(
    hwnd_,
    D2D1::SizeU(width, height),
    D2D1_PRESENT_OPTIONS_IMMEDIATELY
);

d2d_factory_->CreateHwndRenderTarget(rtProps, hwndProps, &render_target_);
```

### 2.2 Device Loss (`D2DERR_RECREATE_TARGET`)
When display configurations change (such as connecting a laptop to an external monitor or graphics driver recovery events), Direct2D may invalidate GPU resources. Orca Light intercepts `D2DERR_RECREATE_TARGET` at `EndDraw()`, releases all cached brush pointers, and cleanly recreates the HWND render target.

---

## 3. DirectWrite Typography

### 3.1 Font Metrics & Anti-Aliasing
DirectWrite formats text with subpixel anti-aliasing:
- **Font Family**: Segoe UI Variable (falling back to Segoe UI on Windows 10).
- **Text Formats**:
  - Primary Query: 22.0pt, Light Weight (`DWRITE_FONT_WEIGHT_LIGHT`).
  - Result Title: 14.0pt, Semi-Bold (`DWRITE_FONT_WEIGHT_SEMI_BOLD`).
  - Result Path / Subtitle: 11.5pt, Regular (`DWRITE_FONT_WEIGHT_REGULAR`).
- **Rendering Parameters**:
  - Anti-alias mode: `D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE`.
  - Measuring mode: `DWRITE_MEASURING_MODE_NATURAL`.

---

## 4. Shell Icon Caching & Bitmap Conversion

1. Icons are retrieved as Win32 `HICON` handles via `SHGetFileInfoW` or `IExtractIconW`.
2. The `HICON` is converted to an uncompressed 32-bit RGBA bitmap via the Windows Imaging Component (WIC):
   ```cpp
   wic_imaging_factory_->CreateBitmapFromHICON(hIcon, &wic_bitmap);
   render_target_->CreateBitmapFromWicBitmap(wic_bitmap, &d2d_bitmap);
   ```
3. The resulting `ID2D1Bitmap` is cached in a thread-safe LRU hash map keyed by executable file path, ensuring zero disk reads during subsequent search queries.
