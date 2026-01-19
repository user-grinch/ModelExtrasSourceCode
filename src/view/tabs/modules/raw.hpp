#pragma once
#include "IEditor.hpp"

class RawJsonModule : public IEditorModule {
private:
    std::string editBuffer;
    void* lastVehicle = nullptr;
    bool isError = false;
    bool needsSync = true;

public:
    const char* GetName() const override { return "Raw View"; }

    bool Render(json& root) override {
        void* currentVehicle = FindPlayerVehicle(-1, false);
        if (currentVehicle != lastVehicle) {
            lastVehicle = currentVehicle;
            needsSync = true;
        }

        if (needsSync) {
            editBuffer = root.dump(4);
            isError = false;
            needsSync = false;
        }

        bool dirty = false;

        if (isError) {
            ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Syntax Error");
        } else {
            ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "JSON Valid");
        }

        if (ImGui::InputTextMultiline("##rawjson", &editBuffer, ImVec2(-1, -1))) {
            try {
                json temporaryJson = json::parse(editBuffer);

                root = temporaryJson;
                isError = false;
                dirty = true;
            } catch (const json::parse_error& e) {
                isError = true;
            }
        }

        return dirty;
    }
};