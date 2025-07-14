#include "pch.h"
#include "dummy.h"
#include "defines.h"
#include "datamgr.h"
#include "enums/dummypos.h"
#include <CWorld.h>

extern float gfGlobalCoronaSize;
extern int gGlobalCoronaIntensity;
extern int gGlobalShadowIntensity;

int ReadHex(char a, char b)
{
    a = (a <= '9') ? a - '0' : (a & 0x7) + 9;
    b = (b <= '9') ? b - '0' : (b & 0x7) + 9;

    return (a << 4) + b;
}

VehicleDummy::VehicleDummy(const VehicleDummyConfig& config)
{
    data = config;
    float angleVal = 0.0f;

        // Calculate the angle based on the frame's orientation
    float modelAngle = Util::NormalizeAngle(
        CGeneral::GetATanOfXY(data.frame->modelling.right.x, data.frame->modelling.right.y) * 57.295776f
    );

    switch (static_cast<int>(modelAngle)) {
        // Keep this disabled
        // case 0:   data.dummyType = eDummyPos::Front; break;
        case 90:  data.dummyType = eDummyPos::Left;  break;
        case 180: data.dummyType = eDummyPos::Rear;  break;
        case 270: data.dummyType = eDummyPos::Right; break;
        default: break;
    }

    auto &jsonData = DataMgr::Get(data.pVeh->m_nModelIndex);
    std::string name = GetFrameNodeName(data.frame);
  
    if (jsonData.contains("lights"))
    {
        std::string newName = name.substr(0, name.find("_prm"));
        if (jsonData["lights"].contains(newName.c_str()))
        {
            auto &lights = jsonData["lights"][newName.c_str()];

            if (lights.contains("corona"))
            {
                auto &coronaSec = lights["corona"];
                if (coronaSec.contains("color"))
                {
                    data.corona.color.r = coronaSec["color"].value("red", data.corona.color.r);
                    data.corona.color.g = coronaSec["color"].value("green", data.corona.color.g);
                    data.corona.color.b = coronaSec["color"].value("blue", data.corona.color.b);
                    data.corona.color.a = coronaSec["color"].value("alpha", gGlobalCoronaIntensity);
                }
                data.corona.size = coronaSec.value("size", gfGlobalCoronaSize);
                data.corona.lightingType = GetLightingMode(coronaSec.value("type", "directional"));
            }

            if (lights.contains("shadow"))
            {
                auto &shadow = lights["shadow"];
                if (shadow.contains("color"))
                {
                    data.shadow.color.r = shadow["color"].value("red", data.shadow.color.r);
                    data.shadow.color.g = shadow["color"].value("green", data.shadow.color.g);
                    data.shadow.color.b = shadow["color"].value("blue", data.shadow.color.b);
                    data.shadow.color.a = shadow["color"].value("alpha", gGlobalShadowIntensity);
                }
                data.shadow.offset = {shadow.value("offsetx", 0.0f), shadow.value("offsety", 0.0f)};

                // This needs to be like this
                data.shadow.size = {shadow.value("width", 1.0f), shadow.value("length", 1.0f)};
                angleVal = shadow.value("angleoffset", 0.0f);
                data.shadow.texture = shadow.value("texture", "");

                // shadows will be force enabled if there is JSON data for it.
                data.shadow.render = true;
            }

            // Only for StrobeLights
            if (lights.contains("strobedelay"))
            {
                data.strobe.delay = lights.value("strobedelay", 1000);
            }
        }
    }
    else
    {
        // Legacy support for ImVehFt vehicles
        size_t prmPos = name.find("_prm");
        if (prmPos != std::string::npos)
        {
            if (prmPos + 9 < name.size())
            {
                data.shadow.color.r = data.corona.color.r = ReadHex(name[prmPos + 4], name[prmPos + 5]);
                data.shadow.color.g = data.corona.color.g = ReadHex(name[prmPos + 6], name[prmPos + 7]);
                data.shadow.color.b = data.corona.color.b = ReadHex(name[prmPos + 8], name[prmPos + 9]);
            }
            else
            {
                LOG_VERBOSE("Model {} has issue with node `{}`: invalid color format", data.pVeh->m_nModelIndex, name);
            }

            if (prmPos + 10 < name.size())
            {
                data.corona.lightingType = static_cast<eLightingMode>(name[prmPos + 10] - '0');
            }
            else
            {
                LOG_VERBOSE("Model {} has issue with node `{}`: invalid light type", data.pVeh->m_nModelIndex, name);
            }

            if (prmPos + 11 < name.size())
            {
                data.corona.size = static_cast<float>(name[prmPos + 11] - '0') / 10.0f;
                if (data.corona.size < 0.0f)
                {
                    data.corona.size = 0.0f;
                }
            }
            else
            {
                LOG_VERBOSE("Model {} has issue with node `{}`: invalid corona size", data.pVeh->m_nModelIndex, name);
            }

            if (prmPos + 12 < name.size())
            {
                float shadowValue = static_cast<float>(name[prmPos + 12] - '0') / 7.5f;
                data.shadow.size = {shadowValue, shadowValue};
                if (data.shadow.size.x < 0.0f || data.shadow.size.y < 0.0f)
                {
                    data.shadow.size = {0.0f, 0.0f};
                }
            }
            else
            {
                LOG_VERBOSE("Model {} has issue with node `{}`: invalid shadow size", data.pVeh->m_nModelIndex, name);
            }
        }
    }

    switch (data.dummyType) {
        case eDummyPos::Front:
            data.rotation.angle = 0.0f - angleVal;
            break;
        case eDummyPos::Left:
            data.rotation.angle = 90.0f - angleVal;
            break;
        case eDummyPos::Right:
            data.rotation.angle = 270.0f - angleVal;
            break;
        case eDummyPos::Rear:
            data.rotation.angle = 180.0f - angleVal;
            break;
        case eDummyPos::None:
        default:
            data.rotation.angle = modelAngle;
            break;
    }
}

