#pragma once
#include "IEditor.hpp"

class ExhaustsModule : public IEditorModule {
public:
    const char* GetName() const override { return "Visuals"; }

    bool Render(json& root) override {
        bool dirty = false;

        if (ImGui::Button("Add")) {
            std::string n = "x_exhaust";
            int s = 2;
            while (root["exhausts"].contains(n)) n = "x_exhaust" + std::to_string(s++);
            root["exhausts"][n] = {{"lifetime", 0.2f}, {"speed", 1.0f}, {"size", 1.0f}};
            dirty = true;
        }

        for (auto& [k, v] : root["exhausts"].items()) {
            if (ImGui::TreeNode(k.c_str())) {
                float fL = v.value("lifetime", 0.2f), fS = v.value("speed", 1.0f), fSz = v.value("size", 1.0f);
                ImGui::SetNextItemWidth(80); if (ImGui::DragFloat("Life", &fL, 0.01f)) { v["lifetime"] = fL; dirty = true; }
                ImGui::SameLine(); ImGui::SetNextItemWidth(80); if (ImGui::DragFloat("Size", &fSz, 0.01f)) { v["size"] = fSz; dirty = true; }
                
                if (ImGui::SmallButton("Remove")) { root["exhausts"].erase(k); dirty = true; ImGui::TreePop(); break; }
                ImGui::TreePop();
            }
        }
        return dirty;
    }
};