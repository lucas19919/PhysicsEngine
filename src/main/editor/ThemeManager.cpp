#include "main/editor/ThemeManager.h"

#include "external/imgui/imgui.h"
#include "raylib.h"

#include "main/editor/EditorState.h"

namespace Editor {

void ThemeManager::ApplyTheme(EditorTheme theme) {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // Reset base style
    ImGui::StyleColorsDark();
    style.WindowRounding = 12.0f; // Increased for modern feel
    style.FrameRounding = 8.0f;
    style.GrabRounding = 8.0f;
    style.TabRounding = 8.0f;
    style.PopupRounding = 12.0f;
    style.ScrollbarRounding = 12.0f;
    style.WindowPadding = ImVec2(15, 15);
    style.FramePadding = ImVec2(10, 8);
    style.ItemSpacing = ImVec2(12, 10);
    style.FrameBorderSize = 0.0f; // Thinner/None for Modern
    style.WindowBorderSize = 0.0f;

    switch (theme) {
        case EditorTheme::ModernDark:
        {
            // VSCode "Dark+" inspired palette
            ImVec4 vscodeBg      = ImVec4(0.12f, 0.12f, 0.12f, 1.00f); // #1E1E1E
            ImVec4 vscodeSidebar = ImVec4(0.15f, 0.15f, 0.15f, 1.00f); // #252526
            ImVec4 vscodeAccent  = ImVec4(0.00f, 0.48f, 0.80f, 1.00f); // #007ACC
            ImVec4 text          = ImVec4(0.80f, 0.80f, 0.80f, 1.00f); // #CCCCCC
            ImVec4 textBright    = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);

            colors[ImGuiCol_Text]                   = text;
            colors[ImGuiCol_WindowBg]               = vscodeBg;
            colors[ImGuiCol_ChildBg]                = vscodeSidebar;
            colors[ImGuiCol_PopupBg]                = vscodeBg;
            colors[ImGuiCol_Border]                 = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
            colors[ImGuiCol_FrameBg]                = ImVec4(0.25f, 0.25f, 0.26f, 1.00f); // #3E3E42
            colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.30f, 0.30f, 0.31f, 1.00f);
            colors[ImGuiCol_FrameBgActive]          = ImVec4(0.35f, 0.35f, 0.36f, 1.00f);
            colors[ImGuiCol_TitleBg]                = vscodeBg;
            colors[ImGuiCol_TitleBgActive]          = vscodeBg;
            colors[ImGuiCol_CheckMark]              = vscodeAccent;
            colors[ImGuiCol_SliderGrab]             = vscodeAccent;
            colors[ImGuiCol_Button]                 = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
            colors[ImGuiCol_ButtonHovered]          = vscodeAccent;
            colors[ImGuiCol_Header]                 = ImVec4(0.18f, 0.18f, 0.19f, 1.00f);
            colors[ImGuiCol_HeaderHovered]          = vscodeAccent;
            colors[ImGuiCol_Tab]                    = vscodeBg;
            colors[ImGuiCol_TabActive]              = vscodeSidebar;

            EditorState::Get().SetThemeColors({
                Color{30, 30, 30, 255},      // VSCode Editor Bg
                Color{60, 60, 65, 100},      
                Color{45, 45, 48, 255},      
                Color{0, 122, 204, 255}      // VSCode Blue Selection
            });
            break;
        }

