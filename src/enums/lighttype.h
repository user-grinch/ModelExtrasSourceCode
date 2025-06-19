#pragma once

enum eLightType
{
    // Order can't be changed
    // Game internally uses them
    UnknownLight = -1,
    HeadLightLeft,
    HeadLightRight,
    TailLightLeft,
    TailLightRight,

    // ModelExtras
    ReverseLight,
    BrakeLight,
    AllDayLight,
    DayLight,
    NightLight,
    FogLight,
    SideLightLeft,
    SideLightRight,
    STTLightLeft,
    STTLightRight,
    NABrakeLightLeft,
    NABrakeLightRight,
    SpotLight,
    StrobeLight,
    SirenLight,

    IndicatorLightLeftFront,
    IndicatorLightLeftMiddle,
    IndicatorLightLeftRear,
    IndicatorLightRightFront,
    IndicatorLightRightMiddle,
    IndicatorLightRightRear,
    TotalLight,
};

#define INDICATOR_LIGHTS_TYPE { \
    IndicatorLightLeftFront, \
    IndicatorLightLeftMiddle, \
    IndicatorLightLeftRear, \
    IndicatorLightRightFront, \
    IndicatorLightRightMiddle, \
    IndicatorLightRightRear \
}