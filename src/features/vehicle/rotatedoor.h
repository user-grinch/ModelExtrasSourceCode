#pragma once
#include <plugin.h>
#include <vector>

class RotateDoor
{
protected:
    struct DoorConfig {
      RwFrame* frame = nullptr;
      float originalRot = 0.0f;
      float mul = 1.0f;
      float popOutAmount = 0.0f;
      float prevRot = 0.0f;
    };

    struct VehData {
      DoorConfig leftFront, rightFront;
      DoorConfig leftRear, rightRear;

      VehData(CVehicle* pVeh) {}
      ~VehData() {}
    };

    static inline VehicleExtendedData<VehData> xData;
    static void UpdateRotatingDoor(CVehicle* pVeh, DoorConfig& config, eDoors doorID);

public:
    static void Initialize();
};