float CLAMP_OFFSET_X = 0.25f;
float CLAMP_OFFSET_Y = 0.25f;

void VehicleDummy::Update()
{
    CMatrix &vehMatrix = *(CMatrix *)data.pVeh->GetMatrix();
    CVector pos = data.pVeh->GetPosition();
    CVector dummyPos = data.frame->ltm.pos;
    CVector offset = dummyPos - pos;

    // Transform to local space using  transpose of the rotation matrix
    data.shadow.position.x = data.position.x = vehMatrix.right.x * offset.x + vehMatrix.right.y * offset.y + vehMatrix.right.z * offset.z;
    data.shadow.position.y = data.position.y = vehMatrix.up.x * offset.x + vehMatrix.up.y * offset.y + vehMatrix.up.z * offset.z;
    data.shadow.position.z = data.position.z = vehMatrix.at.x * offset.x + vehMatrix.at.y * offset.y + vehMatrix.at.z * offset.z;

    CVehicleModelInfo* pInfo = (CVehicleModelInfo*)CModelInfo::GetModelInfo(data.pVeh->m_nModelIndex);
    CVector min = pInfo->m_pColModel->m_boundBox.m_vecMin;
    CVector max = pInfo->m_pColModel->m_boundBox.m_vecMax;

    if (data.dummyType == eDummyPos::Front) {
        data.shadow.position.y = std::max(data.shadow.position.y, max.y + CLAMP_OFFSET_Y);
    }
    else if (data.dummyType == eDummyPos::Rear) {
        data.shadow.position.y = std::min(data.shadow.position.y, min.y - CLAMP_OFFSET_Y);
    } else if (data.dummyType == eDummyPos::Left) {
        data.shadow.position.x = std::min(data.shadow.position.x, min.x - CLAMP_OFFSET_X);
    }
    else if (data.dummyType == eDummyPos::Right) {
        data.shadow.position.x = std::max(data.shadow.position.x, max.x + CLAMP_OFFSET_X);
    }

    if (data.mirroredX)
    {
        data.position.x *= -1;
        data.shadow.position.x *= -1;
    }
}