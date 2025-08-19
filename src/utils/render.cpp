#include "pch.h"
#include "render.h"
#include "util.h"

#include <CCoronas.h>
#include <CShadows.h>
#include <CBike.h>
#include <CWorld.h>
#include "texmgr.h"
#include "defines.h"
#include "vehicle/core/dummy.h"

void RenderUtil::RegisterCorona(CEntity *pEntity, int coronaID, CVector pos, CRGBA col, float size)
{
    if (!gConfig.ReadBoolean("VEHICLE_FEATURES", "LightCoronas", false))
    {
        return;
    }

    CCoronas::RegisterCorona(coronaID, pEntity, col.r, col.g, col.b, col.a, pos,
                             size * CORONA_SZ_MUL, 260.0f, CORONATYPE_SHINYSTAR, FLARETYPE_NONE, false, false, 0, 0.0f, false, 0.3f, 0, 30.0f, false, false);
};

void RenderUtil::RegisterCoronaWithAngle(CEntity *pEntity, int coronaID, CVector posn, CRGBA col, float angle, float radius, float size)
{
    constexpr float RAD_TO_DEG = 180.0f / 3.141592653589793f;

    float vehicleAngle = Util::NormalizeAngle(pEntity->GetHeading() * RAD_TO_DEG);
    float cameraAngle = Util::NormalizeAngle(TheCamera.GetHeading() * RAD_TO_DEG);
    float dummyAngle = Util::NormalizeAngle(vehicleAngle + angle);
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

#include "plugin.h"
#include "CEntity.h"
#include "CVehicle.h"
#include "CPed.h"
#include "CVector.h"
#include <cmath>

inline CVector2D GetPerpRight(const CVector2D& vec) {
    return { vec.y, -vec.x };
}

inline CVector2D Rotate2D(const CVector2D& vec, float angle) {
    float cosA = cos(angle);
    float sinA = sin(angle);
    return CVector2D(
        vec.x * cosA - vec.y * sinA,
        vec.x * sinA + vec.y * cosA
    );
}

inline float DotProduct(const CVector& a, const CVector& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

#define PI      3.14159265358979323846f
#define HALF_PI PI * 0.5f

bool IsShadowTowardVehicle(CMatrix* dummyMatrix, CVector vehicleCenter) {
    // Dummy world position
    CVector dummyPos = dummyMatrix->pos;

    // Shadow direction (assume 'up' is forward in dummy frame)
    CVector shadowDir = dummyMatrix->up;
    shadowDir.z = 0.0f;
    shadowDir.Normalise();

    // Vector from dummy to vehicle center
    CVector toVehicle = vehicleCenter - dummyPos;
    toVehicle.z = 0.0f;
    toVehicle.Normalise();

    // If dot > 0, shadow is cast toward vehicle
    return DotProduct(shadowDir, toVehicle) > 0.0f;
}

void RotateMatrix180Z(CMatrix& mat) {
    // Flip X and Y of right and up vectors
    mat.right.x = -mat.right.x;
    mat.right.y = -mat.right.y;

    mat.up.x = -mat.up.x;
    mat.up.y = -mat.up.y;

    // forward stays unchanged (Z axis)
}

void RenderUtil::RegisterShadowNew(CVehicle* pVeh, VehicleDummyConfig* pConfig, const std::string& shadwTexName, float shdwSz)
{
    if (!pVeh || !pConfig || shdwSz == 0.0f || !gConfig.ReadBoolean("VEHICLE_FEATURES", "LightShadows", false)) {
        return;
    }

    float heading = pVeh->GetHeading();
    CMatrix mat = *(CMatrix*)&pConfig->frame->ltm;
    if (IsShadowTowardVehicle((CMatrix*)&pConfig->frame->ltm, pVeh->GetPosition())) {
        RotateMatrix180Z(mat);
    }

    // Dummy offset in local space
    CMatrix vehMat = *(CMatrix*)pVeh->GetMatrix();
    CVector worldOffset = mat.pos - vehMat.pos; // world-space vector from vehicle to dummy

    // Apply inverse rotation manually
    CVector dummyOffset;
    dummyOffset.x = DotProduct(worldOffset, vehMat.right);
    dummyOffset.y = DotProduct(worldOffset, vehMat.up);
    dummyOffset.z = DotProduct(worldOffset, vehMat.at);

    if (pConfig->mirroredX) {
        dummyOffset.x *= -1.0f;
    }

    // Light direction from dummy (forward vector)
    CVector lightDir = mat.up; // up is forward in psdk
    lightDir.z = 0.0f;
    lightDir.Normalise();

    CVector rotatedLightDir = lightDir;
    // CVector rotatedLightDir(
    //     lightDir.x * cos(heading) - lightDir.y * sin(heading),
    //     lightDir.x * sin(heading) + lightDir.y * cos(heading),
    //     0.0f
    // );

    // Rotate dummy offset into world space
    CVector2D localOffset(dummyOffset.x, dummyOffset.y);
    CVector2D rotatedOffset = Rotate2D(localOffset, heading);

    // Push shadow forward along light direction
    rotatedOffset += CVector(rotatedLightDir.x, rotatedLightDir.y, 0.0f) * (shdwSz * 2.0f + 1.0f);

    CVector2D shdwFront(rotatedLightDir.x * (shdwSz * 2.0f), rotatedLightDir.y * (shdwSz * 2.0f));
    CVector2D perpVec(rotatedLightDir.x * shdwSz, rotatedLightDir.y * shdwSz);
    CVector2D shdwSide = GetPerpRight(perpVec);

    RwTexture* pTex = TextureMgr::Get(shadwTexName, gGlobalShadowIntensity);
    if (!pTex) {
        return;
    }

    CVector shdwPos = pVeh->GetPosition() + CVector(rotatedOffset.x, rotatedOffset.y, 2.0f);
    CShadows::StoreCarLightShadow(
        pVeh,
        reinterpret_cast<int32_t>(&pConfig->frame->modelling) + 2,
        pTex,
        &shdwPos,
        shdwFront.x, shdwFront.y,
        shdwSide.x,  shdwSide.y,
        pConfig->shadow.color.r,
        pConfig->shadow.color.g,
        pConfig->shadow.color.b,
        7.0f
    );
}

void RenderUtil::RegisterShadow(CEntity* pEntity, CVector position, CRGBA col, float angle,
                                eDummyPos dummyPos, const std::string& shadwTexName,
                                CVector2D shdwSz, CVector2D shdwOffset, RwTexture* pTexture) {
    // if (!pEntity || shdwSz.x == 0.0f || shdwSz.y == 0.0f ||
    //     !gConfig.ReadBoolean("VEHICLE_FEATURES", "LightShadows", false))
    // {
    //     return;
    // }

    // const CVector vehiclePos = pEntity->GetPosition();
    // const CVector dPos = pEntity->TransformFromObjectSpace(position);

    // // Direction from dummy to vehicle (2D)
    // CVector2D dirToVehicle = CVector2D(vehiclePos - dPos).Normalized();
    // CVector2D shadowDir = -dirToVehicle; // Flip to point away

    // // Shadow rectangle vectors
    // CVector2D shdwFront = shadowDir * shdwSz.y;
    // CVector2D shdwSide  = shadowDir.GetPerpRight() * shdwSz.x;

    // // Shadow position offset (in front of dummy)
    // CVector shdwPos = dummyPos + CVector(shadowDir * (shdwSz.y * 0.5f), 2.0f);

    // // Optional: clamp to ground
    // shdwPos.z = CWorld::FindGroundZFor3DCoord(shdwPos.x, shdwPos.y, shdwPos.z + 100.0f, nullptr, &pEntity) + 2.0f;

    // if (fabs(shdwPos.z - vehiclePos.z) > 15.0f)
    //     return;

    // RwTexture* pTex = pTexture ? pTexture : TextureMgr::Get(shadwTexName, gGlobalShadowIntensity);

    // if (pTex)
    // {
    //     CShadows::StoreShadowToBeRendered(2, pTex, &shdwPos,
    //                                       shdwFront.x, shdwFront.y,
    //                                       shdwSide.x,  shdwSide.y,
    //                                       col.a, col.r, col.g, col.b,
    //                                       6.0f, false, 1.0f, 0, true);
    // }
}