#pragma once
#include "matrix.h"
#include "frame.h"

typedef enum class eModelEntityType eModelEntityType;
enum class eDummyPos;

class Util
{
public:
  static float NormalizeAngle(float angle);
  static void RegisterCorona(CEntity *pEntity, int coronaID, CVector pos, CRGBA col, float size);
  static void RegisterCoronaWithAngle(CEntity *pEntity, int coronaID, CVector posn, CRGBA col, float angle, float radius, float size);
  static void RegisterShadow(CEntity *pEntity, CVector position, CRGBA col, float angle, eDummyPos dummyPos, const std::string &shadwTexName, CVector2D shdwSz = {1.0f, 1.0f}, CVector2D shdwOffset = {0.0f, 0.0f}, RwTexture *pTexture = nullptr);

  // Returns the speed of the vehicle handler
  static float GetVehicleSpeed(CVehicle *pVeh);
  static float GetVehicleSpeedRealistic(CVehicle *vehicle);
  static unsigned int GetEntityModel(void *ptr, eModelEntityType type);
  static void GetModelsFromIni(std::string &line, std::vector<int> &vec);

  static std::optional<int> GetDigitsAfter(const std::string &str, const std::string &prefix);
  static std::optional<std::string> GetCharsAfterPrefix(const std::string &str, const std::string &prefix, size_t num_chars);
};
