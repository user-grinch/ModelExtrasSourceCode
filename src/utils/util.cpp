#include "pch.h"
#include "util.h"
#include <regex>
#include <CWeaponInfo.h>
#include <CCamera.h>
#include <CCoronas.h>
#include <CShadows.h>
#include <CBike.h>
#include <CWorld.h>
#include "texmgr.h"
#include "defines.h"
#include "vehicle/core/dummy.h"

void Util::RegisterCorona(CEntity *pEntity, int coronaID, CVector pos, CRGBA col, float size)
{
    if (!gConfig.ReadBoolean("VEHICLE_FEATURES", "LightCoronas", false))
    {
        return;
    }

    CCoronas::RegisterCorona(coronaID, pEntity, col.r, col.g, col.b, col.a, pos,
                             size * CORONA_SZ_MUL, 260.0f, CORONATYPE_SHINYSTAR, FLARETYPE_NONE, false, false, 0, 0.0f, false, 0.3f, 0, 30.0f, false, false);
};

float Util::NormalizeAngle(float angle)
{
    while (angle < 0.0f)
        angle += 360.0f;
    while (angle >= 360.0f)
        angle -= 360.0f;
    return angle;
}

void Util::RegisterCoronaWithAngle(CEntity *pEntity, int coronaID, CVector posn, CRGBA col, float angle, float radius, float size)
{
    constexpr float RAD_TO_DEG = 180.0f / 3.141592653589793f;

    float vehicleAngle = NormalizeAngle(pEntity->GetHeading() * RAD_TO_DEG);
    float cameraAngle = NormalizeAngle(TheCamera.GetHeading() * RAD_TO_DEG);
    float dummyAngle = NormalizeAngle(vehicleAngle + angle);
    float fadeRange = 20.0f;
    float cutoff = (radius / 2.0f);
    float diffAngle = std::fabs(std::fmod(std::fabs(cameraAngle - dummyAngle) + 180.0f, 360.0f) - 180.0f);

    if (diffAngle < cutoff || diffAngle > (360.0f - cutoff))
    {
        return;
    }

    if (diffAngle < cutoff + fadeRange)
    {
        float adjustedAngle = cutoff - diffAngle;
        float mul = std::fabs(adjustedAngle / fadeRange);
        col.a *= mul;
    }
    else if (diffAngle > (360.0f - cutoff - fadeRange))
    {
        float adjustedAngle = fadeRange - (diffAngle - (360.0f - cutoff - fadeRange));
        float mul = std::fabs(adjustedAngle / fadeRange);
        col.a *= mul;
    }

    RegisterCorona(pEntity, coronaID, posn, col, size);
}

extern int gGlobalShadowIntensity;
void Util::RegisterShadow(CEntity *pEntity, CVector position, CRGBA col, float angle, eDummyPos dummyPos, const std::string &shadwTexName, CVector2D shdwSz, CVector2D shdwOffset, RwTexture *pTexture)
{
    if (shdwSz.x == 0.0f || shdwSz.y == 0.0f || !gConfig.ReadBoolean("VEHICLE_FEATURES", "LightShadows", false))
    {
        return;
    }

    float angleRad = DegToRad(angle);
    float fAngle = pEntity->GetHeading() + angleRad;
    CVector vehPos = pEntity->GetPosition();

    CVector up = CVector(-sin(fAngle), cos(fAngle), 0.0f) * shdwSz.y;
    CVector right = CVector(cos(fAngle), sin(fAngle), 0.0f) * shdwSz.x;
    CVector center, nSize = {0.0f, 0.0f, 0.0f};

    if (dummyPos == eDummyPos::Right)
    {
        nSize = {shdwSz.y, 0.0f, 0};
    }
    else if (dummyPos == eDummyPos::Left)
    {
        nSize = {-shdwSz.y, 0.0f, 0};
    }
    else if (dummyPos == eDummyPos::Front)
    {
        nSize = {0.0f, shdwSz.y, 0};
    }
    else if (dummyPos == eDummyPos::Rear)
    {
        nSize = {0.0f, -shdwSz.y, 0};
    }

    // rotation matrix
    //
    // |cos  -sin|
    // |sin   cos|

    CVector nOffset = {shdwOffset.x * cos(angleRad) - shdwOffset.y * sin(angleRad),
                       shdwOffset.x * sin(angleRad) + shdwOffset.y * cos(angleRad),
                       0};
    center = pEntity->TransformFromObjectSpace(position + nOffset + nSize);

    center.z = CWorld::FindGroundZFor3DCoord(center.x, center.y, center.z + 100, nullptr, nullptr) + 2.0f;

    // Fix issues like under bridges
    if (abs(center.z - vehPos.z) > 3.0f)
    {
        center.z = vehPos.z + position.z + 1.0f;
    }

    // Fix heli drawing shadow from sky
    if (abs(vehPos.z - center.z) > 15.0f)
    {
        return;
    }
    RwTexture *pTex = (pTexture != NULL ? pTexture : TextureMgr::Get(shadwTexName, gGlobalShadowIntensity));
    if (pTex) {
        CShadows::StoreShadowToBeRendered(2, pTex, &center,
                                        up.x, up.y,
                                        right.x, right.y,
                                        col.a, col.r, col.g, col.b,
                                        6.0f, false, 1.0f, 0, true);
    }
}

