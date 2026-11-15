cmake_minimum_required(VERSION 3.20)
project(Wakbg VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/third_party/imgui)

set(SOURCES
    src/main.cpp
    src/core/Application.cpp
    src/core/Config.cpp
    src/core/AnimationManager.cpp
    src/renderer/GraphicsDevice.cpp
    src/renderer/Renderer.cpp
    src/renderer/ShaderManager.cpp
    src/desktop/DesktopHost.cpp
    src/desktop/MonitorManager.cpp
    src/ui/MainWindow.cpp
    src/ui/LibraryView.cpp
    src/ui/PreviewView.cpp
    src/ui/SettingsView.cpp
    src/ui/Theme.cpp
    src/utils/Logger.cpp
    src/utils/FileUtils.cpp
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_win32.cpp
    ${IMGUI_DIR}/backends/imgui_impl_dx11.cpp
)

add_executable(Wakbg WIN32 ${SOURCES})

target_include_directories(Wakbg PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

target_compile_definitions(Wakbg PRIVATE
    UNICODE
    _UNICODE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
)

target_link_libraries(Wakbg PRIVATE
    d3d11
    dxgi
    d3dcompiler
    dcomp
    dwmapi
    shell32
    ole32
    user32
    gdi32
    advapi32
)

set(SHADER_SRC_DIR ${CMAKE_SOURCE_DIR}/shaders)
set(SHADER_DST_DIR ${CMAKE_BINARY_DIR}/shaders)
set(CONFIG_SRC_DIR ${CMAKE_SOURCE_DIR}/config)
set(CONFIG_DST_DIR ${CMAKE_BINARY_DIR}/config)
set(LIBRARY_SRC_DIR ${CMAKE_SOURCE_DIR}/library)
set(LIBRARY_DST_DIR ${CMAKE_BINARY_DIR}/library)
set(ASSETS_SRC_DIR ${CMAKE_SOURCE_DIR}/assets)
set(ASSETS_DST_DIR ${CMAKE_BINARY_DIR}/assets)

add_custom_command(TARGET Wakbg POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${SHADER_SRC_DIR} $<TARGET_FILE_DIR:Wakbg>/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CONFIG_SRC_DIR} $<TARGET_FILE_DIR:Wakbg>/config
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${LIBRARY_SRC_DIR} $<TARGET_FILE_DIR:Wakbg>/library
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${ASSETS_SRC_DIR} $<TARGET_FILE_DIR:Wakbg>/assets
)

if(MSVC)
    target_compile_options(Wakbg PRIVATE /W4 /permissive-)
endif()
