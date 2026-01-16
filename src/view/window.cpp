#include "pch.h"
#include "defines.h"
#include "imgui.h"
#include "imgui/rw/renderhook.h"


extern void Tab_Inspector();
extern void Tab_Status();
extern void Tab_Lights();
extern void Tab_FrameView();
extern void Tab_Editor();

extern bool bWindowOpenFlag;
void DevWindow() {
    if (!bWindowOpenFlag) return;
    RenderHook::SetCursorVisible(bWindowOpenFlag);

    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver); // sensible default
    if (!ImGui::Begin("ModelExtras Developer Window", &bWindowOpenFlag)) {
        ImGui::End();
        return;
    }

    if (!PATRON_BUILD) {
        float winWidth = ImGui::GetWindowSize().x;
        float textWidth = ImGui::CalcTextSize("Only available to patron supporters").x;

        ImGui::Dummy(ImVec2(0, ImGui::GetWindowSize().y * 0.4f)); // Vertical center
        ImGui::SetCursorPosX((winWidth - textWidth) * 0.5f);
        ImGui::TextColored(ImVec4(1, 0.3f, 0.3f, 1.0f), "Only available to patron supporters");
        ImGui::End();
        return;
    }

    ImGui::TextDisabled("Work in Progress");
    if (FindPlayerVehicle()) {
        ImGui::Text("%d", FindPlayerVehicle()->m_nCurrentGear);
    }
    ImGui::Separator();

    if (ImGui::BeginTabBar("ModelExtras", ImGuiTabBarFlags_FittingPolicyScroll)) {
        Tab_Status();
        Tab_Lights();
        Tab_FrameView();
        Tab_Inspector();
        Tab_Editor();
        ImGui::EndTabBar();
    }
    ImGui::End();
}