float Util::GetVehicleSpeed(CVehicle *pVeh)
{
    return pVeh->m_vecMoveSpeed.Magnitude2D() * 50.0f;
}

// Taken from vehfuncs
float Util::GetVehicleSpeedRealistic(CVehicle *vehicle)
{
    float wheelSpeed = 0.0;
    CVehicleModelInfo *vehicleModelInfo = (CVehicleModelInfo *)CModelInfo::GetModelInfo(vehicle->m_nModelIndex);
    if (vehicle->m_nVehicleSubClass == VEHICLE_BIKE || vehicle->m_nVehicleSubClass == VEHICLE_BMX)
    {
        CBike *bike = (CBike *)vehicle;
        wheelSpeed = ((bike->m_fWheelSpeed[0] * vehicleModelInfo->m_fWheelSizeFront) +
                      (bike->m_fWheelSpeed[1] * vehicleModelInfo->m_fWheelSizeRear)) /
                     2.0f;
    }
    else if (vehicle->m_nVehicleSubClass == VEHICLE_AUTOMOBILE || vehicle->m_nVehicleSubClass == VEHICLE_MTRUCK || vehicle->m_nVehicleSubClass == VEHICLE_QUAD)
    {
        CAutomobile *automobile = (CAutomobile *)vehicle;
        wheelSpeed = ((automobile->m_fWheelSpeed[0] + automobile->m_fWheelSpeed[1] * vehicleModelInfo->m_fWheelSizeFront) +
                      (automobile->m_fWheelSpeed[2] + automobile->m_fWheelSpeed[3] * vehicleModelInfo->m_fWheelSizeRear)) /
                     4.0f;
    }
    else
    {
        return (Util::GetVehicleSpeed(vehicle)) * 3.6f;
    }
    wheelSpeed /= 2.45f;   // tweak based on distance (manually testing)
    wheelSpeed *= -186.0f; // tweak based on km/h

    return wheelSpeed;
}

unsigned int Util::GetEntityModel(void *ptr, eModelEntityType type)
{
    int model = 0;
    if (type == eModelEntityType::Weapon)
    {
        CWeaponInfo *pWeaponInfo = CWeaponInfo::GetWeaponInfo(reinterpret_cast<CWeapon *>(ptr)->m_eWeaponType,
                                                              FindPlayerPed()->GetWeaponSkill(reinterpret_cast<CWeapon *>(ptr)->m_eWeaponType));
        if (pWeaponInfo)
        {
            model = pWeaponInfo->m_nModelId;
        }
    }
    else
    {
        model = reinterpret_cast<CEntity *>(ptr)->m_nModelIndex;
    }
    return model;
}

void Util::GetModelsFromIni(std::string &line, std::vector<int> &vec)
{
    std::stringstream ss(line);
    while (ss.good())
    {
        std::string model;
        getline(ss, model, ',');
        vec.push_back(std::stoi(model));
    }
}

std::optional<int> Util::GetDigitsAfter(const std::string &str, const std::string &prefix)
{
    if (str.rfind(prefix, 0) == 0)
    {
        std::string numberPart = str.substr(prefix.size());
        if (!numberPart.empty() &&
            std::all_of(numberPart.begin(), numberPart.end(), [](char c)
                        { return std::isdigit(c) || c == '.'; }) &&
            std::count(numberPart.begin(), numberPart.end(), '.') <= 1)
        {
            try
            {
                return std::stoi(numberPart);
            }
            catch (...)
            {
                return std::nullopt;
            }
        }
    }
    return std::nullopt;
}

std::optional<std::string> Util::GetCharsAfterPrefix(const std::string &str, const std::string &prefix, size_t num_chars)
{
    if (str.size() > prefix.size() && str.substr(0, prefix.size()) == prefix)
    {
        std::string suffix = str.substr(prefix.size(), num_chars);
        std::transform(suffix.begin(), suffix.end(), suffix.begin(), ::toupper);
        return suffix;
    }
    return std::nullopt;
}
