#include "pch.h"
#include "steerwheel.h"
#include "modelinfomgr.h"

void SteerWheel::Initialize()
{
    ModelInfoMgr::RegisterDummy([](CVehicle *pVeh, RwFrame *pFrame) {
		if (gbVehIKInstalled) {
            return;
		}
        auto& data = xData.Get(pVeh);

        // VehFuncs
        std::string name = GetFrameNodeName(pFrame);
        if (name.starts_with("f_steer")) {
            if (name.length() >= 7 && isdigit(name[7]))
            {
                data.factor = (float)std::stoi(&name[7]) / 2;
            }
            data.pFrame = pFrame;
        } else if (name.starts_with("movsteer") || name.starts_with("steering_dummy") || name.starts_with("ik_steer")) {
            data.pFrame = pFrame;
        } 
    });

    ModelInfoMgr::RegisterRender([](CVehicle *pVeh) {
        if (gbVehIKInstalled || !pVeh || !pVeh->GetIsOnScreen())
        {
            return;
        }

        VehData &data = xData.Get(pVeh); 
        if (!data.pFrame) {
            return;
        }

        float angle = Util::RadToDeg(pVeh->m_fSteerAngle);
        if (std::abs(angle) > 1.0f) {
            MatrixUtil::SetRotationYAbsolute(&data.pFrame->modelling, (angle - data.prevAngle) * data.factor); 
            data.prevAngle = angle;
        }
    });
}