#include "pch.h"
#include "Meter.h"
#include "datamgr.h"
#include <CBike.h>
#include "modelinfomgr.h"

void GearIndicator::Initialize()
{
    ModelInfoMgr::RegisterDummy([](CVehicle *pVeh, RwFrame *pFrame)
                                {
        std::string name = GetFrameNodeName(pFrame);
        if (name.starts_with("x_gearmeter") || name.starts_with("fc_gm")) {
            VehData &data = vehData.Get(pVeh);

            IndicatorData indData;
            indData.pRoot = pFrame;
            FrameUtil::StoreChilds(pFrame, indData.vecFrameList);
            data.vecIndicatorData.push_back(std::move(indData));
        } });

    ModelInfoMgr::RegisterRender([](CVehicle *pVeh)
                                 {
        if (!pVeh || !pVeh->GetIsOnScreen()) return;

        VehData &data = vehData.Get(pVeh);

        for (auto&e : data.vecIndicatorData) {
            if (!e.vecFrameList.empty() &&  pVeh->m_nCurrentGear != e.iCurrent) {
                FrameUtil::HideAllChilds(e.pRoot);
                if (e.vecFrameList.size() > static_cast<size_t>(e.iCurrent))
                {
                    FrameUtil::ShowAllAtomics(e.vecFrameList[e.iCurrent]);
                }
                e.iCurrent = pVeh->m_nCurrentGear;
            }
        }

         });
}

// This is an unoptimized piece of shit, but... I'm too afraid to touch it
void MileageIndicator::Initialize()
{
    ModelInfoMgr::RegisterDummy([](CVehicle *pVeh, RwFrame *pFrame)
                                {
        std::string name = GetFrameNodeName(pFrame);
        if (name.starts_with("x_odometer") || name.starts_with("fc_om")) {
            VehData &data = vehData.Get(pVeh);
            FrameUtil::StoreChilds(pFrame, data.vecIndicatorData[name].vecFrameList);
            data.vecIndicatorData[name].iPrevRot = 1234 + rand() % (57842 - 1234);

            auto &jsonData = DataMgr::Get(pVeh->m_nModelIndex);
            if (jsonData.contains(name))
            {
                if (jsonData[name].contains("kph"))
                {
                    data.vecIndicatorData[name].fMul = jsonData[name]["kph"].get<bool>() ? 160.9f : 1;
                }
                if (jsonData[name].contains("digital"))
                {
                    data.vecIndicatorData[name].bDigital = jsonData[name]["digital"].get<bool>();
                }
            }
            data.vecIndicatorData[name].pFrame = pFrame;
            data.bInitialized = true;
        } });

    ModelInfoMgr::RegisterRender([](CVehicle *pVeh)
                                 {
        if (!pVeh || !pVeh->GetIsOnScreen()) return;

        VehData &data = vehData.Get(pVeh);
        if (data.bInitialized) {
            for (auto& e : data.vecIndicatorData) {
                if (e.second.vecFrameList.size() < 6) {
                   LOG_VERBOSE("Vehicle ID: {}. {} odometer childs detected, 6 expected", pVeh->m_nModelIndex, e.second.vecFrameList.size());
                   return;
               }

               float curRot = (pVeh->m_nVehicleSubClass == VEHICLE_BIKE) ? static_cast<CBike *>(pVeh)->m_afWheelRotationX[1] : static_cast<CAutomobile *>(pVeh)->m_fWheelRotation[3];
               curRot /= (2.86 * e.second.fMul);

               int displayVal = std::stoi(e.second.sScreenText) + abs(e.second.iPrevRot - curRot);
               displayVal = plugin::Clamp(displayVal, 0, 999999);
               e.second.iPrevRot = curRot;

               std::stringstream ss;
               ss << std::setw(6) << std::setfill('0') << displayVal;
               std::string updatedText = ss.str();

               if (e.second.sScreenText != updatedText)
               {
                   for (unsigned int i = 0; i < 6; i++)
                   {
                       if (updatedText[i] != e.second.sScreenText[i])
                       {
                           float angle = (updatedText[i] - e.second.sScreenText[i]) * 36.0f;
                           FrameUtil::SetRotationX(e.second.vecFrameList[i], angle);
                       }
                   }
                   e.second.sScreenText = std::move(updatedText);
               }
            }
        } });
}

void RPMGauge::Initialize()
{
    ModelInfoMgr::RegisterDummy([](CVehicle *pVeh, RwFrame *pFrame)
                                {
        std::string name = GetFrameNodeName(pFrame);
        if (name.starts_with("x_rpm") || name.starts_with("fc_rpm") || name.starts_with("tahook")) {
            VehData &data = vehData.Get(pVeh);
            auto &jsonData = DataMgr::Get(pVeh->m_nModelIndex);
            if (jsonData.contains(name))
            {
                if (jsonData[name].contains("maxrpm"))
                {
                    data.vecGaugeData[name].iMaxRPM = jsonData[name].value("maxrpm", data.vecGaugeData[name].iMaxRPM);
                }
                if (jsonData[name].contains("maxrotation"))
                {
                    data.vecGaugeData[name].fMaxRotation = jsonData[name].value("maxrotation", data.vecGaugeData[name].fMaxRotation);
                }
            }
            data.vecGaugeData[name].pFrame = pFrame;
            data.bInitialized = true;
        } });

    ModelInfoMgr::RegisterRender([](CVehicle *pVeh)
                                 {
        if (!pVeh || !pVeh->GetIsOnScreen()) return;

        VehData &data = vehData.Get(pVeh);
        if (data.bInitialized) {
            float delta = CTimer::ms_fTimeScale;
            float speed = Util::GetVehicleSpeedRealistic(pVeh);


            for (auto& e : data.vecGaugeData) {
                float rpm = 0.0f;

                if (pVeh->m_nCurrentGear > 0) {
                  rpm = (speed / pVeh->m_nCurrentGear) * 100.0f;
                }

                if (pVeh->bEngineOn) {
                  rpm = std::max(rpm, 0.1f * e.second.iMaxRPM);
                }

                rpm = plugin::Clamp(rpm, 0.0f, static_cast<float>(e.second.iMaxRPM));

                float targetRotation = (rpm / e.second.iMaxRPM) * e.second.fMaxRotation;
                targetRotation = plugin::Clamp(targetRotation, 0.0f, e.second.fMaxRotation);

                float change = (targetRotation - e.second.fCurRotation) * 0.25f * delta;
                FrameUtil::SetRotationY(e.second.pFrame, change);
                e.second.fCurRotation += change;
            }
        } });
}

