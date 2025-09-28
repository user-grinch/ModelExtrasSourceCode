#include "pch.h"
#include "exhausts.h"
#include <CWorld.h>
#include <CCamera.h>
#include <CGeneral.h>
#include <CWaterLevel.h>
#include <Fx_c.h>

#include "modelinfomgr.h"
#include "datamgr.h"

#define NODE_NAME "x_exhaust"

template<uintptr_t addr>
void ExhaustFx::hkAddExhaustParticles() {
	using func_hook = injector::function_hooker_thiscall<injector::scoped_call, addr, void(CVehicle *)>;
	injector::make_static_hook<func_hook>([](func_hook::func_type ogFunc, CVehicle *pVeh) {
		auto &data = xData.Get(pVeh); 
        if (!data.isUsed) {
            ogFunc(pVeh);
        }
	});
}

template<uintptr_t addr>
void ExhaustFx::hkDoNitroEffect() {
    using func_hook = injector::function_hooker_thiscall<injector::scoped_call, addr, char(CAutomobile*, float)>;
    injector::make_static_hook<func_hook>([](func_hook::func_type ogFunc, CAutomobile* pVeh, float power) {
        auto& data = xData.Get(pVeh);
        if (data.isUsed) {
			RenderNitroFx(pVeh, power);
        }
        else {
            ogFunc(pVeh, power);
        }
        return 1;
    });
}

void ExhaustFx::Initialize() {
    m_bEnabled = true;

    ModelInfoMgr::RegisterDummy([](CVehicle *pVeh, RwFrame *pFrame) { 
        std::string name = GetFrameNodeName(pFrame);
        if (name.starts_with(NODE_NAME)) {
            auto &data = xData.Get(pVeh); 
            data.isUsed = true;
            data.m_pDummies[std::move(name)] = std::move(LoadData(pVeh, pFrame));
        }
    });

    ModelInfoMgr::RegisterRender([](CVehicle *pVeh) {
        if (!pVeh || !pVeh->GetIsOnScreen()) {
            return;
        }

        VehData &data = xData.Get(pVeh); 
        for (auto& e : data.m_pDummies) {
            RenderSmokeFx(pVeh, e.second);
        }
    });

    hkAddExhaustParticles<0x6AB344>();
    hkAddExhaustParticles<0x6BD3FF>();
    hkDoNitroEffect<0x6A405A>();
    hkDoNitroEffect<0x6A406B>();
    hkDoNitroEffect<0x6A40E1>();
}

ExhaustFx::FrameData ExhaustFx::LoadData(CVehicle *pVeh, RwFrame *pFrame) {
    FrameData f;
    f.name = GetFrameNodeName(pFrame);
    f.pFrame = pFrame;
    
    auto &jsonData = DataMgr::Get(pVeh->m_nModelIndex);
    if (jsonData.contains("exhausts") && jsonData["exhausts"].contains(f.name)) {
        auto& data = jsonData["exhausts"][f.name];
        f.lifetime = data.value("lifetime", f.lifetime);
        f.speedMul *= data.value("speed", 1.0f);
        f.sizeMul = data.value("size", f.sizeMul);
        f.supportsNitro = data.value("nitro_effect", f.supportsNitro);

        if (data.contains("color")) {
            f.col.r = data["color"].value("red", f.col.r);
            f.col.g = data["color"].value("green", f.col.g);
            f.col.b = data["color"].value("blue", f.col.b);
            f.col.a = data["color"].value("alpha", f.col.a);
        }
    }

    return f;
}

