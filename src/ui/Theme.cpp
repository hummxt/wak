#include "ui/Theme.h"

#include "imgui.h"

namespace ge {

void ApplyDarkTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 8.0f;
    style.ChildRounding = 8.0f;
    style.FrameRounding = 6.0f;
    style.GrabRounding = 4.0f;
    style.ScrollbarRounding = 6.0f;
    style.PopupRounding = 6.0f;
    style.WindowPadding = ImVec2(14.0f, 14.0f);
    style.FramePadding = ImVec2(10.0f, 6.0f);
    style.ItemSpacing = ImVec2(10.0f, 8.0f);
    style.WindowBorderSize = 0.0f;
    style.ChildBorderSize = 1.0f;

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.07f, 0.08f, 0.10f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.09f, 0.10f, 0.12f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.09f, 0.11f, 0.98f);
    colors[ImGuiCol_Border] = ImVec4(0.18f, 0.20f, 0.24f, 1.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.12f, 0.14f, 0.17f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.16f, 0.18f, 0.22f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.07f, 0.08f, 0.10f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.08f, 0.10f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.16f, 0.28f, 0.42f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.20f, 0.34f, 0.50f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.24f, 0.40f, 0.58f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.18f, 0.32f, 0.48f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.22f, 0.40f, 0.58f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.26f, 0.46f, 0.66f, 1.00f);
    colors[ImGuiCol_Tab] = ImVec4(0.10f, 0.12f, 0.15f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.20f, 0.34f, 0.50f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.16f, 0.28f, 0.42f, 1.00f);
    colors[ImGuiCol_Text] = ImVec4(0.92f, 0.93f, 0.95f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.52f, 0.56f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.45f, 0.75f, 1.00f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.35f, 0.60f, 0.90f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.45f, 0.70f, 1.00f, 1.00f);
    colors[ImGuiCol_Separator] = ImVec4(0.18f, 0.20f, 0.24f, 1.00f);
}

}
