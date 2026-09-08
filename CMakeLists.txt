cmake_minimum_required(VERSION 3.20)
project(Orca-Light VERSION 1.0.0 LANGUAGES CXX RC)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Source files
set(SOURCES
    src/main.cpp
    src/app.cpp
    src/calculator/calc_engine.cpp
    src/calculator/currency_converter.cpp
    src/clipboard/clipboard_history.cpp
    src/config/config_manager.cpp
    src/indexer/app_indexer.cpp
    src/indexer/file_indexer.cpp
    src/search/fuzzy_matcher.cpp
    src/search/search_engine.cpp
    src/system/system_commands.cpp
    src/ui/renderer.cpp
    src/ui/window.cpp
    src/utils/icon_loader.cpp
    src/utils/logger.cpp
    src/utils/shell_utils.cpp
    src/utils/string_utils.cpp
)

set(HEADERS
    src/app.h
    src/calculator/calc_engine.h
    src/calculator/currency_converter.h
    src/clipboard/clipboard_history.h
    src/config/config_manager.h
    src/indexer/app_indexer.h
    src/indexer/file_indexer.h
    src/search/fuzzy_matcher.h
    src/search/search_engine.h
    src/system/system_commands.h
    src/ui/renderer.h
    src/ui/theme.h
    src/ui/window.h
    src/utils/icon_loader.h
    src/utils/logger.h
    src/utils/shell_utils.h
    src/utils/string_utils.h
    res/resource.h
)

set(RESOURCES
    res/resource.rc
)

add_executable(Orca-Light WIN32 ${SOURCES} ${HEADERS} ${RESOURCES})

target_include_directories(Orca-Light PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/res
)

target_compile_definitions(Orca-Light PRIVATE
    UNICODE
    _UNICODE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    _CRT_SECURE_NO_WARNINGS
)

if(MSVC)
    target_compile_options(Orca-Light PRIVATE
        /W4
        /permissive-
        /EHsc
        $<$<CONFIG:Release>:/O2 /Oi /Ot /GL /Gy>
    )
    target_link_options(Orca-Light PRIVATE
        $<$<CONFIG:Release>:/LTCG /OPT:REF /OPT:ICF>
    )
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(Orca-Light PRIVATE
        -Wall
        -Wextra
        $<$<CONFIG:Release>:-O3 -flto>
    )
    target_link_options(Orca-Light PRIVATE
        $<$<CONFIG:Release>:-flto -s>
    )
endif()

# Windows System and DirectX Libraries
target_link_libraries(Orca-Light PRIVATE
    d2d1
    dwrite
    windowscodecs
    dxgi
    shlwapi
    shell32
    ole32
    oleaut32
    advapi32
    user32
    gdi32
    propsys
    uuid
    dwmapi
    wininet
)
