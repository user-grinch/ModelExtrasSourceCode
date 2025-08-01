#include "pch.h"
#include "GrinchTrainerAPI.h"
#include "mgr.h"
#include "datamgr.h"

extern float mx1, my1, mz1;
extern float mx2, my2, mz2;

void LightsTab() {
    CVehicle *pVeh = FindPlayerVehicle(0, true);

    if (pVeh)
    {
        TAPI_Button("Reload", [](void *val){
            FeatureMgr::Reload(FindPlayerVehicle(0, true));
        });
        TAPI_Spacing(0, 10);
        TAPI_InputFloat("Min x",  &mx1, NULL, -10.0f, 10.0f);
        TAPI_InputFloat("Min y",  &my1, NULL, -10.0f, 10.0f);
        TAPI_InputFloat("Min z",  &mz1, NULL, -10.0f, 10.0f);

        TAPI_Spacing(0, 10);
        TAPI_InputFloat("Max x",  &mx2, NULL, -10.0f, 10.0f);
        TAPI_InputFloat("Max y",  &my2, NULL, -10.0f, 10.0f);
        TAPI_InputFloat("Max z",  &mz2, NULL, -10.0f, 10.0f);

        // auto& data = DataMgr::Get(pVeh->m_nModelIndex);
        // if (data.contains("lights")) {
        //     for (auto& light : data["lights"]) {
        //         if (light.contains("corona")) {
        //             TAPI_ColorPicker3("Color", light["corona"]["red"].get<int>())
        //         }
        //     }
        // }
        TAPI_Text("Dashboard work in progress");
    }
    else
    {
        TAPI_Text("Player must be inside a vehicle");
    }
}