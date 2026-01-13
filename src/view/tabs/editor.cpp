#pragma once
#include "pch.h"
#include "defines.h"
#include "imgui/imgui.h"
#include "imgui/imgui_stdlib.h"
#include "json.hpp"
#include "view/common.hpp"

using json = nlohmann::json;

// =========================================================
// 0. Helpers (UI Utilities)
// =========================================================
namespace UI {
    // Safe string getter
    static std::string GetString(const json& j, const char* key, const char* defaultVal = "") {
        return j.value(key, defaultVal);
    }

    // Helper for RGBA JSON objects
    static bool DrawJsonColor(const char* label, json& jNode) {
        float col[4] = {
            jNode.value("red", 0) / 255.0f,
            jNode.value("green", 0) / 255.0f,
            jNode.value("blue", 0) / 255.0f,
            jNode.value("alpha", 255) / 255.0f
        };

        if (ImGui::ColorEdit4(label, col, ImGuiColorEditFlags_AlphaPreviewHalf)) {
            jNode["red"] = static_cast<int>(col[0] * 255.0f);
            jNode["green"] = static_cast<int>(col[1] * 255.0f);
            jNode["blue"] = static_cast<int>(col[2] * 255.0f);
            jNode["alpha"] = static_cast<int>(col[3] * 255.0f);
            return true;
        }
        return false;
    }

