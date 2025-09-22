#pragma once
#include "plugin.h"
#include <unordered_map>

using namespace plugin;

class ExhaustFx
{
private:
	static inline bool m_bEnabled = false;
    static inline size_t reloadCount = 0;

    struct FrameData {
        RwFrame *pFrame = nullptr;
        std::string name;
        
        CRGBA col {230, 230, 230, 64};
        float speedMul = 0.5f;
        float lifetime = 0.2f;
        float sizeMul = 1.0f;
    };

    struct VehData {
        bool isUsed = false;
        size_t reloadCount = 0;
        std::unordered_map<std::string, FrameData> m_pDummies;
        VehData(CVehicle *pVeh) {
            isUsed = false;
        }
        ~VehData() {}
    };

    static void RenderParticles(CVehicle *pVeh, const FrameData &info);
    static FrameData LoadData(CVehicle *pVeh, RwFrame *pFrame);
    static inline VehicleExtendedData<VehData> xData;

    template<uintptr_t addr>
    static void hkAddExhaustParticles();

public:

	static void Initialize();
    static void Reload();
};