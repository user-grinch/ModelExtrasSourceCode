#pragma once
#include <plugin.h>
#include <vector>

class ConvertibleRoof {
protected:
    struct RoofConfig {
      RwFrame* pFrame = nullptr;
      float speed = 1.0f;
      float currentRot = 0.0f;
      float targetRot = 30.0f;
    };

    struct VehData {
        bool m_bInit = false;
        std::vector<RoofConfig> m_Roofs;    
        bool m_bRoofTargetExpanded = true;

        VehData(CVehicle* pVeh) {
            m_bRoofTargetExpanded = RandomNumberInRange(0, 1);
        }
        ~VehData() {}
    };

    static inline VehicleExtendedData<VehData> xData;
public:
    static void Initialize();
};
