#include "core/AnimationManager.h"
#include "utils/FileUtils.h"

#include <algorithm>
#include <regex>

namespace ge {
namespace {

std::string ExtractString(const std::string& json, const std::string& key, const std::string& fallback = {}) {
    const std::regex pattern("\"" + key + "\"\\s*:\\s*\"([^\"]*)\"");
    std::smatch match;
    if (std::regex_search(json, match, pattern) && match.size() > 1) {
        return match[1].str();
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

bool ContainsIgnoreCase(const std::string& haystack, const std::string& needle) {
    auto lower = [](std::string s) {
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return s;
    };
    return lower(haystack).find(lower(needle)) != std::string::npos;
}

}

bool AnimationManager::LoadPresets(const std::wstring& presetsDirectory) {
    m_presets.clear();
    const auto files = ListFiles(presetsDirectory, L".json");
    for (const auto& file : files) {
        LoadPresetFile(file);
    }
    std::sort(m_presets.begin(), m_presets.end(), [](const AnimationPreset& a, const AnimationPreset& b) {
        return a.name < b.name;
    });
    return !m_presets.empty();
}

bool AnimationManager::LoadPresetFile(const std::wstring& path) {
    const std::string json = ReadTextFile(path);
    if (json.empty()) {
        return false;
    }

    AnimationPreset preset;
    preset.id = ExtractString(json, "id");
    preset.name = ExtractString(json, "name");
    preset.category = ExtractString(json, "category");
    preset.shader = ExtractString(json, "shader");
    preset.description = ExtractString(json, "description");
    preset.thumbnail = ExtractString(json, "thumbnail");
    preset.defaultSpeed = ExtractFloat(json, "speed", 0.5f);
    preset.defaultIntensity = ExtractFloat(json, "intensity", 0.7f);
    preset.defaultAngle = ExtractFloat(json, "angle", 90.0f);
    ExtractColor(json, "colorA", preset.colorA);
    ExtractColor(json, "colorB", preset.colorB);
    ExtractColor(json, "colorC", preset.colorC);
    ExtractColor(json, "colorD", preset.colorD);

    if (preset.id.empty() || preset.shader.empty()) {
        return false;
    }

    m_presets.push_back(std::move(preset));
    return true;
}

const std::vector<AnimationPreset>& AnimationManager::GetPresets() const {
    return m_presets;
}

const AnimationPreset* AnimationManager::FindById(const std::string& id) const {
    for (const auto& preset : m_presets) {
        if (preset.id == id) {
            return &preset;
        }
    }
    return nullptr;
}

std::vector<AnimationPreset> AnimationManager::Filter(const std::string& category, const std::string& query) const {
    std::vector<AnimationPreset> result;
    for (const auto& preset : m_presets) {
        if (!category.empty() && category != "All" && preset.category != category) {
            continue;
        }
        if (!query.empty() && !ContainsIgnoreCase(preset.name, query) && !ContainsIgnoreCase(preset.description, query)) {
            continue;
        }
        result.push_back(preset);
    }
    return result;
}

std::vector<std::string> AnimationManager::GetCategories() const {
    std::vector<std::string> categories{ "All" };
    for (const auto& preset : m_presets) {
        if (std::find(categories.begin(), categories.end(), preset.category) == categories.end()) {
            categories.push_back(preset.category);
        }
    }
    return categories;
}

void AnimationManager::AddFavorite(const std::string& id) {
    if (!IsFavorite(id)) {
        m_favorites.push_back(id);
    }
}

void AnimationManager::RemoveFavorite(const std::string& id) {
    m_favorites.erase(std::remove(m_favorites.begin(), m_favorites.end(), id), m_favorites.end());
}

bool AnimationManager::IsFavorite(const std::string& id) const {
    return std::find(m_favorites.begin(), m_favorites.end(), id) != m_favorites.end();
}

const std::vector<std::string>& AnimationManager::GetFavorites() const {
    return m_favorites;
}

void AnimationManager::PushRecent(const std::string& id) {
    m_recent.erase(std::remove(m_recent.begin(), m_recent.end(), id), m_recent.end());
    m_recent.insert(m_recent.begin(), id);
    if (m_recent.size() > 12) {
        m_recent.pop_back();
    }
}

const std::vector<std::string>& AnimationManager::GetRecent() const {
    return m_recent;
}

}
