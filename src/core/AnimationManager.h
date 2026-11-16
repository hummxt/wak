#pragma once

#include <string>
#include <vector>
#include <array>

namespace ge {

struct AnimationPreset {
    std::string id;
    std::string name;
    std::string category;
    std::string shader;
    std::string description;
    std::string thumbnail;
    float defaultSpeed = 0.5f;
    float defaultIntensity = 0.7f;
    float defaultAngle = 90.0f;
    std::array<float, 4> colorA{ 0.45f, 0.20f, 0.85f, 1.0f };
    std::array<float, 4> colorB{ 0.15f, 0.35f, 0.95f, 1.0f };
    std::array<float, 4> colorC{ 0.90f, 0.35f, 0.55f, 1.0f };
    std::array<float, 4> colorD{ 0.10f, 0.75f, 0.80f, 1.0f };
};

class AnimationManager {
public:
    bool LoadPresets(const std::wstring& presetsDirectory);
    const std::vector<AnimationPreset>& GetPresets() const;
    const AnimationPreset* FindById(const std::string& id) const;
    std::vector<AnimationPreset> Filter(const std::string& category, const std::string& query) const;
    std::vector<std::string> GetCategories() const;

    void AddFavorite(const std::string& id);
    void RemoveFavorite(const std::string& id);
    bool IsFavorite(const std::string& id) const;
    const std::vector<std::string>& GetFavorites() const;
    void PushRecent(const std::string& id);
    const std::vector<std::string>& GetRecent() const;

private:
    bool LoadPresetFile(const std::wstring& path);

    std::vector<AnimationPreset> m_presets;
    std::vector<std::string> m_favorites;
    std::vector<std::string> m_recent;
};

}
