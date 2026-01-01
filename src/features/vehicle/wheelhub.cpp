#include "pch.h"
#include "wheelhub.h"
#include "modelinfomgr.h"

void UpdateRotation(CVehicle *pVeh, RwFrame *ori, RwFrame *tar, bool left = false)
{
    if (ori && tar)
    {
        double oriRot = MatrixUtil::GetRotationZ(&ori->modelling);
        double tarRot = MatrixUtil::GetRotationZ(&tar->modelling);
        MatrixUtil::SetRotationZAbsolute(&tar->modelling, (oriRot - tarRot));
        tar->modelling.pos.z = ori->modelling.pos.z;
        pVeh->UpdateRwFrame();
    }
}

void WheelHub::Initialize()
{
    ModelInfoMgr::RegisterDummy([](CVehicle *pVeh, RwFrame *pFrame)
    {
        VehData& data = xData.Get(pVeh);
        std::string name = GetFrameNodeName(pFrame);
        if (name == "wheel_rf_dummy") data.m_pWRF = pFrame;
        else if (name == "wheel_rm_dummy") data.m_pWRM = pFrame;
        else if (name == "wheel_rr_dummy" || name == "wheel_rb_dummy") data.m_pWRR = pFrame;
        else if (name == "wheel_lf_dummy") data.m_pWLF = pFrame;
        else if (name == "wheel_lm_dummy") data.m_pWLM = pFrame;
        else if (name == "wheel_lr_dummy" || name == "wheel_lb_dummy") data.m_pWLR = pFrame;
        else if (name == "hub_rf") data.m_pHRF = pFrame;
        else if (name == "hub_rm") data.m_pHRM = pFrame;
        else if (name == "hub_rb") data.m_pHRR = pFrame;
        else if (name == "hub_lf") data.m_pHLF = pFrame;
        else if (name == "hub_lm") data.m_pHLM = pFrame;
        else if (name == "hub_lb") data.m_pHLR = pFrame; 
    });

    ModelInfoMgr::RegisterRender([](CVehicle *pVeh)
    {
        if (!pVeh || !pVeh->GetIsOnScreen())
        {
            return;
        }

        VehData& data = xData.Get(pVeh);
        UpdateRotation(pVeh, data.m_pWRF, data.m_pHRF);
        UpdateRotation(pVeh, data.m_pWRM, data.m_pHRM);
        UpdateRotation(pVeh, data.m_pWRR, data.m_pHRR);
        UpdateRotation(pVeh, data.m_pWRF, data.m_pHLF, true);
        UpdateRotation(pVeh, data.m_pWRM, data.m_pHLM, true);
        UpdateRotation(pVeh, data.m_pWRR, data.m_pHLR, true); 
    });
}