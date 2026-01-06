#include "pch.h"
#include "view/vehframeview.hpp"
#include "defines.h"
#include "imgui/rw/renderhook.h"
#include <extensions/ScriptCommands.h>
#include <imgui/fonts/fontmgr.h>
#include <imgui/fonts/fonts.h>

extern bool gbGlobalIndicatorLights;
extern float gfGlobalCoronaSize;
extern int gGlobalCoronaIntensity;
extern int gGlobalShadowIntensity;

extern RwSurfaceProperties& gLightSurfProps;

#define F_NULL [](void *) {}

extern void LightsTab();

bool bWindowOpenFlag = false;
extern void DrawMonsterInspector();
void ViewInit() {
    if (bWindowOpenFlag) {
        RenderHook::SetCursorVisible(bWindowOpenFlag);
        if (ImGui::Begin("ModelExtras Developer Window", &bWindowOpenFlag)) {
            if (!PATRON_BUILD) {
                ImGui::Text("Only available to patron supporters");
				ImGui::End();
                return;
            }

            if (ImGui::BeginTabBar("ModelExtras", ImGuiTabBarFlags_FittingPolicyScroll)) {
                ImGui::Dummy(ImVec2(0, 10));

                if (ImGui::BeginTabItem("Features", NULL, ImGuiTabItemFlags_NoTooltip)) {
                    ImGui::Dummy(ImVec2(0, 10));

                    ImGui::Columns(2, nullptr, false);
                    ImGui::Checkbox("Global Indicator Lights", &gbGlobalIndicatorLights);
                    ImGui::NextColumn();
                    ImGui::Columns(1);

                    ImGui::Dummy(ImVec2(0, 10));
                    ImGui::Text("Light defaults");

                    ImGui::SliderInt("Corona Intensity", &gGlobalCoronaIntensity, 0, 255);
                    ImGui::SliderFloat("Corona Size", &gfGlobalCoronaSize, 0.0f, 10.0f);
                    ImGui::SliderInt("Shadow Intensity", &gGlobalShadowIntensity, 0, 255);

                    ImGui::Dummy(ImVec2(0, 10));
                    ImGui::SliderFloat3("Material ambient on", (float*)&gLightSurfProps.ambient, 0.0f, 20.0f);

                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("NodeExplorer")) {
                    CVehicle* pVeh = FindPlayerVehicle(0, true);
                    if (pVeh) {
                        VehicleFrameView::Render(pVeh);
                    }
                    else {
                        ImGui::Text("Player must be inside a vehicle");
                    }

                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Lights")) {
                    LightsTab();
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Vehicle Inspector")) {
                    DrawMonsterInspector();
                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
            }
        }
        ImGui::End();
    }
}

#define TEST_CHEAT 0x0ADC

void InjectImGuiHooks() {
    ImGui::CreateContext();
    Events::initGameEvent.after += []()
    {
        RenderHook::Init(ViewInit);
    };

    Events::processScriptsEvent.after += []()
    {
        static float screenX = -1;
        static float screenY = -1;
        if (screenX != SCREEN_WIDTH || screenY != SCREEN_HEIGHT) {
            if (screenX == -1 && screenY == -1)
            {
                auto& io = ImGui::GetIO();
                io.FontDefault = FontMgr::LoadFont("text", textFont, 28.0f);
                FontMgr::LoadFont("title", titleFont, 40.0f);
            }

            FontMgr::RescaleFonts(SCREEN_WIDTH, SCREEN_HEIGHT);
            screenX = SCREEN_WIDTH;
            screenY = SCREEN_HEIGHT;
        }

        if (plugin::Command<TEST_CHEAT>("MEDEV")) {
            bWindowOpenFlag = !bWindowOpenFlag;
			RenderHook::SetCursorVisible(bWindowOpenFlag);  
        }
	};
}