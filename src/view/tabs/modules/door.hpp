#pragma once
#include "IEditor.hpp"

class DoorsModule : public IEditorModule {
public:
    const char* GetName() const override { return "Doors"; }

    bool Render(json& root) override {
        bool dirty = false;

        // --- Add New Door Utility ---
        static int selectedType = 0;
        const char* nodeKeys[] = { "x_rd_lf", "x_rd_rf", "x_rd_lr", "x_rd_rr", "x_rd_bonnet", "x_rd_boot", "x_sd_lf", "x_sd_rf" };
        const char* nodeNames[] = { "Front Left", "Front Right", "Rear Left", "Rear Right", "Bonnet", "Boot", "Scissor LF", "Scissor RF" };

        ImGui::TextDisabled("Quick Add Standard Nodes:");
        ImGui::SetNextItemWidth(200);
        ImGui::Combo("##doorselect", &selectedType, nodeNames, IM_ARRAYSIZE(nodeNames));
        ImGui::SameLine();
        
        if (ImGui::Button("Add Door")) {
            std::string base = nodeKeys[selectedType];
            std::string finalName = base;
            int suffix = 2;
            
            if (!root.contains("doors")) root["doors"] = json::object();
            
            while (root["doors"].contains(finalName)) {
                finalName = base + std::to_string(suffix++);
            }
            
            root["doors"][finalName] = { {"mul", 1.0f}, {"popout", 0.15f} };
            dirty = true;
        }

        ImGui::Separator();

        // --- List Active Doors ---
        if (root.contains("doors") && !root["doors"].empty()) {
            std::string toDelete = "";

            for (auto& [key, val] : root["doors"].items()) {
                if (ImGui::TreeNodeEx(key.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
                    
                    float fMul = val.value("mul", 1.0f);
                    float fPop = val.value("popout", 0.0f);

                    ImGui::SetNextItemWidth(120);
                    if (ImGui::DragFloat("Open Multiplier", &fMul, 0.05f, 0.0f, 5.0f)) {
                        val["mul"] = fMul;
                        dirty = true;
                    }

                    ImGui::SetNextItemWidth(120);
                    if (ImGui::DragFloat("Pop-out Distance", &fPop, 0.01f, 0.0f, 1.0f)) {
                        val["popout"] = fPop;
                        dirty = true;
                    }

                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
                    if (ImGui::SmallButton("Delete Node")) toDelete = key;
                    ImGui::PopStyleColor();

                    ImGui::TreePop();
                }
                ImGui::Spacing();
            }

            if (!toDelete.empty()) {
                root["doors"].erase(toDelete);
                dirty = true;
            }
        } else {
            ImGui::TextDisabled("No custom doors configured for this vehicle.");
        }

        return dirty;
    }
};