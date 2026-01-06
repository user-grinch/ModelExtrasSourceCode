#include "pch.h"
#include "SlideDoor.h"
#include "datamgr.h"
#include "modelinfomgr.h"

void SlideDoor::UpdateSlidingDoor(CVehicle* pVeh, DoorConfig& config, eDoors doorID)
{
    if (!config.frame) return;

    float sideMult = (doorID == DOOR_FRONT_LEFT || doorID == DOOR_REAR_LEFT) ? -1.0f : 1.0f;

    float ratio = pVeh->GetDooorAngleOpenRatio(doorID);
    config.frame->modelling.pos.y = config.mul * ratio * -1;

    float popFactor = std::min(1.0f, ratio * 5.0f);
    config.frame->modelling.pos.x = popFactor * config.popOutAmount * sideMult;
    RwMatrixUpdate(&config.frame->modelling);
}

void SlideDoor::Initialize()
{
    ModelInfoMgr::RegisterDummy([](CVehicle* pVeh, RwFrame* pFrame)
    {
        std::string name = GetFrameNodeName(pFrame);
        if (!name.starts_with("dvan_") && !name.starts_with("dmbus_") && !name.starts_with("x_sd_")) return;

        auto& jsonData = DataMgr::Get(pVeh->m_nModelIndex);
        VehData& data = xData.Get(pVeh);

        float mul = 1.0f;
        float popOutAmount = 0.0f;

        if (jsonData.contains("doors") && jsonData["doors"].contains(name)) {
            mul = jsonData["doors"][name].value("movmul", 0.0f);
            popOutAmount = jsonData["doors"][name].value("popout", 0.15f);
        }

        if (name == "dvan_l" || name == "dmbus_l" || name == "x_sd_lf") {
            data.leftFront = { pFrame, mul, popOutAmount };
        } else if (name == "dvan_r" || name == "dmbus_r" || name == "x_sd_rf") {
            data.rightFront = { pFrame, mul, popOutAmount };
        } else if (name == "x_sd_lr") {
            data.leftRear = { pFrame, mul, popOutAmount };
        } else if (name == "x_sd_rr") {
            data.rightRear = { pFrame, mul, popOutAmount };
        }
    });

    ModelInfoMgr::RegisterRender([](CVehicle* pVeh)
    {
        if (!pVeh || !pVeh->GetIsOnScreen()) return;

        VehData& data = xData.Get(pVeh);
        UpdateSlidingDoor(pVeh, data.leftFront, eDoors::DOOR_FRONT_LEFT);
        UpdateSlidingDoor(pVeh, data.rightFront, eDoors::DOOR_FRONT_RIGHT);
        UpdateSlidingDoor(pVeh, data.leftRear, eDoors::DOOR_REAR_LEFT);
        UpdateSlidingDoor(pVeh, data.rightRear, eDoors::DOOR_REAR_RIGHT);
    });
}
