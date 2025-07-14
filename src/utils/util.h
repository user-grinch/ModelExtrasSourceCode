#pragma once
#include "matrix.h"
#include "frame.h"
#include "render.h"

typedef enum class eModelEntityType eModelEntityType;

class Util
{
public:
  static bool IsNightTime();
  static bool IsTailLightOn(CVehicle *pVeh);
  static bool IsEngineOff(CVehicle *pVeh);
  static void UpdateRelativeToBoundingBox(CVehicle *pVeh, eDummyPos dummyPos, CVector &pos);

  static float NormalizeAngle(float angle);
 
  // Returns the speed of the vehicle handler
  static float GetVehicleSpeed(CVehicle *pVeh);
  static float GetVehicleSpeedRealistic(CVehicle *vehicle);
  static unsigned int GetEntityModel(void *ptr, eModelEntityType type);
  static void GetModelsFromIni(std::string &line, std::vector<int> &vec);

  static std::optional<int> GetDigitsAfter(const std::string &str, const std::string &prefix);
  static std::optional<std::string> GetCharsAfterPrefix(const std::string &str, const std::string &prefix, size_t num_chars);
};
