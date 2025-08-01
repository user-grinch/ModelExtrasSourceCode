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
void RenderUtil::RegisterShadow(CEntity* pEntity, CVector position, CRGBA col, float angle,
                                eDummyPos dummyPos, const std::string& shadwTexName,
                                CVector2D shdwSz, CVector2D shdwOffset, RwTexture* pTexture)
{
    if (shdwSz.x == 0.0f || shdwSz.y == 0.0f ||
        !gConfig.ReadBoolean("VEHICLE_FEATURES", "LightShadows", false))
    {
        return;
    }

    const float angleRad = DegToRad(angle);
    const CVector vehPos = pEntity->GetPosition();
    const CMatrix& entityMatrix = *(CMatrix*)pEntity->m_matrix;

    auto RotateVector2D = [angleRad](const CVector& v) -> CVector {
        return {
            v.x * cos(angleRad) - v.y * sin(angleRad),
            v.x * sin(angleRad) + v.y * cos(angleRad),
            v.z
        };
    };

    CVector up    = RotateVector2D(entityMatrix.up * shdwSz.y);
    CVector right = RotateVector2D(entityMatrix.right * shdwSz.x);

    CVector nSize = {0.0f, 0.0f, 0.0f};
    switch (dummyPos)
    {
        case eDummyPos::Right: nSize = { shdwSz.y,  0.0f, 0.0f }; break;
        case eDummyPos::Left:  nSize = {-shdwSz.y,  0.0f, 0.0f }; break;
        case eDummyPos::Front: nSize = { 0.0f,      shdwSz.y, 0.0f }; break;
        case eDummyPos::Rear:  nSize = { 0.0f,     -shdwSz.y, 0.0f }; break;
        default: break;
    }

    CVector nOffset = {
        shdwOffset.x * cos(angleRad) - shdwOffset.y * sin(angleRad),
        shdwOffset.x * sin(angleRad) + shdwOffset.y * cos(angleRad),
        0.0f
    };

    CVector shdwPos = pEntity->TransformFromObjectSpace(position + nOffset + nSize);

    const float zDiff = abs(shdwPos.z - vehPos.z);
    if (zDiff > 3.0f)
        shdwPos.z = vehPos.z + position.z + 1.0f;

    if (abs(vehPos.z - shdwPos.z) > 15.0f)
        return;

    RwTexture* pTex = (pTexture != nullptr)
        ? pTexture
        : TextureMgr::Get(shadwTexName, gGlobalShadowIntensity);

    if (pTex)
    {
        CShadows::StoreShadowToBeRendered(2, pTex, &shdwPos,
                                          up.x, up.y,
                                          right.x, right.y,
                                          col.a, col.r, col.g, col.b,
                                          6.0f, false, 1.0f, 0, true);
    }
}