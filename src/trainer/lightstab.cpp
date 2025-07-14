#include "pch.h"
#include "GrinchTrainerAPI.h"
#include "mgr.h"
#include "datamgr.h"

extern float CLAMP_OFFSET_X;
extern float CLAMP_OFFSET_Y;
void LightsTab() {
    CVehicle *pVeh = FindPlayerVehicle(0, true);

    if (pVeh)
    {
        TAPI_Button("Reload", [](void *val){
            FeatureMgr::Reload(FindPlayerVehicle(0, true));
        });
        TAPI_Spacing(0, 10);
        TAPI_InputFloat("Clamp offset X",  &CLAMP_OFFSET_X, NULL, -10.0f, 10.0f);
        TAPI_InputFloat("Clamp offset Y",  &CLAMP_OFFSET_Y, NULL, -10.0f, 10.0f);

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