    // Helper for Enums/Combos
    static bool DrawJsonCombo(const char* label, json& jNode, const char* key, const std::vector<std::string>& options) {
        std::string current = jNode.value(key, options.empty() ? "" : options[0]);
        bool changed = false;

        if (ImGui::BeginCombo(label, current.c_str())) {
            for (const auto& opt : options) {
                bool isSelected = (current == opt);
                if (ImGui::Selectable(opt.c_str(), isSelected)) {
                    jNode[key] = opt;
                    changed = true;
                }
                if (isSelected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        return changed;
    }
}

// =========================================================
// 1. Abstract Base Module
// =========================================================
class IEditorModule {
public:
    virtual ~IEditorModule() = default;
    virtual const char* GetName() const = 0;
    // Returns true if json was modified
    virtual bool Render(json& root) = 0;
};

// =========================================================
// 2. Concrete Modules
// =========================================================

// --- Metadata Module ---
class MetadataModule : public IEditorModule {
public:
    const char* GetName() const override { return "Metadata"; }

    bool Render(json& root) override {
        bool dirty = false;
        if (!root.contains("metadata")) root["metadata"] = json::object();
        json& meta = root["metadata"];

        std::string author = UI::GetString(meta, "author");
        if (ImGui::InputText("Author", &author)) { meta["author"] = author; dirty = true; }

        std::string desc = UI::GetString(meta, "desc");
        if (ImGui::InputText("Description", &desc)) { meta["desc"] = desc; dirty = true; }

        std::string date = UI::GetString(meta, "creationtime");
        if (ImGui::InputText("Creation Date (YYYY-MM-DD)", &date)) { meta["creationtime"] = date; dirty = true; }

        int ver = meta.value("minver", 10700);
        if (ImGui::InputInt("Min Plugin Version", &ver)) { meta["minver"] = ver; dirty = true; }

        return dirty;
    }
};

// --- Lights Module ---
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

// --- Sirens Module ---
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

// --- Mechanical Module (Doors, Spoilers, etc.) ---
class MechanicalModule : public IEditorModule {
public:
    const char* GetName() const override { return "Mechanics"; }

    bool Render(json& root) override {
        bool dirty = false;

        // Spoilers
        if (ImGui::CollapsingHeader("Spoilers")) {
            if (!root.contains("spoilers")) root["spoilers"] = json::object();
            json& spoilers = root["spoilers"];

            static char spName[32] = "movspoiler";
            ImGui::InputText("Dummy Name", spName, 32);
            ImGui::SameLine();
            if (ImGui::Button("Add/Edit Spoiler")) {
                if (!spoilers.contains(spName)) spoilers[spName] = {{"rotation", 30.0}, {"time", 3000}, {"triggerspeed", 20}};
                dirty = true;
            }

            for (auto& [key, val] : spoilers.items()) {
                if (ImGui::TreeNode(key.c_str())) {
                    float rot = val.value("rotation", 30.0f);
                    if (ImGui::DragFloat("Max Rotation", &rot)) { val["rotation"] = rot; dirty = true; }

                    int time = val.value("time", 3000);
                    if (ImGui::DragInt("Time (ms)", &time)) { val["time"] = time; dirty = true; }

                    int spd = val.value("triggerspeed", 20);
                    if (ImGui::DragInt("Trigger Speed (kmh)", &spd)) { val["triggerspeed"] = spd; dirty = true; }

                    ImGui::TreePop();
                }
            }
        }

        // Doors
        if (ImGui::CollapsingHeader("Doors")) {
            if (!root.contains("doors")) root["doors"] = json::object();
            json& doors = root["doors"];

            static std::vector<std::string> doorNodes = {"x_sd_lf", "x_sd_rf", "x_sd_lr", "x_sd_rr", "x_rd_lf", "x_rd_rf"};
            static int doorIdx = 0;
            if (ImGui::Combo("Select Door Node", &doorIdx, [](void* data, int idx, const char** out_text) {
                auto& v = *static_cast<std::vector<std::string>*>(data);
                if (idx < 0 || idx >= v.size()) return false;
                *out_text = v[idx].c_str();
                return true;
            }, &doorNodes, doorNodes.size())) {}

            ImGui::SameLine();
            if (ImGui::Button("Add Door")) {
                doors[doorNodes[doorIdx]] = {{"mul", 1.0}, {"popout", 0.15}};
                dirty = true;
            }

            for (auto& [key, val] : doors.items()) {
                if (ImGui::TreeNode(key.c_str())) {
                    float mul = val.value("mul", 1.0f);
                    if (ImGui::DragFloat("Multiplier (Neg for reverse)", &mul, 0.1f)) { val["mul"] = mul; dirty = true; }

                    float pop = val.value("popout", 0.15f);
                    if (ImGui::DragFloat("Popout Amount", &pop, 0.01f)) { val["popout"] = pop; dirty = true; }

                    ImGui::TreePop();
                }
            }
        }

        return dirty;
    }
};

// --- Raw JSON View (Backup) ---
class RawJsonModule : public IEditorModule {
public:
    const char* GetName() const override { return "Raw View"; }
    bool Render(json& root) override {
        // Simple recursive tree node viewer for debugging
        std::string s = root.dump(4);
        ImGui::InputTextMultiline("##rawjson", &s, ImVec2(-1, -1), ImGuiInputTextFlags_ReadOnly);
        return false;
    }
};

// =========================================================
// 3. Main Editor Class
// =========================================================
class ModelExtrasEditor {
private:
    int currentModelId = -1;
    std::string basePath = "./ModelExtras/data/";
    json rootData;
    bool isDirty = false;
    char statusMessage[512] = "Ready";

    // Modules
    std::vector<std::unique_ptr<IEditorModule>> modules;

    void MarkDirty() { isDirty = true; }

public:
    ModelExtrasEditor() {
        // Register Modules Here
        modules.push_back(std::make_unique<MetadataModule>());
        modules.push_back(std::make_unique<LightsModule>());
        modules.push_back(std::make_unique<SirensModule>());
        modules.push_back(std::make_unique<MechanicalModule>());
        modules.push_back(std::make_unique<RawJsonModule>());
    }

    void SetBasePath(const std::string& path) { basePath = path; }

    void SetModel(int id) {
        if (currentModelId != id) {
            currentModelId = id;
            Load();
        }
    }

    void Load() {
        if (!std::filesystem::exists(basePath)) std::filesystem::create_directories(basePath);

        std::string fullPath = basePath + std::to_string(currentModelId) + ".jsonc";
        std::ifstream file(fullPath);
        if (file.is_open()) {
            try {
                // Use allow_exceptions = false to prevent crashes on bad parsing
                rootData = json::parse(file, nullptr, true, true);
                snprintf(statusMessage, sizeof(statusMessage), "Loaded: %s", fullPath.c_str());
            } catch (const std::exception& e) {
                snprintf(statusMessage, sizeof(statusMessage), "Error: %s", e.what());
                rootData = json::object(); // Reset on error
            }
        } else {
            snprintf(statusMessage, sizeof(statusMessage), "New config created for ID %d", currentModelId);
            rootData = json::object();
            rootData["metadata"] = {{"author", "Me"}, {"minver", 10700}};
        }
        isDirty = false;
    }

    void Save() {
        if (currentModelId < 0) return;
        std::string fullPath = basePath + std::to_string(currentModelId) + ".jsonc";
        std::ofstream file(fullPath);
        if (file.is_open()) {
            file << rootData.dump(4);
            snprintf(statusMessage, sizeof(statusMessage), "Saved successfully.");
            isDirty = false;
        } else {
            snprintf(statusMessage, sizeof(statusMessage), "Failed to save file!");
        }
    }

    void DrawUI() {
        // Main Window Setup
        // Note: Logic inside "Begin" is skipped if collapsed/closed, but we need the external logic to keep it open
        // Using a child or simple layout since Tab_Editor handles the Window.

        // --- Toolbar ---
        ImGui::TextDisabled("Model ID: %d", currentModelId);
        ImGui::SameLine();
        if (ImGui::Button("Save JSON")) Save();
        ImGui::SameLine();
        if (ImGui::Button("Reload")) Load();
        ImGui::SameLine();

        if (isDirty) ImGui::TextColored(ImVec4(1, 1, 0, 1), "(* Unsaved)");
        else ImGui::TextColored(ImVec4(0, 1, 0, 1), "(Synced)");

        ImGui::SameLine();
        ImGui::TextDisabled("| %s", statusMessage);

        ImGui::Separator();

        // --- Modules Tabs ---
        if (ImGui::BeginTabBar("ModulesBar")) {
            for (auto& mod : modules) {
                if (ImGui::BeginTabItem(mod->GetName())) {
                    if (mod->Render(rootData)) {
                        MarkDirty();
                    }
                    ImGui::EndTabItem();
                }
            }
            ImGui::EndTabBar();
        }
    }
};

// =========================================================
// 4. Tab Rendering Function (Mandatory)
// =========================================================

void Tab_Editor() {
    if (ImGui::BeginTabItem("Editor")) {
        CVehicle* pVeh = FindPlayerVehicle();
        if (pVeh) {
            static ModelExtrasEditor editor;
            static int lastModelIndex = -1;
            int curModelIndex = pVeh->m_nModelIndex;
            if (curModelIndex != lastModelIndex) {
                editor.SetModel(curModelIndex);
                editor.SetBasePath("./ModelExtras/data/"); // Ensure this path macro matches your setup
                editor.Load();
                lastModelIndex = curModelIndex;
            }
            editor.DrawUI();
        } else {
            DrawVehicleRequiredMessage();
        }
        ImGui::EndTabItem();
    }
}