#include "core/Config.h"
#include "utils/FileUtils.h"

#include <regex>
#include <sstream>

namespace ge {
namespace {

std::string ExtractString(const std::string& json, const std::string& key, const std::string& fallback) {
    const std::regex pattern("\"" + key + "\"\\s*:\\s*\"([^\"]*)\"");
    std::smatch match;
    if (std::regex_search(json, match, pattern) && match.size() > 1) {
        return match[1].str();
    }
    return fallback;
}

bool ExtractBool(const std::string& json, const std::string& key, bool fallback) {
    const std::regex pattern("\"" + key + "\"\\s*:\\s*(true|false)");
    std::smatch match;
    if (std::regex_search(json, match, pattern) && match.size() > 1) {
        return match[1].str() == "true";
    }
    return fallback;
}

int ExtractInt(const std::string& json, const std::string& key, int fallback) {
    const std::regex pattern("\"" + key + "\"\\s*:\\s*(-?[0-9]+)");
    std::smatch match;
    if (std::regex_search(json, match, pattern) && match.size() > 1) {
        return std::stoi(match[1].str());
    }
    return fallback;
}

float ExtractFloat(const std::string& json, const std::string& key, float fallback) {
    const std::regex pattern("\"" + key + "\"\\s*:\\s*(-?[0-9]+(?:\\.[0-9]+)?)");
    std::smatch match;
    if (std::regex_search(json, match, pattern) && match.size() > 1) {
        return std::stof(match[1].str());
    }
    return fallback;
}

bool ExtractColor(const std::string& json, const std::string& key, std::array<float, 4>& out) {
    const std::regex pattern(
        "\"" + key + "\"\\s*:\\s*\\[\\s*(-?[0-9]+(?:\\.[0-9]+)?)\\s*,\\s*(-?[0-9]+(?:\\.[0-9]+)?)\\s*,\\s*(-?[0-9]+(?:\\.[0-9]+)?)\\s*,\\s*(-?[0-9]+(?:\\.[0-9]+)?)\\s*\\]"
    );
    std::smatch match;
    if (std::regex_search(json, match, pattern) && match.size() > 4) {
        out[0] = std::stof(match[1].str());
        out[1] = std::stof(match[2].str());
        out[2] = std::stof(match[3].str());
        out[3] = std::stof(match[4].str());
        return true;
    }
    return false;
}

std::string ColorToJson(const std::array<float, 4>& c) {
    std::ostringstream ss;
    ss << "[" << c[0] << ", " << c[1] << ", " << c[2] << ", " << c[3] << "]";
    return ss.str();
}

}

void Config::ApplyDefaults() {
    m_config = AppConfig{};
}

bool Config::Load(const std::wstring& path) {
    ApplyDefaults();
    const std::string json = ReadTextFile(path);
    if (json.empty()) {
        return false;
    }

    m_config.application.startWithWindows = ExtractBool(json, "startWithWindows", m_config.application.startWithWindows);
    m_config.application.theme = ExtractString(json, "theme", m_config.application.theme);
    m_config.application.wallpaperMode = ExtractBool(json, "wallpaperMode", m_config.application.wallpaperMode);

    m_config.performance.fpsLimit = ExtractInt(json, "fpsLimit", m_config.performance.fpsLimit);
    m_config.performance.renderScale = ExtractFloat(json, "renderScale", m_config.performance.renderScale);
    m_config.performance.pauseWhenFullscreen = ExtractBool(json, "pauseWhenFullscreen", m_config.performance.pauseWhenFullscreen);
    m_config.performance.powerSaving = ExtractBool(json, "powerSaving", m_config.performance.powerSaving);

    m_config.animation.speed = ExtractFloat(json, "speed", m_config.animation.speed);
    m_config.animation.intensity = ExtractFloat(json, "intensity", m_config.animation.intensity);
    m_config.animation.brightness = ExtractFloat(json, "brightness", m_config.animation.brightness);
    m_config.animation.angle = ExtractFloat(json, "angle", m_config.animation.angle);
    ExtractColor(json, "colorA", m_config.animation.colorA);
    ExtractColor(json, "colorB", m_config.animation.colorB);
    ExtractColor(json, "colorC", m_config.animation.colorC);
    ExtractColor(json, "colorD", m_config.animation.colorD);

    m_config.wallpaper.selected = ExtractString(json, "selected", m_config.wallpaper.selected);
    m_config.wallpaper.palette = ExtractString(json, "palette", m_config.wallpaper.palette);
    return true;
}

bool Config::Save(const std::wstring& path) const {
    std::ostringstream ss;
    ss << "{\n";
    ss << "  \"application\": {\n";
    ss << "    \"startWithWindows\": " << (m_config.application.startWithWindows ? "true" : "false") << ",\n";
    ss << "    \"theme\": \"" << m_config.application.theme << "\",\n";
    ss << "    \"wallpaperMode\": " << (m_config.application.wallpaperMode ? "true" : "false") << "\n";
    ss << "  },\n";
    ss << "  \"performance\": {\n";
    ss << "    \"fpsLimit\": " << m_config.performance.fpsLimit << ",\n";
    ss << "    \"renderScale\": " << m_config.performance.renderScale << ",\n";
    ss << "    \"pauseWhenFullscreen\": " << (m_config.performance.pauseWhenFullscreen ? "true" : "false") << ",\n";
    ss << "    \"powerSaving\": " << (m_config.performance.powerSaving ? "true" : "false") << "\n";
    ss << "  },\n";
    ss << "  \"animation\": {\n";
    ss << "    \"speed\": " << m_config.animation.speed << ",\n";
    ss << "    \"intensity\": " << m_config.animation.intensity << ",\n";
    ss << "    \"brightness\": " << m_config.animation.brightness << ",\n";
    ss << "    \"angle\": " << m_config.animation.angle << ",\n";
    ss << "    \"colorA\": " << ColorToJson(m_config.animation.colorA) << ",\n";
    ss << "    \"colorB\": " << ColorToJson(m_config.animation.colorB) << ",\n";
    ss << "    \"colorC\": " << ColorToJson(m_config.animation.colorC) << ",\n";
    ss << "    \"colorD\": " << ColorToJson(m_config.animation.colorD) << "\n";
    ss << "  },\n";
    ss << "  \"wallpaper\": {\n";
    ss << "    \"selected\": \"" << m_config.wallpaper.selected << "\",\n";
    ss << "    \"palette\": \"" << m_config.wallpaper.palette << "\"\n";
    ss << "  }\n";
    ss << "}\n";
    return WriteTextFile(path, ss.str());
}

AppConfig& Config::Get() {
    return m_config;
}

const AppConfig& Config::Get() const {
    return m_config;
}

}