        case EditorTheme::ModernLight:
        {
            ImGui::StyleColorsLight();
            ImVec4 mainBg      = ImVec4(0.92f, 0.92f, 0.94f, 1.00f); // Darkened from 0.98
            ImVec4 childBg     = ImVec4(0.96f, 0.96f, 0.97f, 1.00f); // Darkened from 1.00
            ImVec4 frameBg     = ImVec4(0.88f, 0.88f, 0.90f, 1.00f); // Darkened from 0.94
            ImVec4 accent      = ImVec4(0.00f, 0.45f, 0.90f, 1.00f);
            ImVec4 text        = ImVec4(0.05f, 0.05f, 0.07f, 1.00f);
            ImVec4 border      = ImVec4(0.75f, 0.75f, 0.78f, 1.00f); // Much more visible border

            colors[ImGuiCol_Text]                   = text;
            colors[ImGuiCol_WindowBg]               = mainBg;
            colors[ImGuiCol_ChildBg]                = childBg;
            colors[ImGuiCol_PopupBg]                = ImVec4(1.0f, 1.0f, 1.0f, 1.00f);
            colors[ImGuiCol_Border]                 = border;
            colors[ImGuiCol_FrameBg]                = frameBg;
            colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.82f, 0.82f, 0.85f, 1.00f);
            colors[ImGuiCol_FrameBgActive]          = ImVec4(0.78f, 0.78f, 0.82f, 1.00f);
            colors[ImGuiCol_TitleBg]                = mainBg;
            colors[ImGuiCol_TitleBgActive]          = mainBg;
            colors[ImGuiCol_TitleBgCollapsed]       = mainBg;
            colors[ImGuiCol_MenuBarBg]              = mainBg;
            colors[ImGuiCol_CheckMark]              = accent;
            colors[ImGuiCol_SliderGrab]             = accent;
            colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.00f, 0.38f, 0.85f, 1.00f);
            colors[ImGuiCol_Button]                 = ImVec4(0.85f, 0.85f, 0.88f, 1.00f);
            colors[ImGuiCol_ButtonHovered]          = accent;
            colors[ImGuiCol_ButtonActive]           = ImVec4(0.00f, 0.38f, 0.85f, 1.00f);
            colors[ImGuiCol_Header]                 = ImVec4(0.85f, 0.85f, 0.88f, 1.00f);
            colors[ImGuiCol_HeaderHovered]          = accent;
            colors[ImGuiCol_HeaderActive]           = accent;
            colors[ImGuiCol_Separator]              = border;
            colors[ImGuiCol_Tab]                    = ImVec4(0.88f, 0.88f, 0.90f, 1.00f);
            colors[ImGuiCol_TabHovered]             = accent;
            colors[ImGuiCol_TabActive]              = childBg;
            colors[ImGuiCol_TabUnfocused]           = mainBg;
            colors[ImGuiCol_TabUnfocusedActive]     = childBg;

            style.WindowBorderSize = 1.0f; // Add borders for better definition
            style.FrameBorderSize  = 1.0f;

            EditorState::Get().SetThemeColors({
                Color{220, 222, 225, 255},    // Darkened Viewport BG so WHITE objects pop
                Color{180, 182, 185, 120},    
                Color{170, 172, 175, 255},    
                Color{0, 122, 255, 255}       
            });
            break;
        }

        case EditorTheme::Retro:
        {
            ImGui::StyleColorsLight();
            
            // "SolidWorks Industrial" Palette
            ImVec4 swBg        = ImVec4(0.86f, 0.86f, 0.87f, 1.00f); // Professional Neutral Grey
            ImVec4 swFrame     = ImVec4(1.00f, 1.00f, 1.00f, 1.00f); // Clean White Inputs
            ImVec4 swAccent    = ImVec4(0.18f, 0.45f, 0.68f, 1.00f); // Muted "Engineer Blue"
            ImVec4 swTitle     = ImVec4(0.75f, 0.75f, 0.77f, 1.00f);

            colors[ImGuiCol_Text]                   = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
            colors[ImGuiCol_WindowBg]               = swBg;
            colors[ImGuiCol_ChildBg]                = swBg;
            colors[ImGuiCol_PopupBg]                = swFrame;
            colors[ImGuiCol_Border]                 = ImVec4(0.60f, 0.60f, 0.63f, 1.00f); 
            
            colors[ImGuiCol_FrameBg]                = swFrame;
            colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.94f, 0.94f, 0.97f, 1.00f);
            colors[ImGuiCol_FrameBgActive]          = ImVec4(0.90f, 0.90f, 0.93f, 1.00f);
            
            colors[ImGuiCol_TitleBg]                = swTitle;
            colors[ImGuiCol_TitleBgActive]          = swTitle;
            colors[ImGuiCol_TitleBgCollapsed]       = swTitle;
            colors[ImGuiCol_MenuBarBg]              = swBg;
            
            colors[ImGuiCol_CheckMark]              = swAccent;
            colors[ImGuiCol_SliderGrab]             = swAccent;
            colors[ImGuiCol_Button]                 = ImVec4(0.80f, 0.80f, 0.82f, 1.00f);
            colors[ImGuiCol_ButtonHovered]          = swAccent;
            colors[ImGuiCol_ButtonActive]           = ImVec4(0.15f, 0.40f, 0.60f, 1.00f);
            
            colors[ImGuiCol_Header]                 = ImVec4(0.78f, 0.78f, 0.81f, 1.00f);
            colors[ImGuiCol_HeaderHovered]          = swAccent;
            colors[ImGuiCol_HeaderActive]           = swAccent;
            
            colors[ImGuiCol_Tab]                    = swBg;
            colors[ImGuiCol_TabActive]              = swFrame;

            style.WindowRounding = 0.0f;    // Geometric Sharpness
            style.FrameRounding  = 0.0f;
            style.PopupRounding  = 0.0f;
            style.TabRounding    = 0.0f;
            style.GrabRounding   = 0.0f;
            style.WindowBorderSize = 1.0f;
            style.FrameBorderSize  = 1.0f;

            EditorState::Get().SetThemeColors({
                Color{220, 220, 225, 255},    // Neutral technical dead-space
                Color{180, 180, 190, 80},     // Subtler technical grid
                Color{160, 160, 170, 255},    
                Color{40, 110, 200, 255}      // SolidWorks Selection Blue
            });
            break;
        }
    }
}

} // namespace Editor
