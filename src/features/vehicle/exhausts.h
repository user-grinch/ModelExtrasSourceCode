#pragma once
#include "plugin.h"
#include <unordered_map>

using namespace plugin;

extern struct ME_ExhaustInfo;

struct ExhaustData {
public:
    RwFrame* pFrame = nullptr;
    std::string sName = "";
    CRGBA Color{ 230, 230, 230, 64 };
    float fSpeedMul = 0.5f;
    float fLifeTime = 0.2f;
    float fSizeMul = 0.5f;
    bool bNitroEffect = true;
    FxSystem_c* pFxSysem = nullptr;
};

struct VehData {
    bool isUsed = false;
    size_t reloadCount = 0;
    std::unordered_map<std::string, ExhaustData> m_pDummies;
    VehData(CVehicle* pVeh) {
        isUsed = false;
    }
    ~VehData() {}
};

class ExhaustFx
{
private:
	static inline bool bEnabled = false;
    static inline size_t nReloadCount = 0;

    static void RenderSmokeFx(CVehicle *pVeh, const ExhaustData &info);
    static void RenderNitroFx(CVehicle* pVeh, float power);

    static ExhaustData LoadData(CVehicle *pVeh, RwFrame *pFrame);

    template<uintptr_t addr>
    static void hkAddExhaustParticles();
    template<uintptr_t addr>
    static void hkDoNitroEffect();

public:
    static inline VehicleExtendedData<VehData> xData;

	static void Initialize();
    static void Reload();
};