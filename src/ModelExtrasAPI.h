/*
* API provided for ModelExtras v2.0 Release
*/

#pragma once
#define ME_API_VERSION 10000

typedef void (*T_FUNC)(void *value);

#ifdef MODELEXTRAS_DEV
#define ME_WRAPPER __declspec(dllexport)
#else
#define ME_WRAPPER __declspec(dllimport)
#endif

enum eFeatureMatrix {
    // Common Features
    TextureRemapper,
    ModelRandomizer,

    // Bike Features
    AnimatedBrakes,
    AnimatedClutch,
    AnimatedGearLever,
    RotatingHandleBar,

    // Vehicle Features
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
    
    FeatureCount
};

#ifdef __cplusplus
extern "C"
{
#endif

    // Functions to get API and ModelExtras version
    ME_WRAPPER int ME_GetAPIVersion();
    ME_WRAPPER int ME_GetVersion();
    
    ME_WRAPPER bool ME_IsFeatureAvail(eFeatureMatrix featureId);

    ME_WRAPPER int ME_GetPedRemap(CPed * ped, int index);
    ME_WRAPPER void ME_SetPedRemap(CPed * ped, int index, int num);
    ME_WRAPPER void ME_SetAllPedRemaps(CPed * ped, int num);
    
#ifdef __cplusplus
}
#endif