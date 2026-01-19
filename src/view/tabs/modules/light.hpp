#pragma once
#include "IEditor.hpp"

class LightsModule : public IEditorModule {
    std::string selectedLight;

    // Templates for adding new lights
    const std::map<std::string, json> templates = {
        {"Left Indicator", {{"corona", {{"color", {{"red", 183}, {"green", 255}, {"blue", 0}, {"alpha", 200}}}, {"size", 0.5}, {"type", "directional"}}}, {"shadow", {{"size", 1.0}, {"texture", "pointlight"}}}, {"strobedelay", 0}}},
        {"Right Indicator", {{"corona", {{"color", {{"red", 255}, {"green", 58}, {"blue", 0}, {"alpha", 200}}}, {"size", 0.5}, {"type", "directional"}}}}},
        {"Brake Light", {{"corona", {{"color", {{"red", 255}, {"green", 0}, {"blue", 0}, {"alpha", 200}}}, {"size", 0.5}, {"type", "directional"}}}}},
        {"Fog Light", {{"corona", {{"color", {{"red", 255}, {"green", 174}, {"blue", 0}, {"alpha", 200}}}, {"size", 0.6}, {"type", "directional"}}}}},
        {"Reverse Light", {{"corona", {{"color", {{"red", 255}, {"green", 255}, {"blue", 255}, {"alpha", 200}}}, {"size", 0.5}, {"type", "directional"}}}}}
    };

    // Node name defaults for the templates
    const std::map<std::string, std::string> nodeDefaults = {
        {"Left Indicator", "indicator_lf"}, {"Right Indicator", "indicator_rf"},
        {"Brake Light", "breaklight_l"}, {"Fog Light", "foglight_l"},
        {"Reverse Light", "reversingl_l"}
    };

public:
    const char* GetName() const override { return "Lights"; }

    bool Render(json& root) override {
        bool dirty = false;
        if (!root.contains("lights")) root["lights"] = json::object();
        json& lights = root["lights"];

        // Layout: Sidebar (List) | Main (Details)
        if (ImGui::BeginTable("LightsTable", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Resizable)) {
            ImGui::TableSetupColumn("List", ImGuiTableColumnFlags_WidthFixed, 200.0f);
            ImGui::TableSetupColumn("Details", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableNextRow();

            // --- Sidebar ---
            ImGui::TableSetColumnIndex(0);

            // Add Button
            ImGui::SetNextItemWidth(-1);
            if (ImGui::Button("Add Light...")) ImGui::OpenPopup("AddLightPopup");

            if (ImGui::BeginPopup("AddLightPopup")) {
                for (auto& [name, tempJson] : templates) {
                    if (ImGui::Selectable(name.c_str())) {
                        std::string defaultNode = nodeDefaults.at(name);
                        // Ensure unique name
                        if (lights.contains(defaultNode)) defaultNode += "_new";
                        lights[defaultNode] = tempJson;
                        selectedLight = defaultNode;
                        dirty = true;
                    }
                }
                ImGui::EndPopup();
            }

            ImGui::Separator();

            // List items
            std::string toDelete = "";
            for (auto& [key, val] : lights.items()) {
                if (ImGui::Selectable(key.c_str(), selectedLight == key)) {
                    selectedLight = key;
                }
                if (ImGui::BeginPopupContextItem()) {
                    if (ImGui::MenuItem("Delete")) toDelete = key;
                    ImGui::EndPopup();
                }
            }
            if (!toDelete.empty()) {
                lights.erase(toDelete);
                if (selectedLight == toDelete) selectedLight = "";
                dirty = true;
            }

            // --- Details ---
            ImGui::TableSetColumnIndex(1);
            if (!selectedLight.empty() && lights.contains(selectedLight)) {
                json& l = lights[selectedLight];

                ImGui::TextColored(ImVec4(0,1,0,1), "Node: %s", selectedLight.c_str());
                ImGui::Separator();

                // Corona Settings
                if (ImGui::CollapsingHeader("Corona", ImGuiTreeNodeFlags_DefaultOpen)) {
                    if (!l.contains("corona")) l["corona"] = json::object();

                    if (UI::DrawJsonColor("Color##c", l["corona"]["color"])) dirty = true;

                    float size = l["corona"].value("size", 0.5f);
                    if (ImGui::DragFloat("Size##c", &size, 0.05f, 0.0f, 10.0f)) { l["corona"]["size"] = size; dirty = true; }

                    static std::vector<std::string> types = {"directional", "non-directional", "inversed-directional"};
                    if (UI::DrawJsonCombo("Type", l["corona"], "type", types)) dirty = true;
                }

                // Shadow Settings
                if (ImGui::CollapsingHeader("Shadow")) {
                    bool hasShadow = l.contains("shadow");
                    if (ImGui::Checkbox("Enable Shadow", &hasShadow)) {
                        if (hasShadow) l["shadow"] = {{"size", 1.0}, {"texture", "pointlight"}};
                        else l.erase("shadow");
                        dirty = true;
                    }

                    if (hasShadow) {
                        json& s = l["shadow"];
                        float sz = s.value("size", 1.0f);
                        if (ImGui::DragFloat("Size##s", &sz, 0.1f)) { s["size"] = sz; dirty = true; }

                        // Textures from me_texdb.txd
                        static std::vector<std::string> texs = {"pointlight", "narrow", "round", "foglight", "spotlight"};
                        if (UI::DrawJsonCombo("Texture", s, "texture", texs)) dirty = true;

                        bool rc = s.value("rotationchecks", false);
                        if (ImGui::Checkbox("Rotation Checks", &rc)) { s["rotationchecks"] = rc; dirty = true; }
                    }
                }

                int delay = l.value("strobedelay", 0);
                if (ImGui::DragInt("Strobe Delay (ms)", &delay, 10, 0, 5000)) { l["strobedelay"] = delay; dirty = true; }
            } else {
                ImGui::TextDisabled("Select a light to edit.");
            }

            ImGui::EndTable();
        }
        return dirty;
    }
};