void SpeedGauge::Initialize()
{
    ModelInfoMgr::RegisterDummy([](CVehicle *pVeh, RwFrame *pFrame)
                                {
    std::string name = GetFrameNodeName(pFrame);
    if (name.starts_with("x_sm") || name.starts_with("fc_sm") || name.starts_with("speedook")) {
        VehData &data = vehData.Get(pVeh);
        auto &jsonData = DataMgr::Get(pVeh->m_nModelIndex);
        if (jsonData.contains(name))
        {
            if (jsonData[name].contains("kph"))
            {
                data.vecGaugeData[name].fMul = jsonData[name]["kph"].get<bool>() ? 160.9f : 1;
            }
            if (jsonData[name].contains("maxspeed"))
            {
                data.vecGaugeData[name].iMaxSpeed = jsonData[name].value("maxspeed", data.vecGaugeData[name].iMaxSpeed);
            }
            if (jsonData[name].contains("maxrotation"))
            {
                data.vecGaugeData[name].fMaxRotation = jsonData[name].value("maxrotation", data.vecGaugeData[name].fMaxRotation);
            }
        }
        data.vecGaugeData[name].pFrame = pFrame;
        data.bInitialized = true;
    } });

    ModelInfoMgr::RegisterRender([](CVehicle *pVeh)
                                 {
        if (!pVeh || !pVeh->GetIsOnScreen()) return;

        VehData &data = vehData.Get(pVeh);
        if (data.bInitialized) {
            float speed = Util::GetVehicleSpeedRealistic(pVeh);
            float delta = CTimer::ms_fTimeScale;

            for (auto& e : data.vecGaugeData) {
                float newRot = (e.second.fMaxRotation / e.second.iMaxSpeed) * speed * delta;
                newRot = plugin::Clamp(newRot, 0, e.second.fMaxRotation);

                float change = (newRot - e.second.fCurRotation) * 0.5f * delta;
                FrameUtil::SetRotationY(e.second.pFrame, change);
                e.second.fCurRotation += change;
            }
        } });
}

void TurboGauge::Initialize()
{
    ModelInfoMgr::RegisterDummy([](CVehicle *pVeh, RwFrame *pFrame)
                                {
        std::string name = GetFrameNodeName(pFrame);
        if (name.starts_with("x_tm")) {
            VehData &data = vehData.Get(pVeh);
            auto &jsonData = DataMgr::Get(pVeh->m_nModelIndex);
            if (jsonData.contains(name))
            {
                if (jsonData[name].contains("maxturbo"))
                {
                    data.vecGaugeData[name].iMaxTurbo = jsonData[name].value("maxturbo", data.vecGaugeData[name].iMaxTurbo);
                }
                if (jsonData[name].contains("maxrotation"))
                {
                    data.vecGaugeData[name].fMaxRotation = jsonData[name].value("maxrotation", data.vecGaugeData[name].fMaxRotation);
                }
            }
            data.vecGaugeData[name].pFrame = pFrame;
            data.bInitialized = true;
        } });

    ModelInfoMgr::RegisterRender([](CVehicle *pVeh)
                                 {
        if (!pVeh || !pVeh->GetIsOnScreen()) return;

        VehData &data = vehData.Get(pVeh);
        if (data.bInitialized) {
            float speed = Util::GetVehicleSpeedRealistic(pVeh);
            float delta = CTimer::ms_fTimeScale;

            for (auto& e : data.vecGaugeData) {
                float turbo = abs(e.second.fPrevTurbo - speed);

                if (pVeh->m_nCurrentGear != 0)
                {
                    turbo += 10.0f;
                }

                float newRot = (e.second.fMaxRotation / e.second.iMaxTurbo) * abs(e.second.fPrevTurbo - speed) * delta * 1.0f;
                newRot = plugin::Clamp(newRot, 0, e.second.fMaxRotation);

                float change = (newRot - e.second.fCurRotation) * 0.25f * delta;
                FrameUtil::SetRotationY(e.second.pFrame, change);
                e.second.fCurRotation += change;
            }
        } });
}

void FixedGauge::Initialize()
{
    ModelInfoMgr::RegisterDummy([](CVehicle *pVeh, RwFrame *pFrame)
                                {
        std::string name = GetFrameNodeName(pFrame);

        // rest are for backward compatibility
        if (name.starts_with("x_gauge_fixed") || name == "x_gasmeter" || name == "x_gm" || name == "petrolok") {
            auto &jsonData = DataMgr::Get(pVeh->m_nModelIndex);

            float angle = 0;
            if (jsonData.contains(name) && jsonData[name].contains("angle")) {
                angle = jsonData[name]["angle"];
            } else {
                angle = RandomNumberInRange(20.0f, 70.0f);
            }
            FrameUtil::SetRotationY(pFrame, angle);
        } });
}