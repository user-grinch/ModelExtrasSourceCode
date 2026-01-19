#pragma once
#include "IEditor.hpp"

class SpoilerModule : public IEditorModule {
public:
    const char* GetName() const override { return "Spoilers"; }

    bool Render(json& root) override {
        bool dirty = false;
        if (!root.contains("spoilers")) root["spoilers"] = json::object();
        json& spoilers = root["spoilers"];

        static char spName[32] = "movspoiler";

        ImGui::Spacing();
        ImGui::PushItemWidth((ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x) / 2);
        ImGui::InputText("Dummy Name", spName, 32);
        ImGui::SameLine();
        if (ImGui::Button("Add")) {
            if (!spoilers.contains(spName)) {
                spoilers[spName] = {{"rotation", 30.0}, {"time", 3000}, {"triggerspeed", 20}};
                dirty = true;
            }
        }

        std::string keyToDelete = "";

        for (auto& [key, val] : spoilers.items()) {
            bool nodeOpen = ImGui::TreeNodeEx(key.c_str());

            if (nodeOpen) {
                float rot = val.value("rotation", 30.0f);
                if (ImGui::DragFloat("Max Rotation", &rot)) { val["rotation"] = rot; dirty = true; }

                int time = val.value("time", 3000);
                if (ImGui::DragInt("Time (ms)", &time)) { val["time"] = time; dirty = true; }

                int spd = val.value("triggerspeed", 20);
                if (ImGui::DragInt("Trigger Speed (kmh)", &spd)) { val["triggerspeed"] = spd; dirty = true; }

                ImGui::Spacing();
                if (ImGui::Button(("Remove##" + key).c_str())) {
                    keyToDelete = key;
                    dirty = true;
                }
                ImGui::TreePop();
            }
        }

        // Perform deletion safely
        if (!keyToDelete.empty()) {
            spoilers.erase(keyToDelete);
        }

        ImGui::PopItemWidth();
        return dirty;
    }
};