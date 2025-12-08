#include "pch.h"
#include "wheel.h"
#include "modelinfomgr.h"

void UpdateWheelRotation(CVehicle *pVeh, RwFrame *ori, RwFrame *tar)
{
    if (ori && tar)
    {
        // MatrixUtil::SetRotationZ(&tar->modelling, MatrixUtil::GetRotationZ(&ori->modelling));
        // MatrixUtil::SetRotationY(&tar->modelling, MatrixUtil::GetRotationY(&ori->modelling));
        MatrixUtil::SetRotationXAbsolute(&tar->modelling, MatrixUtil::GetRotationX(&ori->modelling) - MatrixUtil::GetRotationX(&tar->modelling));
        pVeh->UpdateRwFrame();
    }
}

void ExtraWheel::Initialize()
{
    ModelInfoMgr::RegisterDummy([](CVehicle *pVeh, RwFrame *pFrame)
                                {
        VehData& data = xData.Get(pVeh);
        std::string name = GetFrameNodeName(pFrame);

        if (name == "wheel_rf_dummy") {
            data.pOriginals[static_cast<int>(eWheelPos::RightFront)].push_back(pFrame);
        }
        else if (name == "wheel_rm_dummy") {
            data.pOriginals[static_cast<int>(eWheelPos::RightMiddle)].push_back(pFrame);
        }
        else if (name == "wheel_rr_dummy" || name == "wheel_rb_dummy") {
            data.pOriginals[static_cast<int>(eWheelPos::RightRear)].push_back(pFrame);
        }
        else if (name == "wheel_lf_dummy") {
            data.pOriginals[static_cast<int>(eWheelPos::LeftFront)].push_back(pFrame);
        }
        else if (name == "wheel_lm_dummy") {
            data.pOriginals[static_cast<int>(eWheelPos::LeftMiddle)].push_back(pFrame);
        }
        else if (name == "wheel_lr_dummy" || name == "wheel_lb_dummy") {
            data.pOriginals[static_cast<int>(eWheelPos::LeftRear)].push_back(pFrame);
        }
        
        if (!name.starts_with("x_wheel") || name.length() < 8) {
            return;
        }

        if (name.starts_with("x_wheel_lf")) {
            data.pExtras[static_cast<int>(eWheelPos::LeftFront)].push_back(pFrame);
        }else   if (name.starts_with("x_wheel_lm")) {
            data.pExtras[static_cast<int>(eWheelPos::LeftMiddle)].push_back(pFrame);
        } else   if (name.starts_with("x_wheel_lr")) {
            data.pExtras[static_cast<int>(eWheelPos::LeftRear)].push_back(pFrame);
        } else   if (name.starts_with("x_wheel_rf")) {
            data.pExtras[static_cast<int>(eWheelPos::RightFront)].push_back(pFrame);
        } else   if (name.starts_with("x_wheel_rm")) {
            data.pExtras[static_cast<int>(eWheelPos::RightMiddle)].push_back(pFrame);
        } else   if (name.starts_with("x_wheel_rr")) {
            data.pExtras[static_cast<int>(eWheelPos::RightRear)].push_back(pFrame);
        } });

    ModelInfoMgr::RegisterRender([](CVehicle *pVeh)
                                 {
        if (!pVeh || !pVeh->GetIsOnScreen())
        {
            return;
        }

        VehData &data = xData.Get(pVeh);

        for (int i = 0; i < data.pExtras.size(); i++)
        {
            for (int j = 0; j < data.pExtras[i].size(); j++) {
                UpdateWheelRotation(pVeh, data.pOriginals[i][j], data.pExtras[i][j]);
            }
        } });
}