void ExhaustFx::RenderSmokeFx(CVehicle* pVeh, const FrameData &info) {
    if (!pVeh || !pVeh->GetIsOnScreen()) {
        return;
    }

    float dist = DistanceBetweenPoints(pVeh->GetPosition(), TheCamera.GetPosition());
    dist *= dist;

    if (dist > 256.0f || (dist > 64.0f && !((CTimer::m_FrameCounter + pVeh->m_nModelIndex) & 1))) {
        return;
    }

    CVector exhaustPos = info.pFrame->ltm.pos;
    if (exhaustPos.IsZero()) {
        return;
    }

    auto& data = xData.Get(pVeh);
    if (data.reloadCount < reloadCount) {
        for (auto& e : data.m_pDummies) {
            e.second = LoadData(pVeh, e.second.pFrame);
        }
        data.reloadCount++;
    }

    // properties
    float moveSpeed = pVeh->m_vecMoveSpeed.Magnitude() * info.speedMul;
    float life      = std::max(info.lifetime - moveSpeed, 0.0f);
    float alpha = std::max(info.col.a / 255.0f - moveSpeed, 0.0f);

    CVector particleDir = info.pFrame->ltm.up; // forward is up in psdk
    particleDir.x *= -1;
    particleDir.y *= -1;

    CVector parVelocity;
    if (DotProduct(particleDir, pVeh->m_vecMoveSpeed) >= 0.05f) {
        parVelocity = pVeh->m_vecMoveSpeed * 30.0f;
    } else {
        static float randomFactor = CGeneral::GetRandomNumberInRange(-1.8f, -0.9f);
        parVelocity = randomFactor * particleDir;
    }

    bool isExhaustSubmerged = false;
    float waterLevel = 0.0f;
    if (pVeh->bTouchingWater &&
        CWaterLevel::GetWaterLevel(exhaustPos.x, exhaustPos.y, exhaustPos.z, &waterLevel, true, nullptr) &&
        waterLevel >= exhaustPos.z) {
        isExhaustSubmerged = true;
    }

    float randomFactor = CGeneral::GetRandomNumberInRange(1.0f, 3.0f);
    if (randomFactor * (pVeh->m_fGasPedal + 1.1f) <= 2.5f) {
        return;
    }

    FxPrtMult_c fxPrt(info.col.r / 255.0f, info.col.g / 255.0f, info.col.b / 255.0f, alpha, 0.2f * info.sizeMul, 1.0f, life);

    for (int i = 0; i < 2; i++) {
        FxSystem_c* fxSystem = isExhaustSubmerged ? g_fx.m_pPrtBubble : g_fx.m_pPrtSmokeII3expand;

        if (isExhaustSubmerged) {
            fxPrt.m_color.alpha = alpha * 0.5f;
            fxPrt.m_fSize       = 0.6f * info.sizeMul;
        }

        fxSystem->AddParticle(
            (RwV3d*)&exhaustPos,
            (RwV3d*)&parVelocity,
            0.0f,
            &fxPrt,
            -1.0f,
            pVeh->m_fContactSurfaceBrightness,
            0.6f,
            0
        );

        // secondary emission
        if (pVeh->m_fGasPedal > 0.5f && pVeh->m_nCurrentGear < 3 && (CGeneral::GetRandomNumber() % 2)) {
            FxSystem_c* secondaryFxSystem = isExhaustSubmerged ? g_fx.m_pPrtBubble : g_fx.m_pPrtSmokeII3expand;

            if (isExhaustSubmerged) {
                fxPrt.m_color.alpha = alpha * 0.5f;
                fxPrt.m_fSize       = 0.6f * info.sizeMul;
            }

            secondaryFxSystem->AddParticle(
                (RwV3d*)&exhaustPos,
                (RwV3d*)&parVelocity,
                0.0f,
                &fxPrt,
                -1.0f,
                pVeh->m_fContactSurfaceBrightness,
                0.6f,
                0
            );
        }
    }
}

void ExhaustFx::RenderNitroFx(CVehicle *pVeh, float power) {
	const auto& mi = CModelInfo::GetModelInfo(pVeh->m_nModelIndex);

    auto& data = xData.Get(pVeh);

    if (!data.isUsed) {
        return;
    }

    for (auto& e : data.m_pDummies) {
        if (!e.second.supportsNitro) {
            continue;
        }

        RwMatrix* dummyMatrix = &e.second.pFrame->ltm;

        bool isExhaustSubmerged = false;
        if (pVeh->bTouchingWater) {
            float level = 0.0f;
            CVector pos = dummyMatrix->pos;
            if (CWaterLevel::GetWaterLevel(pos.x, pos.y, pos.z, &level, true, nullptr)) {
                if (level >= pos.z) {
                    isExhaustSubmerged = true;
                }
            }
        }

        if (e.second.fxSystem) {
            e.second.fxSystem->SetConstTime(1, std::fabs(power));
            if (e.second.fxSystem->m_nPlayStatus == eFxSystemPlayStatus::FX_PLAYING && isExhaustSubmerged) {
                e.second.fxSystem->Stop();
            }
            else if (e.second.fxSystem->m_nPlayStatus == eFxSystemPlayStatus::FX_STOPPED && !isExhaustSubmerged) {
                e.second.fxSystem->Play();
            }
        }
        else if (!isExhaustSubmerged && dummyMatrix) {
            static RwMatrixTag gFlipForward = {
                { 1.0f, 0.0f, 0.0f },   // right (X)
                0,                       // flags
                { 0.0f, -1.0f, 0.0f },   // up (forward flipped)
                0,
                { 0.0f, 0.0f, 1.0f },  // at 
                0,
                { 0.0f, 0.0f, 0.0f },   // pos
                0
            };

            e.second.fxSystem = g_fxMan.CreateFxSystem((char*)"nitro", &gFlipForward, dummyMatrix, true);
            if (e.second.fxSystem) {
                e.second.fxSystem->SetLocalParticles(true);
                e.second.fxSystem->Play();
            }
        }
    }
}

void ExhaustFx::Reload() {
    reloadCount++;
}