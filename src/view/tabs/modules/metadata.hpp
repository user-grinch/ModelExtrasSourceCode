#pragma once
#include "IEditor.hpp"

class MetadataModule : public IEditorModule {
public:
    const char* GetName() const override { return "Metadata"; }

    bool Render(json& root) override {
        bool dirty = false;
        if (!root.contains("metadata")) root["metadata"] = json::object();
        json& meta = root["metadata"];

        ImGui::Spacing();
        ImGui::PushItemWidth((ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x)/2);
        std::string author = UI::GetString(meta, "author");
        if (ImGui::InputText("Author", &author)) { meta["author"] = author; dirty = true; }

        std::string desc = UI::GetString(meta, "desc");
        if (ImGui::InputText("Description", &desc)) { meta["desc"] = desc; dirty = true; }

        std::string date = UI::GetString(meta, "creationtime");
        if (ImGui::InputText("Creation Date", &date)) { meta["creationtime"] = date; dirty = true; }

        int ver = meta.value("minver", 10700);
        if (ImGui::InputInt("Min Plugin Version", &ver)) { meta["minver"] = ver; dirty = true; }
        ImGui::PopItemWidth();
        return dirty;
    }
};
