/*
 * API provided for ModelExtras v2.1 Release
 */

#pragma once
#define ME_API_VERSION 11001

#ifdef MODELEXTRAS_DEV
#define ME_WRAPPER __declspec(dllexport)
#else
#define ME_WRAPPER __declspec(dllimport)
#endif

struct ME_ExhaustInfo
{
public:
    RwFrame *pFrame;
    CRGBA Color;
    float fSpeedMul;
    float fLifeTime;
    float fSizeMul;
    bool bNitroEffect;
};

enum ME_FeatureID
{
    TextureRemapper,
    REMOVED_NULL,
    AnimatedBrakes,
    AnimatedClutch,
    AnimatedGearLever,
    RotatingHandleBar,
    AnimatedChain,
    AnimatedDoors,
    AnimatedGasMeter,
    AnimatedGearMeter,
    AnimatedOdoMeter,
    AnimatedRpmMeter,
    AnimatedSpeedMeter,
    AnimatedSpoiler,
    AnimatedTurboMeter,
    BackfireEffect,
    DirtFX,
    HDLicensePlate,
    IVFCarcols,
    RotatingSteeringWheel,
    RotatingWheelHubs,
    StandardLights,
    SirenLights,
    SoundEffects,
    SpotLights,
    BodyStateVariation,
    CustomSounds,
    GangHands,
    PedCols,
    ExhaustFx,
    ConvertableRoof,
    DashboardLEDs,
    RollbackBed,
    FeatureCount
};

enum ME_MaterialID
{
    ME_HeadLightLeft,
    ME_HeadLightRight,
    ME_TailLightLeft,
    ME_TailLightRight,
    ME_ReverseLightLeft,
    ME_ReverseLightRight,
    ME_BrakeLightLeft,
    ME_BrakeLightRight,
    ME_AllDayLight,
    ME_DayLight,
    ME_NightLight,
    ME_FogLightLeft,
    ME_FogLightRight,
    ME_SideLightLeft,
    ME_SideLightRight,
    ME_STTLightLeft,
    ME_STTLightRight,
    ME_NABrakeLightLeft,
    ME_NABrakeLightRight,
    ME_SpotLight,
    ME_StrobeLight,
    ME_SirenLight,
    ME_IndicatorLightLeftFront,
    ME_IndicatorLightLeftMiddle,
    ME_IndicatorLightLeftRear,
    ME_IndicatorLightRightFront,
    ME_IndicatorLightRightMiddle,
    ME_IndicatorLightRightRear,
    ME_EngineOnLed,
    ME_EngineBrokenLed,
    ME_FogLightLed,
    ME_HighBeamLed,
    ME_LowBeamLed,
    ME_IndicatorLeftLed,
    ME_IndicatorRightLed,
    ME_SirenLed,
    ME_BootOpenLed,
    ME_BonnetOpenLed,
    ME_DoorOpenLed,
    ME_RoofOpenLed,
    ME_LightCount,
};

#ifdef __cplusplus
extern "C"
{
#endif

    // Core
    ME_WRAPPER int ME_GetAPIVersion();
    ME_WRAPPER int ME_GetVersion();
    ME_WRAPPER bool ME_IsFeatureAvail(ME_FeatureID featureId);

    // Vehicle
    ME_WRAPPER bool ME_GetVehicleLEDState(CVehicle *pVeh, ME_MaterialID ledID);
    ME_WRAPPER void ME_SetVehicleLEDState(CVehicle *pVeh, ME_MaterialID ledID, bool state);

    ME_WRAPPER bool ME_GetVehicleLightState(CVehicle *pVeh, ME_MaterialID lightId);
    ME_WRAPPER void ME_SetVehicleLightState(CVehicle *pVeh, ME_MaterialID lightId, bool state);

    ME_WRAPPER unsigned int ME_GetExhaustCount(CVehicle *pVeh);
    ME_WRAPPER ME_ExhaustInfo ME_GetExhaustData(CVehicle *pVeh, int index);
    ME_WRAPPER void ME_SetExhaustData(CVehicle *pVeh, int index, ME_ExhaustInfo &data);

    // Ped
    ME_WRAPPER int ME_GetPedRemap(CPed *ped, int index);
    ME_WRAPPER void ME_SetPedRemap(CPed *ped, int index, int num);
    ME_WRAPPER void ME_SetAllPedRemaps(CPed *ped, int num);

#ifdef __cplusplus
}
#endif