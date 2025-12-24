#pragma once
#include <plugin.h>

class WheelHub
{
protected:
    struct VehData
    {
        RwFrame* m_pHRF = nullptr, * m_pHRM = nullptr, * m_pHRR = nullptr;
        RwFrame* m_pHLF = nullptr, * m_pHLM = nullptr, * m_pHLR = nullptr;

        VehData(CVehicle* pVeh) {}
        ~VehData() {}
    };

    static inline VehicleExtendedData<VehData> xData;

public:
    static void Initialize();
};
