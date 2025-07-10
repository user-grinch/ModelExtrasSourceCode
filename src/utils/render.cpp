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
void RenderUtil::RegisterShadow(CEntity *pEntity, CVector position, CRGBA col, float angle, eDummyPos dummyPos, const std::string &shadwTexName, CVector2D shdwSz, CVector2D shdwOffset, RwTexture *pTexture)
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