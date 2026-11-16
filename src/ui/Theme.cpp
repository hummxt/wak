#include "ui/Theme.h"

#include "imgui.h"

#include <algorithm>
#include <cmath>

namespace ge {
namespace {

ImVec4 Hex(int rgb) {
    const float r = static_cast<float>((rgb >> 16) & 0xFF) / 255.0f;
    const float g = static_cast<float>((rgb >> 8) & 0xFF) / 255.0f;
    const float b = static_cast<float>(rgb & 0xFF) / 255.0f;
    return ImVec4(r, g, b, 1.0f);
}

}

float Clamp01(float value) {
    return (std::max)(0.0f, (std::min)(1.0f, value));
}

float SmoothApproach(float current, float target, float speed, float dt) {
    const float t = 1.0f - std::exp(-speed * dt);
    return current + (target - current) * Clamp01(t);
}

void ApplyDarkTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 10.0f;
    style.ChildRounding = 10.0f;
    style.FrameRounding = 8.0f;
    style.GrabRounding = 6.0f;
    style.ScrollbarRounding = 8.0f;
    style.PopupRounding = 8.0f;
    style.WindowPadding = ImVec2(14.0f, 14.0f);
    style.FramePadding = ImVec2(12.0f, 8.0f);
    style.ItemSpacing = ImVec2(10.0f, 10.0f);
    style.ItemInnerSpacing = ImVec2(8.0f, 6.0f);
    style.WindowBorderSize = 0.0f;
    style.ChildBorderSize = 0.0f;
    style.FrameBorderSize = 0.0f;
    style.ScrollbarSize = 10.0f;
    style.GrabMinSize = 12.0f;

    const ImVec4 bg = Hex(0x0a0f1d);
    const ImVec4 panel = Hex(0x070b16);
    const ImVec4 deep = Hex(0x04070e);
    const ImVec4 frame = ImVec4(0.09f, 0.12f, 0.20f, 1.0f);
    const ImVec4 frameHover = ImVec4(0.13f, 0.18f, 0.28f, 1.0f);
    const ImVec4 accent = ImVec4(0.32f, 0.52f, 0.88f, 1.0f);
    const ImVec4 accentHover = ImVec4(0.40f, 0.60f, 0.96f, 1.0f);
    const ImVec4 accentActive = ImVec4(0.24f, 0.42f, 0.78f, 1.0f);

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = bg;
    colors[ImGuiCol_ChildBg] = panel;
    colors[ImGuiCol_PopupBg] = ImVec4(deep.x, deep.y, deep.z, 0.98f);
    colors[ImGuiCol_Border] = ImVec4(0.14f, 0.18f, 0.28f, 1.0f);
    colors[ImGuiCol_FrameBg] = frame;
    colors[ImGuiCol_FrameBgHovered] = frameHover;
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.16f, 0.22f, 0.34f, 1.0f);
    colors[ImGuiCol_TitleBg] = deep;
    colors[ImGuiCol_TitleBgActive] = deep;
    colors[ImGuiCol_TitleBgCollapsed] = deep;
    colors[ImGuiCol_MenuBarBg] = deep;
    colors[ImGuiCol_Header] = ImVec4(0.14f, 0.20f, 0.34f, 1.0f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.18f, 0.28f, 0.44f, 1.0f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.22f, 0.34f, 0.52f, 1.0f);
    colors[ImGuiCol_Button] = ImVec4(0.14f, 0.22f, 0.38f, 1.0f);
    colors[ImGuiCol_ButtonHovered] = accent;
    colors[ImGuiCol_ButtonActive] = accentActive;
    colors[ImGuiCol_Tab] = deep;
    colors[ImGuiCol_TabHovered] = ImVec4(0.18f, 0.28f, 0.44f, 1.0f);
    colors[ImGuiCol_TabActive] = ImVec4(0.14f, 0.22f, 0.38f, 1.0f);
    colors[ImGuiCol_Text] = ImVec4(0.93f, 0.95f, 0.98f, 1.0f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.48f, 0.52f, 0.62f, 1.0f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.60f, 0.78f, 1.00f, 1.0f);
    colors[ImGuiCol_SliderGrab] = accent;
    colors[ImGuiCol_SliderGrabActive] = accentHover;
    colors[ImGuiCol_Separator] = ImVec4(0.14f, 0.18f, 0.28f, 1.0f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(deep.x, deep.y, deep.z, 0.35f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.18f, 0.24f, 0.36f, 1.0f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.26f, 0.36f, 0.54f, 1.0f);
    colors[ImGuiCol_ScrollbarGrabActive] = accent;
}

}
