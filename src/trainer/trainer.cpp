#include "pch.h"
#include "GrinchTrainerAPI.h"
#include "trainer/vehframeview.hpp"
#include "defines.h"

extern bool gbGlobalIndicatorLights;
extern bool gbGlobalReverseLights;
extern float gfGlobalCoronaSize;
extern int gGlobalCoronaIntensity;
extern int gGlobalShadowIntensity;

extern RwSurfaceProperties gLightSurfPropsOn;
extern RwSurfaceProperties gLightSurfProps;

#define F_NULL [](void *) {}

extern void LightsTab();

void TrainerInit()
{
    TAPI_ClearWidgets();
    if (TAPI_InitConnect("ModelExtras", TAPI_VERSION) == TReturn_Success)
    {
        if (PATRON_BUILD) {
            static const char *tabs[] = {"Features", "NodeExplorer", "Lights"};
            static size_t selectedTab = 0;
            TAPI_Spacing(0, 10);
            if (TAPI_Tabs(tabs, 3, &selectedTab) == TReturn_Success)
            {
                if (selectedTab == 0)
                {
                    TAPI_Spacing(0, 10);
                    TAPI_Columns(2);
                    TAPI_Checkbox("Global Indicator Lights", &gbGlobalIndicatorLights, F_NULL);
                    TAPI_NextColumn();
                    TAPI_Checkbox("Global Reverse Lights", &gbGlobalReverseLights, F_NULL);
                    TAPI_Columns(1);

                    TAPI_Spacing(0, 10);
                    TAPI_Text("Light defaults");
                    TAPI_InputInt("Corona Intensity", &gGlobalCoronaIntensity, F_NULL, 0, 255);
                    TAPI_InputFloat("Corona Size", &gfGlobalCoronaSize, F_NULL, 0.0f, 10.0f);
                    TAPI_InputInt("Shadow Intensity", &gGlobalShadowIntensity, F_NULL, 0, 255);

                    TAPI_Spacing(0, 10);
                    TAPI_Text("Material (Ambient, Diffuse, Specular)");
                    TAPI_InputFloat3("On##Material", &gLightSurfPropsOn.ambient, F_NULL, 0.0f, 20.0f);
                    TAPI_InputFloat3("Off##Material", &gLightSurfProps.ambient, F_NULL, 0.0f, 20.0f);
                }

                if (selectedTab == 1)
                {
                    CVehicle *pVeh = FindPlayerVehicle(0, true);

                    if (pVeh)
                    {
                        VehicleFrameView::Render(pVeh);
                    }
                    else
                    {
                        TAPI_Text("Player must be inside a vehicle");
                    }
                }

                if (selectedTab == 2)
                {
                    LightsTab();
                }
            }
        } else {
            TAPI_Text("Only available to patron supporters");
        }
        TAPI_CloseConnect();
    }
}