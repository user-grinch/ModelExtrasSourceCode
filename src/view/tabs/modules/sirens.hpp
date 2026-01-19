#pragma once
#include "IEditor.hpp"

class SirensModule : public IEditorModule {
    // This is complex, implementing basic structure.
    // Ideally needs nested tabs for Siren Groups -> Siren IDs.
public:
    const char* GetName() const override { return "Sirens"; }

    bool Render(json& root) override {
        bool dirty = false;
        if (!root.contains("sirens")) root["sirens"] = json::object();
        json& sirens = root["sirens"];

        if (ImGui::CollapsingHeader("Global Settings")) {
            bool imveh = sirens.value("ImVehFt", false);
            if (ImGui::Checkbox("ImVehFt Compatibility", &imveh)) { sirens["ImVehFt"] = imveh; dirty = true; }
        }

        if (!sirens.contains("states")) sirens["states"] = json::object();
        json& states = sirens["states"];

        if (ImGui::Button("Add Siren Group")) {
            states["New Group"] = json::object();
            dirty = true;
        }

        // Iterate Groups (states)
        for (auto& [groupName, groupData] : states.items()) {
            if (ImGui::TreeNode(groupName.c_str())) {

                static char newSirenID[16] = "";
                ImGui::InputText("Siren ID (dummy suffix)", newSirenID, 16);
                ImGui::SameLine();
                if (ImGui::Button("Add Siren")) {
                    if (strlen(newSirenID) > 0) {
                        groupData[newSirenID] = {
                            {"color", {{"red", 255}, {"green", 0}, {"blue", 0}, {"alpha", 255}}},
                            {"size", 0.4},
                            {"state", 1},
                            {"pattern", {100, 100}}
                        };
                        dirty = true;
                    }
                }

                // Iterate Sirens in Group
                for (auto& [sirenId, sirenData] : groupData.items()) {
                    if (ImGui::TreeNode(sirenId.c_str())) {
                        if (UI::DrawJsonColor("Color", sirenData["color"])) dirty = true;

                        float size = sirenData.value("size", 0.4f);
                        if (ImGui::DragFloat("Size", &size, 0.05f)) { sirenData["size"] = size; dirty = true; }

                        // Pattern editor (simple comma text)
                        // A proper pattern editor would be a timeline, but simple text is safer for now
                        if (sirenData.contains("pattern") && sirenData["pattern"].is_array()) {
                            ImGui::Text("Pattern (ms):");
                            ImGui::SameLine();
                            for(auto& t : sirenData["pattern"]) {
                                ImGui::Text("%d ", t.get<int>());
                                ImGui::SameLine();
                            }
                            if(ImGui::Button("Clear Pattern")) { sirenData["pattern"] = json::array(); dirty = true;}
                        }

                        // Rotator toggle
                        std::string type = sirenData.value("type", "directional");
                        bool isRotator = (type == "rotator");
                        if (ImGui::Checkbox("Is Rotator", &isRotator)) {
                            sirenData["type"] = isRotator ? "rotator" : "directional";
                            if (isRotator) sirenData["rotator"] = {{"time", 1000}, {"radius", 360}};
                            dirty = true;
                        }

                        if (isRotator && sirenData.contains("rotator")) {
                            json& rot = sirenData["rotator"];
                            int time = rot.value("time", 1000);
                            if (ImGui::DragInt("Rotation Time (ms)", &time)) { rot["time"] = time; dirty = true; }
                        }

                        ImGui::TreePop();
                    }
                }
                ImGui::TreePop();
            }
        }

        return dirty;
    }
};
