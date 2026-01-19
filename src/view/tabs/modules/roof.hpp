#pragma once
#include "IEditor.hpp"

class RoofModule : public IEditorModule {
public:
    const char* GetName() const override { return "Roof & Boot"; }

    bool Render(json& root) override {
        bool dirty = false;

        // --- Add New Part Utility ---
        ImGui::TextDisabled("Add Convertible Components:");
        static int selectedPart = 0;
        const char* partKeys[] = { "x_convertible_roof", "x_convertible_boot" };
        const char* partLabels[] = { "Main Roof Part", "Boot/Cover Part" };

        ImGui::SetNextItemWidth(200);
        ImGui::Combo("##partselect", &selectedPart, partLabels, IM_ARRAYSIZE(partLabels));
        ImGui::SameLine();
        
        if (ImGui::Button("Add Part")) {
            if (!root.contains("roofs")) root["roofs"] = json::object();
            
            std::string base = partKeys[selectedPart];
            std::string finalName = base;
            int suffix = 2;
            
            while (root["roofs"].contains(finalName)) {
                finalName = base + std::to_string(suffix++);
            }
            
            root["roofs"][finalName] = { {"rotation", 60.0f}, {"speed", 2.0f} };
            dirty = true;
        }

        ImGui::Separator();

        // --- List Active Roof Components ---
        if (root.contains("roofs") && !root["roofs"].empty()) {
            std::string toDelete = "";

            for (auto& [key, val] : root["roofs"].items()) {
                if (ImGui::TreeNodeEx(key.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
                    
                    // JSON to Proxy
                    float fRot = val.value("rotation", 0.0f);
                    float fSpd = val.value("speed", 1.0f);

                    ImGui::SetNextItemWidth(150);
                    if (ImGui::DragFloat("Max Rotation", &fRot, 1.0f, -360.0f, 360.0f, "%.1f deg")) {
                        val["rotation"] = fRot;
                        dirty = true;
                    }

                    ImGui::SetNextItemWidth(150);
                    if (ImGui::DragFloat("Anim Speed", &fSpd, 0.05f, 0.1f, 10.0f, "x%.2f")) {
                        val["speed"] = fSpd;
                        dirty = true;
                    }

                    if (ImGui::SmallButton("Remove Part")) toDelete = key;

                    ImGui::TreePop();
                }
                ImGui::Spacing();
            }

            if (!toDelete.empty()) {
                root["roofs"].erase(toDelete);
                dirty = true;
            }
        } else {
            ImGui::TextDisabled("No convertible components defined.");
        }

        return dirty;
    }
};