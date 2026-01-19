#pragma once
#include "IEditor.hpp"

class BrakesGearsModule : public IEditorModule {
public:
    const char* GetName() const override { return "Transmission"; }

    bool Render(json& root) override {
        bool dirty = false;
        dirty |= RenderMultiOffset(root, "Gear Lever", "gearlever");
        dirty |= RenderMultiOffset(root, "Clutch", "clutch");
        dirty |= RenderMultiOffset(root, "Front Brake", "frontbrake");
        dirty |= RenderMultiOffset(root, "Rear Brake", "rearbrake");
        return dirty;
    }

private:
    bool RenderMultiOffset(json& root, const char* label, const char* keyBase) {
        bool dirty = false;
        std::vector<std::string> keys;
        if (root.contains(keyBase)) keys.push_back(keyBase);
        int suffix = 2;
        while (root.contains(keyBase + std::to_string(suffix))) keys.push_back(keyBase + std::to_string(suffix++));

        ImGui::TextDisabled("%s", label); ImGui::SameLine();
        if (ImGui::SmallButton((std::string("+##") + keyBase).c_str())) {
            root[keys.empty() ? keyBase : keyBase + std::to_string(suffix)] = {{"offset", 0.0f}};
            dirty = true;
        }

        for (const auto& k : keys) {
            float fVal = root[k].value("offset", 0.0f);
            ImGui::SetNextItemWidth(120);
            if (ImGui::DragFloat(k.c_str(), &fVal, 0.1f)) { root[k]["offset"] = fVal; dirty = true; }
            ImGui::SameLine();
            if (ImGui::SmallButton((std::string("X##") + k).c_str())) { root.erase(k); dirty = true; }
        }
        ImGui::Separator();
        return dirty;
    }
};