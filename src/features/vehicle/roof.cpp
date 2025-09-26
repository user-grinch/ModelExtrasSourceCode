#include "pch.h"
#include "roof.h"
#include "modelinfomgr.h"
#include "datamgr.h"
#include "audiomgr.h"
#include "util.h"
#include "CWeather.h"

void ConvertibleRoof::Initialize() {
    ModelInfoMgr::RegisterDummy([](CVehicle* pVeh, RwFrame* pFrame) {
        std::string name = GetFrameNodeName(pFrame);
        if (!name.starts_with("x_convertable_roof"))  {
            return;
        }

        RoofConfig c;
        c.pFrame = pFrame;
        auto& jsonData = DataMgr::Get(pVeh->m_nModelIndex);
        if (jsonData.contains("roofs") && jsonData["roofs"].contains(name)) {
            auto& data = jsonData["roofs"][name];
            c.targetRot = jsonData["roofs"][name].value("rotation", c.targetRot);
            c.speed = jsonData["roofs"][name].value("speed", c.speed);
        }

        VehData& data = xData.Get(pVeh);

        // Randomly open the roofs
        if (!data.m_bRoofTargetExpanded 
            && CWeather::NewWeatherType != eWeatherType::WEATHER_RAINY_SF && CWeather::OldWeatherType != eWeatherType::WEATHER_RAINY_SF
            && CWeather::NewWeatherType != eWeatherType::WEATHER_RAINY_COUNTRYSIDE  && CWeather::OldWeatherType != eWeatherType::WEATHER_RAINY_COUNTRYSIDE) {
            MatrixUtil::SetRotationX(&pFrame->modelling, c.targetRot);
            c.currentRot = c.targetRot;
        }
        data.m_bInit = true;
        data.m_Roofs.push_back(std::move(c));
    });

    plugin::Events::vehicleRenderEvent += [](CVehicle *pVeh) {
        if (CWeather::NewWeatherType != eWeatherType::WEATHER_RAINY_SF && CWeather::OldWeatherType != eWeatherType::WEATHER_RAINY_SF
        && CWeather::NewWeatherType != eWeatherType::WEATHER_RAINY_COUNTRYSIDE  && CWeather::OldWeatherType != eWeatherType::WEATHER_RAINY_COUNTRYSIDE) {
            return;
        }

        VehData& data = xData.Get(pVeh);
        if (data.m_bInit && !data.m_bRoofTargetExpanded && pVeh->m_pDriver && !pVeh->IsDriver(FindPlayerPed())) {
            data.m_bRoofTargetExpanded = true;
        }
    };

    ModelInfoMgr::RegisterRender([](CVehicle* pVeh) {
        if (!pVeh || !pVeh->GetIsOnScreen()) {
            return;
        }

        VehData& data = xData.Get(pVeh);
        for (auto& e : data.m_Roofs) {
            if (!e.pFrame) {
                continue;
            }
            
            MatrixUtil::SetRotationX(&e.pFrame->modelling, e.currentRot);
            float target = data.m_bRoofTargetExpanded ? 0.0f : e.targetRot;
            float delta = target - e.currentRot;
            float step = CTimer::ms_fTimeStep * std::abs(e.targetRot) / 360.0f * e.speed;

            if (std::abs(delta) > step) {
                e.currentRot += step * (delta > 0.0f ? 1.0f : -1.0f);
            } else {
                e.currentRot = target;
            }

        }
    });

    plugin::Events::processScriptsEvent += []() {
        size_t now = CTimer::m_snTimeInMilliseconds;
        static size_t prev = 0;
        static uint32_t roofToggleKey = gConfig.ReadInteger("KEYS", "RoofToggleKey", VK_R);

        if (KeyPressed(roofToggleKey) && now - prev > 500.0f) {
            CVehicle *pVeh = FindPlayerVehicle();
            if (pVeh) {
                VehData& data = xData.Get(pVeh);
                data.m_bRoofTargetExpanded = !data.m_bRoofTargetExpanded;
                prev = now;
                AudioMgr::PlaySwitchSound(pVeh);
            }
        }
    };
}
