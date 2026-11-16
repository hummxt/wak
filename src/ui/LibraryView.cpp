#include "ui/LibraryView.h"
#include "ui/Theme.h"

#include "imgui.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <unordered_map>

namespace ge {
namespace {

std::string TitleCase(std::string value) {
    bool cap = true;
    for (char& c : value) {
        if (std::isspace(static_cast<unsigned char>(c)) || c == '-' || c == '_') {
            cap = true;
            continue;
        }
        if (cap) {
            c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
            cap = false;
        }
    }
    return value;
}

std::unordered_map<std::string, float> g_cardHover;
std::unordered_map<std::string, float> g_cardPress;

}

void LibraryView::Draw(
    AnimationManager& animations,
    LibraryViewState& state,
    const std::function<ID3D11ShaderResourceView*(const std::string&)>& getThumbnail,
    const std::function<void(const AnimationPreset&)>& onSelect
) {
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 12.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(18.0f, 16.0f));
    ImGui::BeginChild("LibraryList", ImVec2(0, 0), false, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_AlwaysUseWindowPadding);

    ImGui::Dummy(ImVec2(0.0f, 4.0f));
    ImGui::Indent(4.0f);
    ImGui::TextUnformatted("Wallpapers");
    ImGui::Unindent(4.0f);
    ImGui::Spacing();

    const auto categories = animations.GetCategories();
    for (size_t i = 0; i < categories.size(); ++i) {
        if (i > 0) {
            ImGui::SameLine();
        }
        const auto& category = categories[i];
        const bool selected = state.selectedCategory == category;
        const std::string label = category == "All" ? category : TitleCase(category);
        if (selected) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
        }
        if (ImGui::SmallButton(label.c_str())) {
            state.selectedCategory = category;
        }
        if (selected) {
            ImGui::PopStyleColor();
        }
    }

    char buffer[128]{};
    std::snprintf(buffer, sizeof(buffer), "%s", state.searchQuery.c_str());
    ImGui::SetNextItemWidth(-1.0f);
    if (ImGui::InputTextWithHint("##search", "Search wallpapers", buffer, sizeof(buffer))) {
        state.searchQuery = buffer;
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    const auto filtered = animations.Filter(state.selectedCategory, state.searchQuery);
    const float avail = ImGui::GetContentRegionAvail().x;
    const float gap = 12.0f;
    const float minCard = 170.0f;
    int columns = (std::max)(1, static_cast<int>((avail + gap) / (minCard + gap)));
    if (columns > 3) {
        columns = 3;
    }
    const float cardWidth = (avail - gap * static_cast<float>(columns - 1)) / static_cast<float>(columns);
    const float cardHeight = cardWidth * 0.62f;
    const float rowStep = cardHeight + gap;
    const int total = static_cast<int>(filtered.size());
    const int rowCount = (total + columns - 1) / columns;

    ImGuiIO& io = ImGui::GetIO();
    const float dt = io.DeltaTime;
    if (ImGui::IsWindowHovered() && std::fabs(io.MouseWheel) > 0.0f) {
        m_scrollTarget -= io.MouseWheel * 96.0f;
    }

    const float maxScroll = ImGui::GetScrollMaxY();
    m_scrollTarget = (std::max)(0.0f, (std::min)(m_scrollTarget, maxScroll));
    if (!m_scrollInitialized) {
        m_scrollCurrent = ImGui::GetScrollY();
        m_scrollTarget = m_scrollCurrent;
        m_scrollInitialized = true;
    }
    m_scrollCurrent = SmoothApproach(m_scrollCurrent, m_scrollTarget, 16.0f, dt);
    ImGui::SetScrollY(m_scrollCurrent);

    ImDrawList* draw = ImGui::GetWindowDrawList();
    const ImVec2 gridOrigin = ImGui::GetCursorScreenPos();

    ImGuiListClipper clipper;
    clipper.Begin(rowCount, rowStep);
    while (clipper.Step()) {
        for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row) {
            const float rowY = gridOrigin.y + static_cast<float>(row) * rowStep;

            for (int col = 0; col < columns; ++col) {
                const int index = row * columns + col;
                if (index >= total) {
                    break;
                }

                const float cellX = gridOrigin.x + static_cast<float>(col) * (cardWidth + gap);
                const ImVec2 cell(cellX, rowY);
                const ImVec2 cellEnd(cell.x + cardWidth, cell.y + cardHeight);

                const auto& preset = filtered[static_cast<size_t>(index)];
                ID3D11ShaderResourceView* thumb = getThumbnail ? getThumbnail(preset.id) : nullptr;

                ImGui::PushID(preset.id.c_str());
                float& hover = g_cardHover[preset.id];
                float& press = g_cardPress[preset.id];

                ImGui::SetCursorScreenPos(cell);
                if (ImGui::InvisibleButton("##card", ImVec2(cardWidth, cardHeight))) {
                    state.selectedPresetId = preset.id;
                    onSelect(preset);
                }

                const bool hovered = ImGui::IsItemHovered();
                const bool held = ImGui::IsItemActive();
                hover = SmoothApproach(hover, hovered ? 1.0f : 0.0f, 14.0f, dt);
                press = SmoothApproach(press, held ? 1.0f : 0.0f, 18.0f, dt);

                const float lift = hover * 2.0f - press * 1.0f;
                const ImVec2 p0(cell.x, cell.y - lift);
                const ImVec2 p1(cellEnd.x, cellEnd.y - lift);

                if (hover > 0.01f) {
                    draw->AddRectFilled(
                        ImVec2(p0.x + 6.0f, p1.y + 1.0f),
                        ImVec2(p1.x - 6.0f, p1.y + 5.0f + hover * 2.0f),
                        IM_COL32(0, 0, 0, static_cast<int>(35.0f + hover * 45.0f)),
                        8.0f
                    );
                }

                if (thumb) {
                    draw->AddImage(reinterpret_cast<ImTextureID>(thumb), p0, p1);
                } else {
                    const ImU32 c0 = ImGui::ColorConvertFloat4ToU32(ImVec4(preset.colorA[0], preset.colorA[1], preset.colorA[2], 1.0f));
                    const ImU32 c1 = ImGui::ColorConvertFloat4ToU32(ImVec4(preset.colorD[0], preset.colorD[1], preset.colorD[2], 1.0f));
                    draw->AddRectFilledMultiColor(p0, p1, c0, c1, c1, c0);
                }

                draw->AddRectFilled(ImVec2(p0.x, p1.y - 38.0f), p1, IM_COL32(0, 0, 0, 150));
                draw->AddText(ImVec2(p0.x + 10.0f, p1.y - 20.0f), IM_COL32(255, 255, 255, 245), preset.name.c_str());

                if (state.selectedPresetId == preset.id) {
                    draw->AddRect(p0, p1, IM_COL32(120, 180, 255, 255), 8.0f, 0, 2.0f);
                } else if (hover > 0.05f) {
                    draw->AddRect(p0, p1, IM_COL32(255, 255, 255, static_cast<int>(45.0f + hover * 70.0f)), 8.0f, 0, 1.0f);
                }

                ImGui::PopID();
            }
        }
    }

    // Reserve full gallery height so scrolling/clipping stay correct.
    ImGui::SetCursorScreenPos(gridOrigin);
    ImGui::Dummy(ImVec2(avail, static_cast<float>(rowCount) * rowStep));

    ImGui::EndChild();
    ImGui::PopStyleVar(2);
}

}
