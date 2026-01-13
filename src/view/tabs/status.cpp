#include "pch.h"
#include "imgui.h"
#include "mgr.h"
#include "magic_enum/magic_enum.hpp"

void Tab_Status() {
    if (ImGui::BeginTabItem("Status")) {
        ImGui::Spacing();
        ImGui::Text("Feature Matrix Status:");
        ImGui::Separator();

        if (ImGui::BeginChild("FeatureList", ImVec2(0, 0), true)) {
            for (size_t i = 0; i < FeatureMgr::m_bEnabledFeatures.size(); i++) {
                bool isEnabled = FeatureMgr::m_bEnabledFeatures[i];
                const char* name = magic_enum::enum_name((eFeatureMatrix)i).data();

                if (isEnabled) {
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "[ON]  %s", name);
                } else {
                    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "[OFF] %s", name);
                }
            }
        }
        ImGui::EndChild();
        ImGui::EndTabItem();
    }
}
