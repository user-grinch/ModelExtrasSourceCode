
#include "pch.h"
#include "wheelhub.h"
#include "modelinfomgr.h"

void ApplySteerAngle(CVehicle* pVeh, RwFrame* hub, bool leftSide)
{
    if (!hub || !pVeh) return;

    // Apply steering angle
    float angle = pVeh->m_fSteerAngle;
    if (leftSide) angle = -angle;

    MatrixUtil::SetRotationZAbsolute(&hub->modelling, angle);
    pVeh->UpdateRwFrame();
}

void WheelHub::Initialize()
{
    ModelInfoMgr::RegisterDummy([](CVehicle* pVeh, RwFrame* pFrame)
        {
            VehData& data = xData.Get(pVeh);
            std::string name = GetFrameNodeName(pFrame);

            if (name == "hub_rf") data.m_pHRF = pFrame;
            else if (name == "hub_rm") data.m_pHRM = pFrame;
            else if (name == "hub_rb") data.m_pHRR = pFrame;
            else if (name == "hub_lf") data.m_pHLF = pFrame;
            else if (name == "hub_lm") data.m_pHLM = pFrame;
            else if (name == "hub_lb") data.m_pHLR = pFrame;
        });

    ModelInfoMgr::RegisterRender([](CVehicle* pVeh)
        {
            if (!pVeh || !pVeh->GetIsOnScreen())
                return;

            VehData& data = xData.Get(pVeh);

            ApplySteerAngle(pVeh, data.m_pHRF, false);
            ApplySteerAngle(pVeh, data.m_pHRM, false);
            ApplySteerAngle(pVeh, data.m_pHRR, false);

            ApplySteerAngle(pVeh, data.m_pHLF, true);
            ApplySteerAngle(pVeh, data.m_pHLM, true);
            ApplySteerAngle(pVeh, data.m_pHLR, true);
        });
}
