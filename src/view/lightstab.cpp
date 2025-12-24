#include "pch.h"
#include "mgr.h"
#include "datamgr.h"
#include "imgui/imgui.h"

extern float mx1, my1, mz1;
extern float mx2, my2, mz2;

void LightsTab()
{
    CVehicle* pVeh = FindPlayerVehicle(0, true);

    if (pVeh)
    {
        // Reload button
        if (ImGui::Button("Reload"))
        {
            FeatureMgr::Reload(FindPlayerVehicle(0, true));
        }

        ImGui::Dummy(ImVec2(0, 10));

        // Min values
        ImGui::SliderFloat("Min x", &mx1, -10.0f, 10.0f);
        ImGui::SliderFloat("Min y", &my1, -10.0f, 10.0f);
        ImGui::SliderFloat("Min z", &mz1, -10.0f, 10.0f);

        ImGui::Dummy(ImVec2(0, 10));

        // Max values
        ImGui::SliderFloat("Max x", &mx2, -10.0f, 10.0f);
        ImGui::SliderFloat("Max y", &my2, -10.0f, 10.0f);
        ImGui::SliderFloat("Max z", &mz2, -10.0f, 10.0f);

        // TODO: Dashboard section
        ImGui::Dummy(ImVec2(0, 10));
        ImGui::Text("Dashboard work in progress");
    }
    else
    {
        ImGui::Text("Player must be inside a vehicle");
    }
}