#include "pch.h"
#include "rotatedoor.h"
#include "datamgr.h"
#include "modelinfomgr.h"

void RotateDoor::UpdateRotatingDoor(CVehicle* pVeh, DoorConfig& config, eDoors doorID)
{
    if (!config.frame) return;

    float ratio = pVeh->GetDooorAngleOpenRatio(doorID);

    float popFactor = std::min(1.0f, ratio * 5.0f);
    float sideMult = (doorID == DOOR_FRONT_LEFT || doorID == DOOR_REAR_LEFT) ? 1.0f : -1.0f;
    config.frame->modelling.pos.x = popFactor * config.popOutAmount * sideMult;

    float targetRot = ratio * config.mul * 90.0f * sideMult;

    MatrixUtil::SetRotationZAbsolute(&config.frame->modelling, targetRot - config.prevRot);
    config.prevRot = targetRot;

    RwMatrixUpdate(&config.frame->modelling);
}

void RotateDoor::Initialize()
{
    ModelInfoMgr::RegisterDummy([](CVehicle* pVeh, RwFrame* pFrame)
    {
        std::string name = GetFrameNodeName(pFrame);
        if (!name.starts_with("x_rd_")) return;

        auto& jsonData = DataMgr::Get(pVeh->m_nModelIndex);
        VehData& data = xData.Get(pVeh);

        float mul = 1.0f;
        float popOutAmount = 0.0f;

        if (jsonData.contains("doors") && jsonData["doors"].contains(name)) {
            mul = jsonData["doors"][name].value("movmul", 1.0f);
            popOutAmount = jsonData["doors"][name].value("popout", 0.15f);
        }

        float orgRot = MatrixUtil::GetRotationZ(&pFrame->modelling);

        if (name == "x_rd_lf ") {
            data.leftFront = { pFrame, orgRot, mul, popOutAmount };
        } else if (name == "x_rd_rf") {
            data.rightFront = { pFrame, orgRot, mul, popOutAmount };
        } else if (name == "x_sd_lr") {
            data.leftRear = { pFrame, orgRot, mul, popOutAmount };
        } else if (name == "x_sd_rr") {
            data.rightRear = { pFrame, orgRot, mul, popOutAmount };
        }
    });

    ModelInfoMgr::RegisterRender([](CVehicle* pVeh)
    {
        if (!pVeh || !pVeh->GetIsOnScreen()) return;

        VehData& data = xData.Get(pVeh);
        UpdateRotatingDoor(pVeh, data.leftFront, eDoors::DOOR_FRONT_LEFT);
        UpdateRotatingDoor(pVeh, data.rightFront, eDoors::DOOR_FRONT_RIGHT);
        UpdateRotatingDoor(pVeh, data.leftRear, eDoors::DOOR_REAR_LEFT);
        UpdateRotatingDoor(pVeh, data.rightRear, eDoors::DOOR_REAR_RIGHT);
    });
}