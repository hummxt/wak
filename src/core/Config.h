#pragma once

#include <array>
#include <string>

namespace ge {

struct ApplicationSettings {
    bool startWithWindows = false;
    std::string theme = "dark";
    bool wallpaperMode = false;
};

struct PerformanceSettings {
    int fpsLimit = 30;
    float renderScale = 1.0f;
    bool pauseWhenFullscreen = true;
    bool powerSaving = false;
};

struct AnimationSettings {
    float speed = 0.75f;
    float intensity = 0.85f;
    float brightness = 1.0f;
    std::array<float, 4> colorA{ 0.45f, 0.20f, 0.85f, 1.0f };
    std::array<float, 4> colorB{ 0.15f, 0.35f, 0.95f, 1.0f };
    std::array<float, 4> colorC{ 0.90f, 0.35f, 0.55f, 1.0f };
    std::array<float, 4> colorD{ 0.10f, 0.75f, 0.80f, 1.0f };
};

struct WallpaperSettings {
    std::string selected = "mesh-flow";
    std::string palette = "custom";
};

struct AppConfig {
    ApplicationSettings application;
    PerformanceSettings performance;
    AnimationSettings animation;
    WallpaperSettings wallpaper;
};

class Config {
public:
    bool Load(const std::wstring& path);
    bool Save(const std::wstring& path) const;
    void ApplyDefaults();

    AppConfig& Get();
    const AppConfig& Get() const;

private:
    AppConfig m_config;
};

}
